#include "WeaponDefinition.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "ActorDefinition.hpp"
#include "Engine/Renderer/SpriteAnimationDefinition.hpp"

std::vector<WeaponDefinition*> WeaponDefinition::s_definitions;

WeaponDefinition::WeaponDefinition()
{

}

bool WeaponDefinition::LoadFromXmlElement(const XmlElement& element)
{
	m_name = ParseXmlAttribute(element, "name", "UNKNOWN");

	m_refireTime = ParseXmlAttribute(element, "refireTime", 1.0f);
	m_numRays = ParseXmlAttribute(element, "numRays", 0);
	m_rayCone = ParseXmlAttribute(element, "rayCone", 0.0f);
	m_rayRange = ParseXmlAttribute(element, "rayRange", 0.0f);
	std::string damageRange = ParseXmlAttribute(element, "rayDamage", "0.0~0.0");
	Strings rangeValues = SplitStringOnDelimiter(damageRange, '~');
	m_rayDamage = FloatRange(std::stof(rangeValues[0]), std::stof(rangeValues[1]));
	m_rayImpulse = ParseXmlAttribute(element, "rayImpulse", 0.0f);

	m_numProjectiles = ParseXmlAttribute(element, "numProjectiles", 0);
	std::string projectileActor = ParseXmlAttribute(element, "projectileActor", "NONE");
	// create actor if needed
	if (projectileActor != "NONE")
	{
		m_projectileActorDefinition = ActorDefinition::GetByName(projectileActor);
	}
	m_projectileCone = ParseXmlAttribute(element, "projectileCone", 0.0f);
	m_projectileSpeed = ParseXmlAttribute(element, "projectileSpeed", 1.0f);

	// create weapon animations
	XmlElement const* SubElement = element.FirstChildElement();
	// HUD tag
	std::string shader = ParseXmlAttribute(*SubElement, "shader", "NONE");
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
	std::string hudTexture = ParseXmlAttribute(*SubElement, "baseTexture", "NONE");
	if (hudTexture == "NONE")
	{
		ERROR_AND_DIE("no hud texture");
	}
	else
	{
		m_hudBaseTexture = g_theRenderer->CreateOrGetTextureFromFile(hudTexture.c_str());
	}
	std::string reticleTexture = ParseXmlAttribute(*SubElement, "reticleTexture", "NONE");
	if (reticleTexture == "NONE")
	{
		ERROR_AND_DIE("no reticle texture");
	}
	else
	{
		m_reticleTexture = g_theRenderer->CreateOrGetTextureFromFile(reticleTexture.c_str());
	}
	m_reticleSize = ParseXmlAttribute(*SubElement, "reticleSize", Vec2(16.f, 16.f));
	m_spriteSize = ParseXmlAttribute(*SubElement, "spriteSize", Vec2(256.f, 256.f));
	m_spritePivot = ParseXmlAttribute(*SubElement, "spritePivot", Vec2(0.5f, 0.0f));

	XmlElement const* AnimElement = SubElement->FirstChildElement();
	// idle animation
	std::string name = ParseXmlAttribute(*AnimElement, "name", "NONE");
	if (shader == "NONE")
	{
		ERROR_AND_DIE("no name for animation");
	}
	std::string shaderName = ParseXmlAttribute(*AnimElement, "shader", "NONE");
	if (shaderName == "NONE")
	{
		ERROR_AND_DIE("no shader");
	}
	ShaderConfig shaderConfig;
	shaderConfig.m_name = shaderName;
	Shader* pShader = new Shader(shaderConfig);
	std::string spriteSheet = ParseXmlAttribute(*AnimElement, "spriteSheet", "NONE");
	if (spriteSheet == "NONE")
	{
		ERROR_AND_DIE("no sprite sheet");
	}
	IntVec2 cellCount = ParseXmlAttribute(*AnimElement, "cellCount", IntVec2(1, 1));
	Texture* texture = g_theRenderer->CreateOrGetTextureFromFile(spriteSheet.c_str());
	SpriteSheet const* pSpriteSheet = new SpriteSheet(*texture, cellCount);
	float duration = ParseXmlAttribute(*AnimElement, "secondsPerFrame", 1.0f);
	int startFrame = ParseXmlAttribute(*AnimElement, "startFrame", 0);
	int endFrame = ParseXmlAttribute(*AnimElement, "endFrame", 0);
	duration *= (float)(endFrame - startFrame + 1);
	m_idleAnimationDefinition = new SpriteAnimationDefinition(pShader, *pSpriteSheet, startFrame, endFrame, duration, SpriteAnimPlaybackType::LOOP);
	AnimElement = AnimElement->NextSiblingElement();

	// attack animation
	name = ParseXmlAttribute(*AnimElement, "name", "NONE");
	if (shader == "NONE")
	{
		ERROR_AND_DIE("no name for animation");
	}
	shaderName = ParseXmlAttribute(*AnimElement, "shader", "NONE");
	if (shaderName == "NONE")
	{
		ERROR_AND_DIE("no shader");
	}
	shaderConfig;
	shaderConfig.m_name = shaderName;
	pShader = new Shader(shaderConfig);
	spriteSheet = ParseXmlAttribute(*AnimElement, "spriteSheet", "NONE");
	if (spriteSheet == "NONE")
	{
		ERROR_AND_DIE("no sprite sheet");
	}
	cellCount = ParseXmlAttribute(*AnimElement, "cellCount", IntVec2(1, 1));
	texture = g_theRenderer->CreateOrGetTextureFromFile(spriteSheet.c_str());
	pSpriteSheet = new SpriteSheet(*texture, cellCount);
	duration = ParseXmlAttribute(*AnimElement, "secondsPerFrame", 1.0f);
	startFrame = ParseXmlAttribute(*AnimElement, "startFrame", 0);
	endFrame = ParseXmlAttribute(*AnimElement, "endFrame", 0);
	duration *= (float)(endFrame - startFrame + 1);
	m_attackAnimationDefinition = new SpriteAnimationDefinition(pShader, *pSpriteSheet, startFrame, endFrame, duration, SpriteAnimPlaybackType::LOOP);

	SubElement = SubElement->NextSiblingElement();
	SubElement = SubElement->FirstChildElement();
	std::string fireSoundName = ParseXmlAttribute(*SubElement, "Fire", "NONE");
	m_fireSoundName = ParseXmlAttribute(*SubElement, "name", "NONE");
	return true;
}

void WeaponDefinition::InitializeDefinitions(const char* path)
{
	// parse configuration file
	tinyxml2::XMLDocument doc;
	doc.LoadFile(path);
	XmlElement* root = doc.RootElement();
	if (!root)
	{
		ERROR_AND_DIE("WeaponDefinitions.xml problem reading file!");
	}

	XmlElement* element = root->FirstChildElement();
	while (element)
	{
		WeaponDefinition* pDefinition = new WeaponDefinition();
		pDefinition->LoadFromXmlElement(*element);
		push<WeaponDefinition>(pDefinition, s_definitions);
		element = element->NextSiblingElement();
	}
}

void WeaponDefinition::ClearDefinitions()
{
	destroy<WeaponDefinition>(s_definitions);
}

const WeaponDefinition* WeaponDefinition::GetByName(const std::string& name)
{
	for (int index = 0; index < static_cast<int>(s_definitions.size()); index++)
	{
		if (s_definitions[index]->m_name == name)
		{
			return s_definitions[index];
		}
	}
	return nullptr;
}

