/*                                              -*- mode:C++ -*-
  Element_Composer.h Musical note organizer
  Copyright (C) 2014 The Regents of the University of New Mexico.  All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
  USA
*/

/**
  \file Element_Composer.h Musical note organizer
  \author Max R. Ottesen.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef ELEMENT_COMPOSER_H
#define ELEMENT_COMPOSER_H

#include "P3Atom.h"
#include "Element.h"
#include "EventWindow.h"
#include "ElementTable.h"
#include "itype.h"
#include "Element_Note.h"
#include "Element_Res.h"

namespace MFM
{
  #define MUSIC_VERSION 1

  template <class CC>
  class Element_Composer : public Element<CC>
  {
    // Extract short names for parameter types
    typedef typename CC::ATOM_TYPE T;
    typedef typename CC::PARAM_CONFIG P;
    enum
    {
      R = P::EVENT_WINDOW_RADIUS,
      BITS = P::BITS_PER_ATOM,

      SCALE_POS = P3Atom<P>::P3_STATE_BITS_POS,
      SCALE_LEN = 8,

      SCALE_DEGREE_POS = SCALE_POS + SCALE_LEN,
      SCALE_DEGREE_LEN = 3,

      CHANGE_SD_POS = SCALE_DEGREE_POS + SCALE_DEGREE_LEN,
      CHANGE_SD_LEN = 8
    };

    typedef BitField<BitVector<BITS>, VD::U32, SCALE_LEN, SCALE_POS> AFScale;
    typedef BitField<BitVector<BITS>, VD::U32, SCALE_DEGREE_LEN, SCALE_DEGREE_POS> AFScaleDegree;
    typedef BitField<BitVector<BITS>, VD::U32, CHANGE_SD_LEN, CHANGE_SD_POS> AFChangeSD;

    u32 GetScale(const T& us) const
    {
      return AFScale::Read(this->GetBits(us));
    }
    void SetScale(T& us, const u32 newScale) const
    {
      AFScale::Write(this->GetBits(us), newScale);
    }

    u32 GetScaleDegree(const T& us) const
    {
      return AFScaleDegree::Read(this->GetBits(us));
    }
    void SetScaleDegree(T& us, const u32 newScaleDegree) const
    {
      AFScaleDegree::Write(this->GetBits(us), newScaleDegree);
    }

    void SetChangeChance(T& us, u32 change) const
    {
      AFChangeSD::Write(this->GetBits(us), change);
    }

    bool ShouldChange(const T& us, Random & random) const
    {
      u32 changesd = AFChangeSD::Read(this->GetBits(us));
      return random.OneIn(changesd);
    }
    void IncreaseChangeChance(T& us) const
    {
      u32 newChance = AFChangeSD::Read(this->GetBits(us));
      if (newChance > 1) {
        newChance--; //-- because we do OneIn(chance)
        AFChangeSD::Write(this->GetBits(us), newChance);
      }
    }
    void DecreaseChangeChance(T& us) const
    {
      u32 newChance = AFChangeSD::Read(this->GetBits(us));
      if (newChance < 255) {
        newChance++; //++ because we do OneIn(chance)
        AFChangeSD::Write(this->GetBits(us), newChance);
      }
    }


    ElementParameterS32<CC> m_scaleDegree;
    ElementParameterS32<CC> m_behavior;


  public:

    static Element_Composer<CC> THE_INSTANCE;
    static const u32 TYPE()
    {
      return THE_INSTANCE.GetType();
    }

    Element_Composer()
      : Element<CC>(MFM_UUID_FOR("Composer", MUSIC_VERSION)),
        m_scaleDegree(this, "chord", "Target Chord",
                      "The chord that this composer is trying to make.", 0, 0, 6),
        m_behavior(this, "behavior", "Behavior level",
                   "The complexity of this Composer's behavior.", 1, 3, 3)
    {
      Element<CC>::SetAtomicSymbol("Cp");
      Element<CC>::SetName("Composer");
    }

    virtual u32 PercentMovable(const T& you,
                               const T& me, const SPoint& offset) const
    {
      return 100;
    }

    virtual u32 DefaultPhysicsColor() const
    {
      return 0xffff0500;
    }

    virtual u32 DefaultLowlightColor() const
    {
      return 0xffb27300;
    }

    virtual u32 LocalPhysicsColor(const T & atom, u32 selector) const
    {
      u32 sd = GetScaleDegree(atom);
      switch (sd) {
      case 0 :
        return 0xed1d2b;
      case 1:
        return 0x283891;
      case 2:
        return 0xffffff;
      case 3:
        return 0x00a651;
      case 4:
        return 0xf7941e;
      case 5:
        return 0xfff200;
      case 6:
        return 0x662d91;
      default:
        return 0x0;
      }
      return 0x0;
    }

    virtual const T& GetDefaultAtom() const
    {
      static T defaultAtom(TYPE(), 0, 0, 0);

      SetScale(defaultAtom, 0);
      SetScaleDegree(defaultAtom, m_scaleDegree.GetValue());
      SetChangeChance(defaultAtom, 255);

      return defaultAtom;
    }

    SPoint random_movement(EventWindow<CC>& window) const {
      return SPoint(0, window.GetRandom().Create(3) - 1);
    }


    SPoint crit_2(EventWindow<CC>& window, const SPoint sp) const {
      SPoint newPoint = SPoint(0, 0);
      Random & random = window.GetRandom();
      SPoint lowestNote;
      for (s32 x = 0; x > -4; x--) {
        const SPoint dif = SPoint(sp.GetX(), x);
        if (window.GetRelativeAtom(dif).GetType() == Element_Note<CC>::THE_INSTANCE.GetType()) {
          lowestNote = dif;
        }
      }

      //if this note is the lowest note in the chord, we will probably keep it
      if (lowestNote == sp) {
        newPoint = random.OneIn(10) ? (random.OneIn(2) ? SPoint (0, 1) : SPoint(0, -1)) : SPoint(0, 0);
      }
      //if this note is not the lowest note in the chord, we will look to make a triad
      else {
        //our note is not in the triad
        if (lowestNote != SPoint(sp.GetX(), -2)) {
          newPoint = random.OneIn(10) ? SPoint(0, 0) : (random.OneIn(2) ? SPoint(0, 1) : SPoint(0, -1));
        }
        //our note is in the triad
        else {
          newPoint = random.OneIn(10) ? (random.OneIn(2) ? SPoint(0, 1) : SPoint(0, -1)) : SPoint(0, 0);
        }
      }
      return newPoint;
    }

    SPoint crit_3_1(EventWindow<CC>& window, const SPoint sp, const T us) const {
      u32 us_sd = GetScaleDegree(us);
      u32 ok_sds [] = {us_sd, (us_sd + 2) % 7, (us_sd + 4) % 7};
      SPoint newPoint = SPoint(0, 0);

      const T note = window.GetRelativeAtom(sp);
      u32 note_sd = Element_Note<CC>::THE_INSTANCE.GetScaleDegree(note);

      if (note_sd == ok_sds[0] || note_sd == ok_sds[1] || note_sd == ok_sds[2]) {
	//We are good to leave the note alone
      }
      else if (note_sd == ((ok_sds[0] + 1) % 7) || note_sd == ((ok_sds[1] + 1) % 7)) {
	//We can move the note up or down
	if (window.GetRandom().OneIn(2)) { //try to move up
	  const T above = window.GetRelativeAtom(sp + SPoint(0, -1));
	  if (above.GetType() == Element_Note<CC>::THE_INSTANCE.GetType()) {
	    window.SetRelativeAtom(sp, Element_Empty<CC>::THE_INSTANCE.GetDefaultAtom());
	  }
	  else {
	    return SPoint(0, -1);
	  }
	}
	else { //try to move down
	  const T below = window.GetRelativeAtom(sp + SPoint(0, 1)); //+y axis is down
	  if (below.GetType() == Element_Note<CC>::THE_INSTANCE.GetType()) {
	    //Can't move the note down because another one is already there. Delete our atom
	    window.SetRelativeAtom(sp, Element_Empty<CC>::THE_INSTANCE.GetDefaultAtom());
	  }
	  else {
	    return SPoint (0, 1);
	  }
	}
      }
      else if (note_sd == ((ok_sds[2] + 1) % 7)) {
	//move the note down
	const T below = window.GetRelativeAtom(sp + SPoint(0, 1)); //+y axis is down
	if (below.GetType() == Element_Note<CC>::THE_INSTANCE.GetType()) {
	  //Can't move the note down because another one is already there. Delete our atom
	  window.SetRelativeAtom(sp, Element_Empty<CC>::THE_INSTANCE.GetDefaultAtom());
	}
	else {
	  return SPoint (0, 1);
	}
      }
      else {
	//move the note up
	const T above = window.GetRelativeAtom(sp + SPoint(0, -1));
	if (above.GetType() == Element_Note<CC>::THE_INSTANCE.GetType()) {
	  window.SetRelativeAtom(sp, Element_Empty<CC>::THE_INSTANCE.GetDefaultAtom());
	}
	else {
	  return SPoint(0, -1);
	}
      }

      return SPoint(0, 0);
    }


    virtual void Behavior(EventWindow<CC>& window) const
    {
      T us = window.GetCenterAtom();
      Random & random = window.GetRandom();
      const MDist<R>& md = MDist<R>::get();
      bool changedSD = false;

      for (u32 idx = md.GetFirstIndex(1); idx <= md.GetLastIndex(1); ++idx) {
        const SPoint sp = md.GetPoint(idx);

        const T other = window.GetRelativeAtom(sp);
        const u32 otherType = other.GetType();


        if (otherType == Element_Res<CC>::THE_INSTANCE.GetType()) {
          if (random.OneIn(2)) {  //turn res into note
            window.SetRelativeAtom(sp, Element_Note<CC>::THE_INSTANCE.GetDefaultAtom());
            break;
          }
          else if (random.OneIn(100)) { //turn res into composer
            window.SetRelativeAtom(sp, GetDefaultAtom());
            break;
          }
        }

        //have a chance to change your SD. the chance increases or decreases based on the other composer's SD
        if (otherType == Element_Composer<CC>::THE_INSTANCE.GetType()) {
          if (GetScaleDegree(us) == GetScaleDegree(other)) {
            DecreaseChangeChance(us);
          }
          else {
            IncreaseChangeChance(us);
          }

          if(ShouldChange(us, random)) {
            SetScaleDegree(us, GetScaleDegree(other));
            SetChangeChance(us, 255);
            changedSD = true;
            break;
          }
        }


        if (otherType == Element_Note<CC>::THE_INSTANCE.GetType()) {
          SPoint newPoint = SPoint(0, 0);

          //only look to the sides
          if (sp.GetY() == 0) {

              switch(m_behavior.GetValue()) {
              case 1:
                newPoint = random_movement(window);
                break;
              case 2:
                newPoint = crit_2(window, sp);
                break;
              case 3:
                newPoint = crit_3_1(window, sp, us);
                break;
              default:
		break;
              }

            window.SwapAtoms(sp, sp + newPoint);
          }
	  else if (m_behavior.GetValue() == 3 && random.OneIn(1000)) {
	    u32 us_sd = GetScaleDegree(us);
	    u32 ok_sds [] = {us_sd, (us_sd + 2) % 7, (us_sd + 4) % 7};
	    u32 other_sd = Element_Note<CC>::THE_INSTANCE.GetScaleDegree(other);
	    //special behavior for crit_3 since rows of notes get stuck between the staff
	    if (ok_sds[0] != other_sd && ((ok_sds[1] + 2) % 7) != other_sd && ((ok_sds[2] + 4) % 7) != other_sd) {
	      window.SetRelativeAtom(sp, Element_Empty<CC>::THE_INSTANCE.GetDefaultAtom());
	    }
	  }
        }
      }
      //chance to randomly change SD
      if (!changedSD && random.OneIn(1000)) {
        SetScaleDegree(us, random.Create(7));
      }
      window.SetCenterAtom(us); //Just in case we changed the SD
      this->Diffuse(window);
    }
  };

  template <class CC>
  Element_Composer<CC> Element_Composer<CC>::THE_INSTANCE;

}

#endif /* ELEMENT_COMPOSER_H */
