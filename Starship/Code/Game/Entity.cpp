#include "Game/Entity.hpp"
#include "Game/Game.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/Time.hpp"
#include "Game/Wormhole.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Audio/AudioSystem.hpp"

extern AudioSystem* g_theAudio;

Entity::Entity(Game *game, Vec2 startPos)
{
	m_game = game;
	m_position = startPos;
	m_savedTime = GetCurrentTimeSeconds() - WORMHOLE_COOLDOWN;
}

void Entity::Update(float deltaSeconds)
{
	m_position += deltaSeconds * m_velocity;
}

void Entity::Render() const
{
	// Entity has no vertices stored to render
}

void Entity::DebugRender() const
{
	DebugDrawRing(m_position, m_cosmeticRadius, Rgba8(255, 0, 255, 255), 0.25f);
	DebugDrawRing(m_position, m_physicalRadius, Rgba8(0, 255, 255, 255), 0.25f);
	DebugDrawLine(m_position, m_position + GetForwardNormal() * m_cosmeticRadius, Rgba8(255, 0, 0, 255), 0.25f);
	DebugDrawLine(m_position, m_position + GetForwardNormal().GetRotated90Degrees() * m_cosmeticRadius, Rgba8(0, 255, 0, 255), 0.25f);
	DebugDrawLine(m_position, m_position + m_velocity, Rgba8(255, 255, 0, 255), 0.25f);
}

void Entity::Die()
{
	m_isDead = true;
	m_isGarbage = true;
}

bool Entity::IsAlive()
{
	return !m_isDead;
}

bool Entity::IsOffscreen()
{
	if (m_position.x > WORLD_SIZE_X + m_cosmeticRadius)
	{
		return true;
	}

	if (m_position.x < -m_cosmeticRadius)
	{
		return true;
	}

	if (m_position.y > WORLD_SIZE_Y + m_cosmeticRadius)
	{
		return true;
	}

	if (m_position.y < -m_cosmeticRadius)
	{
		return true;
	}

	return false;
}

void Entity::SpawnOffscreen()
{
	Vec2 direction(0.0f, 0.0f);
	// choose side to spawn on randomly (top, bottom, left or right)
	// Use all four sides so that any entity can call this function
	switch (random.RollRandomIntLessThan(4))
	{
	case 0:
		m_position.x = -m_cosmeticRadius;
		m_position.y = random.RollRandomFloatZeroToOne() * WORLD_SIZE_Y;
		// random orientation that does not end up off screen
		direction = Vec2(random.RollRandomIntInRange(0, 1) ? 0.5f : -0.5f, random.RollRandomFloatZeroToOne());
		direction.Normalize();
		m_orientationDegrees = direction.GetOrientationDegrees();
		break;
	case 1:
		m_position.x = WORLD_SIZE_X + m_cosmeticRadius;
		m_position.y = random.RollRandomFloatZeroToOne() * WORLD_SIZE_Y;
		// random orientation that does not end up off screen
		direction = Vec2(random.RollRandomIntInRange(0, 1) ? -0.5f : 0.5f, random.RollRandomFloatZeroToOne());
		direction.Normalize();
		m_orientationDegrees = direction.GetOrientationDegrees();
		break;
	case 2:
		m_position.y = -m_cosmeticRadius;
		m_position.x = random.RollRandomFloatZeroToOne() * WORLD_SIZE_X;
		// random orientation that does not end up off screen
		direction = Vec2(random.RollRandomFloatZeroToOne(), random.RollRandomIntInRange(0, 1) ? 0.5f : -0.5f);
		direction.Normalize();
		m_orientationDegrees = direction.GetOrientationDegrees();
		break;
	case 3:
		m_position.y = WORLD_SIZE_Y + m_cosmeticRadius;
		m_position.x = random.RollRandomFloatZeroToOne() * WORLD_SIZE_X;
		// random orientation that does not end up off screen
		direction = Vec2(random.RollRandomFloatZeroToOne(), random.RollRandomIntInRange(0, 1) ? -0.5f : 0.5f);
		direction.Normalize();
		m_orientationDegrees = direction.GetOrientationDegrees();
		break;
	}
}

Vec2 const Entity::GetForwardNormal() const
{
	return Vec2::MakeFromPolarDegrees(m_orientationDegrees);
}

void Entity::BounceOffWalls()
{
	if (m_position.x > WORLD_SIZE_X - m_physicalRadius)
	{
		m_position.x = WORLD_SIZE_X - m_physicalRadius; // rubber band back in bounds
		m_velocity.x *= -1.0f; // reflect speed for bounce
	}

	if (m_position.x < m_physicalRadius)
	{
		m_position.x = m_physicalRadius; // rubber band back in bounds
		m_velocity.x *= -1.0f; // reflect speed for bounce
	}

	if (m_position.y > WORLD_SIZE_Y - m_physicalRadius)
	{
		m_position.y = WORLD_SIZE_Y - m_physicalRadius; // rubber band back in bounds
		m_velocity.y *= -1.0f; // reflect speed for bounce
	}

	if (m_position.y < m_physicalRadius)
	{
		m_position.y = m_physicalRadius; // rubber band back in bounds
		m_velocity.y *= -1.0f; // reflect speed for bounce
	}
}

Vec2 Entity::SpawnOnBorder()
{
	Vec2 direction(0.0f, 0.0f);

	if (random.RollRandomIntInRange(0, 1))
	{
		direction.x = random.RollRandomFloatInRange(0.7f, 0.9f) * WORLD_SIZE_X;
	}
	else
	{
		direction.x = random.RollRandomFloatInRange(0.1f, 0.3f) * WORLD_SIZE_X;
	}

	if (random.RollRandomIntInRange(0, 1))
	{
		direction.y = random.RollRandomFloatInRange(0.7f, 0.9f) * WORLD_SIZE_Y;
	}
	else
	{
		direction.y = random.RollRandomFloatInRange(0.1f, 0.3f) * WORLD_SIZE_Y;
	}

	return direction;
}

void Entity::SetupTeleportation(Wormhole*& wormhole)
{
	SoundID testSound = g_theAudio->CreateOrGetSound("Data/Audio/Scifi_Heal_Cloak06.wav");
	g_theAudio->StartSound(testSound);
	m_wormhole = wormhole; // save the hole we are falling into
	m_savedTime = GetCurrentTimeSeconds(); // start timer for safe period
	m_shrinking = true; // first phase is shrink into wormhole
	m_savedVelocity = m_velocity; // save the current velocity to restore later
	m_wormholeVelocity = wormhole->m_velocity; // save for bounce checks
	m_velocity = m_wormholeVelocity + Vec2(wormhole->m_position - m_position);
}

void Entity::TeleportThroughWormhole(float deltaSeconds)
{
	if (m_shrinking)
	{
		m_scalingFactor -= SHRINK_FACTOR * deltaSeconds;
		if (m_scalingFactor < 0.0f)
		{
			m_shrinking = false;
			m_velocity = m_savedVelocity.GetNormalized(); // restore velocity
			Wormhole* originWormhole = m_wormhole;
			do
			{
				m_wormhole = m_game->m_wormhole[random.RollRandomIntInRange(0, MAX_WORMHOLES - 1)];
			} 
			while (m_wormhole == originWormhole); // pick a different wormhole
			m_position = m_wormhole->m_position; // translate to new wormhole
		}
		m_scalingFactor = ClampZeroToOne(m_scalingFactor);
	}
	else
	{
		if (m_scalingFactor < 1.0f)
		{
			m_scalingFactor += SHRINK_FACTOR * deltaSeconds; // one second for scaling
			m_scalingFactor = ClampZeroToOne(m_scalingFactor);
			if (m_scalingFactor == 1.0f)
			{
				m_velocity = m_savedVelocity;
			}
		}
	}
}

