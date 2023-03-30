#include "Engine/Renderer/SpriteAnimationDefinition.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

SpriteAnimationDefinition::SpriteAnimationDefinition(Shader const* shader, const SpriteSheet& sheet, int startSpriteIndex, int endSpriteIndex, float durationSeconds, SpriteAnimPlaybackType playbackType)
	: m_shader(shader), m_spriteSheet(sheet), m_startSpriteIndex(startSpriteIndex), m_endSpriteIndex(endSpriteIndex), m_durationSeconds(durationSeconds), m_playbackType(playbackType)
{

}

SpriteAnimationDefinition::SpriteAnimationDefinition(Shader const* shader, const SpriteSheet& sheet, int startSpriteIndex, int endSpriteIndex, SpriteAnimPlaybackType playbackType /*= SpriteAnimPlaybackType::LOOP*/)
	: m_shader(shader), m_spriteSheet(sheet), m_startSpriteIndex(startSpriteIndex), m_endSpriteIndex(endSpriteIndex), m_playbackType(playbackType)
{

}

bool SpriteAnimationDefinition::LoadFromXmlElement(const XmlElement& element)
{
	UNUSED(element);
	return true;
}

const SpriteDefinition& SpriteAnimationDefinition::GetSpriteDefAtTime(float seconds) const
{
	float framesPerSecond = 1.0f;

	switch (m_playbackType)
	{
	case SpriteAnimPlaybackType::ONCE:
		framesPerSecond = static_cast<float>(m_endSpriteIndex - m_startSpriteIndex + 1) / m_durationSeconds;
		m_currentFrame = static_cast<int>(framesPerSecond * seconds);
		if (m_currentFrame > (m_endSpriteIndex - m_startSpriteIndex))
		{
			m_currentFrame = m_endSpriteIndex; // once will hold at last frame after playing sequence
		}
		else
		{
			m_currentFrame += m_startSpriteIndex;
		}
		return m_spriteSheet.GetSpriteDef(m_currentFrame);
		break;

	case SpriteAnimPlaybackType::LOOP:
		framesPerSecond = static_cast<float>(m_endSpriteIndex - m_startSpriteIndex + 1) / m_durationSeconds;
		m_currentFrame = static_cast<int>(framesPerSecond * seconds) % (m_endSpriteIndex - m_startSpriteIndex + 1); // looping will restart sequence
		return m_spriteSheet.GetSpriteDef(m_startSpriteIndex + m_currentFrame);
		break;

	case SpriteAnimPlaybackType::PINGPONG:
		if (m_endSpriteIndex == m_startSpriteIndex)
		{
			return m_spriteSheet.GetSpriteDef(m_startSpriteIndex); // only one sprite will repeat indefinitely
		}

		framesPerSecond = (2.0f * static_cast<float>(m_endSpriteIndex - m_startSpriteIndex)) / m_durationSeconds;
		m_currentFrame = static_cast<int>(framesPerSecond * seconds) % (2 * (m_endSpriteIndex - m_startSpriteIndex));
		if (m_currentFrame < (m_endSpriteIndex - m_startSpriteIndex + 1))
		{
			return m_spriteSheet.GetSpriteDef(m_startSpriteIndex + m_currentFrame); // return frame in sequence in first part of ping pong
		}
		else
		{
			m_currentFrame -= (m_endSpriteIndex - m_startSpriteIndex);
			return m_spriteSheet.GetSpriteDef(m_endSpriteIndex - m_currentFrame);
		}
		break;
	default:
		ERROR_AND_DIE("Bad SpriteAnimPlaybackType (which should be impossible)");
		break;
	}
}

