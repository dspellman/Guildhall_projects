#include "Game/GameCommon.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"

#define _USE_MATH_DEFINES
#include <math.h>

// convenience constants to avoid division and remember relationships
constexpr float degreesToRadians = static_cast<float>(M_PI) / 180.0f;
constexpr float radiansToDegrees = 180.0f / static_cast<float>(M_PI);

extern Renderer* g_theRenderer; // created and owned by the App

void DebugDrawLine(Vec2 start, Vec2 end, Rgba8 color, float width)
{
	Vec2 displacement = end - start;
	displacement.Normalize();
	displacement *= (0.5f * width);

	Vec2 corner[4]; // always 4 corners to a line
	corner[0] = end + (displacement + displacement.GetRotated90Degrees());
	corner[1] = end + (displacement + displacement.GetRotatedMinus90Degrees());
	corner[2] = start - (displacement + displacement.GetRotated90Degrees());
	corner[3] = start - (displacement + displacement.GetRotatedMinus90Degrees());

	Vertex_PCU vertex[6]; // lines are always 6
	vertex[0] = Vertex_PCU(Vec3(corner[0].x, corner[0].y, 0.0f), color, Vec2(0.f, 0.f));
	vertex[1] = Vertex_PCU(Vec3(corner[2].x, corner[2].y, 0.0f), color, Vec2(0.f, 0.f));
	vertex[2] = Vertex_PCU(Vec3(corner[3].x, corner[3].y, 0.0f), color, Vec2(0.f, 0.f));
	vertex[3] = Vertex_PCU(Vec3(corner[2].x, corner[2].y, 0.0f), color, Vec2(0.f, 0.f));
	vertex[4] = Vertex_PCU(Vec3(corner[1].x, corner[1].y, 0.0f), color, Vec2(0.f, 0.f));
	vertex[5] = Vertex_PCU(Vec3(corner[0].x, corner[0].y, 0.0f), color, Vec2(0.f, 0.f));

	g_theRenderer->DrawVertexArray(6, vertex);
}

void DebugDrawRing(Vec2 center, float radius, Rgba8 color, float width)
{
	Vertex_PCU vertex[6 * RING_SEGMENTS]; // 2 triangles
	Vec2 corner[4]; // always 4 corners to a rhombus

	for (int segment = 0; segment < RING_SEGMENTS; segment++)
	{
		float theta = segmentAngle * static_cast<float>(segment);
		corner[0].x = center.x + CosDegrees(theta) * (radius + 0.5f * width);
		corner[0].y = center.y + SinDegrees(theta) * (radius + 0.5f * width);
		corner[1].x = center.x + CosDegrees(theta) * (radius - 0.5f * width);
		corner[1].y = center.y + SinDegrees(theta) * (radius - 0.5f * width);

		theta += segmentAngle; // move to next radial
		corner[2].x = center.x + CosDegrees(theta) * (radius + 0.5f * width);
		corner[2].y = center.y + SinDegrees(theta) * (radius + 0.5f * width);
		corner[3].x = center.x + CosDegrees(theta) * (radius - 0.5f * width);
		corner[3].y = center.y + SinDegrees(theta) * (radius - 0.5f * width);

		vertex[6 * segment + 0] = Vertex_PCU(Vec3(corner[0].x, corner[0].y, 0.0f), color, Vec2(0.f, 0.f));
		vertex[6 * segment + 1] = Vertex_PCU(Vec3(corner[2].x, corner[2].y, 0.0f), color, Vec2(0.f, 0.f));
		vertex[6 * segment + 2] = Vertex_PCU(Vec3(corner[3].x, corner[3].y, 0.0f), color, Vec2(0.f, 0.f));
		vertex[6 * segment + 3] = Vertex_PCU(Vec3(corner[2].x, corner[2].y, 0.0f), color, Vec2(0.f, 0.f));
		vertex[6 * segment + 4] = Vertex_PCU(Vec3(corner[1].x, corner[1].y, 0.0f), color, Vec2(0.f, 0.f));
		vertex[6 * segment + 5] = Vertex_PCU(Vec3(corner[0].x, corner[0].y, 0.0f), color, Vec2(0.f, 0.f));
	}
	
	g_theRenderer->DrawVertexArray(6 * RING_SEGMENTS, vertex);
}
