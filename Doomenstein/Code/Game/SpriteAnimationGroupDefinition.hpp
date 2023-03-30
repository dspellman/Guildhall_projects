#pragma once
#include "Engine/Renderer/SpriteAnimationDefinition.hpp"
#include <string>
#include <vector>

//------------------------------------------------------------------------------------------------
class Camera;
class Shader;
class SpriteSheet;
class Texture;
struct Vec3;

//------------------------------------------------------------------------------------------------
class SpriteAnimationGroupDefinition
{
public:
	bool LoadFromXmlElement( const XmlElement& element );

	const SpriteAnimationDefinition& GetAnimationForDirection( const Vec3& direction ) const;
	float GetDuration() const;

	std::string m_name;

	Shader* m_shader;
	const SpriteSheet* m_spriteSheet;
	float m_fps = 1.0f;
	float m_duration = 1.0f;
	SpriteAnimPlaybackType m_playbackMode = SpriteAnimPlaybackType::LOOP;
	bool m_scaleBySpeed = false;

	std::vector<Vec3> m_directions;
	std::vector<SpriteAnimationDefinition> m_spriteAnimationDefinitions;
};
