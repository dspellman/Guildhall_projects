#pragma once
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Math/Vec2.hpp"
#include <vector>
#include "Engine/Renderer/SpriteSheet.hpp"
#include <string>
#include "../Core/Vertex_PCU.hpp"
#include "../Core/Rgba8.hpp"

struct Vec2;

enum TextBoxMode
{
	SHRINK,
	OVERRUN
};

class BitmapFont
{
	friend class Renderer; // Only the RenderContext can create new BitmapFont objects!

private:
	BitmapFont(std::string const& fontFilePathNameWithNoExtension, Texture& fontTexture);

public:
	Texture& GetTexture();

	void AddVertsForText2D(std::vector<Vertex_PCU>& vertexArray, Vec2 const& textMins,
		float cellHeight, std::string const& text, Rgba8 const& tint = Rgba8::WHITE, float cellAspect = 1.f);

	float GetTextWidth(float cellHeight, std::string const& text, float cellAspect = 1.f);

	float AddVertsForTextInBox2D(std::vector<Vertex_PCU>& vertexArray, AABB2 const& box, float cellHeight,
		std::string const& text, Rgba8 const& tint = Rgba8::WHITE, float cellAspect = 1.f,
		Vec2 const& alignment = Vec2(.5f, .5f), TextBoxMode mode = TextBoxMode::SHRINK, int maxGlyphsToDraw = 99999999);

protected:
	float GetGlyphAspect(int glyphUnicode) const; // For now this will always return 1.0f!!!

protected:
	std::string	m_fontFilePathNameWithNoExtension;
	SpriteSheet m_fontGlyphsSpriteSheet;
};
