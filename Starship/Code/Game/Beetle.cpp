#include "Engine/Math/MathUtils.hpp"
#define _USE_MATH_DEFINES
#include <math.h>
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Game/Game.hpp"
#include "Beetle.hpp"
#include "PlayerShip.hpp"
#include "Game/Wormhole.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/Time.hpp"

extern RandomNumberGenerator random; // singleton for now only used by entities in the Game

Rgba8 beetleColor(200, 0, 200, 255);

Beetle::Beetle(Game* game, Vec2 startPos) : Entity(game, startPos)
{
	m_health = BEETLE_HEALTH;
	m_physicalRadius = PLAYER_SHIP_PHYSICS_RADIUS;
	m_cosmeticRadius = PLAYER_SHIP_COSMETIC_RADIUS;

	// middle triangle
	vertex[0] = Vertex_PCU(Vec3(-1.0f, -1.0f, 0.0f), beetleColor, Vec2(0.f, 0.f));
	vertex[1] = Vertex_PCU(Vec3(1.0f, 0.0f, 0.0f), beetleColor, Vec2(0.f, 0.f));
	vertex[2] = Vertex_PCU(Vec3(-1.0f, 1.0f, 0.0f), beetleColor, Vec2(0.f, 0.f));
	// upper front triangle
	vertex[3] = Vertex_PCU(Vec3(0.0f, 0.0f, 0.0f), beetleColor, Vec2(0.f, 0.f));
	vertex[4] = Vertex_PCU(Vec3(2.0f, 1.0f, 0.0f), beetleColor, Vec2(0.f, 0.f));
	vertex[5] = Vertex_PCU(Vec3(0.0f, 1.0f, 0.0f), beetleColor, Vec2(0.f, 0.f));
	// lower front triangle
	vertex[6] = Vertex_PCU(Vec3(0.0f, 0.0f, 0.0f), beetleColor, Vec2(0.f, 0.f));
	vertex[7] = Vertex_PCU(Vec3(0.0f, -1.0f, 0.0f), beetleColor, Vec2(0.f, 0.f));
	vertex[8] = Vertex_PCU(Vec3(2.0f, -1.0f, 0.0f), beetleColor, Vec2(0.f, 0.f));
	// upper back triangle
	vertex[9] = Vertex_PCU(Vec3(-1.0f, 0.0f, 0.0f), beetleColor, Vec2(0.f, 0.f));
	vertex[10] = Vertex_PCU(Vec3(-1.0f, 1.0f, 0.0f), beetleColor, Vec2(0.f, 0.f));
	vertex[11] = Vertex_PCU(Vec3(-2.0f, 1.0f, 0.0f), beetleColor, Vec2(0.f, 0.f));
	// lower back triangle
	vertex[12] = Vertex_PCU(Vec3(-1.0f, 0.0f, 0.0f), beetleColor, Vec2(0.f, 0.f));
	vertex[13] = Vertex_PCU(Vec3(-2.0f, -1.0f, 0.0f), beetleColor, Vec2(0.f, 0.f));
	vertex[14] = Vertex_PCU(Vec3(-1.0f, -1.0f, 0.0f), beetleColor, Vec2(0.f, 0.f));

	SpawnOffscreen();
	m_velocity = Vec2(WASP_SPEED * CosDegrees(m_orientationDegrees), WASP_SPEED * SinDegrees(m_orientationDegrees));
}

void Beetle::Update(float deltaSeconds)
{
	if (m_shrinking && m_wormhole && m_wormhole->m_velocity != m_wormholeVelocity)
	{
		m_velocity -= m_wormholeVelocity; // remove old velocity component and add new velocity component
		m_velocity += m_wormhole->m_velocity;
		m_wormholeVelocity = m_wormhole->m_velocity;
	}
	TeleportThroughWormhole(deltaSeconds);

	// disable behavior if translating through wormhole
	if (!m_shrinking)
	{
		PlayerShip* playerShip = m_game->GetNearestLivingPlayer(m_position);
		if (playerShip)
		{
			Vec2 orientation = Vec2(playerShip->m_position - m_position);
			m_orientationDegrees = Atan2Degrees(orientation.y, orientation.x);
		}
		Vec2 forwardNormal = GetForwardNormal();
		m_velocity = forwardNormal * BEETLE_SPEED;
	}

	m_position += deltaSeconds * m_velocity;
}

void Beetle::Render() const
{
	Vertex_PCU tvertex[BEETLE_VERTS];
	for (int vertIndex = 0; vertIndex < BEETLE_VERTS; vertIndex++)
	{
		tvertex[vertIndex] = vertex[vertIndex];
	}
	TransformVertexArrayXY3D(BEETLE_VERTS, tvertex, m_scalingFactor, m_orientationDegrees, m_position);
	g_theRenderer->DrawVertexArray(BEETLE_VERTS, tvertex);
}

void Beetle::AttractRender(Vec2 position, float orientationDegrees, float scale) const
{
	UNUSED( orientationDegrees );

	Vertex_PCU tvertex[BEETLE_VERTS];
	for (int vertIndex = 0; vertIndex < BEETLE_VERTS; vertIndex++)
	{
		tvertex[vertIndex] = vertex[vertIndex];
		tvertex[vertIndex].m_color.a /= 2; // make transparent
	}
	// use sinusoidal behavior for spice
	double time = GetCurrentTimeSeconds();
//	float cycleTransparency = static_cast<float>(sin(time + Pi()));
	float sine = static_cast<float>(sin(time + Pi()));
	float cosine = static_cast<float>(cos(time + Pi()));
	// do static thrust animation
	unsigned char timeA = static_cast<unsigned char>(64.0f + 191.0f * (sine + 1.0f) * 0.5f);
	for (int vert = 0; vert < BEETLE_VERTS; vert++)
		tvertex[vert].m_color.a = timeA;
	position.x += 20.0f * cosine * scale;
	position.y += 8.0f * sine * scale;
	float orthoAngleDegrees = Atan2Degrees(sine, cosine) + 90.0f;

	TransformVertexArrayXY3D(BEETLE_VERTS, tvertex, scale * ((sine + 1.0f) * 0.25f + 0.75f), orthoAngleDegrees, position);
	g_theRenderer->DrawVertexArray(BEETLE_VERTS, tvertex);
}

