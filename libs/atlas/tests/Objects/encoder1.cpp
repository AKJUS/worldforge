#include <Atlas/Objects/Root.h>
#include <Atlas/Objects/SmartPtr.h>
#include <Atlas/Objects/Encoder.h>
#include <Atlas/Message/DecoderBase.h>
#include "loadDefaults.h"

#include <string>
#include <iostream>
#include <cstdlib>
#include <Atlas/Objects/Factories.h>

class RootDecoder : public Atlas::Message::DecoderBase
{
protected:
    virtual void messageArrived(Atlas::Message::MapType o)
    {
        assert(o.find(std::string("parent")) != o.end());
        assert((*o.find("parent")).second.asString() ==
                std::string("root"));
    }
};

int main(int argc, char** argv)
{
	Atlas::Objects::Factories factories;
    std::string atlas_xml_path;
    char * srcdir_env = getenv("srcdir");
    if (srcdir_env != 0) {
        atlas_xml_path = srcdir_env;
        atlas_xml_path += "/";
    }
    atlas_xml_path += "../../protocol/spec/atlas.xml";
    try {
        Atlas::Objects::loadDefaults(atlas_xml_path, factories);
    } catch(const Atlas::Objects::DefaultLoadingException& e) {
        std::cout << "DefaultLoadingException: "
             << e.getDescription() << std::endl;
    }
    RootDecoder rd;
    Atlas::Objects::ObjectsEncoder re(rd);

    rd.streamBegin(); // important, otherwise we'll segfault!
    Atlas::Objects::Root root_inst;
    root_inst->setAttr("id", std::string("root_instantiation"));
    re.streamObjectsMessage(root_inst);
    rd.streamEnd();
}
