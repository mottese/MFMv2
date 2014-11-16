/*                                              -*- mode:C++ -*-
  Element_Note.h Represents a musical tone in the context of a specific scale
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
  \file Element_Note.h Musical tone in the context of a specific scale
  \author Max R. Ottesen.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef ELEMENT_NOTE_H
#define ELEMENT_NOTE_H

#include "P3Atom.h"
#include "Element.h"
#include "EventWindow.h"
#include "ElementTable.h"
#include "itype.h"
#include "Element_Staff.h"

namespace MFM
{
  #define MUSIC_VERSION 1

  template <class CC>
  class Element_Note : public Element<CC>
  {
    // Extract short names for parameter types
    typedef typename CC::ATOM_TYPE T;
    typedef typename CC::PARAM_CONFIG P;
    enum
    {
      R = P::EVENT_WINDOW_RADIUS,
      BITS = P::BITS_PER_ATOM,

      SCALE_DEGREE_POS = P3Atom<P>::P3_STATE_BITS_POS,
      SCALE_DEGREE_LEN = 3,

      ACCIDENTAL_POS = SCALE_DEGREE_POS + SCALE_DEGREE_LEN,
      ACCIDENTAL_LEN = 3
    };

    typedef BitField<BitVector<BITS>, VD::U32, SCALE_DEGREE_LEN, SCALE_DEGREE_POS> AFScaleDegree;
    typedef BitField<BitVector<BITS>, VD::U32, ACCIDENTAL_LEN, ACCIDENTAL_POS> AFAccidental;

  public:

    u32 GetScaleDegree(const T& us) const
    {
      return AFScaleDegree::Read(this->GetBits(us));
    }
    void SetScaleDegree(T& us, const u32 newScaleDegree) const
    {
      AFScaleDegree::Write(this->GetBits(us), newScaleDegree);
    }

    u32 GetAccidental(const T& us) const
    {
      return AFAccidental::Read(this->GetBits(us));
    }
    void SetAccidental(T& us, const u32 newAccidental) const
    {
      AFAccidental::Write(this->GetBits(us), newAccidental);
    }
  
    static Element_Note<CC> THE_INSTANCE;
    static const u32 TYPE()
    {
      return THE_INSTANCE.GetType();
    }

    Element_Note()
      : Element<CC>(MFM_UUID_FOR("Note", MUSIC_VERSION))
    {
      Element<CC>::SetAtomicSymbol("N");
      Element<CC>::SetName("Note");
    }

    virtual u32 PercentMovable(const T& you,
                               const T& me, const SPoint& offset) const
    {
      return 0;
    }

    virtual u32 Diffusability(EventWindow<CC> & ew, SPoint nowAt, SPoint maybeAt) const
    {
      return nowAt.Equals(maybeAt)?Element<CC>::COMPLETE_DIFFUSABILITY:0;
    }

    virtual u32 DefaultPhysicsColor() const
    {
      return 0xff0000ff;
    }

    virtual u32 DefaultLowlightColor() const
    {
      return 0xff000077;
    }

    virtual const T& GetDefaultAtom() const
    {
      static T defaultAtom(TYPE(), 0, 0, 0);

      SetAccidental(defaultAtom, 0);
      SetScaleDegree(defaultAtom, 0);

      return defaultAtom;
    }

    virtual void Behavior(EventWindow<CC>& window) const
    {
      T us = window.GetCenterAtom();
      const MDist<R>& md = MDist<R>::get();
      for (u32 idx = md.GetFirstIndex(1); idx <= md.GetLastIndex(4); ++idx) {
        const SPoint sp = md.GetPoint(idx);
        const T other = window.GetRelativeAtom(sp);
        const u32 otherType = other.GetType();

        if (otherType == Element_Staff<CC>::THE_INSTANCE.GetType()) {
          SPoint p = Element_Staff<CC>::THE_INSTANCE.GetLocation(other);
          p *= 8;
          p -= sp;

          //LOG.Message("sd: %d", p.GetY() % 7);
          SetScaleDegree(us, (p.GetY() % 7)); //Scale degree is 0-6
          window.SetCenterAtom(us);

          break;
        }
      }
    }
  };

  template <class CC>
  Element_Note<CC> Element_Note<CC>::THE_INSTANCE;

}

#endif /* ELEMENT_NOTE_H */
