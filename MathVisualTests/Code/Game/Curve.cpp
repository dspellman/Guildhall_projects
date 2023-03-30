#include "Game/Curve.hpp"
#include <vector>
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/LineSegment2.hpp"
#include "GameCommon.hpp"
#include "CubicBezierCurve2D.hpp"

Curve::Curve()
{

}

void Curve::SetSubdivisions(int numSubdivisions)
{
	m_numSubdivisions = numSubdivisions;
	m_increment = 1.0f / (float)m_numSubdivisions;
}

void Curve::SetViewPort(Camera const& camera, AABB2 viewport)
{
	m_viewport = viewport;
	float h = camera.GetOrthoTopRight().x - camera.GetOrthoBottomLeft().x;
	float w = camera.GetOrthoTopRight().y - camera.GetOrthoBottomLeft().y;
	float scaleX = h * viewport.GetDimensions().x;
	float scaleY = w * viewport.GetDimensions().y;
	float xlateX = camera.GetOrthoBottomLeft().x + h * viewport.m_mins.x;
	float xlateY = camera.GetOrthoBottomLeft().y + w * viewport.m_mins.y;
	m_transform = Mat44::CreateNonUniformScale2D(Vec2(scaleX, scaleY));
	m_transform.SetTranslation2D(Vec2(xlateX, xlateY));
}

void Curve::DrawCubicHermiteCurve(std::vector<CubicBezierCurve2D> const& hermite, float time, int segment)
{
 	float distance = 0.0f;
	for (int seg = 0; seg < (int)hermite.size(); seg++)
	{
		distance += hermite[seg].GetApproximateLength(m_numSubdivisions);
	}
	float fraction = ((float)segment + time) / (float)hermite.size(); // fraction of total length traversed so far
	float partial = fraction * distance; // how far we have traveled
	float cumulative = 0.0f;
	float remainder = 0.0f;
	int pSeg = 0;
	for (int seg = 0; seg < (int)hermite.size(); seg++)
	{
		remainder = cumulative;
		cumulative += hermite[seg].GetApproximateLength(m_numSubdivisions);
		if (cumulative >= partial)
		{
			pSeg = seg;
			remainder = partial - remainder; // save how far in we need to be
			break;
		}
	}
	float proportion = remainder /  hermite[pSeg].GetApproximateLength(m_numSubdivisions);

	float f = 0.0f;
	float t = 0.0f;
	for (int index = 0; index < (int)hermite.size(); index++)
	{
		f = (index == pSeg) ? proportion : -1.0f;
		t = (index == segment) ? time : -1.0f;
		DrawCubicHermiteSegment(hermite[index], t, f, (index < (int)hermite.size() - 1));
	}
}

void Curve::DrawCubicHermiteSegment(CubicBezierCurve2D const& bezier, float time, float fraction, bool arrow)
{
	float delta = m_increment;
	std::vector< Vertex_PCU> verts;
	LineSegment2 lineSegment;
	float thickness = 0.75f;
	Rgba8 const& color = Rgba8::GREEN;
	Rgba8 const& purple = Rgba8(90, 16, 115);

	Vec2 sp;
	Vec2 sv;
	Vec2 ep;
	Vec2 ev;

	bezier.GetCubicHermiteCurve2D(&sp, &sv, &ep, &ev);
	Vec2 s = m_transform.TransformPosition2D(sp);
	Vec2 v1 = m_transform.TransformPosition2D(sv);
	Vec2 e = m_transform.TransformPosition2D(ep);
	Vec2 v2 = m_transform.TransformPosition2D(ev);

	// draw control lines first
	AddVertsForLineSegment2D(verts, LineSegment2(s, e), 0.5f, purple);
	if (arrow)
	{
		AddVertsForArrow2D(verts, e, e + v2, 2.0f, 0.5f, Rgba8::RED);
	}

	for (float t = 0.0f; t < 1.0f; t += delta)
	{
		lineSegment.m_startPos = m_transform.TransformPosition2D(bezier.EvaluateAtParametric(t));
		lineSegment.m_endPos = m_transform.TransformPosition2D(bezier.EvaluateAtParametric(t + delta));
		AddVertsForLineSegment2D(verts, lineSegment, thickness, color);
	}

	// draw points on line last
	AddVertsForDisc2D(verts, s, 1.5f, purple);
// 	AddVertsForDisc2D(verts, g1, 1.5f, purple);
// 	AddVertsForDisc2D(verts, g2, 1.5f, purple);
	AddVertsForDisc2D(verts, e, 1.5f, purple);

	if (time != -1.0f)
	{
	Vec2 timePoint = m_transform.TransformPosition2D(bezier.EvaluateAtParametric(time));
	AddVertsForDisc2D(verts, timePoint, 1.5f, Rgba8::WHITE);
	}
	if (fraction != -1.0f)
	{
	Vec2 distancePoint = m_transform.TransformPosition2D(bezier.EvaluateApproximateDistance(fraction * bezier.GetApproximateLength(m_numSubdivisions)));
	AddVertsForDisc2D(verts, distancePoint, 1.5f, Rgba8::GOLD);
	}

	g_theRenderer->DrawVertexArray(int(verts.size()), verts.data());
}

void Curve::DrawCubicBezierCurve(CubicBezierCurve2D const& bezier, float time)
{
	float delta = m_increment;
	std::vector< Vertex_PCU> verts;
	LineSegment2 lineSegment;
	float thickness = 0.75f;
	Rgba8 const& color = Rgba8::GREEN;
	Rgba8 const& purple = Rgba8(90, 16, 115);

	Vec2 s = m_transform.TransformPosition2D(bezier.m_startPos);
	Vec2 g1 = m_transform.TransformPosition2D(bezier.m_guidePos1);
	Vec2 g2 = m_transform.TransformPosition2D(bezier.m_guidePos2);
	Vec2 e = m_transform.TransformPosition2D(bezier.m_endPos);

	// draw control lines first
	AddVertsForLineSegment2D(verts, LineSegment2(s, g1), 0.5f, purple);
	AddVertsForLineSegment2D(verts, LineSegment2(g1, g2), 0.5f, purple);
	AddVertsForLineSegment2D(verts, LineSegment2(g2, e), 0.5f, purple);

	for (float t = 0.0f; t < 1.0f; t += delta)
	{
		lineSegment.m_startPos = m_transform.TransformPosition2D(bezier.EvaluateAtParametric(t));
		lineSegment.m_endPos = m_transform.TransformPosition2D(bezier.EvaluateAtParametric(t + delta));
		AddVertsForLineSegment2D(verts, lineSegment, thickness, color);
	}

	// draw points on line last
	AddVertsForDisc2D(verts, s, 1.5f, purple);
	AddVertsForDisc2D(verts, g1, 1.5f, purple);
	AddVertsForDisc2D(verts, g2, 1.5f, purple);
	AddVertsForDisc2D(verts, e, 1.5f, purple);

	Vec2 timePoint = m_transform.TransformPosition2D(bezier.EvaluateAtParametric(time));
	AddVertsForDisc2D(verts, timePoint, 1.5f, Rgba8::WHITE);
	Vec2 distancePoint = m_transform.TransformPosition2D(bezier.EvaluateApproximateDistance(time * bezier.GetApproximateLength(m_numSubdivisions)));
	AddVertsForDisc2D(verts, distancePoint, 1.5f, Rgba8::GOLD);

	g_theRenderer->DrawVertexArray(int(verts.size()), verts.data());
}

void Curve::DrawEasingFunction(pointFunction Evaluate, float time)
{
	float delta = m_increment;
	std::vector< Vertex_PCU> verts;
	LineSegment2 lineSegment;
	float thickness = 0.75f;
	Rgba8 const& color = Rgba8::GREEN;
	Rgba8 const& purple = Rgba8(90, 16, 115);

	Texture* testTexture = g_theRenderer->CreateOrGetTextureFromFile((char const*)"Data/Images/graphpaper.png");
	g_theRenderer->BindTexture(testTexture);
	AddVertsForAABB2D(verts, AABB2(m_transform.TransformPosition2D(Vec2::ZERO), m_transform.TransformPosition2D(Vec2::ONE)), Rgba8::GRAY);
	g_theRenderer->DrawVertexArray(int(verts.size()), &verts[0]);
	verts.clear();
	g_theRenderer->BindTexture(0);

	for (float t = 0.0f; t < 1.0f; t += delta)
	{
		lineSegment.m_startPos = m_transform.TransformPosition2D(Vec2(t, Evaluate(t)));
		lineSegment.m_endPos = m_transform.TransformPosition2D(Vec2(t+delta, Evaluate(t+delta)));
		AddVertsForLineSegment2D(verts, lineSegment, thickness, color);
	}
	// draw points on line and other info too
	Vec2 timePoint = m_transform.TransformPosition2D(Vec2(time, Evaluate(time)));
	Vec2 xAxis = m_transform.TransformPosition2D(Vec2(0.0f, Evaluate(time)));
	Vec2 yAxis = m_transform.TransformPosition2D(Vec2(time, 0.0f));
	AddVertsForLineSegment2D(verts, LineSegment2(xAxis, timePoint), 0.5f, purple);
	AddVertsForLineSegment2D(verts, LineSegment2(timePoint, yAxis), 0.5f, purple);
	AddVertsForDisc2D(verts, timePoint, 1.5f, Rgba8::WHITE);

	g_theRenderer->DrawVertexArray(int(verts.size()), verts.data());
}
