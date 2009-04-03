// Shape.cpp:  Mouse/Character handling, for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//

#include "StaticText.h"
#include "swf/DefineTextTag.h"

namespace gnash
{

StaticText*
StaticText::getStaticText(std::vector<const SWF::TextRecord*>& to,
        size_t& numChars)
{
    _selectedText.clear();

    if (_def->extractStaticText(to, numChars)) {
        _selectedText.resize(numChars);
        return this;
    }
    
    return 0;
}

bool
StaticText::pointInShape(boost::int32_t  x, boost::int32_t  y) const
{
    SWFMatrix wm = getWorldMatrix();
    SWFMatrix wm_inverse = wm.invert();
    point lp(x, y);
    wm_inverse.transform(lp);
    return _def->point_test_local(lp.x, lp.y, wm);
}


void  
StaticText::display()
{
    _def->display(this); // pass in transform info
    clear_invalidated();
}

} // namespace gnash

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
