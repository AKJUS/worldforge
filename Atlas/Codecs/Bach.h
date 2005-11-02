// This file may be redistributed and modified under the terms of the
// GNU Lesser General Public License (See COPYING for details).
// Copyright (C) 2000-2001 Michael Day, Stefanus Du Toit

#ifndef ATLAS_CODECS_BACH_H
#define ATLAS_CODECS_BACH_H

#include <Atlas/Codec.h>

#include <iosfwd>
#include <stack>

namespace Atlas { namespace Codecs {

/** @file Codecs/Bach.h
 * This class implements Bach Codec
 */

class Bach : public Codec
{
  public:

    Bach(std::iostream& s, Atlas::Bridge & b);

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

    unsigned linenum() const {return m_linenum;}

  protected:

    std::iostream& m_socket;
    Bridge & m_bridge;
    bool m_comma;
    unsigned m_linenum;

    enum State
    {
        PARSE_INIT,
	PARSE_STREAM,
        PARSE_MAP,
        PARSE_LIST,
        PARSE_NAME,
        PARSE_DATA,
        PARSE_INT,
        PARSE_FLOAT,
        PARSE_STRING,
	PARSE_LITERAL, // for literal character escaped with backslash
	PARSE_COMMENT // for when we're in the middle of a comment field
    };

    bool stringmode() const;

    std::string m_name, m_data;
    std::stack<State> m_state;

    inline void parseInit(char);
    inline void parseStream(char);
    inline void parseMap(char);
    inline void parseList(char);
    inline void parseData(char);
    inline void parseInt(char);
    inline void parseFloat(char);
    inline void parseString(char);
    inline void parseLiteral(char);
    inline void parseName(char);
    inline void parseComment(char);

    inline const std::string encodeString(const std::string &);
    inline const std::string decodeString(const std::string &);

    void writeIntItem(const std::string &,long);
    void writeFloatItem(const std::string &,double);
    void writeStringItem(const std::string &,const std::string &);
    void writeLine(const std::string &,bool=true,bool=false);
};

} } // namespace Atlas::Codecs

#endif // ATLAS_CODECS_BACH_H
