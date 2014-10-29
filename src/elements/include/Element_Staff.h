/*                                              -*- mode:C++ -*-
  Element_Staff.h Self organizing grid
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
  \file Element_Staff.h Self organizing grid
  \author Trent R. Small.
  \author Max R. Ottesen.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef ELEMENT_STAFF_H
#define ELEMENT_STAFF_H

#include "Dirs.h"
#include "P3Atom.h"
#include "Element.h"
#include "EventWindow.h"
#include "ElementTable.h"
#include "itype.h"
#include "Element_Empty.h"

namespace MFM
{
  #define MUSIC_VERSION 1

  template <class CC>
  class Element_Staff : public Element<CC>
  {
    // Extract short names for parameter types
    typedef typename CC::ATOM_TYPE T;
    typedef typename CC::PARAM_CONFIG P;
    enum
    {
      R = P::EVENT_WINDOW_RADIUS,
      BITS = P::BITS_PER_ATOM,

      LOC_X_POS = P3Atom<P>::P3_STATE_BITS_POS,
      LOC_X_LEN = 8,

      LOC_Y_POS = LOC_X_POS + LOC_X_LEN,
      LOC_Y_LEN = 8

    };

    typedef BitField<BitVector<BITS>, VD::S32, LOC_X_LEN, LOC_X_POS> AFLocX;
    typedef BitField<BitVector<BITS>, VD::S32, LOC_Y_LEN, LOC_Y_POS> AFLocY;

    s32 GetXLoc(const T& us) const
    {
      return AFLocX::Read(this->GetBits(us));
    }
    void SetXLoc(T& us, const s32 newX) const
    {
      AFLocX::Write(this->GetBits(us), newX);
    }

    s32 GetYLoc(const T& us) const
    {
      return AFLocY::Read(this->GetBits(us));
    }
    void SetYLoc(T& us, const s32 newY) const
    {
      AFLocY::Write(this->GetBits(us), newY);
    }

    void SetLocation(T& us, SPoint pt) const
    {
      SetXLoc(us, pt.GetX());
      SetYLoc(us, pt.GetY());
    }


  public:

    static Element_Staff<CC> THE_INSTANCE;
    static const u32 TYPE()
    {
      return THE_INSTANCE.GetType();
    }

    SPoint GetLocation(const T& us) const
    {
      return SPoint(GetXLoc(us), GetYLoc(us));
    }

    Element_Staff()
      : Element<CC>(MFM_UUID_FOR("Staff", MUSIC_VERSION))
    {
      Element<CC>::SetAtomicSymbol("Sf");
      Element<CC>::SetName("Staff");
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
      return 0xff808080;
    }

    virtual u32 DefaultLowlightColor() const
    {
      return 0xff404040;
    }

    virtual const T& GetDefaultAtom() const
    {
      static T defaultAtom(TYPE(), 0, 0, 0);

      SetLocation(defaultAtom, SPoint(0, 0));

      return defaultAtom;
    }

    const T& MakeAtomWithLocation(SPoint& location) const
    {
      static T defaultAtom(TYPE(), 0, 0, 0);

      SetLocation(defaultAtom, location);

      return defaultAtom;
    }

    virtual void Behavior(EventWindow<CC>& window) const
    {
      const T& self = window.GetCenterAtom();
      Dir dirs[4] =
      {
        Dirs::NORTH, Dirs::EAST, Dirs::SOUTH, Dirs::WEST
      };



      const MDist<R>& md = MDist<R>::get();
      for (u32 idx = md.GetFirstIndex(1); idx <= md.GetLastIndex(3); ++idx) {
        const SPoint sp = md.GetPoint(idx);
        const T other = window.GetRelativeAtom(sp);
        //if you see other staff elements in the wrong position, assume that you are out of place and delete yourself
        if (other.GetType() == Element_Staff<CC>::THE_INSTANCE.GetType()) {
          const T empty = Element_Empty<CC>::THE_INSTANCE.GetDefaultAtom();
          window.SetCenterAtom(empty);
          return;
	}
      }



      for(u32 i = 0; i < 4; i++)
      {
        SPoint pt;
        Dirs::FillDir(pt, dirs[i]);

        const T& atom = window.GetRelativeAtom(pt * R);
        SPoint atomLoc = GetLocation(atom);
        SPoint checkLoc = GetLocation(self) + pt;

        if((TYPE() != atom.GetType()) ||
           (atomLoc != checkLoc))
        {
          const T& newAtom = MakeAtomWithLocation(checkLoc);

          window.SetRelativeAtom(pt * R, newAtom);
        }
      }
    }
  };

  template <class CC>
  Element_Staff<CC> Element_Staff<CC>::THE_INSTANCE;

}

#endif /* ELEMENT_STAFF_H */
