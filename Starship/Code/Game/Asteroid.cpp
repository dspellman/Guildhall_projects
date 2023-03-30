#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Entity.hpp"
#include "Game/Asteroid.hpp"
#include "Game/Game.hpp"
#include "Game/App.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Game/Wormhole.hpp"

constexpr float arcDegrees = 360.0f / ASTEROID_VERTS;
Rgba8 asteroidColor(100, 100, 100, 255);
Vertex_PCU centerVertex(Vec3(0.0f, 0.0f, 0.0f), asteroidColor, Vec2(0.f, 0.f));

Asteroid::Asteroid(Game* game, Vec2 startPos) : Entity(game, startPos)
{
	m_health = ASTEROID_HEALTH;

	m_physicalRadius = ASTEROID_PHYSICS_RADIUS;
	m_cosmeticRadius = ASTEROID_COSMETIC_RADIUS;

	SpawnOffscreen();
	m_velocity.x = ASTEROID_SPEED * CosDegrees(m_orientationDegrees);
	m_velocity.y = ASTEROID_SPEED * SinDegrees(m_orientationDegrees);

	m_angularVelocity = ASTEROID_ANGULAR_VELOCITY_DEGREES * random.RollRandomFloatInRange(-1.0f, 1.0f);

	for (int vert = 0; vert < ASTEROID_VERTS; vert++)
	{
		float theta = arcDegrees * vert;
		float edgeX = CosDegrees(theta) * random.RollRandomFloatInRange(ASTEROID_PHYSICS_RADIUS, ASTEROID_COSMETIC_RADIUS);
		float edgeY = SinDegrees(theta) * random.RollRandomFloatInRange(ASTEROID_PHYSICS_RADIUS, ASTEROID_COSMETIC_RADIUS);	
		vertex[vert] = Vertex_PCU(Vec3(edgeX, edgeY, 0.0f), asteroidColor, Vec2(0.f, 0.f));
	}
}

void Asteroid::Update(float deltaSeconds)
{
	if (m_shrinking && m_wormhole && m_wormhole->m_velocity != m_wormholeVelocity)
	{
		m_velocity -= m_wormholeVelocity; // remove old velocity component and add new velocity component
		m_velocity += m_wormhole->m_velocity;
		m_wormholeVelocity = m_wormhole->m_velocity;
	}
	TeleportThroughWormhole(deltaSeconds);

	m_position += deltaSeconds * m_velocity;
	m_orientationDegrees += deltaSeconds * m_angularVelocity;
	DisappearIntoTheVoid(); // check whether asteroid is off screen
}

void Asteroid::Render() const
{
	Vertex_PCU tvertex[3 * ASTEROID_VERTS];
	for (int vertIndex = 0; vertIndex < ASTEROID_VERTS; vertIndex++)
	{
		tvertex[3 * vertIndex] = centerVertex;
		tvertex[3 * vertIndex + 1] = vertex[vertIndex];
		tvertex[3 * vertIndex + 2] = vertex[(vertIndex + 1) % ASTEROID_VERTS];
	}
	TransformVertexArrayXY3D(3 * ASTEROID_VERTS, tvertex, m_scalingFactor, m_orientationDegrees, m_position);
	g_theRenderer->DrawVertexArray(3 * ASTEROID_VERTS, tvertex);
}


void Asteroid::DisappearIntoTheVoid()
{
	if (IsOffscreen())
	{
		//Die();
		// Wrap around screen by trans locating to opposite side
		// monitor for two crossings without being visible for bad asteroids
		if (m_position.x > WORLD_SIZE_X)
		{
			m_position.x -= (WORLD_SIZE_X + 2 * m_cosmeticRadius);
		}
		if (m_position.x < 0.0f)
		{
			m_position.x += (WORLD_SIZE_X + 2 * m_cosmeticRadius);
		}

		if (m_position.y > WORLD_SIZE_Y)
		{
			m_position.y -= (WORLD_SIZE_Y + 2 * m_cosmeticRadius);
		}
		if (m_position.y < 0.0f)
		{
			m_position.y += (WORLD_SIZE_Y + 2 * m_cosmeticRadius);
		}
	}
}