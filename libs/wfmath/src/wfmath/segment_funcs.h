// segment_funcs.h (line segment implementation)
//
//  The WorldForge Project
//  Copyright (C) 2000, 2001  The WorldForge Project
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
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
//  For information about WorldForge and its authors, please contact
//  the Worldforge Web Site at http://www.worldforge.org.
//

// Author: Ron Steinke

#ifndef WFMATH_SEGMENT_FUNCS_H
#define WFMATH_SEGMENT_FUNCS_H

#include <wfmath/segment.h>

#include <cassert>

namespace WFMath {

template<int dim>
inline Segment<dim>& Segment<dim>::moveCornerTo(const Point<dim>& p, size_t corner) {
	assert(corner == 0 || corner == 1);

	Vector<dim> diff = m_p2 - m_p1;

	if (!corner) {
		m_p1 = p;
		m_p2 = p + diff;
	} else {
		m_p2 = p;
		m_p1 = p - diff;
	}

	return *this;
}

template<int dim>
inline Segment<dim>& Segment<dim>::rotateCorner(const RotMatrix<dim>& m, size_t corner) {
	assert(corner == 0 || corner == 1);

	if (corner)
		m_p1.rotate(m, m_p2);
	else
		m_p2.rotate(m, m_p1);

	return *this;
}

template<>
inline Segment<3>& Segment<3>::rotateCorner(const Quaternion& q, size_t corner) {
	assert(corner == 0 || corner == 1);

	if (corner)
		m_p1.rotate(q, m_p2);
	else
		m_p2.rotate(q, m_p1);

	return *this;
}

} // namespace WFMath

#endif  // WFMATH_SEGMENT_FUNCS_H
