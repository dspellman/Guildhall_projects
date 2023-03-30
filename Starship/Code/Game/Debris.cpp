#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Entity.hpp"
#include "Game/Game.hpp"
#include "Game/App.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Game/Debris.hpp"
#include "Engine/Core/Time.hpp"
#include "Game/Wormhole.hpp"

constexpr float arcDegrees = 360.0 / DEBRIS_VERTS;

Debris::Debris(Game* game, Vec2 startPos, Vec2 velocity, float orientationDegrees, float scale, Rgba8 color) : Entity(game, startPos)
{
	m_color = color;
	m_color.a = 127;

	m_position = startPos;
	m_orientationDegrees = orientationDegrees;
	m_velocity.x = velocity.x + DEBRIS_SPEED * CosDegrees(m_orientationDegrees) * random.RollRandomFloatInRange(0.5f, 1.0f);
	m_velocity.y = velocity.y + DEBRIS_SPEED * SinDegrees(m_orientationDegrees) * random.RollRandomFloatInRange(0.5f, 1.0f);
	m_angularVelocity = DEBRIS_ANGULAR_VELOCITY_DEGREES * random.RollRandomFloatInRange(-1.0f, 1.0f);
	m_startTime = GetCurrentTimeSeconds();

	m_physicalRadius = DEBRIS_MIN_RADIUS; // placeholder changed below
	m_cosmeticRadius = DEBRIS_MIN_RADIUS;

	for (int vert = 0; vert < DEBRIS_VERTS; vert++)
	{
		float theta = arcDegrees * vert;
		float edgeX = scale * CosDegrees(theta) * random.RollRandomFloatInRange(DEBRIS_MIN_RADIUS, DEBRIS_MAX_RADIUS);
		float edgeY = scale * SinDegrees(theta) * random.RollRandomFloatInRange(DEBRIS_MIN_RADIUS, DEBRIS_MAX_RADIUS);
		Vec3 spoke = Vec3(edgeX, edgeY, 0.0f);
		if (spoke.GetLength() > m_cosmeticRadius)
		{
			m_cosmeticRadius = spoke.GetLength();
		}
		vertex[vert] = Vertex_PCU(spoke, m_color, Vec2(0.f, 0.f));
	}

	m_physicalRadius = 0.8f * m_cosmeticRadius;  // set value after determining maximum cosmetic radius
}

Debris::Debris(Game* game, Vec2 startPos, Vec2 velocity, float orientationDegrees, float scale, Wormhole* wormhole) : Entity(game, startPos)
{
	m_wormhole = wormhole;
	m_color = Rgba8(255, 255, 255, 127); // fixed color for wormhole debris

	m_position = startPos;
	m_orientationDegrees = orientationDegrees;
	m_wormholeVelocity = velocity;
	m_velocity = velocity;
	float speedVariance = random.RollRandomFloatInRange(0.25f, 0.4f);
	m_velocity.x += DEBRIS_SPEED * CosDegrees(m_orientationDegrees) * speedVariance;
	m_velocity.y += DEBRIS_SPEED * SinDegrees(m_orientationDegrees) * speedVariance;
	m_angularVelocity = DEBRIS_ANGULAR_VELOCITY_DEGREES * random.RollRandomFloatInRange(-1.0f, 1.0f);
	m_startTime = GetCurrentTimeSeconds();

	m_physicalRadius = DEBRIS_MIN_RADIUS; // placeholder changed below
	m_cosmeticRadius = DEBRIS_MIN_RADIUS;

	for (int vert = 0; vert < DEBRIS_VERTS; vert++)
	{
		float theta = arcDegrees * vert;
		float edgeX = scale * CosDegrees(theta) * random.RollRandomFloatInRange(DEBRIS_MIN_RADIUS, DEBRIS_MAX_RADIUS);
		float edgeY = scale * SinDegrees(theta) * random.RollRandomFloatInRange(DEBRIS_MIN_RADIUS, DEBRIS_MAX_RADIUS);
		Vec3 spoke = Vec3(edgeX, edgeY, 0.0f);
		if (spoke.GetLength() > m_cosmeticRadius)
		{
			m_cosmeticRadius = spoke.GetLength();
		}
		vertex[vert] = Vertex_PCU(spoke, m_color, Vec2(0.f, 0.f));
	}

	m_physicalRadius = 0.8f * m_cosmeticRadius;  // set value after determining maximum cosmetic radius
}

void Debris::Update(float deltaSeconds)
{
	float lifeTime = static_cast<float>(GetCurrentTimeSeconds() - m_startTime);
	m_color.a = static_cast<unsigned char>(RangeMap(lifeTime, 0.0f, DEBRIS_LIFETIME, 127.0f, 0.0f));
	if (m_wormhole && m_wormhole->m_velocity != m_wormholeVelocity)
	{
		m_velocity -= m_wormholeVelocity; // remove old velocity component and add new velocity component
		m_velocity += m_wormhole->m_velocity;
		m_wormholeVelocity = m_wormhole->m_velocity;
	}
	m_position += m_velocity * deltaSeconds;
	m_orientationDegrees += deltaSeconds * m_angularVelocity;
	DisappearIntoTheVoid(); // check whether bullet is off screen
	if (lifeTime > DEBRIS_LIFETIME)
	{
		Die();
	}
}

void Debris::Render() const
{
	Vertex_PCU centerVertex = Vertex_PCU(Vec3(0.0f, 0.0f, 0.0f), m_color, Vec2(0.f, 0.f));
	Vertex_PCU tvertex[3 * DEBRIS_VERTS];
	for (int vertIndex = 0; vertIndex < DEBRIS_VERTS; vertIndex++)
	{
		tvertex[3 * vertIndex] = centerVertex;
		tvertex[3 * vertIndex + 1] = vertex[vertIndex];
		tvertex[3 * vertIndex + 1].m_color = m_color;
		tvertex[3 * vertIndex + 2] = vertex[(vertIndex + 1) % DEBRIS_VERTS];
		tvertex[3 * vertIndex + 2].m_color = m_color;
	}
	TransformVertexArrayXY3D(3 * DEBRIS_VERTS, tvertex, m_scalingFactor, m_orientationDegrees, m_position);
	g_theRenderer->DrawVertexArray(3 * DEBRIS_VERTS, tvertex);
}

void Debris::DisappearIntoTheVoid()
{
	if (IsOffscreen())
	{
		Die();
	}
}
