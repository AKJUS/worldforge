/*
 *  dynbase.h - interface for dynamically derived value container class
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
 
#ifndef VARCONF_DYNBASE_H
#define VARCONF_DYNBASE_H

#include <string>
#include <varconf/variable.h>

namespace varconf {
namespace dynvar {

class Base : public VarBase {
public:
  Base() : VarBase(), m_looping(false) {}
  // Don't copy m_looping
  Base( const Base& d) : VarBase(d), m_looping(false) {}

  virtual ~Base() {}

  Base& operator= (const Base& b);

  // Don't call the const versions of these functions
  // for VarBase

private: // Does making these private do anything?
  friend std::ostream& operator<<( std::ostream& out, const Base& v);
  friend bool operator ==( const Base& one, const VarBase& two);
  friend bool operator ==( const VarBase& one, const Base& two);
  friend bool operator ==( const Base& one, const Base& two);
public:

  // The real versions

  friend std::ostream& operator<<( std::ostream& out, Base& v);
  friend bool operator ==( Base& one, const VarBase& two);
  friend bool operator ==( const VarBase& one, Base& two);
  friend bool operator ==( Base& one, Base& two);

  virtual operator bool();
  virtual operator int();
  virtual operator double();
  virtual operator std::string();

  virtual bool is_bool();
  virtual bool is_int();
  virtual bool is_double();
  virtual bool is_string();

protected:

  virtual void set_val() = 0;

private:

  void call_set_val();

  bool m_looping;
};

}} // namespace varconf::dynvar

#endif
