//
// C++ Implementation: LuaConnectors
//
// Description:
//
//
// Author: Erik Ogenvik <erik@ogenvik.org>, (C) 2005
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.//
//
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <Atlas/Message/Element.h>
#include "TypeResolving.h"

#include "components/ogre/MousePicker.h"


namespace Ember
{

namespace Lua
{

/**
 * For every type you want to move between C++ and Lua through either an Ember::Lua::RefValueAdapter or an Ember::Lua::PtrValueAdapter you must provide a suitable method here.
 */

template <>
const char* resolveLuaTypename<const Ogre::LodConfig>(){return "Ogre::LodConfig";}
template <>
const char* resolveLuaTypename<const Ember::EmberEntity>(){return "Ember::EmberEntity";}
template <>
const char* resolveLuaTypename<const Ogre::Camera>(){return "Ogre::Camera";}
template <>
const char* resolveLuaTypename<const Eris::Connection>(){return "Eris::Connection";}
template <>
const char* resolveLuaTypename<const Eris::ServerInfo>(){return "Eris::ServerInfo";}
template <>
const char* resolveLuaTypename<const Ember::OgreView::EntityPickResult>(){return "Ember::OgreView::EntityPickResult";}
template <>
const char* resolveLuaTypename<const Ember::OgreView::MousePickerArgs>(){return "Ember::OgreView::MousePickerArgs";}
template <>
const char* resolveLuaTypename<const Ember::OgreView::EmberEntityFactory>(){return "Ember::OgreView::EmberEntityFactory";}
template <>
const char* resolveLuaTypename<const Ember::OgreView::Authoring::EntityMover>(){return "Ember::OgreView::Authoring::EntityMover";}
template <>
const char* resolveLuaTypename<const Ember::OgreView::Terrain::BasePointUserObject>(){return "Ember::OgreView::Terrain::BasePointUserObject";}
template <>
const char* resolveLuaTypename<const Ember::OgreView::Terrain::TerrainEditAction>(){return "Ember::OgreView::Terrain::TerrainEditAction";}
template <>
const char* resolveLuaTypename<const Ember::OgreView::Terrain::TerrainEditorOverlay>(){return "Ember::OgreView::Terrain::TerrainEditorOverlay";}
template <>
const char* resolveLuaTypename<const Ember::OgreView::Terrain::TerrainManager>(){return "Ember::OgreView::Terrain::TerrainManager";}
template <>
const char* resolveLuaTypename<const Eris::Task>(){return "Eris::Task";}
template <>
const char* resolveLuaTypename<const Ember::OgreView::Gui::EntityIcon>(){return "Ember::OgreView::Gui::EntityIcon";}
template <>
const char* resolveLuaTypename<const Ember::OgreView::Gui::EntityIconSlot>(){return "Ember::OgreView::Gui::EntityIconSlot";}
template <>
const char* resolveLuaTypename<const Ember::OgreView::Gui::ActionBarIcon>(){return "Ember::OgreView::Gui::ActionBarIcon";}
template <>
const char* resolveLuaTypename<const Ember::OgreView::Gui::HelpMessage>(){return "Ember::OgreView::Gui::HelpMessage";}
template <>
const char* resolveLuaTypename<const Ember::OgreView::Terrain::TerrainPage>(){return "Ember::OgreView::Terrain::TerrainPage";}
template <>
const char* resolveLuaTypename<const Eris::Avatar>(){return "Eris::Avatar";}
template <>
const char* resolveLuaTypename<const Eris::View>(){return "Eris::View";}
template <>
const char* resolveLuaTypename<const Eris::Account>(){return "Eris::Account";}
template <>
const char* resolveLuaTypename<const Ember::OgreView::World>(){return "Ember::OgreView::World";}
template <>
const char* resolveLuaTypename<const std::vector<Ember::OgreView::EntityPickResult>>(){return "std::vector<Ember::OgreView::EntityPickResult>";}
template <>
const char* resolveLuaTypename<const std::set<std::string>>(){return "std::set<std::string>";}
template <>
const char* resolveLuaTypename<const Atlas::Message::Element>(){return "Atlas::Message::Element";}
template <>
const char* resolveLuaTypename<const Atlas::Objects::Root>(){return "Atlas::Objects::Root";}
template <>
const char* resolveLuaTypename<const Ember::OgreView::MotionManager>(){return "Ember::OgreView::MotionManager";}
template <>
const char* resolveLuaTypename<const Ember::OgreView::GUIManager>(){return "Ember::OgreView::GUIManager";}
template <>
const char* resolveLuaTypename<const Eris::TypeInfo>(){return "Eris::TypeInfo";}
template <>
const char* resolveLuaTypename<const Ember::EntityTalk>(){return "Ember::EntityTalk";}
template <>
const char* resolveLuaTypename<const Ember::MouseMotion>(){return "Ember::MouseMotion";}
template <>
const char* resolveLuaTypename<const Atlas::Message::MapType>(){return "Atlas::Message::MapType";}
template <>
const char* resolveLuaTypename<const Eris::Entity>(){return "Eris::Entity";}


}
}

