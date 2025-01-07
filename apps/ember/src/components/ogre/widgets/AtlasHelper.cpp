/*
 Copyright (C) 2009 Erik Ogenvik <erik@ogenvik.org>

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software Foundation,
 Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "AtlasHelper.h"
#include "framework/Log.h"
#include "framework/AtlasObjectDecoder.h"
#include <Atlas/Objects/Operation.h>
#include <Atlas/Objects/Encoder.h>
#include <Atlas/Message/QueuedDecoder.h>
#include <Atlas/Codecs/Bach.h>
#include <Atlas/Codecs/XML.h>
#include <Atlas/Formatter.h>
#include <Atlas/PresentationBridge.h>
#include <memory>

namespace Ember::OgreView::Gui {

std::string AtlasHelper::serialize(const Atlas::Objects::Root& obj, const std::string& codecType) {
	std::stringstream ss;
	Atlas::Message::QueuedDecoder decoder;
	std::unique_ptr<Atlas::Bridge> codec;

	if (codecType == "bach") {
		codec = std::make_unique<Atlas::Codecs::Bach>(ss, ss, decoder);
	} else if (codecType == "xml") {
		codec = std::make_unique<Atlas::Codecs::XML>(ss, ss, decoder);
	} else if (codecType == "presentation") {
		codec = std::make_unique<Atlas::PresentationBridge>(ss);
	} else {
		logger->warn("Could not recognize codec type '{}'. Supported types are: xml, bach", codecType);
		return "";
	}

	Atlas::Objects::ObjectsEncoder encoder(*codec);
	encoder.streamObjectsMessage(obj);
	ss << std::flush;
	return ss.str();
}

Atlas::Objects::Root AtlasHelper::deserialize(const std::string& text, const std::string& codecType) {
	std::stringstream ss(text);
	Atlas::Objects::Factories factories;
	AtlasObjectDecoder atlasLoader(factories);

	std::unique_ptr<Atlas::Codec> codec;

	if (codecType == "bach") {
		codec = std::make_unique<Atlas::Codecs::Bach>(ss, ss, atlasLoader);
	} else if (codecType == "xml") {
		codec = std::make_unique<Atlas::Codecs::XML>(ss, ss, atlasLoader);
	} else {
		logger->warn("Could not recognize codec type '{}'. Supported types are: xml, bach", codecType);
		return {};
	}
	codec->poll();

	return atlasLoader.getLastObject();

}


}

