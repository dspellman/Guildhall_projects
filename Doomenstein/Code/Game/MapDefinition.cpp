#include "Game/GameCommon.hpp"
#include "MapDefinition.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Math/Vec3.hpp"
#include <string>
#include "TileSetDefinition.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "SpawnInfo.hpp"

std::vector<MapDefinition*> MapDefinition::s_definitions;

//------------------------------------------------------------------------------------------------
bool MapDefinition::LoadFromXmlElement(const XmlElement& element)
{
	m_name = ParseXmlAttribute(element, "name", "UNKNOWN");
	std::string imageFile = ParseXmlAttribute(element, "image", "UNKNOWN");
	if (imageFile.compare("UNKNOWN") == 0)
	{
		m_image = Image( IntVec2(10, 10), Rgba8::WHITE, "UNKNOWN" ); // need a default interpretation?
	}
	else
	{
		m_image = Image(imageFile.c_str());
	}
	std::string definition = ParseXmlAttribute(element, "tileSet", "Default");
	m_tileSetDefinition = TileSetDefinition::GetByName(definition);

	XmlElement const* SubElement = element.FirstChildElement();
	SubElement = SubElement->FirstChildElement();
	SpawnInfo pInfo;
	while (SubElement)
	{
		pInfo.LoadFromXmlElement(*SubElement);
		m_spawnInfos.push_back(pInfo);
		SubElement = SubElement->NextSiblingElement();
	}

	return true;
}

void MapDefinition::InitializeDefinitions()
{
	TileSetDefinition::InitializeDefinitions();
	// parse configuration file
	tinyxml2::XMLDocument doc;
	doc.LoadFile("Data/Definitions/MapDefinitions.xml");
	XmlElement* root = doc.RootElement();
	if (!root)
	{
		ERROR_AND_DIE("MapDefinitions.xml problem reading file!");
	}

	XmlElement* element = root->FirstChildElement();
	while (element)
	{
		MapDefinition* pDefinition = new MapDefinition();
		pDefinition->LoadFromXmlElement(*element);
		MapDefinition::s_definitions.push_back(pDefinition);
		element = element->NextSiblingElement();
	}
}

void MapDefinition::ClearDefinitions()
{
	destroy<MapDefinition>(s_definitions);
}

const MapDefinition* MapDefinition::GetByName(const std::string& name)
{
	for (MapDefinition* definition : s_definitions)
	{
		if (definition && definition->m_name == name)
		{
			return definition;
		}
	}
	return nullptr;
}

