#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Entity.hpp"
#include "Game/Asteroid.hpp"
#include "Game/Game.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Game/App.hpp"
#include "Game/Bullet.hpp"
#include "Engine/Core/Time.hpp"
#include "Game/Wormhole.hpp"

Rgba8 bulletColor(255, 127, 0, 255);

Bullet::Bullet(Game* game, Vec2 const& position, float forwardDegrees) : Entity(game, position)
{
  	m_orientationDegrees = forwardDegrees;
	m_physicalRadius = BULLET_PHYSICS_RADIUS;
	m_cosmeticRadius = BULLET_COSMETIC_RADIUS;
	m_velocity.x = BULLET_SPEED * CosDegrees(forwardDegrees);
	m_velocity.y = BULLET_SPEED * SinDegrees(forwardDegrees);
	m_lifeTime = GetCurrentTimeSeconds();

	// front triangle
	vertex[0] = Vertex_PCU(Vec3(0.0f, -0.5f, 0.0f), Rgba8 (255, 0, 0, 255), Vec2(0.f, 0.f));
	vertex[1] = Vertex_PCU(Vec3(0.5f, 0.0f, 0.0f), Rgba8 (255, 255, 0, 255), Vec2(0.f, 0.f));
	vertex[2] = Vertex_PCU(Vec3(0.0f, 0.5f, 0.0f), Rgba8 (255, 0, 0, 255), Vec2(0.f, 0.f));
	// back triangle
	vertex[3] = Vertex_PCU(Vec3(-2.0f, 0.0f, 0.0f), Rgba8 (255, 0, 0, 0), Vec2(0.f, 0.f));
	vertex[4] = Vertex_PCU(Vec3(0.0f, -0.5f, 0.0f), Rgba8 (255, 0, 0, 255), Vec2(0.f, 0.f));
	vertex[5] = Vertex_PCU(Vec3(0.0f, 0.5f, 0.0f), Rgba8 (255, 0, 0, 255), Vec2(0.f, 0.f));
}

void Bullet::Update(float deltaSeconds)
{
	if (m_shrinking && m_wormhole && m_wormhole->m_velocity != m_wormholeVelocity)
	{
		m_velocity -= m_wormholeVelocity; // remove old velocity component and add new velocity component
		m_velocity += m_wormhole->m_velocity;
		m_wormholeVelocity = m_wormhole->m_velocity;
	}
	TeleportThroughWormhole(deltaSeconds);

	m_position += m_velocity * deltaSeconds;
	DisappearIntoTheVoid(); // check whether bullet is off screen
	if (GetCurrentTimeSeconds() - m_lifeTime > BULLET_LIFETIME_SECONDS)
	{
		Die();
	}
}

void Bullet::Render() const
{
	Vertex_PCU tvertex[BULLET_VERTS];
	for (int vertIndex = 0; vertIndex < BULLET_VERTS; vertIndex++)
	{
		tvertex[vertIndex] = vertex[vertIndex];
	}
	TransformVertexArrayXY3D(BULLET_VERTS, tvertex, m_scalingFactor, m_orientationDegrees, m_position);
	g_theRenderer->DrawVertexArray(BULLET_VERTS, tvertex);
}

void Bullet::DisappearIntoTheVoid()
{
	if (IsOffscreen())
	{
		Die();
	}
}
