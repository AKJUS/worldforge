// -*-C++-*-
// basis.h (Cartesian/polar/spherical conversion for the WFMath library)
//
//  The WorldForge Project
//  Copyright (C) 2001  The WorldForge Project
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
//  For information about WorldForge and its authors, please contact
//  the Worldforge Web Site at http://www.worldforge.org.

// Author: Ron Steinke
// Created: 2001-12-15

#ifndef WFMATH_BASIS_H
#define WFMATH_BASIS_H

#include <math.h>
#include <wfmath/const.h>

namespace WF { namespace Math {

// Expects (r, theta) for polar, (x, y) for cart
inline void _CartToPolar(const CoordType *in, CoordType *out)
{
  out[0] = sqrt(in[0] * in[0] + in[1] * in[1]);
  out[1] = atan2(in[0], in[1]);
}

// Expects (r, theta) for polar, (x, y) for cart
inline void _PolarToCart(const CoordType *in, CoordType *out)
{
  out[0] = in[0] * cos(in[1]);
  out[1] = in[0] * sin(in[1]);
}

// Expects (r, theta, phi) for spherical, (x, y, z) for cart
inline void _CartToSpherical(const CoordType *in, CoordType *out)
{
  out[0] = sqrt(in[0] * in[0] + in[1] * in[1] + in[2] * in[2]);
  out[1] = atan2(in[2], sqrt(in[0] * in[0] + in[1] * in[1]));
  out[2] = atan2(in[0], in[1]);
}

// Expects (r, theta, phi) for spherical, (x, y, z) for cart
inline void _SphericalToCart(const CoordType *in, CoordType *out)
{
  double stheta = sin(in[1]);

  out[0] = in[0] * stheta * cos(in[2]);
  out[1] = in[0] * stheta * sin(in[2]);
  out[2] = in[0] * cos(in[1]);
}

}} // namespace WF::Math
#endif // WFMATH_BASIS_H
