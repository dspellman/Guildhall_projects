#include "TileSetDefinition.hpp"
#include "TileDefinition.hpp"

std::vector<TileSetDefinition*> TileSetDefinition::s_definitions;

bool TileMapping::LoadFromXmlElement(const XmlElement& element)
{
	m_color = ParseXmlAttribute(element, "color", Rgba8::WHITE);
	std::string tileName = ParseXmlAttribute(element, "tile", "None");
	m_tileDefinition = TileDefinition::GetByName(tileName); 
	return true;
}

bool TileSetDefinition::LoadFromXmlElement(XmlElement const& element)
{
	m_name = ParseXmlAttribute(element, "name", "UNKNOWN");

	XmlElement const* SubElement = element.FirstChildElement();
	TileMapping pDefinition;
	while (SubElement)
	{
		pDefinition.LoadFromXmlElement(*SubElement);
		m_mappings.push_back(pDefinition);
		SubElement = SubElement->NextSiblingElement();
	}

	std::string defaultName = ParseXmlAttribute(element, "defaultTile", "None");
	m_defaultTile = TileDefinition::GetByName(defaultName);
	return true;
}

const TileDefinition* TileSetDefinition::GetTileDefinitionByColor(const Rgba8& color) const
{
	for (TileMapping definition : m_mappings)
	{
		if (definition.m_color == color)
		{
			return definition.m_tileDefinition;
		}
	}
	return m_defaultTile; // alternative if tile is not found
}

void TileSetDefinition::InitializeDefinitions()
{
	TileDefinition::InitializeDefinitions();
	// parse configuration file
	tinyxml2::XMLDocument doc;
	doc.LoadFile("Data/Definitions/TileSetDefinitions.xml");
	XmlElement* root = doc.RootElement();
	if (!root)
	{
		ERROR_AND_DIE("TileSetDefinitions.xml problem reading file!");
	}

	XmlElement* element = root->FirstChildElement();
	while (element)
	{
		TileSetDefinition* pDefinition = new TileSetDefinition();
		pDefinition->LoadFromXmlElement(*element);
		TileSetDefinition::s_definitions.push_back(pDefinition);
		element = element->NextSiblingElement();
	}
}

const TileSetDefinition* TileSetDefinition::GetByName(const std::string& name)
{
	for (TileSetDefinition* definition : s_definitions)
	{
		if (definition->m_name == name)
		{
			return definition;
		}
	}
	return nullptr;
}
