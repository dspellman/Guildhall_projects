#include "Engine\math\Capsule2.hpp"
#include "Engine/Math/MathUtils.hpp"

Capsule2::~Capsule2()
{

}

Capsule2::Capsule2()
{

}

Capsule2::Capsule2(Vec2 const& startPos, Vec2 const& endPos, float radius)
	: m_startPos(startPos), m_endPos(endPos), m_radius(radius)
{
	m_boundingRadius = radius + (startPos - endPos).GetLength() * 0.5f;
	m_center = startPos + (endPos - startPos) * 0.5f;
}

