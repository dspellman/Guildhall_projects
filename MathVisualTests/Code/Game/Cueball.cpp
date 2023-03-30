#include "Game/Cueball.hpp"
#include "GameCommon.hpp"

bool Cueball::s_bottomless = false;
Vec2 Cueball::s_gravity = Vec2(0.0f, 10.0f);

Cueball::Cueball(Vec2 position, Vec2 velocity, float radius, bool gravity, float elasticity /*= 0.9f*/, Rgba8 color /*= Rgba8::WHITE*/)
	: m_position( position), m_velocity( velocity), m_radius(radius), m_gravity(gravity), m_elasticity( elasticity), m_color( color)
{
}

void Cueball::UpdateBall(float deltaSeconds)
{
	if (m_gravity)
	{
		m_velocity -= deltaSeconds * s_gravity;
	}
	m_position += deltaSeconds * m_velocity;
	if (m_gravity)
	{
		BounceOffWalls(); // Pachenko 2D behavior
	}
	else
	{
		BounceAgainstWalls(); // billiards 2D behavior
	}
}

void Cueball::AddVerts(std::vector<Vertex_PCU>& drawVerts) const
{
	AddVertsForDisc2D(drawVerts, m_position, m_radius, m_color);
}

void Cueball::BounceAgainstWalls()
{
	if (m_position.x > WORLD_SIZE_X - m_radius)
	{
		m_position.x = WORLD_SIZE_X - m_radius; // rubber band back in bounds
		m_velocity.x *= -WALL_ELASTICITY * m_elasticity; // reflect speed for bounce
	}

	if (m_position.x < m_radius)
	{
		m_position.x = m_radius; // rubber band back in bounds
		m_velocity.x *= -WALL_ELASTICITY * m_elasticity; // reflect speed for bounce
	}

	if (m_position.y > WORLD_SIZE_Y - m_radius)
	{
		m_position.y = WORLD_SIZE_Y - m_radius; // rubber band back in bounds
		m_velocity.y *= -WALL_ELASTICITY * m_elasticity; // reflect speed for bounce
	}

	if (m_position.y < m_radius)
	{
		m_position.y = m_radius; // rubber band back in bounds
		m_velocity.y *= -WALL_ELASTICITY * m_elasticity; // reflect speed for bounce
	}
}

void Cueball::BounceOffWalls()
{
	if (m_position.x > WORLD_SIZE_X - m_radius)
	{
		m_position.x = WORLD_SIZE_X - m_radius; // rubber band back in bounds
		m_velocity.x *= -WALL_ELASTICITY * m_elasticity; // reflect speed for bounce
	}

	if (m_position.x < m_radius)
	{
		m_position.x = m_radius; // rubber band back in bounds
		m_velocity.x *= -WALL_ELASTICITY * m_elasticity; // reflect speed for bounce
	}

	if (s_bottomless)
	{
		if (m_position.y < -m_radius)
		{
			m_position.y = WORLD_SIZE_Y * 1.1f + m_radius; // move to top
		}
	}
	else
	{
		if (m_position.y < m_radius)
		{
			m_position.y = m_radius; // rubber band back in bounds
			m_velocity.y *= -WALL_ELASTICITY * m_elasticity; // reflect speed for bounce
		}
	}
}

void Cueball::BounceOffBumperDisc(Vec2 const position, float radius, float elasticity)
{
	if (DoDiscsOverlap(m_position, m_radius, position, radius) == false)
	{
		return; // early exit
	}
	// push out of bumper first
	PushDiscOutOfDisc2D(m_position, m_radius, position, radius);
	// get collision normal
	Vec2 incomingNormal = (position - m_position).GetNormalized(); // bumper - cue ball
	// change velocity with damping from collision
	Vec2 An = DotProduct2D(m_velocity, incomingNormal) * incomingNormal;
	Vec2 At = m_velocity - An;
	m_velocity = At - An * m_elasticity * elasticity;
}

void Cueball::BounceOffBumperOBB(OBB2 const& obb, float elasticity)
{
	if ((m_position - obb.m_center).GetLengthSquared() > (m_radius + obb.m_boundingRadius) * (m_radius + obb.m_boundingRadius))
	{
		return;
	}

	Vec2 impact = GetNearestPointOnOBB2D(m_position, obb);
	if (PushDiscOutOfPoint2D(m_position, m_radius, impact) == false)
	{
		return;
	}

	Vec2 incomingNormal = (impact - m_position).GetNormalized(); // bumper - cue ball
	// change velocity with damping from collision
	Vec2 An = DotProduct2D(m_velocity, incomingNormal) * incomingNormal;
	Vec2 At = m_velocity - An;
	m_velocity = At - An * m_elasticity * elasticity;
}

void Cueball::BounceOffBumperCapsule(Capsule2 const& capsule, float elasticity)
{
	if ((m_position - capsule.m_center).GetLengthSquared() > (m_radius + capsule.m_boundingRadius) * (m_radius + capsule.m_boundingRadius))
	{
		return;
	}

	Vec2 impact = GetNearestPointOnCapsule2D(m_position, capsule);
	if (PushDiscOutOfPoint2D(m_position, m_radius, impact) == false)
	{
		return;
	}

	Vec2 incomingNormal = (impact - m_position).GetNormalized(); // bumper - cue ball
	// change velocity with damping from collision
	Vec2 An = DotProduct2D(m_velocity, incomingNormal) * incomingNormal;
	Vec2 At = m_velocity - An;
	m_velocity = At - An * m_elasticity * elasticity;
}

void Cueball::BounceOffBall(Cueball& B)
{
	if (DoDiscsOverlap(m_position, m_radius, B.m_position, B.m_radius) == false)
	{
		return; // early exit
	}
	// push balls out of each other first
	PushDiscsOutOfEachOther2D(m_position, m_radius, B.m_position, B.m_radius);
	// get collision normal
	Vec2 incomingNormal = (B.m_position - m_position).GetNormalized(); // AB = B-A
	float Avn = DotProduct2D(m_velocity, incomingNormal);
	float Bvn = DotProduct2D(B.m_velocity, incomingNormal);
	// change velocity with damping from collision
	if (Bvn - Avn < 0)
	{
		Vec2 An = DotProduct2D(m_velocity, incomingNormal) * incomingNormal;
		Vec2 At = m_velocity - An;
		Vec2 Bn = DotProduct2D(B.m_velocity, incomingNormal) * incomingNormal;
		Vec2 Bt = B.m_velocity - Bn;
		m_velocity = At + Bn * m_elasticity * B.m_elasticity;
		B.m_velocity = Bt + An * m_elasticity * B.m_elasticity;
	}
}
