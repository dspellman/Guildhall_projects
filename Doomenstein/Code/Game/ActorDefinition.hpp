#pragma once
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Game/Actor.hpp"
#include "Game/SpriteAnimationGroupDefinition.hpp"
#include <string>

//------------------------------------------------------------------------------------------------
class Camera;
class Texture;
class WeaponDefinition;

enum Faction
{
	NEUTRAL,
	MARINE,
	DEMON,
	FACTION_COUNT
};

enum 	BillboardType
{
	NONE,
//	AXIAL,
	ALIGNED,
	FACING
};

//------------------------------------------------------------------------------------------------
class ActorDefinition
{
public:
	bool LoadFromXmlElement( const XmlElement& element );

	std::string m_name;
	std::vector<const WeaponDefinition*> m_weaponDefinitions;

	// Physics
	float m_physicsRadius = -1.0f;
	float m_physicsHeight = -1.0f;
	float m_walkSpeed = 0.0f;
	float m_runSpeed = 0.0f;
	float m_drag = 0.0f;
	float m_turnSpeed = 0.0f;
	bool m_flying = false;
	bool m_simulated = false;
	bool m_collidesWithWorld = false;
	bool m_collidesWithActors = false;

	// Possession
	bool m_canBePossessed = true;
	float m_eyeHeight = 0.75f;
	float m_cameraFOVDegrees = 60.0f;

	// AI
	bool m_aiEnabled = false;
	float m_sightRadius = 64.0f;
	float m_sightAngle = 180.0f;
	FloatRange m_meleeDamage = FloatRange( 1.0f, 2.0f );
	float m_meleeDelay = 1.0f;
	float m_meleeRange = 0.5f;

	// Combat
	float m_health = 1.0f;
	Faction m_faction = Faction::NEUTRAL;
	float m_corpseLifetime = 0.0f;
	bool m_dieOnSpawn = false;
	bool m_dieOnCollide = false;
	FloatRange m_damageOnCollide = FloatRange( 0.0f, 0.0f );
	float m_impulseOnCollide = 0.0f;

	// Visuals
	bool m_visible = true;
	Vec2 m_spriteSize = Vec2(1.0f, 1.0f);
	Vec2 m_spritePivot = Vec2(0.5f, 0.0f);
	BillboardType m_billboardType = BillboardType::NONE;
	bool m_renderDepth = true;
	bool m_renderLit = true;
	bool m_renderRounded = false;
	std::vector<SpriteAnimationGroupDefinition> m_spriteAnimationGroupDefinitions;

	// Sounds
	std::string m_attackSoundName;
	std::string m_hurtSoundName;
	std::string m_deathSoundName;

	static void InitializeDefinitions( const char* path );
	static void ClearDefinitions();
	static const ActorDefinition* GetByName( const std::string& name );
	static std::vector<ActorDefinition*> s_definitions;
	int GetGroupIndexByName(const std::string& name) const;
	XmlElement const* ParseAppearance(XmlElement const* SubElement);
};

