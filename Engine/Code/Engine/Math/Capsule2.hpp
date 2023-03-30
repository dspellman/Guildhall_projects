#pragma once
#include "Engine\Math\Vec2.hpp"

struct Capsule2
{
public:
	Vec2 m_startPos = Vec2::ZERO;
	Vec2 m_endPos = Vec2::ZERO;
	float m_radius = 0.0f;
	float m_boundingRadius = 0.0f;
	Vec2 m_center = Vec2::ZERO;

	~Capsule2();
	Capsule2();
	explicit Capsule2(Vec2 const& startPos, Vec2 const& endPos, float radius);
};