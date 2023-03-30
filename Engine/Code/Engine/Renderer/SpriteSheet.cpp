#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Math/IntVec2.hpp"

SpriteSheet::SpriteSheet(Texture& texture, IntVec2 const& simpleGridLayout)
	: m_texture(texture), m_dimensions(simpleGridLayout)
{
	Vec2 uvAtMins;
	Vec2 uvAtMaxs;

	for (int row = 0; row < simpleGridLayout.y; row++)
	{
		for (int column = 0; column < simpleGridLayout.x; column++)
		{
			GetSpriteUVs(uvAtMins, uvAtMaxs, row * simpleGridLayout.x + column);
			SpriteDefinition sprite(*this, row * simpleGridLayout.x + column, uvAtMins, uvAtMaxs);
			m_spriteDefs.push_back(sprite);
		}
	}
}

void SpriteSheet::GetSpriteUVs(Vec2& out_uvAtMins, Vec2& out_uvAtMaxs, int spriteIndex) const
{
	float rowSize = 1.0f / static_cast<float>(m_dimensions.y);
	float columnSize = 1.0f / static_cast<float>(m_dimensions.x);
	out_uvAtMins = Vec2(GetClamped(columnSize * static_cast<float>(spriteIndex % m_dimensions.x), 0.0f, 1.0f), 
						GetClamped(rowSize * static_cast<float>(m_dimensions.y - spriteIndex / m_dimensions.x - 1), 0.0f, 1.0f));
	out_uvAtMaxs = Vec2(GetClamped(columnSize * static_cast<float>(spriteIndex % m_dimensions.x + 1), 0.0f, 1.0f), 
						GetClamped( rowSize * static_cast<float>(m_dimensions.y - spriteIndex / m_dimensions.x), 0.0f, 1.0f));
	Vec2 Correction = Vec2(rowSize * 0.01f, columnSize * 0.01f);
	out_uvAtMins += Correction;
	out_uvAtMaxs -= Correction;
}

AABB2 SpriteSheet::GetSpriteUVs(int spriteIndex) const
{
	Vec2 uvAtMins;
	Vec2 uvAtMaxs;

	GetSpriteUVs(uvAtMins, uvAtMaxs, spriteIndex);
	return AABB2(uvAtMins, uvAtMaxs);
}

AABB2 SpriteSheet::GetSpriteUVs(IntVec2 coords) const
{
	return GetSpriteUVs(GetSpriteIndex(coords));
}

const SpriteDefinition& SpriteSheet::GetSpriteDefinition(int spriteIndex) const
{
	return m_spriteDefs[spriteIndex];
}

SpriteDefinition::SpriteDefinition(SpriteSheet const& spriteSheet, int spriteIndex, Vec2 const& uvAtMins, Vec2 const& uvAtMaxs)
	: m_spriteSheet(spriteSheet), m_spriteIndex(spriteIndex), m_uvAtMins(uvAtMins), m_uvAtMaxs(uvAtMaxs)
{
}
