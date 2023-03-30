#pragma once
#include "Game/GameCommon.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Renderer/SpriteAnimationDefinition.hpp"
#include "Engine/Math/FloatRange.hpp"
#include <vector>
#include <string>

class ActorDefinition;
class SpriteAnimation;
class SpriteSheet;
class Shader;
class Texture;
namespace tinyxml2 { class XMLElement; }
typedef tinyxml2::XMLElement XmlElement;

class WeaponDefinition
{
public:
	WeaponDefinition();
	bool LoadFromXmlElement( const XmlElement& element );

	std::string m_name;
	float m_refireTime = 0.5f;

	int m_numRays = 0;
	float m_rayCone = 0.0f;
	float m_rayRange = 40.0f;
	FloatRange m_rayDamage = FloatRange( 0.0f, 1.0f );
	float m_rayImpulse = 4.0f;

	int m_numProjectiles = 0;
	const ActorDefinition* m_projectileActorDefinition = nullptr;
	float m_projectileCone = 1.0f;
	float m_projectileSpeed = 40.0f;

	SpriteAnimationDefinition const* m_idleAnimationDefinition = nullptr;
	SpriteAnimationDefinition const* m_attackAnimationDefinition = nullptr;

	Shader* m_shader = nullptr;
	Texture* m_hudBaseTexture = nullptr;
	SpriteSheet* m_spriteSheet = nullptr;
	Vec2 m_spriteSize = Vec2(1.0f, 1.0f);
	Vec2 m_spritePivot = Vec2(0.5f, 0.0f);
	Texture* m_reticleTexture = nullptr;
	Vec2 m_reticleSize = Vec2(1.0f, 1.0f);

	std::string m_fireSoundName;

	static void InitializeDefinitions( const char* path );
	static void ClearDefinitions();
	static const WeaponDefinition* GetByName( const std::string& name );
	static std::vector<WeaponDefinition*> s_definitions;
};

