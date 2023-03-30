#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"

class Shader;

enum class SpriteAnimPlaybackType
{
	ONCE,		// for 5-frame anim, plays 0,1,2,3,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4...
	LOOP,		// for 5-frame anim, plays 0,1,2,3,4,0,1,2,3,4,0,1,2,3,4,0,1,2,3,4,0,1,2,3,4,0...
	PINGPONG,	// for 5-frame anim, plays 0,1,2,3,4,3,2,1,0,1,2,3,4,3,2,1,0,1,2,3,4,3,2,1,0,1...
};

//------------------------------------------------------------------------------------------------
class SpriteAnimationDefinition
{
public:
	SpriteAnimationDefinition(Shader const* shader, const SpriteSheet& sheet, int startSpriteIndex, int endSpriteIndex, SpriteAnimPlaybackType playbackType = SpriteAnimPlaybackType::LOOP);
	SpriteAnimationDefinition(Shader const* shader, const SpriteSheet& sheet, int startSpriteIndex, int endSpriteIndex, float durationSeconds, SpriteAnimPlaybackType playbackType = SpriteAnimPlaybackType::LOOP);

	bool LoadFromXmlElement(const XmlElement& element);

	const SpriteDefinition& GetSpriteDefAtTime(float seconds) const;
	const float GetDuration() const { return m_durationSeconds; };
	const int GetTotalFramesInCycle() const { return m_endSpriteIndex - m_startSpriteIndex + 1; };

	void SetTotalDuration(float totalSeconds) { m_durationSeconds = totalSeconds; };
	void SetSecondsPerFrame(float secondsPerFrame) { m_durationSeconds = static_cast<float>(GetTotalFramesInCycle()) * secondsPerFrame; };
	void SetFramesPerSecond(float fps) { m_durationSeconds = static_cast<float>(GetTotalFramesInCycle()) / fps; };

	Shader const* m_shader;
	const SpriteSheet& m_spriteSheet;
	int		m_startSpriteIndex = -1;
	int		m_endSpriteIndex = -1;
	float	m_durationSeconds = 1.0f;
	mutable int m_currentFrame = 0;
	SpriteAnimPlaybackType	m_playbackType = SpriteAnimPlaybackType::LOOP;
};
