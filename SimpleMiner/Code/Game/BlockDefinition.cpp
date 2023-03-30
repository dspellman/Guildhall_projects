#include "Game/BlockDefinition.hpp"

std::vector<BlockDefinition> BlockDefinition::s_definitions;
SpriteSheet* BlockDefinition::s_spriteSheet;

BlockDefinition::BlockDefinition()
{

}

bool BlockDefinition::LoadFromXmlElement(const XmlElement& element)
{
	m_name = ParseXmlAttribute(element, "name", "UNKNOWN");
	if (m_name == "UNKNOWN")
	{
		return false;
	}
	m_type = (uint8_t)ParseXmlAttribute(element, "type", 0);
	m_visible = ParseXmlAttribute(element, "visible", true);
	m_solid = ParseXmlAttribute(element, "solid", true);
	m_opaque = ParseXmlAttribute(element, "opaque", true);
	m_light =  ParseXmlAttribute(element, "light", 0);
	m_topSprite = ParseXmlAttribute(element, "topSprite", IntVec2(1, 1));
	m_sideSprite = ParseXmlAttribute(element, "sideSprite", IntVec2::ZERO);
	m_bottomSprite = ParseXmlAttribute(element, "bottomSprite", IntVec2::ZERO);
	m_top_uvs = s_spriteSheet->GetSpriteUVs(m_topSprite);
	m_side_uvs = s_spriteSheet->GetSpriteUVs(m_sideSprite);
	m_bottom_uvs = s_spriteSheet->GetSpriteUVs(m_bottomSprite);
	return true;
}

void BlockDefinition::Initialize(const char* source)
{
	// parse configuration file
	tinyxml2::XMLDocument doc;
	doc.LoadFile(source);
	XmlElement* root = doc.RootElement();
	if (!root)
	{
		return;
	}

	XmlElement* element = root->FirstChildElement();
	while (element)
	{
		BlockDefinition definition;
		definition.LoadFromXmlElement(*element);
		BlockDefinition::s_definitions.push_back(definition);
		element = element->NextSiblingElement();
	}
}
