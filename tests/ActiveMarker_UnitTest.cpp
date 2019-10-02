// Eris Online RPG Protocol Library
// Copyright (C) 2017 Erik Ogenvik
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

// $Id$

#ifdef NDEBUG
#undef NDEBUG
#endif
#ifndef DEBUG
#define DEBUG
#endif

#include "Eris/ActiveMarker.h"
#include <cassert>

using namespace Eris;

int main()
{

    ActiveMarker* marker = new ActiveMarker();
    assert(*marker->getMarker() == true);

    std::shared_ptr<bool> shared_marker = marker->getMarker();
    delete marker;
    assert(*shared_marker == false);


    return 0;
}
