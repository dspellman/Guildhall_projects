#include "Engine/Math/MathUtils.hpp"
#define _USE_MATH_DEFINES
#include <math.h>
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Game/Game.hpp"
#include "Wasp.hpp"
#include "PlayerShip.hpp"
#include "Game/Wormhole.hpp"

Rgba8 waspColor(200, 200, 0, 255);

Wasp::Wasp(Game* game, Vec2 startPos) : Entity(game, startPos)
{
	m_health = WASP_HEALTH;
	m_physicalRadius = WASP_PHYSICS_RADIUS;
	m_cosmeticRadius = WASP_COSMETIC_RADIUS;

	// upper triangle
	vertex[0] = Vertex_PCU(Vec3(0.0f, 1.0f, 0.0f), waspColor, Vec2(0.f, 0.f));
	vertex[1] = Vertex_PCU(Vec3(0.0f, -1.0f, 0.0f), waspColor, Vec2(0.f, 0.f));
	vertex[2] = Vertex_PCU(Vec3(2.0f, 0.0f, 0.0f), waspColor, Vec2(0.f, 0.f));
	// left triangle
	vertex[3] = Vertex_PCU(Vec3(1.0f, 2.0f, 0.0f), waspColor, Vec2(0.f, 0.f));
	vertex[4] = Vertex_PCU(Vec3(0.0f, 0.0f, 0.0f), waspColor, Vec2(0.f, 0.f));
	vertex[5] = Vertex_PCU(Vec3(1.0f, 0.0f, 0.0f), waspColor, Vec2(0.f, 0.f));
	// right triangle
	vertex[6] = Vertex_PCU(Vec3(0.0f, 0.0f, 0.0f), waspColor, Vec2(0.f, 0.f));
	vertex[7] = Vertex_PCU(Vec3(1.0f, -2.0f, 0.0f), waspColor, Vec2(0.f, 0.f));
	vertex[8] = Vertex_PCU(Vec3(1.0f, 0.0f, 0.0f), waspColor, Vec2(0.f, 0.f));
	// lower left triangle
	vertex[9] = Vertex_PCU(Vec3(0.0f, 0.0f, 0.0f), waspColor, Vec2(0.f, 0.f));
	vertex[10] = Vertex_PCU(Vec3(0.0f, 1.0f, 0.0f), waspColor, Vec2(0.f, 0.f));
	vertex[11] = Vertex_PCU(Vec3(-2.0f, 1.0f, 0.0f), waspColor, Vec2(0.f, 0.f));
	// lower right triangle
	vertex[12] = Vertex_PCU(Vec3(0.0f, 0.0f, 0.0f), waspColor, Vec2(0.f, 0.f));
	vertex[13] = Vertex_PCU(Vec3(-2.0f, -1.0f, 0.0f), waspColor, Vec2(0.f, 0.f));
	vertex[14] = Vertex_PCU(Vec3(0.0f, -1.0f, 0.0f), waspColor, Vec2(0.f, 0.f));

	SpawnOffscreen();
	m_velocity = Vec2(WASP_SPEED * CosDegrees(m_orientationDegrees), WASP_SPEED * SinDegrees(m_orientationDegrees));
}

void Wasp::Update(float deltaSeconds)
{
	if (m_shrinking && m_wormhole && m_wormhole->m_velocity != m_wormholeVelocity)
	{
		m_velocity -= m_wormholeVelocity; // remove old velocity component and add new velocity component
		m_velocity += m_wormhole->m_velocity;
		m_wormholeVelocity = m_wormhole->m_velocity;
	}
	TeleportThroughWormhole(deltaSeconds);

	if (!m_shrinking)
	{
		PlayerShip* playerShip = m_game->GetNearestLivingPlayer(m_position);
		if (playerShip)
		{
			Vec2 orientation = Vec2(playerShip->m_position - m_position);
			m_orientationDegrees = Atan2Degrees(orientation.y, orientation.x);
		}
		Vec2 forwardNormal = GetForwardNormal();
		m_velocity.x += forwardNormal.x * PLAYER_SHIP_ACCELERATION * deltaSeconds;
		m_velocity.y += forwardNormal.y * PLAYER_SHIP_ACCELERATION * deltaSeconds;
		m_velocity.ClampLength(WASP_MAX_SPEED);
	}

	m_position += deltaSeconds * m_velocity;
}

void Wasp::Render() const
{
	Vertex_PCU tvertex[WASP_VERTS];
	for (int vertIndex = 0; vertIndex < WASP_VERTS; vertIndex++)
	{
		tvertex[vertIndex] = vertex[vertIndex];
	}
	TransformVertexArrayXY3D(WASP_VERTS, tvertex, m_scalingFactor, m_orientationDegrees, m_position);
	g_theRenderer->DrawVertexArray(WASP_VERTS, tvertex);
}
