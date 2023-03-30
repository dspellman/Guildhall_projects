#include "Game/CubicBezierCurve2D.hpp"
#include "GameCommon.hpp"

constexpr float third = 1.0f / 3.0f;

CubicBezierCurve2D::CubicBezierCurve2D(Vec2 startPos, Vec2 guidePos1, Vec2 guidePos2, Vec2 endPos)
	: m_startPos( startPos ), m_guidePos1( guidePos1 ), m_guidePos2( guidePos2 ), m_endPos( endPos )
{

}

CubicBezierCurve2D::CubicBezierCurve2D()
{
	m_startPos = GetRandomVec2ZeroToOne();
	m_endPos = GetRandomVec2ZeroToOne();
	m_guidePos1 = GetRandomVec2ZeroToOne();
	m_guidePos2 =GetRandomVec2ZeroToOne();
}

void CubicBezierCurve2D::CubicHermiteCurve2D(Vec2 startPos, Vec2 startVel, Vec2 endPos, Vec2 endVel)
{
	m_startPos = startPos;
	m_endPos = endPos;
	m_guidePos1 = startPos + third * startVel;
	m_guidePos2 = endPos - third * endVel;
}

void CubicBezierCurve2D::GetCubicHermiteCurve2D(Vec2* startPos, Vec2* startVel, Vec2* endPos, Vec2* endVel) const
{
	*startPos = m_startPos;
	*endPos	= m_endPos;
	*startVel = 3.0f * (m_guidePos1 - m_startPos);
	*endVel = 3.0f * (m_endPos - m_guidePos2);
}

Vec2 CubicBezierCurve2D::GetRandomVec2ZeroToOne() const
{
	float xCoord = random.RollRandomFloatZeroToOne();
	float yCoord = random.RollRandomFloatZeroToOne();
	return Vec2(xCoord, yCoord);
}

void CubicBezierCurve2D::SetSubdivisions(int numSubdivisions)
{
	UNUSED(numSubdivisions);
	ERROR_AND_DIE("SetSubdivisions does nothing");
// 	m_numSubdivisions = numSubdivisions;
// 	m_increment = 1.0f / (float)m_numSubdivisions;
}

Vec2 CubicBezierCurve2D::EvaluateAtParametric(float parametricZeroToOne) const
{
	Vec2 p;
	p.x = ComputeCubicBezier1D(m_startPos.x, m_guidePos1.x, m_guidePos2.x, m_endPos.x, parametricZeroToOne);
	p.y = ComputeCubicBezier1D(m_startPos.y, m_guidePos1.y, m_guidePos2.y, m_endPos.y, parametricZeroToOne);
	return p;
}

float CubicBezierCurve2D::GetApproximateLength(int numSubdivisions /*= 64 */) const
{
	float delta = 1.0f / (float)numSubdivisions;
	float length = 0.0f;
	Vec2 S;
	Vec2 E;
	for (float t = 0.0f; t < 1.0f; t += delta)
	{
		S = EvaluateAtParametric(t);
		E = EvaluateAtParametric(t + delta);
		length += (E-S).GetLength();
	}
	return length;
}

Vec2 CubicBezierCurve2D::EvaluateApproximateDistance(float distanceAlongCurve, int numSubdivisions /*= 64 */) const
{
	float delta = 1.0f / (float)numSubdivisions;
	float length = 0.0f;
	float t = 0.0f;
	Vec2 S;
	Vec2 E;
	while (length < distanceAlongCurve)
	{
		S = EvaluateAtParametric(t);
		E = EvaluateAtParametric(t + delta);
		length += (E - S).GetLength();
		t += delta;
	}

	float remainder = distanceAlongCurve - length;
	Vec2 finalPos = E + remainder * (E - S).GetNormalized();

	return finalPos;
}
