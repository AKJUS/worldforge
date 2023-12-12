//
// C++ Implementation: XMLEntityMappingDefinitionSerializer
//
// Description:
//
//
// Author: Erik Ogenvik <erik@ogenvik.org>, (C) 2007
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
#include "XMLEntityMappingDefinitionSerializer.h"
#include "framework/Log.h"
//#include "components/ogre/EmberOgrePrerequisites.h"
using namespace Ember::EntityMapping;
using namespace Ember::EntityMapping::Definitions;


namespace Ember::OgreView::Mapping {


XMLEntityMappingDefinitionSerializer::XMLEntityMappingDefinitionSerializer(EntityMappingManager& entitymappingManager)
		: mEntityMappingManager(entitymappingManager) {
}

void XMLEntityMappingDefinitionSerializer::parseScript(TiXmlDocument xmlDocument, const std::string& path) {

	auto rootElem = xmlDocument.RootElement();

	if (rootElem) {
		if (rootElem->ValueStr() == "entitymapping") {
			parseSingleMapping(rootElem, path);
		} else {
			logger->error("Unrecognized start tag when parsing entity mapping in file '{}': {}", path, rootElem->ValueStr());
		}

	}
}

void XMLEntityMappingDefinitionSerializer::parseMatchElement(EntityMappingDefinition& definition, MatchDefinition& matchDef, TiXmlElement* element) {
	std::string caseType;
	if (element->ValueStr() == "entitymatch") {
		matchDef.Type = "entitytype";
		caseType = "entitytypecase";
	} else if (element->ValueStr() == "attributematch") {
		matchDef.Type = "attribute";
		caseType = "attributecase";

/*		const char* tmp =  smElem->Attribute("attribute");
		matchDef.Properties["attribute"] = std::string(tmp);*/
	} else if (element->ValueStr() == "entityrefmatch") {
		matchDef.Type = "entityref";
		caseType = "entityrefcase";
	}

	for (TiXmlAttribute* attribute = element->FirstAttribute();
		 attribute != nullptr; attribute = attribute->Next()) {
		matchDef.Properties[attribute->Name()] = attribute->Value();
	}

	if (!element->NoChildren()) {
		for (auto childElement = element->FirstChildElement();
			 childElement != nullptr; childElement = childElement->NextSiblingElement()) {
			CaseDefinition caseDef;
			caseDef.Type = caseType;
			parseCaseElement(definition, caseDef, childElement);
			matchDef.Cases.push_back(std::move(caseDef));
		}
	}
}

void XMLEntityMappingDefinitionSerializer::parseCaseElement(EntityMappingDefinition& definition, CaseDefinition& caseDef, TiXmlElement* element) {
	for (TiXmlAttribute* attribute = element->FirstAttribute();
		 attribute != nullptr; attribute = attribute->Next()) {
		caseDef.Properties[attribute->Name()] = attribute->Value();
	}


	if (!element->NoChildren()) {
		for (auto childElement = element->FirstChildElement();
			 childElement != nullptr; childElement = childElement->NextSiblingElement()) {
			if (childElement->ValueStr() == "action") {
				ActionDefinition actionDef;
				parseActionElement(definition, actionDef, childElement);
				caseDef.Actions.push_back(actionDef);
			} else if (childElement->ValueStr() == "caseparam") {
				//it's a case parameter
				if (const char* attributeValue = childElement->Attribute("type")) {
					if (TiXmlNode* textNode = childElement->FirstChild()) {
						caseDef.Parameters.emplace_back(attributeValue, textNode->Value());
					}
				}
			} else {
				//we'll assume it's a match
				MatchDefinition matchDef;
				parseMatchElement(definition, matchDef, childElement);
				caseDef.Matches.push_back(std::move(matchDef));
			}
		}
	}
}

void XMLEntityMappingDefinitionSerializer::parseActionElement(EntityMappingDefinition& definition, ActionDefinition& actionDef, TiXmlElement* element) {
	for (auto attribute = element->FirstAttribute(); attribute != nullptr; attribute = attribute->Next()) {
		actionDef.Properties[attribute->Name()] = attribute->Value();
	}
	actionDef.Type = element->Attribute("type");
	TiXmlNode* textNode = element->FirstChild();
	if (textNode) {
		actionDef.Value = textNode->Value();
	}

}


void XMLEntityMappingDefinitionSerializer::parseSingleMapping(TiXmlElement* rootElem, const std::string& path) {
	auto definition = std::make_unique<EntityMappingDefinition>();

	definition->Name = path;

	parseCaseElement(*definition, definition->RootCase, rootElem);
	mEntityMappingManager.addDefinition(std::move(definition));
}

}


