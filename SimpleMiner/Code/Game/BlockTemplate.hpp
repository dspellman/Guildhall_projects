#pragma once
#include "Game/GameCommon.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include <string>
#include <vector>
#include "Engine/Math/IntVec3.hpp"
#include "BlockDefinition.hpp"

struct TemplateBlock
{
	IntVec3 m_offset = IntVec3::ZERO;
	uint8_t m_block = AIR;
};

class BlockTemplate
{
public:
//	~BlockTemplate();
	BlockTemplate();

	bool LoadFromXmlElement(const XmlElement& element);
	static void Initialize(const char* source);
	static void Destroy();
	static const BlockTemplate* GetByName(const std::string& name);
	static std::vector<BlockTemplate*> s_definitions;

	std::string m_name = {};
	std::vector<TemplateBlock> m_blocks = {};
};