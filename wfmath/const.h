// const.h (Defined constants for the WFMath library)
//
//  The WorldForge Project
//  Copyright (C) 2001, 2002  The WorldForge Project
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
// Created: 2001-12-7

#ifndef WFMATH_CONST_H
#define WFMATH_CONST_H

#include <float.h>
// The next two aren't used in this file, but are used
// everywhere else in the library.
#include <assert.h>
#include <math.h>

#ifdef _MSC_VER
  #if _MSC_VER < 1300
    #error "You are using an older version of MSVC++ with extremely poor"
    #error "template support. Please use at least version 7.0, where the"
    #error "template support is merely bad, or try a different compiler."
  #else
    // The name of this one is somewhat misleading. The problem is,
    // while you can _define_ specializations of template functions,
    // you can't _declare_ them.
    #define WFMATH_NO_CLASS_FUNCTION_SPECIALIZATION 1
    // This one means exactly what it says
    #define WFMATH_NO_TEMPLATES_AS_TEMPLATE_PARAMETERS 1
  #endif
#endif

// Forward declarations for all classes
#include <iosfwd>
namespace Atlas { namespace Message { class Object;}}

/// Generic library namespace
namespace WFMath {

// Constants

/// The constant pi
const double Pi		= 3.14159265358979323846264338327950288419716939937508;
/// The square root of pi
const double SqrtPi	= 1.77245385090551602729816748334114518279754945612237;
/// The natural logarithm of pi
const double LogPi	= 1.14472988584940017414342735135305871164729481291530;
/// The square root of 2
const double Sqrt2	= 1.41421356237309504880168872420969807856967187537693;
/// The square root of 3
const double Sqrt3	= 1.73205080756887729352744634150587236694280525381037;
/// The natural logarithm of 2
const double Log2	= 0.69314718055994530941723212145817656807550013436025;

/// Basic floating point type
typedef float CoordType;

/// This is the attempted precision of the library. It's essentially
/// the machine precision multiplied by a fudge factor.
#define WFMATH_EPSILON		(30 * FLT_EPSILON)

/// Max value of CoordType
#define WFMATH_MAX		FLT_MAX
/// Min value of CoordType
#define WFMATH_MIN		FLT_MIN

// Basic comparisons

double _ScaleEpsilon(double x1, double x2, double epsilon);
double _ScaleEpsilon(const CoordType* x1, const CoordType* x2,
		     int length, double epsilon = WFMATH_EPSILON);

/// Test for equality up to precision epsilon
/**
 * Returns true if the difference between the numbers is less
 * than epsilon. Note that epsilon is multiplied by 2 raised
 * to the power of the exponent of the smaller number. So,
 * for example, Equal(0.00010000, 0.00010002, 1.0e-4) will not
 * compare equal, but Equal(0.00010000, 0.00010002, 1.0e-3) will.
 **/
template<class C>
inline bool Equal(const C& c1, const C& c2, double epsilon = WFMATH_EPSILON)
	{return c1.isEqualTo(c2, epsilon);}

bool Equal(double x1, double x2, double epsilon = WFMATH_EPSILON);
// Avoid template, make sure floats are cast to doubles.
inline bool Equal(float x1, float x2, double epsilon = WFMATH_EPSILON)
	{return Equal((double) x1, (double) x2, epsilon);}

// These let us avoid including <algorithm> for the sake of
// std::max() and std::min().

inline CoordType FloatMax(CoordType a, CoordType b)
	{return (a > b) ? a : b;}
inline CoordType FloatMin(CoordType a, CoordType b)
	{return (a < b) ? a : b;}
inline CoordType FloatClamp(CoordType val, CoordType min, CoordType max)
	{return (min >= val) ? min : (max <= val ? max : val);}

} // namespace WFMath

#endif // WFMATH_CONST_H
