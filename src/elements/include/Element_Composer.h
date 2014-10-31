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
      CHANGE_SD_LEN = 4
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

    bool Change(const T& us) const
    {
      return random.OneIn(AFChangeSD::Read(this->GetBits(us)) + 1);
    }
    void IncreaseChangeChance(T& us) const
    {
      u32 newChance = AFChangeSD::Read(this->GetBits(us));
      if (newChance > 0) newChance--; //-- because we do OneIn(chance)
      AFScaleDegree::Write(this->GetBits(us), newChance);
    }
    void DecreaseChangeChance(T& us) const
    {
      u32 newChance = AFChangeSD::Read(this->GetBits(us));
      if (newChance < 15) newChance++; //++ because we do OneIn(chance)
      AFScaleDegree::Write(this->GetBits(us), newChance);
    }


  public:

    static Element_Composer<CC> THE_INSTANCE;
    static const u32 TYPE()
    {
      return THE_INSTANCE.GetType();
    }

    Element_Composer()
      : Element<CC>(MFM_UUID_FOR("Composer", MUSIC_VERSION))
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

    virtual const T& GetDefaultAtom() const
    {
      static T defaultAtom(TYPE(), 0, 0, 0);

      SetScale(defaultAtom, 0);

      return defaultAtom;
    }

    virtual void Behavior(EventWindow<CC>& window) const
    {
      T us = window.GetCenterAtom();
      Random & random = window.GetRandom();
      const MDist<R>& md = MDist<R>::get();
      bool performedAction = false;

      for (u32 idx = md.GetFirstIndex(1); idx <= md.GetLastIndex(1); ++idx) {
        const SPoint sp = md.GetPoint(idx);

        const T other = window.GetRelativeAtom(sp);
        const u32 otherType = other.GetType();


        if (otherType == Element_Res<CC>::THE_INSTANCE.GetType()) {
          if (random.OneIn(2)) {  //turn res into note
            window.SetRelativeAtom(sp, Element_Note<CC>::THE_INSTANCE.GetDefaultAtom());
          }
          else if (random.OneIn(100)) { //turn res into composer
            window.SetRelativeAtom(sp, GetDefaultAtom());
          }
        }

	//have a chance to change your SD. the chance increases or decreases based on the other composer's SD
	if (otherType == Element_Composer<CC>::THE_INSTANCE.GetType()) {
	  if (GetScaleDegree(us) == other.GetScaleDegree(other)) {
	    IncreaseChangeChance(us);
	  }
	  else {
	    DecreaseChangeChance(us);
	  }
	  
	  if(Change(us)) {
	    SetScaleDegree(us, other.GetScaleDegree(other));
	  }
	  else if (random.OneIn(500)) {
	    SetScaleDegree(us, random.Create(7));
	  }
	}


        if (otherType == Element_Note<CC>::THE_INSTANCE.GetType()) {
          if (random.OneIn(50)) { //move the note
            SPoint newPoint = SPoint(0, 0);

            //only look to the sides
            if (sp.GetY() == 0) {
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
            }
         

            window.SwapAtoms(sp, sp + newPoint);
          }
        }
      }
	window.SetCenterAtom(us); //Just in case we changed the SD
      this->Diffuse(window);
    }
  };

  template <class CC>
  Element_Composer<CC> Element_Composer<CC>::THE_INSTANCE;

}

#endif /* ELEMENT_COMPOSER_H */
