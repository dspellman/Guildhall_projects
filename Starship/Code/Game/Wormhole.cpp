#include "Game/GameCommon.hpp"
#include "Game/Entity.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Vec2.hpp"
#define _USE_MATH_DEFINES
#include <math.h>
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Game/Wormhole.hpp"
#include "Game/Debris.hpp"
#include "Game/Game.hpp"

Rgba8 outerColor(255, 255, 255, 255);
Rgba8 innerColor(0, 0, 0, 0);

Wormhole::Wormhole(Game* game, Vec2 startPos) : Entity(game, startPos)
{
	m_radius = WORMHOLE_COSMETIC_RADIUS;
	m_width = WORMHOLE_WIDTH;
	m_cosmeticRadius = WORMHOLE_COSMETIC_RADIUS;
	m_physicalRadius = WORMHOLE_PHYSICS_RADIUS;
	m_position = SpawnOnBorder();
	m_velocity = Vec2(random.RollRandomFloatInRange(-1.0f, 1.0f), random.RollRandomFloatInRange(-1.0f, 1.0f));
	m_velocity.Normalize();
	m_velocity *= WORMHOLE_VELOCITY;
	m_orientationDegrees = 0.0f; // doesn't rotate
//	m_orientationDegrees = m_velocity.GetOrientationDegrees();
}

void Wormhole::Update(float deltaSeconds)
{
	// spawn inward falling debris at random intervals
	if (deltaSeconds != 0.0f && random.RollRandomFloatZeroToOne() < WORMHOLE_FREQUENCY)
		SpawnInfallingDebris();

	m_position += m_velocity * deltaSeconds;
	BounceOffWalls();
}

void Wormhole::Render()
{
	Vec2 center(0.0f, 0.0f);
	Vertex_PCU vertex[6 * RING_SEGMENTS]; // 2 triangles
	Vec2 corner[4]; // always 4 corners to a rhombus

	for (int segment = 0; segment < RING_SEGMENTS; segment++)
	{
		float theta = segmentAngle * static_cast<float>(segment) + segmentAngle * 0.5f;
		corner[0].x = center.x + CosDegrees(theta) * (m_radius - m_width);
		corner[0].y = center.y + SinDegrees(theta) * (m_radius - m_width);
		corner[1].x = center.x + CosDegrees(theta) * (m_radius);
		corner[1].y = center.y + SinDegrees(theta) * (m_radius);

		theta += segmentAngle; // move to next radial
		corner[2].x = center.x + CosDegrees(theta) * (m_radius - m_width);
		corner[2].y = center.y + SinDegrees(theta) * (m_radius - m_width);
		corner[3].x = center.x + CosDegrees(theta) * (m_radius);
		corner[3].y = center.y + SinDegrees(theta) * (m_radius);

		vertex[6 * segment + 0] = Vertex_PCU(Vec3(corner[1].x, corner[1].y, 0.0f), outerColor, Vec2(0.f, 0.f));
		vertex[6 * segment + 1] = Vertex_PCU(Vec3(corner[3].x, corner[3].y, 0.0f), outerColor, Vec2(0.f, 0.f));
		vertex[6 * segment + 2] = Vertex_PCU(Vec3(corner[2].x, corner[2].y, 0.0f), innerColor, Vec2(0.f, 0.f));
		vertex[6 * segment + 3] = Vertex_PCU(Vec3(corner[0].x, corner[0].y, 0.0f), innerColor, Vec2(0.f, 0.f));
		vertex[6 * segment + 4] = Vertex_PCU(Vec3(corner[1].x, corner[1].y, 0.0f), outerColor, Vec2(0.f, 0.f));
		vertex[6 * segment + 5] = Vertex_PCU(Vec3(corner[2].x, corner[2].y, 0.0f), innerColor, Vec2(0.f, 0.f));
	}

	TransformVertexArrayXY3D(6 * RING_SEGMENTS, vertex, 1.0f, m_orientationDegrees, m_position);
	g_theRenderer->DrawVertexArray(6 * RING_SEGMENTS, vertex);
}

void Wormhole::SpawnInfallingDebris()
{
	float angleDegrees = random.RollRandomFloatInRange(0.0f, 360.0f);
	Vec2 startPos(0.0f, 0.0f);
	startPos.x = m_position.x - CosDegrees(angleDegrees) * (m_radius - m_width * 0.5f);
	startPos.y = m_position.y - SinDegrees(angleDegrees) * (m_radius - m_width * 0.5f);
	bool failed = true;

	for (int i = 0; i < MAX_DEBRIS; i++)
	{
		if (failed && nullptr == m_game->m_debris[i])
		{
			m_game->m_debris[i] = new Debris(m_game, startPos, m_velocity, angleDegrees, WORMHOLE_DEBRIS_SCALE, this);
			failed = false;
			break;
		}
	}
}
