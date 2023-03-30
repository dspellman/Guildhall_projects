#include "Game/Weapon.hpp"
#include "Game/WeaponDefinition.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Actor.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/RaycastUtils.hpp"
#include "Game/Map.hpp"
#include "Engine/Renderer/DebugRenderMode.hpp"
#include "Player.hpp"
#include "Game.hpp"

Weapon::Weapon(const WeaponDefinition* definition)
	: m_definition(definition)
{
	m_refireStopwatch.Start(m_definition->m_refireTime);
}

Weapon::~Weapon()
{

}

void Weapon::Fire(const Vec3& position, const Vec3& forward, Actor* owner)
{
	RaycastResult3D weaponHit;
	Actor* target = nullptr;
	Vec3 firingPosition = position;
	firingPosition.z += owner->m_definition->m_eyeHeight;
	if (owner->m_definition->m_eyeHeight > 1.0)
	{
		ERROR_AND_DIE("eye height problem");
	}
//	firingPosition += forward * (owner->m_definition->m_physicsRadius);

	if (m_refireStopwatch.HasDurationElapsed() == false)
	{
		return;
	}

	// set attack animation
	owner->Attack();
	m_idle = false;
	Actor* projectile = nullptr;

	for (int rays = 0; rays < m_definition->m_numRays; rays++)
	{
		weaponHit = owner->m_map->RaycastAll(firingPosition, forward, m_definition->m_rayRange, &target, RaycastFilter(owner));
//		weaponHit = owner->m_map->RaycastAll(firingPosition, GetRandomDirectionInCone(firingPosition, forward, m_definition->m_rayCone), m_definition->m_rayRange, &target, RaycastFilter(owner));
		if (target)
		{
			target->Damage(random.RollRandomFloatInRange(m_definition->m_rayDamage));
			if (target->m_health <= 0.0f)
				if(target->m_controller)
			{
				if (owner->m_controller)
				{
					Player* player = dynamic_cast<Player*>(owner->m_controller);
					player->m_kills++;
				}
			}
			target->AddImpulse(m_definition->m_rayImpulse * forward);
			target->SetAnimation("Pain");
			owner->SetAnimation("Attack");
			// set hit animation
			projectile = owner->m_map->SpawnProjectile("BloodSplatter");
			projectile->m_owner = owner;
			projectile->m_position = weaponHit.m_impactPos;
			projectile->m_orientation = weaponHit.m_impactNormal;
			SoundID weaponFire = g_theAudio->CreateOrGetSound(m_definition->m_fireSoundName);
			g_theAudio->StartSoundAt(weaponFire, owner->m_position);
		}
		else if (weaponHit.m_didImpact)
		{
			// set hit animation
			projectile = owner->m_map->SpawnProjectile("BulletHit");
			projectile->m_owner = owner;
			projectile->m_position = weaponHit.m_impactPos;
			projectile->m_orientation = weaponHit.m_impactNormal;
			owner->SetAnimation("Attack");
			SoundID weaponFire = g_theAudio->CreateOrGetSound(m_definition->m_fireSoundName);
			g_theAudio->StartSoundAt(weaponFire, owner->m_position);
		}
		else if (m_definition->m_name == "Kick")
		{
			SoundID weaponFire = g_theAudio->CreateOrGetSound(m_definition->m_fireSoundName);
			g_theAudio->StartSoundAt(weaponFire, owner->m_position);
		}
	}

	for (int projectiles = 0; projectiles < m_definition->m_numProjectiles; projectiles++)
	{
		owner->SetAnimation("Attack");
		// create an actor of projectile type and send it on its way
		projectile = owner->m_map->SpawnProjectile(m_definition->m_projectileActorDefinition->m_name);
		projectile->m_owner = owner;
		Vec3 randomFwd = GetRandomDirectionInCone(firingPosition, forward, m_definition->m_projectileCone);
		if (m_definition->m_projectileActorDefinition->m_name == "PlasmaGrenadeProjectile")
		{
			randomFwd.z += 0.2f;
			projectile->m_lifeTime = 2.0f;
			owner->m_grenadeCount--;
			if (owner->m_grenadeCount < 0)
			{
				ERROR_AND_DIE("negative grenade count!");
			}
		}
		projectile->m_velocity = m_definition->m_projectileSpeed * randomFwd;
		firingPosition.z -= owner->m_definition->m_eyeHeight * 0.5f;
		projectile->m_position = firingPosition;
		projectile->m_orientation = EulerAngles(randomFwd);
		SoundID weaponFire = g_theAudio->CreateOrGetSound(m_definition->m_fireSoundName);
		g_theAudio->StartSoundAt(weaponFire, owner->m_position);
// 		// test code
// 		projectile = owner->m_map->SpawnProjectile("ShotgunShells");
// 		projectile->m_position = firingPosition + -2.0f * owner->m_orientation.GetForwardNormal();
	}

	m_refireStopwatch.Restart();
}

Vec3 Weapon::GetRandomDirectionInCone(const Vec3& position, const Vec3& forward, float angle) const
{
	UNUSED(position);
	float theta = random.RollRandomFloatInRange(0.0f, angle) * (random.RollRandomFloatZeroToOne() < 0.5f ? -1.0f : 1.0f);
	float phi = sqrtf(angle * angle - theta * theta) * (random.RollRandomFloatZeroToOne() < 0.5f ? -1.0f : 1.0f);
	EulerAngles conan(forward);
	conan.m_yawDegrees += theta * 0.5f;
	conan.m_pitchDegrees += phi * 0.5f;
	return conan.GetForwardNormal();
}

