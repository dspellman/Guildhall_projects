#pragma once
#include "Engine/Math/AABB2.hpp"
#include <vector>

class Texture;
struct IntVec2;
struct Vec2;

class SpriteDefinition;

///////////////////////////////////
// main class for managing sprites
class SpriteSheet
{
public:
	explicit SpriteSheet( Texture& texture, IntVec2 const& simpleGridLayout );	

	Texture&						GetTexture() const							{ return m_texture; };
	int								GetNumSprites() const						{ return static_cast<int>(m_spriteDefs.size()); };
	SpriteDefinition const&			GetSpriteDef(int spriteIndex) const			{ return m_spriteDefs[spriteIndex]; };
	void							GetSpriteUVs(Vec2& out_uvAtMins, Vec2& out_uvAtMaxs, int spriteIndex) const; 
	AABB2							GetSpriteUVs(int spriteIndex) const;
	AABB2							GetSpriteUVs(IntVec2 coords) const;
	const SpriteDefinition&			GetSpriteDefinition( int spriteIndex ) const;
	const IntVec2&					GetSpriteSheetDimensions() const { return m_dimensions; };
	int								GetSpriteIndex(IntVec2 coords) const { return coords.x + m_dimensions.x * coords.y; }

protected:
	IntVec2 m_dimensions = IntVec2::ONE;
	Texture& m_texture; // reference members must be set in constructor's initializer list
	std::vector<SpriteDefinition> m_spriteDefs;
};

/////////////////////////////////
// Helper class for sprite sheet
class SpriteDefinition
{
public:
	explicit SpriteDefinition(SpriteSheet const& spriteSheet, int spriteIndex, Vec2 const& uvAtMins, Vec2 const& uvAtMaxs);
	void				GetUVs(Vec2& out_uvAtMins, Vec2& out_uvAtMaxs) const	{ out_uvAtMins = m_uvAtMins; out_uvAtMaxs = m_uvAtMaxs; };
	AABB2				GetUVs() const											{ return AABB2(m_uvAtMins, m_uvAtMaxs); };
	SpriteSheet const&	GetSpriteSheet() const									{ return m_spriteSheet; };
	Texture&			GetTexture() const										{ return m_spriteSheet.GetTexture(); };
	float				GetAspect() const										{ return (m_uvAtMaxs.y - m_uvAtMins.y) / (m_uvAtMaxs.x - m_uvAtMins.x); };

protected:
	SpriteSheet const&	m_spriteSheet;
	int					m_spriteIndex = -1;
	Vec2				m_uvAtMins = Vec2::ZERO;
	Vec2				m_uvAtMaxs = Vec2::ONE;
	AABB2				m_spriteBounds = AABB2(Vec2::ZERO, Vec2::ONE);
};
