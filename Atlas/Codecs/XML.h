// This file may be redistributed and modified under the terms of the
// GNU Lesser General Public License (See COPYING for details).
// Copyright (C) 2000-2001 Michael Day, Stefanus Du Toit

#ifndef ATLAS_CODECS_XML_H
#define ATLAS_CODECS_XML_H

#include <Atlas/Codec.h>

#include <iostream>
#include <stack>

namespace Atlas { namespace Codecs {

/*

Sample output for this codec: (whitespace added for clarity)

<atlas>
    <map>
	<int name="foo">13</int>
	<float name="meep">1.5</float>
	<string name="bar">hello</string>
	<list name="args">
	    <int>1</int>
	    <int>2</int>
	    <float>3.0</float>
	</list>
    </map>
</atlas>

The complete specification is located in cvs at:
    forge/protocols/atlas/spec/xml_syntax.html

*/

class XML : public Codec
{
    public:

    XML(std::iostream& s, Atlas::Bridge* b);

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
    
    enum Token
    {
	TOKEN_TAG,
	TOKEN_START_TAG,
	TOKEN_END_TAG,
	TOKEN_DATA
    };
    
    Token m_token;
    
    enum State
    {
	PARSE_NOTHING,
	PARSE_STREAM,
        PARSE_MAP,
        PARSE_LIST,
	PARSE_INT,
	PARSE_FLOAT,
	PARSE_STRING
    };
    
    std::stack<State> m_state;
    std::stack<std::string> m_data;

    std::string m_tag;
    std::string m_name;

    inline void tokenTag(char);
    inline void tokenStartTag(char);
    inline void tokenEndTag(char);
    inline void tokenData(char);

    inline void parseStartTag();
    inline void parseEndTag();
};

} } // namespace Atlas::Codecs

#endif // ATLAS_CODECS_XML_H
