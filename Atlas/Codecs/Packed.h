// This file may be redistributed and modified only under the terms of
// the GNU Lesser General Public License (See COPYING for details).
// Copyright (C) 2000-2001 Stefanus Du Toit, Michael Day

#ifndef ATLAS_CODECS_PACKED_H
#define ATLAS_CODECS_PACKED_H

#include <Atlas/Codecs/Utility.h>
#include <Atlas/Codec.h>

#include <iosfwd>
#include <stack>

namespace Atlas { namespace Codecs {

/*

The form for each element of this codec is as follows:

[type][name=][data][|endtype]
  
( ) for lists
[ ] for maps
$ for string
@ for int
# for float

Sample output for this codec: (whitespace added for clarity)

[@id=17$name=Fred +28the +2b great+29#weight=1.5(args=@1@2@3)]

The complete specification is located in cvs at:
    forge/protocols/atlas/spec/packed_syntax.html
    
*/
  
class Packed : public Codec
{
public:
    
    Packed(std::iostream& s, Atlas::Bridge* b);

    virtual void poll(bool can_read = true);

    virtual void streamBegin();
    virtual void streamMessage();
    virtual void streamEnd();

    virtual void mapMapItem(const std::string& name);
    virtual void mapListItem(const std::string& name);
    virtual void mapIntItem(const std::string& name, long);
    virtual void mapFloatItem(const std::string& name, double);
    virtual void mapStringItem(const std::string& name, const std::string&);
    virtual void mapEnd();
    
    virtual void listMapItem();
    virtual void listListItem();
    virtual void listIntItem(long);
    virtual void listFloatItem(double);
    virtual void listStringItem(const std::string&);
    virtual void listEnd();

protected:
    
    std::iostream& m_socket;
    Bridge* m_bridge;

    enum State
    {
	PARSE_STREAM,
        PARSE_MAP,
        PARSE_LIST,
	PARSE_MAP_BEGIN,
	PARSE_LIST_BEGIN,
        PARSE_INT,
        PARSE_FLOAT,
        PARSE_STRING,
        PARSE_NAME
    };
    
    std::stack<State> m_state;

    std::string m_name;
    std::string m_data;

    inline void parseStream(char);
    inline void parseMap(char);
    inline void parseList(char);
    inline void parseMapBegin(char);
    inline void parseListBegin(char);
    inline void parseInt(char);
    inline void parseFloat(char);
    inline void parseString(char);
    inline void parseName(char);

    inline const std::string hexEncode(const std::string& data)
    {
	return hexEncodeWithPrefix("+", "+[]()@#$=", data);
    }

    inline const std::string hexDecode(const std::string& data)
    {
	return hexDecodeWithPrefix("+", data);
    }
};

} } // namespace Atlas::Codecs

#endif
