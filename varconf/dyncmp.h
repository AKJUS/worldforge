/*
 *  dyncmp.h - interface for dynamically derived value container class compare
 *  Copyright (C) 2001, Ron Steinke
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  Contact:  Joseph Zupko
 *            jaz147@psu.edu
 *
 *            189 Reese St.
 *            Old Forge, PA 18518
 */
 
#ifndef VARCONF_DYNCMP_H
#define VARCONF_DYNCMP_H

#include <varconf/variable.h>
#include <varconf/dynbase.h>

#include <string>

namespace varconf {
namespace dynvar {

class Compare : public Base {
public:
  Compare() : Base(), m_v1(0), m_v2(0) {}
  Compare( const Variable& v1, const Variable& v2) : Base(), m_v1(v1), m_v2(v2) {}
  Compare( const Compare& c) : Base(c), m_v1(c.m_v1), m_v2(c.m_v2) {}

  virtual ~Compare() {}

  Compare& operator=( const Compare& c);

protected:

  virtual void set_val();

  virtual bool bool_cmp(const bool b1, const bool b2) = 0;
  virtual bool int_cmp(const int i1, const int i2) = 0;
  virtual bool double_cmp(const double d1, const double d2) = 0;
  virtual bool string_cmp(const std::string& s1, const std::string& s2) = 0;

private:

  Variable m_v1, m_v2;
};

class Equal : public Compare {
public:
  Equal() : Compare() {}
  Equal( const Variable& v1, const Variable& v2) : Compare(v1, v2) {}
  Equal( const Equal& e) : Compare(e) {}

  virtual ~Equal() {}

protected:

  virtual bool bool_cmp(const bool b1, const bool b2)
	{return b1 == b2;}
  virtual bool int_cmp(const int i1, const int i2)
	{return i1 == i2;}
  virtual bool double_cmp(const double d1, const double d2)
	{return d1 == d2;}
  virtual bool string_cmp(const std::string& s1, const std::string& s2)
	{return s1 == s2;}
};

class NotEq : public Compare {
public:
  NotEq() : Compare() {}
  NotEq( const Variable& v1, const Variable& v2) : Compare(v1, v2) {}
  NotEq( const NotEq& e) : Compare(e) {}

  virtual ~NotEq() {}

protected:

  virtual bool bool_cmp(const bool b1, const bool b2)
	{return b1 != b2;}
  virtual bool int_cmp(const int i1, const int i2)
	{return i1 != i2;}
  virtual bool double_cmp(const double d1, const double d2)
	{return d1 != d2;}
  virtual bool string_cmp(const std::string& s1, const std::string& s2)
	{return s1 != s2;}
};

class Greater : public Compare {
public:
  Greater() : Compare() {}
  Greater( const Variable& v1, const Variable& v2) : Compare(v1, v2) {}
  Greater( const Greater& e) : Compare(e) {}

  virtual ~Greater() {}

protected:

  virtual bool bool_cmp(const bool b1, const bool b2)
	{return b1 > b2;}
  virtual bool int_cmp(const int i1, const int i2)
	{return i1 > i2;}
  virtual bool double_cmp(const double d1, const double d2)
	{return d1 > d2;}
  virtual bool string_cmp(const std::string& s1, const std::string& s2)
	{return s1 > s2;}
};

class GreaterEq : public Compare {
public:
  GreaterEq() : Compare() {}
  GreaterEq( const Variable& v1, const Variable& v2) : Compare(v1, v2) {}
  GreaterEq( const GreaterEq& e) : Compare(e) {}

  virtual ~GreaterEq() {}

protected:

  virtual bool bool_cmp(const bool b1, const bool b2)
	{return b1 >= b2;}
  virtual bool int_cmp(const int i1, const int i2)
	{return i1 >= i2;}
  virtual bool double_cmp(const double d1, const double d2)
	{return d1 >= d2;}
  virtual bool string_cmp(const std::string& s1, const std::string& s2)
	{return s1 >= s2;}
};

class Less : public Compare {
public:
  Less() : Compare() {}
  Less( const Variable& v1, const Variable& v2) : Compare(v1, v2) {}
  Less( const Less& e) : Compare(e) {}

  virtual ~Less() {}

protected:

  virtual bool bool_cmp(const bool b1, const bool b2)
	{return b1 < b2;}
  virtual bool int_cmp(const int i1, const int i2)
	{return i1 < i2;}
  virtual bool double_cmp(const double d1, const double d2)
	{return d1 < d2;}
  virtual bool string_cmp(const std::string& s1, const std::string& s2)
	{return s1 < s2;}
};

class LessEq : public Compare {
public:
  LessEq() : Compare() {}
  LessEq( const Variable& v1, const Variable& v2) : Compare(v1, v2) {}
  LessEq( const LessEq& e) : Compare(e) {}

  virtual ~LessEq() {}

protected:

  virtual bool bool_cmp(const bool b1, const bool b2)
	{return b1 <= b2;}
  virtual bool int_cmp(const int i1, const int i2)
	{return i1 <= i2;}
  virtual bool double_cmp(const double d1, const double d2)
	{return d1 <= d2;}
  virtual bool string_cmp(const std::string& s1, const std::string& s2)
	{return s1 <= s2;}
};

}} // namespace varconf::dynvar

#endif
