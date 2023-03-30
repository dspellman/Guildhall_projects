#pragma once
#include "Engine/Math/Vec2.hpp"

class CubicBezierCurve2D
{
public:
	CubicBezierCurve2D();
	CubicBezierCurve2D(Vec2 startPos, Vec2 guidePos1, Vec2 guidePos2, Vec2 endPos);
//	explicit CubicBezierCurve2D( CubicHermiteCurve2D const& fromHermite );
	void CubicHermiteCurve2D(Vec2 startPos, Vec2 startVel, Vec2 endPos, Vec2 endVel);
	void GetCubicHermiteCurve2D(Vec2* startPos, Vec2* startVel, Vec2* endPos, Vec2* endVel) const;
	Vec2 GetRandomVec2ZeroToOne() const;
	void SetSubdivisions(int numSubdivisions);
	Vec2 EvaluateAtParametric(float parametricZeroToOne) const;
	float GetApproximateLength( int numSubdivisions = 64 ) const;
	Vec2 EvaluateApproximateDistance( float distanceAlongCurve, int numSubdivisions = 64 ) const;

//	int m_numSubdivisions = 64;
//	float m_increment = 0.0f;

	Vec2 m_startPos; 
	Vec2 m_guidePos1; 
	Vec2 m_guidePos2; 
	Vec2 m_endPos;
};