#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/StringUtils.hpp"

BitmapFont::BitmapFont(std::string const& fontFilePathNameWithNoExtension, Texture& fontTexture)
	: m_fontFilePathNameWithNoExtension(fontFilePathNameWithNoExtension), m_fontGlyphsSpriteSheet(fontTexture, IntVec2(16, 16))
{

}

Texture& BitmapFont::GetTexture()
{
	return m_fontGlyphsSpriteSheet.GetTexture();
}

void BitmapFont::AddVertsForText2D(std::vector<Vertex_PCU>& vertexArray, Vec2 const& textMins, float cellHeight, std::string const& text, Rgba8 const& tint /*= Rgba8::WHITE*/, float cellAspect /*= 1.f*/)
{
	AABB2 letter = AABB2::ZERO_TO_ONE;
	letter.m_maxs.y = cellHeight;
	letter.m_maxs.x *= cellAspect * cellHeight;
	letter.Translate(textMins);

	for (int index = 0; index < text.size(); index++)
	{
		AddVertsForAABB2D(vertexArray, letter, tint, m_fontGlyphsSpriteSheet.GetSpriteUVs(text.at(index)).m_mins, m_fontGlyphsSpriteSheet.GetSpriteUVs(text.at(index)).m_maxs);
		letter.m_mins.x += cellAspect * cellHeight; // move to next print position based on aspect ration
		letter.m_maxs.x += cellAspect * cellHeight; // move to next print position based on aspect ration
	}
}

float BitmapFont::GetTextWidth(float cellHeight, std::string const& text, float cellAspect /*= 1.f*/)
{
	return static_cast<float>(text.size()) * cellHeight * cellAspect;
}

float BitmapFont::AddVertsForTextInBox2D(std::vector<Vertex_PCU>& vertexArray, AABB2 const& box, float cellHeight, std::string const& text, Rgba8 const& tint, float cellAspect, Vec2 const& alignment, TextBoxMode mode, int maxGlyphsToDraw)
{
	float ratio = 1.0f;
	float longestLength = 0.0f;
	Strings strings = SplitStringOnDelimiter(text, '\n'); // split into lines of text based on newline character

	// find longest string length
	for (int index = 0; index < strings.size(); index++)
	{
		if (GetTextWidth(cellHeight, strings[index], cellAspect) > longestLength)
		{
			longestLength = GetTextWidth(cellHeight, strings[index], cellAspect);
		}
	}

	if (mode == TextBoxMode::SHRINK)
	{
		float verticalRatio = 1.0f;
		if (strings.size() * cellHeight > box.m_maxs.y - box.m_mins.y)
		{
			verticalRatio = (box.m_maxs.y - box.m_mins.y) / (strings.size() * cellHeight); // how much we need to scale down to fit in box vertically
		}
		if ((box.m_maxs.x - box.m_mins.x) / longestLength < verticalRatio)
		{
			ratio = (box.m_maxs.x - box.m_mins.x) / longestLength;
		}
		else
		{
			ratio = verticalRatio;
		}
	}

	cellHeight *= ratio; // set ratio needed to fit text according to the chosen mode
	longestLength *= ratio; // set length to appropriate ratio, too

	float xOffset = ((box.m_maxs.x - box.m_mins.x) - longestLength) * alignment.x;
	float yOffset = ((box.m_maxs.y - box.m_mins.y) - (strings.size() * cellHeight)) * alignment.y;

	int glyphsRemaining = maxGlyphsToDraw;
	for (int index = 0; index < strings.size(); index++)
	{
		AddVertsForText2D(vertexArray, Vec2(box.m_mins.x + xOffset, box.m_maxs.y - yOffset - (index + 1) * cellHeight), cellHeight, strings[index].substr(0, glyphsRemaining), tint, cellAspect);
		glyphsRemaining -= static_cast<int>(strings[index].length());
		if (glyphsRemaining < 0)
		{
			glyphsRemaining = 0;
		}
	}
	return cellHeight * cellAspect; // the width of the characters just processed by this function call
}

float BitmapFont::GetGlyphAspect(int glyphUnicode) const
{
	UNUSED(glyphUnicode);
	return 1.0f;
}
