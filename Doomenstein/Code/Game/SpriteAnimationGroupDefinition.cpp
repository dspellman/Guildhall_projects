#include "Game/SpriteAnimationGroupDefinition.hpp"
#include "Engine/Renderer/shader.hpp"
#include "GameCommon.hpp"

bool SpriteAnimationGroupDefinition::LoadFromXmlElement(const XmlElement& element)
{
	//AnimationGroup tag
	m_name = ParseXmlAttribute(element, "name", "NONE");
	std::string shader = ParseXmlAttribute(element, "shader", "NONE");
	if (shader == "NONE")
	{
		ERROR_AND_DIE("no shader");
	}
	else
	{
		ShaderConfig shaderConfig;
		shaderConfig.m_name = shader;
		m_shader = new Shader(shaderConfig);
	}
	std::string spriteSheet = ParseXmlAttribute(element, "spriteSheet", "NONE");
	IntVec2 cellCount = ParseXmlAttribute(element, "cellCount", IntVec2(1,1));
	if (spriteSheet == "NONE")
	{
		ERROR_AND_DIE("no sprite sheet");
	}
	Texture* texture = g_theRenderer->CreateOrGetTextureFromFile(spriteSheet.c_str());
// 	ShaderConfig shaderConfig;
// 	shaderConfig.m_name = shader;
	m_spriteSheet = new SpriteSheet(*texture, cellCount);
	m_duration = ParseXmlAttribute(element, "secondsPerFrame", 1.0f);
	m_fps = 1.0f / m_duration; // variable seems to be the inverse of the stored parameter?
	std::string playbackMode = ParseXmlAttribute(element, "playbackMode", "Loop");
	if (playbackMode == "Loop")
	{
		m_playbackMode = SpriteAnimPlaybackType::LOOP;
	}
	else if (playbackMode == "Once")
	{
		m_playbackMode = SpriteAnimPlaybackType::ONCE;
	}
	else // if (playbackMode == "PingPong")
	{
		m_playbackMode = SpriteAnimPlaybackType::PINGPONG;
	}
	m_scaleBySpeed = ParseXmlAttribute(element, "scaleBySpeed", false);

	XmlElement const* dirElement = element.FirstChildElement();
	while (dirElement != nullptr)
	{
		// Direction tag
		Vec3 vector = ParseXmlAttribute(*dirElement, "vector", Vec3::ZERO);
		vector.Normalize();
		m_directions.push_back(vector);
		XmlElement const* animElement = dirElement->FirstChildElement();
		int startFrame = ParseXmlAttribute(*animElement, "startFrame", 0);
		int endFrame = ParseXmlAttribute(*animElement, "endFrame", 0);
		float duration = static_cast<float>(endFrame - startFrame + 1) * m_duration;
		SpriteAnimationDefinition animationDefinition(m_shader, *m_spriteSheet, startFrame, endFrame, duration, m_playbackMode);
		m_spriteAnimationDefinitions.push_back(animationDefinition);
		dirElement = dirElement->NextSiblingElement();
	}
	return true;
}

const SpriteAnimationDefinition& SpriteAnimationGroupDefinition::GetAnimationForDirection(const Vec3& direction) const
{
	int closest = -1;
	float dotClosest = -2.0f;
	for (int index = 0; index < static_cast<int>(m_directions.size()); index++)
	{
		float dotTest = DotProduct3D(direction.GetNormalized(), m_directions[index]);
		if (dotTest > dotClosest)
		{
			closest = index;
			dotClosest = dotTest;
		}
	}
	return m_spriteAnimationDefinitions[closest];
}

float SpriteAnimationGroupDefinition::GetDuration() const
{
	return m_duration;
}
