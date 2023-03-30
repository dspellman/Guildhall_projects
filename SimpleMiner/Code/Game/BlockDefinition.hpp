#pragma once
#include "Game/GameCommon.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include <string>
#include <vector>
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"

#define REGISTER_ENUM(x) x,
typedef enum
{
	#include "Game/BlockNames.hpp"
	BLOCK_COUNT
} BlockType;
#undef REGISTER_ENUM

#define REGISTER_ENUM(x) #x,
static const char* BlockNamesText[] =
{
	#include "Game/BlockNames.hpp"
	"Invalid"
};
#undef REGISTER_ENUM

class BlockDefinition
{
public:
//	~BlockDefinition();
	BlockDefinition();

	bool LoadFromXmlElement(const XmlElement& element);
	static void Initialize(const char* source);
	static std::vector<BlockDefinition> s_definitions;
	static SpriteSheet* s_spriteSheet;

	std::string m_name = {};
	uint8_t m_type = AIR;
	bool m_visible = true;
	bool m_solid = true;
	bool m_opaque = true;
	int m_light = 0;

	IntVec2 m_topSprite = IntVec2::ZERO;
	IntVec2 m_sideSprite = IntVec2::ZERO;
	IntVec2 m_bottomSprite = IntVec2::ZERO;

	AABB2 m_top_uvs = AABB2::ZERO_TO_ONE;
	AABB2 m_side_uvs = AABB2::ZERO_TO_ONE;
	AABB2 m_bottom_uvs = AABB2::ZERO_TO_ONE;
};