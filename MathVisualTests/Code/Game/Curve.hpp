#pragma once
#include "Engine/Renderer/Camera.hpp"

typedef float (*pointFunction)(float);
class CubicBezierCurve2D;

class Curve
{
public:
	Curve();
	void SetSubdivisions(int numSubdivisions);
	void SetViewPort(Camera const& camera, AABB2 viewport);
	void DrawCubicHermiteCurve(std::vector<CubicBezierCurve2D> const& hermite, float time, int segment);
	void DrawCubicHermiteSegment(CubicBezierCurve2D const& bezier, float time, float fraction, bool arrow);
	void DrawCubicBezierCurve(CubicBezierCurve2D const& bezier, float time);
	void DrawEasingFunction(pointFunction Evaluate, float time);

	std::vector<pointFunction> m_functions;
	int m_numSubdivisions = 64;
	AABB2 m_viewport = AABB2::ZERO_TO_ONE;
	float m_increment = 1.0f / 64.0f;
	Mat44 m_transform;
};