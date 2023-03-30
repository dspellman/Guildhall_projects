#include "Game/BlockTemplate.hpp"

std::vector<BlockTemplate*> BlockTemplate::s_definitions;

BlockTemplate::BlockTemplate()
{
}

bool BlockTemplate::LoadFromXmlElement(const XmlElement& element)
{
	m_name = ParseXmlAttribute(element, "name", "UNKNOWN");
	if (m_name == "UNKNOWN")
	{
		return false;
	}
	XmlElement const* SubElement = element.FirstChildElement();
	TemplateBlock tBlock;
	while (SubElement)
	{
		tBlock.m_block = AIR;
		tBlock.m_offset = ParseXmlAttribute(*SubElement, "offset", IntVec3(0, 0, 0));
		std::string block = ParseXmlAttribute(*SubElement, "block", "AIR");
		for (int index = 0; index < BLOCK_COUNT; index++)
		{
			if (block.compare(BlockNamesText[index]) == 0)
			{
				tBlock.m_block = (uint8_t)index;
			}
		}
		m_blocks.push_back(tBlock);
		SubElement = SubElement->NextSiblingElement();
	}
	return true;
}

void BlockTemplate::Initialize(const char* source)
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
		BlockTemplate* definition = new BlockTemplate;
		definition->LoadFromXmlElement(*element);
		BlockTemplate::s_definitions.push_back(definition);
		element = element->NextSiblingElement();
	}
}

void BlockTemplate::Destroy()
{
	for (BlockTemplate* obj : s_definitions)
	{
		delete obj;
	}
}

const BlockTemplate* BlockTemplate::GetByName(const std::string& name)
{
	for (BlockTemplate* definition : s_definitions)
	{
		if (definition->m_name == name)
		{
			return definition;
		}
	}
	return nullptr;
}
