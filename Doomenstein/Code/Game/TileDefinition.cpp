#include "TileDefinition.hpp"
#include "TileMaterialDefinition.hpp"

std::vector<TileDefinition*> TileDefinition::s_definitions;

bool TileDefinition::LoadFromXmlElement(const XmlElement& element)
{
	m_name = ParseXmlAttribute(element, "name", "UNKNOWN");
	m_isSolid = ParseXmlAttribute(element, "isSolid", false);
	std::string materialName = ParseXmlAttribute(element, "ceilingMaterial", "None");
	m_ceilingMaterialDefinition = TileMaterialDefinition::GetByName(materialName);
	materialName = ParseXmlAttribute(element, "floorMaterial", "None");
	m_floorMaterialDefinition = TileMaterialDefinition::GetByName(materialName);
	materialName = ParseXmlAttribute(element, "wallMaterial", "None");
	m_wallMaterialDefinition = TileMaterialDefinition::GetByName(materialName);
	return true;
}

void TileDefinition::InitializeDefinitions()
{
	TileMaterialDefinition::InitializeDefinitions();
	// parse configuration file
	tinyxml2::XMLDocument doc;
	doc.LoadFile("Data/Definitions/TileDefinitions.xml");
	XmlElement* root = doc.RootElement();
	if (!root)
	{
		ERROR_AND_DIE("TileDefinitions.xml problem reading file!");
	}

	XmlElement* element = root->FirstChildElement();
	while (element)
	{
		TileDefinition* pDefinition = new TileDefinition();
		pDefinition->LoadFromXmlElement(*element);
		TileDefinition::s_definitions.push_back(pDefinition);
		element = element->NextSiblingElement();
	}
}

const TileDefinition* TileDefinition::GetByName(const std::string& name)
{
	for (TileDefinition* definition : s_definitions)
	{
		if (definition && definition->m_name == name)
		{
			return definition;
		}
	}
	return nullptr;
}

