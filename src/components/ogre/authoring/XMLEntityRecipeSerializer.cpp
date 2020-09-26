//
// C++ Implementation: XMLEntityRecipeSerializer
//
// Description:
//
//
// Author: Alexey Torkhov <atorkhov@gmail.com>, (C) 2008
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

#include "XMLEntityRecipeSerializer.h"
#include "EntityRecipeManager.h"
#include "components/ogre/XMLHelper.h"

namespace Ember {
namespace OgreView {
namespace Authoring {
XMLEntityRecipeSerializer::XMLEntityRecipeSerializer() = default;

XMLEntityRecipeSerializer::~XMLEntityRecipeSerializer() = default;

std::map<std::string, std::unique_ptr<EntityRecipe>> XMLEntityRecipeSerializer::parseScript(Ogre::DataStreamPtr& stream) {
	TiXmlDocument xmlDoc;
	XMLHelper xmlHelper;
	if (!xmlHelper.Load(xmlDoc, stream)) {
		return {};
	}
	TiXmlElement* rootElem = xmlDoc.RootElement();

	std::map<std::string, std::unique_ptr<EntityRecipe>> entries;
	for (TiXmlElement* smElem = rootElem->FirstChildElement(); smElem != nullptr; smElem = smElem->NextSiblingElement()) {
		const char* tmp = smElem->Attribute("name");
		std::string name;
		if (!tmp) {
			continue;
		} else {
			name = tmp;
		}

		try {
			auto entRecipe = std::make_unique<EntityRecipe>();
			if (entRecipe) {
				entRecipe->mName = name;
				readRecipe(*entRecipe, smElem);
				entries.emplace(name, std::move(entRecipe));
			}
		} catch (const std::exception& ex) {
			S_LOG_FAILURE("Error when parsing entity recipe '" << name << "'." << ex);
		}
	}
	return entries;
}

void XMLEntityRecipeSerializer::readRecipe(EntityRecipe& entRecipe, TiXmlElement* recipeNode) {
	TiXmlElement* elem;

	// Author
	elem = recipeNode->FirstChildElement("author");
	if (elem) {
		const char* text = elem->GetText();
		if (text) {
			entRecipe.setAuthor(text);
		}
	}

	// Description
	elem = recipeNode->FirstChildElement("description");
	if (elem) {
		const char* text = elem->GetText();
		if (text) {
			entRecipe.setDescription(text);
		}
	}

	// Entity specification
	elem = recipeNode->FirstChildElement("entity");
	if (elem) {
		readEntitySpec(entRecipe, elem);
	}

	// GUI adapters
	elem = recipeNode->FirstChildElement("adapters");
	if (elem) {
		readAdapters(entRecipe, elem);
	}

	// Script bindings
	elem = recipeNode->FirstChildElement("bindings");
	if (elem) {
		readBindings(entRecipe, elem);
	}

	// Script
	elem = recipeNode->FirstChildElement("script");
	if (elem) {
		readScript(entRecipe, elem);
	}
}

void XMLEntityRecipeSerializer::readEntitySpec(EntityRecipe& entRecipe, TiXmlElement* entSpecNode) {
	S_LOG_VERBOSE("Read entity spec.");

	// Copy <entity> part of XML into recipe
	entRecipe.mEntitySpec.reset(entSpecNode->Clone()->ToElement());
	const char* type = entRecipe.mEntitySpec->Attribute("type");
	if (type) {
		entRecipe.mEntityType = std::string(type);
	}

	/*
	 // Print <entity> part of XML into string and wrap it with stream
	 TiXmlPrinter printer;
	 printer.SetStreamPrinting();
	 entSpecNode->Accept( &printer );

	 std::stringstream strStream(printer.CStr(), std::ios::in);

	 // Create objects
	 Atlas::Message::QueuedDecoder decoder;
	 Atlas::Codecs::XML codec(strStream, decoder);

	 // Read whole stream into decoder queue
	 while (!strStream.eof())
	 {
	 codec.poll();
	 }

	 // Read decoder queue
	 Atlas::Message::MapType m;
	 std::string str;
	 Atlas::Message::Element elem;
	 while (decoder.queueSize() > 0)
	 {
	 // Decoding map message
	 m = decoder.popMessage();
	 entRecipe.mEntitySpec.push_back(m);
	 for (Atlas::Message::MapType::const_iterator iter = m.begin(); iter != m.end(); iter++)
	 {
	 S_LOG_VERBOSE(" " << iter->first);
	 }
	 }
	 */
}

void XMLEntityRecipeSerializer::readAdapters(EntityRecipe& entRecipe, TiXmlElement* adaptersNode) {
	S_LOG_VERBOSE("Read adapters.");
	for (TiXmlElement* smElem = adaptersNode->FirstChildElement("adapter"); smElem != nullptr; smElem = smElem->NextSiblingElement("adapter")) {
		const std::string* name, * type, * tooltip, * defaultValue;
		std::string tooltipText;
		if (!(name = smElem->Attribute(std::string("name"))))
			continue;

		if (!(type = smElem->Attribute(std::string("type"))))
			continue;

		if ((tooltip = smElem->Attribute(std::string("tooltip")))) {
			tooltipText = *tooltip;
		}

		defaultValue = smElem->Attribute(std::string("default"));

		S_LOG_VERBOSE(" adapter '" << *name << "' of type " << *type);

		GUIAdapter* adapter = entRecipe.createGUIAdapter(*name, *type, tooltipText);

		const std::string* title;
		if ((title = smElem->Attribute(std::string("title")))) {
			adapter->setTitle(*title);
		}

		if (defaultValue) {
			adapter->setDefaultValue(*defaultValue);
		}

		// Custom adapter parameters
		if (*type == "string") {
			for (TiXmlElement* item = smElem->FirstChildElement("item"); item != nullptr; item = item->NextSiblingElement("item")) {
				const char* text;
				const std::string* value;
				text = item->GetText();
				if ((value = item->Attribute(std::string("value")))) {
					adapter->addSuggestion(*value, text);
				} else {
					adapter->addSuggestion(text, text);
				}
			}

			const std::string* allowRandom;
			if ((allowRandom = smElem->Attribute(std::string("allowrandom"))) && (*allowRandom == "yes" || *allowRandom == "true" || *allowRandom == "on")) {
				adapter->setAllowRandom(true);
			}
		}
	}
}

void XMLEntityRecipeSerializer::readBindings(EntityRecipe& entRecipe, TiXmlElement* bindingsNode) {
	S_LOG_VERBOSE("Read bindings.");
	for (TiXmlElement* smElem = bindingsNode->FirstChildElement("bind"); smElem != nullptr; smElem = smElem->NextSiblingElement("bind")) {
		const std::string* name, * func;

		if (!(name = smElem->Attribute(std::string("name"))))
			continue;

		GUIAdapterBindings* bindings = entRecipe.createGUIAdapterBindings(*name);

		if ((func = smElem->Attribute(std::string("func")))) {
			bindings->setFunc(*func);
		}

		readBindAdapters(entRecipe, bindings, smElem);
	}

	// Associating bindings with placeholders after parsing
	entRecipe.associateBindings();
}

void XMLEntityRecipeSerializer::readBindAdapters(EntityRecipe& /*entRecipe*/, GUIAdapterBindings* bindings, TiXmlElement* bindAdaptersNode) {
	S_LOG_VERBOSE("  Reading bind adapters.");
	for (TiXmlElement* elem = bindAdaptersNode->FirstChildElement(); elem != nullptr; elem = elem->NextSiblingElement()) {
		const std::string* name;

		if (!(name = elem->Attribute(std::string("name"))))
			continue;

		bindings->addAdapter(*name);
	}
}

void XMLEntityRecipeSerializer::readScript(EntityRecipe& entRecipe, TiXmlElement* scriptNode) {
	S_LOG_VERBOSE("Read script.");
	const char* text = scriptNode->GetText();
	if (text) {
		entRecipe.mScript = text;
	}
}

}
}
}
