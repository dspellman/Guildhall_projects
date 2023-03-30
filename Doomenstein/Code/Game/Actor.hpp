#pragma once
#include "Game/Weapon.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Game/ActorDefinition.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Game/ActorUID.hpp"
#include "Engine/Core/Clock.hpp"

extern const char* g_factionNames[];
class Map;
class SpawnInfo;
class Controller;
class AI;
class ActorDefinition;

enum animationState
{
//	ANIMATION_IDLE = -1,
	ANIMATION_WALK,
	ANIMATION_ATTACK,
	ANIMATION_PAIN,
	ANIMATION_DEATH,
	ANIMATION_COUNT
};

class Actor
{
public:
	Actor( Map* map, const SpawnInfo& spawnInfo );
	~Actor();

	void Update( float deltaSeconds );
	void UpdatePhysics( float deltaSeconds );
	void Render() const;
	IntVec2 const GetTileCoords() const;
	Mat44 GetModelMatrix() const;
	Vec3 GetForward() const;
	Vec3 GetLeft() const;
	Vec3 BuildNoneBasis(Vec3& i, Vec3& j, Vec3& k) const;
	Vec3 BuildFacingBasis(Vec3& i, Vec3& j, Vec3& k) const;
	Vec3 BuildAlignedBasis(Vec3& i, Vec3& j, Vec3& k) const;
	bool Damage(float damage);
	void Die();

	float GrenadeDamage(float distance, float range, float maxDamage);
	void AddForce(const Vec3& force);
	void AddImpulse( const Vec3& impulse );
	void OnCollide( Actor* other );

	void OnPossessed( Controller* controller );
	void OnUnpossessed( Controller* controller );
	void MoveInDirection( Vec3 direction, float speed );
	
	Weapon* GetEquippedWeapon();
	Weapon* GetAllternateWeapon();
	void EquipWeapon(int weaponIndex);
	void EquipNextWeapon();
	void EquipPreviousWeapon();
	void Attack();

	void SetAnimation(std::string const& state);

public:
	ActorUID m_uid = ActorUID::INVALID;
	ActorDefinition const* m_definition = nullptr;
	Map *m_map = nullptr;

	Vec3 m_position = Vec3::ZERO;
	EulerAngles m_orientation = EulerAngles::ZERO;
	Vec3 m_velocity = Vec3::ZERO;
	EulerAngles m_angularVelocity = EulerAngles::ZERO;
	Vec3 m_acceleration = Vec3::ZERO;

	std::vector<Weapon*> m_weapons;
	int m_equippedWeaponIndex = -1;
	mutable int m_alternateWeapon = -1;
	double m_weaponStart = 0.0;
	int m_bulletCount = 25;
	int m_plasmaCount = 50;
	int m_shellCount = 5;
	int m_grenadeCount = 0;
	float m_lifeTime = 0.0f;

	Actor* m_owner = nullptr;
	bool m_isDestroyed = false;
	bool m_isDead = false;

	float m_health = 200.0f;
	Stopwatch m_lifetimeStopwatch;

	Controller* m_controller = nullptr;
	AI* m_aiController = nullptr;

	bool m_renderForward = false;
	Rgba8 m_solidColor = Rgba8( 192, 0, 0 );
	Rgba8 m_wireframeColor = Rgba8( 255, 192, 192 );
	mutable int m_groupIndex = 0;
	Clock* m_actorClock;
	double m_animationStart = 0.0;
	std::string m_animation = "Walk";
	mutable float m_animationDuration = 2.0f;
};
