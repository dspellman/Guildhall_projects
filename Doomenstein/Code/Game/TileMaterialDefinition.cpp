#include "TileMaterialDefinition.hpp"

std::vector<TileMaterialDefinition*> TileMaterialDefinition::s_definitions;

bool TileMaterialDefinition::LoadFromXmlElement(const XmlElement& element)
{
	m_name = ParseXmlAttribute(element, "name", "UNKNOWN");
	m_isVisible = ParseXmlAttribute(element, "isVisible", true); // not set for normal definitions?
	std::string shaderName = ParseXmlAttribute(element, "shader", "Default"); // default is the value anyway, so maybe a different value to distinguish it?
	m_shader = g_theRenderer->CreateOrGetShaderFromFile(shaderName.c_str());
	std::string textureName = ParseXmlAttribute(element, "texture", "Default");
	Texture* tempTexture;
	IntVec2 cellCount;
	IntVec2 cell;
	if (textureName.compare("Default") == 0)
	{
		tempTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Extras_4x4.png"); // something to debug with
		cellCount = IntVec2(4, 4);
		cell = IntVec2(3, 3);
	}
	else
	{
		tempTexture = g_theRenderer->CreateOrGetTextureFromFile(textureName.c_str());
		cellCount = ParseXmlAttribute(element, "cellCount", IntVec2(8, 8));
		cell = ParseXmlAttribute(element, "cell", IntVec2::ZERO);
	}
	m_texture = tempTexture; // fix up to avoid const problem in SpriteSheet constructor
	SpriteSheet* sprite = new SpriteSheet(*tempTexture, cellCount);
	m_uv = sprite->GetSpriteUVs(cellCount.x * cell.y + cell.x);
	delete sprite; // the easy way to handle this?
	return true;
}

void TileMaterialDefinition::InitializeDefinitions()
{
	// parse configuration file
	tinyxml2::XMLDocument doc;
	doc.LoadFile("Data/Definitions/TileMaterialDefinitions.xml");
	XmlElement* root = doc.RootElement();
	if (!root)
	{
		ERROR_AND_DIE("TileMaterialDefinitions.xml problem reading file!");
	}

	XmlElement* element = root->FirstChildElement();
	while (element)
	{
		TileMaterialDefinition* pDefinition = new TileMaterialDefinition();
		pDefinition->LoadFromXmlElement(*element);
		TileMaterialDefinition::s_definitions.push_back(pDefinition);
		element = element->NextSiblingElement();
	}
}

const TileMaterialDefinition* TileMaterialDefinition::GetByName(const std::string& name)
{
	for (TileMaterialDefinition* definition : s_definitions)
	{
		if (definition->m_name == name)
		{
			return definition;
		}
	}
	return nullptr;
}

