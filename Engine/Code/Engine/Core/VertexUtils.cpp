#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/Vertex_PCU.hpp"

#define _USE_MATH_DEFINES
#include <math.h>
#include <Engine/Math/MathUtils.hpp>
#include "EngineCommon.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/Vec3.hpp"
#include "../Math/Vec4.hpp"

void TransformVertexArrayXY3D(int numVerts, Vertex_PCU* verts, float uniformScaleXY, float rotationDegreesAboutZ, Vec2 const& translationXY)
{
	for (int vertIndex = 0; vertIndex < numVerts; vertIndex++)
	{
		TransformPositionXY3D(verts[vertIndex].m_position, uniformScaleXY, rotationDegreesAboutZ, translationXY);
	}
}

Vec3 GetSurfaceNormal(Vec3 p0, Vec3 p1, Vec3 p2)
{
	Vec3 u = (p1 - p0);
	Vec3 v = (p2 - p0);
	Vec3 surfaceNormal = CrossProduct3D(u, v).GetNormalized();
	return surfaceNormal;
}

void AddVertsForRing2D(std::vector < Vertex_PCU>& verts, Vec2 center, float radius, Rgba8 color, float width, int segments)
{
	float segmentAngle = 360.0f / static_cast<float>(segments);
	Vec2 corner[4]; // always 4 corners to a rhombus

	for (int segment = 0; segment < segments; segment++)
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

		verts.push_back(Vertex_PCU(Vec3(corner[0].x, corner[0].y, 0.0f), color, Vec2(0.f, 0.f)));
		verts.push_back(Vertex_PCU(Vec3(corner[2].x, corner[2].y, 0.0f), color, Vec2(0.f, 0.f)));
		verts.push_back(Vertex_PCU(Vec3(corner[3].x, corner[3].y, 0.0f), color, Vec2(0.f, 0.f)));
		verts.push_back(Vertex_PCU(Vec3(corner[2].x, corner[2].y, 0.0f), color, Vec2(0.f, 0.f)));
		verts.push_back(Vertex_PCU(Vec3(corner[1].x, corner[1].y, 0.0f), color, Vec2(0.f, 0.f)));
		verts.push_back(Vertex_PCU(Vec3(corner[0].x, corner[0].y, 0.0f), color, Vec2(0.f, 0.f)));
	}
}


void TransformVertexArray3D(std::vector < Vertex_PCU>& verts, Mat44 transformMatrix)
{
	for (int index = 0; index < verts.size(); index++)
	{
		verts[index].m_position = transformMatrix.TransformPosition3D(verts[index].m_position);
	}
}

//AddVerts functions
void AddVertsForHalfDisc2D(std::vector < Vertex_PCU>& verts, Vec2 const& center, Vec2 const& centerNormal, float radius, Rgba8 const& color)
{
	// builds a semi-circular disk of 9 triangles
	Vec2 lastPoint = center + centerNormal.GetRotatedMinus90Degrees() * radius;
	Vec2 nextPoint;
	Vertex_PCU vertex[3];

	for (int increment = 1; increment < 19; increment++)
	{
		nextPoint = (lastPoint - center).GetRotatedDegrees(10.0f) + center;
		if (increment == 18)
		{
			nextPoint = center + centerNormal.GetRotated90Degrees() * radius;
		}
		// CCW triangle
		vertex[0] = Vertex_PCU(Vec3(center.x, center.y, 0.0f), color, Vec2(0.f, 0.f));
		vertex[1] = Vertex_PCU(Vec3(lastPoint.x, lastPoint.y, 0.0f), color, Vec2(1.f, 1.f));
		vertex[2] = Vertex_PCU(Vec3(nextPoint.x, nextPoint.y, 0.0f), color, Vec2(0.f, 1.f));
		for (int vertIndex = 0; vertIndex < 3; vertIndex++)
		{
			verts.push_back(vertex[vertIndex]);
		}
		lastPoint = nextPoint;
	}
}

void AddVertsForCapsule2D(std::vector < Vertex_PCU>& verts, Capsule2 const& capsule, Rgba8 const& color)
{
	Vec2 center = (capsule.m_startPos + capsule.m_endPos) * 0.5f;
	Vec2 halfDimensions((capsule.m_endPos - capsule.m_startPos).GetLength() * 0.5f, capsule.m_radius);
	Vec2 normalVector(capsule.m_endPos - capsule.m_startPos);
	normalVector.Normalize();
	OBB2 bone(center, normalVector, halfDimensions);
	AddVertsForOBB2D(verts, bone, color);

	// add the end semi-circle caps
	AddVertsForHalfDisc2D(verts, capsule.m_startPos, Vec2(capsule.m_startPos - capsule.m_endPos).GetNormalized(), capsule.m_radius, color);
	AddVertsForHalfDisc2D(verts, capsule.m_endPos, Vec2(capsule.m_endPos - capsule.m_startPos).GetNormalized(), capsule.m_radius, color);
}

void AddVertsForCapsule2D(std::vector < Vertex_PCU>& verts, Vec2 const& boneStart, Vec2 const& boneEnd, float radius, Rgba8 const& color)
{
	AddVertsForCapsule2D(verts, Capsule2(boneStart, boneEnd, radius), color);
}

void AddVertsForDisc2D(std::vector < Vertex_PCU>& verts, Vec2 const& center, float radius, Rgba8 const& color)
{
	// lets be really lazy since we already did half the work
	AddVertsForHalfDisc2D(verts, center, Vec2(1.0f, 0.0f), radius, color);
	AddVertsForHalfDisc2D(verts, center, Vec2(-1.0f, 0.0f), radius, color);
}

void AddVertsForAABB2D(std::vector < Vertex_PCU>& verts, AABB2 const& bounds, Rgba8 const& color, Vec2 const& uvAtMins, Vec2 const& uvAtMaxs)
{
	Vertex_PCU vertex[6];
	// upper triangle
	vertex[0] = Vertex_PCU(Vec3(bounds.m_mins.x, bounds.m_mins.y, 0.0f), color, uvAtMins);
	vertex[1] = Vertex_PCU(Vec3(bounds.m_maxs.x, bounds.m_maxs.y, 0.0f), color, uvAtMaxs);
	vertex[2] = Vertex_PCU(Vec3(bounds.m_mins.x, bounds.m_maxs.y, 0.0f), color, Vec2(uvAtMins.x, uvAtMaxs.y));
	// lower triangle
	vertex[3] = Vertex_PCU(Vec3(bounds.m_mins.x, bounds.m_mins.y, 0.0f), color, uvAtMins);
	vertex[4] = Vertex_PCU(Vec3(bounds.m_maxs.x, bounds.m_mins.y, 0.0f), color, Vec2(uvAtMaxs.x, uvAtMins.y));
	vertex[5] = Vertex_PCU(Vec3(bounds.m_maxs.x, bounds.m_maxs.y, 0.0f), color, uvAtMaxs);

	for (int vertIndex = 0; vertIndex < 6; vertIndex++)
	{
		verts.push_back(vertex[vertIndex]);
	}
}

void AddVertsForOBB2D(std::vector < Vertex_PCU>& verts, OBB2 const& box, Rgba8 const& color)
{
	Vertex_PCU vertex[6];
	Vec2 c[4];
	box.GetCornerPoints(c); // topRight, topLeft, bottomLeft, bottomRight ordering

	vertex[0] = Vertex_PCU(Vec3(c[2].x, c[2].y, 0.0f), color, Vec2(0.f, 0.f));
	vertex[1] = Vertex_PCU(Vec3(c[0].x, c[0].y, 0.0f), color, Vec2(1.f, 1.f));
	vertex[2] = Vertex_PCU(Vec3(c[1].x, c[1].y, 0.0f), color, Vec2(0.f, 1.f));
	// lower triangle
	vertex[3] = Vertex_PCU(Vec3(c[2].x, c[2].y, 0.0f), color, Vec2(0.f, 0.f));
	vertex[4] = Vertex_PCU(Vec3(c[3].x, c[3].y, 0.0f), color, Vec2(1.f, 0.f));
	vertex[5] = Vertex_PCU(Vec3(c[0].x, c[0].y, 0.0f), color, Vec2(1.f, 1.f));

	for (int vertIndex = 0; vertIndex < 6; vertIndex++)
	{
		verts.push_back(vertex[vertIndex]);
	}
}

void AddVertsForLineSegnent2D(std::vector < Vertex_PCU>& verts, Vec2 const& start, Vec2 const& end, float thickness, Rgba8 const& color)
{
	// why treat a line as an overlapping box when we can make it a great overlapping capsule?
	AddVertsForCapsule2D(verts, start, end, thickness * 0.5f, color);
}

void AddVertsForLineSegment2D(std::vector < Vertex_PCU>& verts, LineSegment2 const& lineSegment, float thickness, Rgba8 const& color)
{
	// why treat a line as an overlapping box when we can make it a great overlapping capsule?
	AddVertsForCapsule2D(verts, lineSegment.m_startPos, lineSegment.m_endPos, thickness * 0.5f, color);
}

void AddVertsForLineSegnent2DBlunt(std::vector < Vertex_PCU>& verts, Vec2 const& start, Vec2 const& end, float thickness, Rgba8 const& color)
{
	Vec2 displacement = end - start;
	displacement.Normalize();
	displacement *= (0.5f * thickness);

	Vec2 corner[4]; // always 4 corners to a line
// 	corner[0] = end + (displacement + displacement.GetRotated90Degrees());
// 	corner[1] = end + (displacement + displacement.GetRotatedMinus90Degrees());
// 	corner[2] = start - (displacement + displacement.GetRotated90Degrees());
// 	corner[3] = start - (displacement + displacement.GetRotatedMinus90Degrees());

	corner[0] = end + (displacement.GetRotated90Degrees());
	corner[1] = end + (displacement.GetRotatedMinus90Degrees());
	corner[2] = start - (displacement.GetRotated90Degrees());
	corner[3] = start - (displacement.GetRotatedMinus90Degrees());

	Vertex_PCU vertex[6]; // lines are always 6
	vertex[0] = Vertex_PCU(Vec3(corner[0].x, corner[0].y, 0.0f), color, Vec2(0.f, 0.f));
	vertex[1] = Vertex_PCU(Vec3(corner[2].x, corner[2].y, 0.0f), color, Vec2(0.f, 0.f));
	vertex[2] = Vertex_PCU(Vec3(corner[3].x, corner[3].y, 0.0f), color, Vec2(0.f, 0.f));
	vertex[3] = Vertex_PCU(Vec3(corner[2].x, corner[2].y, 0.0f), color, Vec2(0.f, 0.f));
	vertex[4] = Vertex_PCU(Vec3(corner[1].x, corner[1].y, 0.0f), color, Vec2(0.f, 0.f));
	vertex[5] = Vertex_PCU(Vec3(corner[0].x, corner[0].y, 0.0f), color, Vec2(0.f, 0.f));

	for (int vertIndex = 0; vertIndex < 6; vertIndex++)
	{
		verts.push_back(vertex[vertIndex]);
	}
}

void AddVertsForArrow2D(std::vector<Vertex_PCU>& verts, Vec2 tailPos, Vec2 tipPos, float arrowSize, float lineThickness, Rgba8 color)
{
	Vec2 arrowNormal = (tipPos - tailPos).GetNormalized();
	Vec2 arrowBase = tipPos - arrowNormal * arrowSize;
	AddVertsForLineSegnent2DBlunt(verts, tailPos, arrowBase, lineThickness, color);	
	verts.push_back(Vertex_PCU(Vec3(tipPos.x, tipPos.y, 0.0f), color, Vec2(0.f, 0.f)));
	Vec2 arrowSide = arrowBase + arrowNormal.GetRotated90Degrees() * 0.5 * arrowSize;
	verts.push_back(Vertex_PCU(Vec3(arrowSide.x, arrowSide.y, 0.0f), color, Vec2(0.f, 0.f)));
	arrowSide = arrowBase + arrowNormal.GetRotatedMinus90Degrees() * 0.5 * arrowSize;
	verts.push_back(Vertex_PCU(Vec3(arrowSide.x, arrowSide.y, 0.0f), color, Vec2(0.f, 0.f)));
}

void AddVertsForQuad3D(std::vector<Vertex_PCU>& verts, const Vec3& topLeft, const Vec3& bottomLeft, const Vec3& bottomRight, const Vec3& topRight, const Rgba8& color /*= Rgba8::WHITE*/, const AABB2& UVs /*= AABB2::ZERO_TO_ONE*/)
{
	verts.push_back(Vertex_PCU(topLeft, color, Vec2(UVs.m_mins.x, UVs.m_maxs.y)));
	verts.push_back(Vertex_PCU(bottomRight, color, Vec2(UVs.m_maxs.x, UVs.m_mins.y)));
	verts.push_back(Vertex_PCU(topRight, color, Vec2(UVs.m_maxs.x, UVs.m_maxs.y)));

	verts.push_back(Vertex_PCU(topLeft, color, Vec2(UVs.m_mins.x, UVs.m_maxs.y)));
	verts.push_back(Vertex_PCU(bottomLeft, color, Vec2(UVs.m_mins.x, UVs.m_mins.y)));
	verts.push_back(Vertex_PCU(bottomRight, color, Vec2(UVs.m_maxs.x, UVs.m_mins.y)));
}

void AddVertsForQuad3D(std::vector<unsigned int>& indexes, std::vector<Vertex_PCU>& verts, const Vec3& topLeft, const Vec3& bottomLeft, const Vec3& bottomRight, const Vec3& topRight, const Rgba8& color /*= Rgba8::WHITE*/, const AABB2& UVs /*= AABB2::ZERO_TO_ONE*/)
{
	unsigned int vertexBase = static_cast<unsigned int>(verts.size());

	verts.push_back(Vertex_PCU(topLeft, color, Vec2(UVs.m_mins.x, UVs.m_maxs.y)));
	verts.push_back(Vertex_PCU(bottomLeft, color, Vec2(UVs.m_mins.x, UVs.m_mins.y)));
	verts.push_back(Vertex_PCU(bottomRight, color, Vec2(UVs.m_maxs.x, UVs.m_mins.y)));
	verts.push_back(Vertex_PCU(topRight, color, Vec2(UVs.m_maxs.x, UVs.m_maxs.y)));

	indexes.push_back(vertexBase);
	indexes.push_back(vertexBase + 1);
	indexes.push_back(vertexBase + 2);

	indexes.push_back(vertexBase);
	indexes.push_back(vertexBase + 2);
	indexes.push_back(vertexBase + 3);
}

void AddVertsForQuad3D(std::vector<Vertex_PNCU>& verts, const Vec3& topLeft, const Vec3& bottomLeft, const Vec3& bottomRight, const Vec3& topRight, const Rgba8& color /*= Rgba8::WHITE*/, const AABB2& UVs /*= AABB2::ZERO_TO_ONE*/)
{
	Vec3 surfaceNormal = GetSurfaceNormal(topLeft, bottomLeft, topRight);

	verts.push_back(Vertex_PNCU(topLeft, surfaceNormal, color, Vec2(UVs.m_mins.x, UVs.m_maxs.y)));
	verts.push_back(Vertex_PNCU(bottomRight, surfaceNormal, color, Vec2(UVs.m_maxs.x, UVs.m_mins.y)));
	verts.push_back(Vertex_PNCU(topRight, surfaceNormal, color, Vec2(UVs.m_maxs.x, UVs.m_maxs.y)));
							
	verts.push_back(Vertex_PNCU(topLeft, surfaceNormal, color, Vec2(UVs.m_mins.x, UVs.m_maxs.y)));
	verts.push_back(Vertex_PNCU(bottomLeft, surfaceNormal, color, Vec2(UVs.m_mins.x, UVs.m_mins.y)));
	verts.push_back(Vertex_PNCU(bottomRight, surfaceNormal, color, Vec2(UVs.m_maxs.x, UVs.m_mins.y)));
}

void AddVertsForRoundedQuad3D(std::vector<Vertex_PNCU>& verts, const Vec3& topLeft, const Vec3& bottomLeft, const Vec3& bottomRight, const Vec3& topRight, const Rgba8& color /*= Rgba8::WHITE*/, const AABB2& UVs /*= AABB2::ZERO_TO_ONE*/)
{
	 Vec3 topCenter = (topLeft + topRight) * 0.5f;
	 Vec3 bottomCenter = (bottomLeft + bottomRight) * 0.5f;
	 Vec3 left = (topLeft - topCenter).GetNormalized();
	 Vec3 right = (topRight - topCenter).GetNormalized();
	 Vec3 surfaceNormal = GetSurfaceNormal(topLeft, bottomLeft, topRight);
	 float centerX = (UVs.m_maxs.x + UVs.m_mins.x) * 0.5f;

	 // left half
	 verts.push_back(Vertex_PNCU(topLeft, left, color, Vec2(UVs.m_mins.x, UVs.m_maxs.y)));
	 verts.push_back(Vertex_PNCU(bottomCenter, surfaceNormal, color, Vec2(centerX, UVs.m_mins.y)));
	 verts.push_back(Vertex_PNCU(topCenter, surfaceNormal, color, Vec2(centerX, UVs.m_maxs.y)));

	 verts.push_back(Vertex_PNCU(topLeft, left, color, Vec2(UVs.m_mins.x, UVs.m_maxs.y)));
	 verts.push_back(Vertex_PNCU(bottomLeft, left, color, Vec2(UVs.m_mins.x, UVs.m_mins.y)));
	 verts.push_back(Vertex_PNCU(bottomCenter, surfaceNormal, color, Vec2(centerX, UVs.m_mins.y)));

	 // right half
	 verts.push_back(Vertex_PNCU(topCenter, surfaceNormal, color, Vec2(centerX, UVs.m_maxs.y)));
	 verts.push_back(Vertex_PNCU(bottomRight, right, color, Vec2(UVs.m_maxs.x, UVs.m_mins.y)));
	 verts.push_back(Vertex_PNCU(topRight, right, color, Vec2(UVs.m_maxs.x, UVs.m_maxs.y)));

	 verts.push_back(Vertex_PNCU(topCenter, surfaceNormal, color, Vec2(centerX, UVs.m_maxs.y)));
	 verts.push_back(Vertex_PNCU(bottomCenter, surfaceNormal, color, Vec2(centerX, UVs.m_mins.y)));
	 verts.push_back(Vertex_PNCU(bottomRight, right, color, Vec2(UVs.m_maxs.x, UVs.m_mins.y)));
}

void AddVertsForQuad3D(std::vector<unsigned int>& indexes, std::vector<Vertex_PNCU>& verts, const Vec3& topLeft, const Vec3& bottomLeft, const Vec3& bottomRight, const Vec3& topRight, const Rgba8& color /*= Rgba8::WHITE*/, const AABB2& UVs /*= AABB2::ZERO_TO_ONE*/)
{
	unsigned int vertexBase = static_cast<unsigned int>(verts.size());

	Vec3 surfaceNormal = GetSurfaceNormal(topLeft, bottomLeft, topRight);

	verts.push_back(Vertex_PNCU(topLeft, surfaceNormal, color, Vec2(UVs.m_mins.x, UVs.m_maxs.y)));
	verts.push_back(Vertex_PNCU(bottomLeft, surfaceNormal, color, Vec2(UVs.m_mins.x, UVs.m_mins.y)));
	verts.push_back(Vertex_PNCU(bottomRight, surfaceNormal, color, Vec2(UVs.m_maxs.x, UVs.m_mins.y)));
	verts.push_back(Vertex_PNCU(topRight, surfaceNormal, color, Vec2(UVs.m_maxs.x, UVs.m_maxs.y)));

	indexes.push_back(vertexBase);
	indexes.push_back(vertexBase + 1);
	indexes.push_back(vertexBase + 2);

	indexes.push_back(vertexBase);
	indexes.push_back(vertexBase + 2);
	indexes.push_back(vertexBase + 3);
}

void AddVertsForAABB3D(std::vector<Vertex_PCU>& verts, const AABB3& bounds, const Rgba8& color /*= Rgba8::WHITE*/, const AABB2& UVs /*= AABB2::ZERO_TO_ONE*/)
{
	Vec3 c000(bounds.m_mins.x, bounds.m_mins.y, bounds.m_mins.z);
	Vec3 c001(bounds.m_mins.x, bounds.m_mins.y, bounds.m_maxs.z);
	Vec3 c010(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_mins.z);
	Vec3 c011(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_maxs.z);
	Vec3 c100(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_mins.z);
	Vec3 c101(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_maxs.z);
	Vec3 c110(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_mins.z);
	Vec3 c111(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_maxs.z);

	AddVertsForQuad3D(verts, c001, c000, c100, c101, color, UVs);
	AddVertsForQuad3D(verts, c101, c100, c110, c111, color, UVs);
	AddVertsForQuad3D(verts, c111, c110, c010, c011, color, UVs);
	AddVertsForQuad3D(verts, c011, c010, c000, c001, color, UVs);
	AddVertsForQuad3D(verts, c011, c001, c101, c111, color, UVs);
	AddVertsForQuad3D(verts, c000, c010, c110, c100, color, UVs);
}

void AddVertsForCylinder3D(std::vector<Vertex_PCU>& verts, Vec3 const& position, EulerAngles const& orientation, float length, float radius, const Rgba8& color /*= Rgba8::WHITE*/, int edges /*= 16*/)
{
	float halfLength = length * 0.5f; // use half the length for centered on origin
	Vec2 uvs = Vec2(0.0f, 0.0f);
	float incDeg = 360.0f / static_cast<float>(edges);

	float cAngDeg = 0.0f;
	float nAngDeg = incDeg;
	float c0 = CosDegrees(cAngDeg);
	float s0 = SinDegrees(cAngDeg);
	float c1 = CosDegrees(nAngDeg);
	float s1 = SinDegrees(nAngDeg);
	std::vector<Vertex_PCU> localVerts;

	for (int index = 0; index < edges; index++)
	{
		// create corner vertices for quad3
		Vec3 ul(c0 * radius, s0 * radius, halfLength);
		Vec3 ll(c0 * radius, s0 * radius, -halfLength);
		Vec3 lr(c1 * radius, s1 * radius, -halfLength);
		Vec3 ur(c1 * radius, s1 * radius, halfLength);

		// create matching uv vertices for quad3 assuming wrapping texture
		float llu = RangeMapClamped(c0, -1.0f, 1.0f, 0.0f, 1.0f);
		float llv = RangeMapClamped(s0, -1.0f, 1.0f, 0.0f, 1.0f);
		float uru = RangeMapClamped(c1, -1.0f, 1.0f, 0.0f, 1.0f);
		float urv = RangeMapClamped(s1, -1.0f, 1.0f, 0.0f, 1.0f);

		// create top and bottom triangles facing outward from cylinder
		localVerts.push_back(Vertex_PCU(Vec3(0.0f, 0.0f, halfLength), color, Vec2(0.5f, 0.5f)));
		localVerts.push_back(Vertex_PCU(ul, color, Vec2(llu, llv)));
		localVerts.push_back(Vertex_PCU(ur, color, Vec2(uru, urv)));
	
		localVerts.push_back(Vertex_PCU(Vec3(0.0f, 0.0f, -halfLength), color, Vec2(0.5f, 0.5f)));
		localVerts.push_back(Vertex_PCU(lr, color, Vec2(uru, 1.0f - urv)));
		localVerts.push_back(Vertex_PCU(ll, color, Vec2(llu, 1.0f - llv)));

		float ub = RangeMapClamped(cAngDeg, 0.0f, 360.0f, -0.5f, 0.5f);
		float ut = RangeMapClamped(nAngDeg, 0.0f, 360.0f, -0.5f, 0.5f); 
		AddVertsForQuad3D(localVerts, ul, ll, lr, ur, color, AABB2(Vec2(ub, 0.0f), Vec2(ut, 1.0f)));

		cAngDeg = nAngDeg;
		c0 = c1;
		s0 = s1;
		nAngDeg += incDeg;
		c1 = CosDegrees(nAngDeg);
		s1 = SinDegrees(nAngDeg);
	}

	// model matrix
	Mat44 orient = orientation.GetAsMatrix_XFwd_YLeft_ZUp();
	orient.Orthonormalize_XFwd_YLeft_ZUp();
	Mat44 trans;
	trans.SetTranslation3D(position);
	trans.Append(orient);
	TransformVertexArray3D(localVerts, trans);
	for (int index = 0; index < localVerts.size(); index++)
	{
		verts.push_back(localVerts[index]);
	}
}

void AddVertsForCylinder3D(std::vector<Vertex_PCU>& verts, Vec3 const& bottomCenter, Vec3 const& topCenter, float radius /*= 0.1f*/, const Rgba8& color /*= Rgba8::WHITE*/, int edges /*= 16*/)
{
	float length = Vec3(topCenter - bottomCenter).GetLength();
	Vec3 position = bottomCenter + (topCenter - bottomCenter) * 0.5f;
	EulerAngles orientation(topCenter - bottomCenter);
	orientation.m_pitchDegrees += 90.0f; // the cylinder is being constructed parallel to the z-axis, so we need to pitch it forward along the x-axis
	AddVertsForCylinder3D(verts, position, orientation, length, radius, color, edges);
}

// void AddVertsForZCylinder(std::vector<Vertex_PCU>& verts, Vec3 const& position /*= Vec3::ZERO*/, float length /*= 1.0f*/, float radius /*= 0.5f*/, const Rgba8& color /*= Rgba8::WHITE*/, int edges /*= 16*/)
// {
// 	UNUSED(position);
// 	float halfLength = length * 0.5f; // use half the length for centered on origin box
// 	Vec2 uvs = Vec2(0.0f, 0.0f);
// 	float incDeg = 360.0f / static_cast<float>(edges);
// 
// 	float cAngDeg = 0.0f;
// 	float nAngDeg = incDeg;
// 	float c0 = CosDegrees(cAngDeg);
// 	float s0 = SinDegrees(cAngDeg);
// 	float c1 = CosDegrees(nAngDeg);
// 	float s1 = SinDegrees(nAngDeg);
// 
// 	for (int index = 0; index < edges; index++)
// 	{
// 		verts.push_back(Vertex_PCU(Vec3(0.0f, 0.0f, halfLength), Rgba8::WHITE, Vec2(0.0f, 0.0f)));
// 		verts.push_back(Vertex_PCU(Vec3(c0 * radius, s0 * radius, halfLength), color, uvs));
// 		verts.push_back(Vertex_PCU(Vec3(c1 * radius, s1 * radius, halfLength), color, uvs));
// 		
// 		verts.push_back(Vertex_PCU(Vec3(0.0f, 0.0f, -halfLength), Rgba8::WHITE, Vec2(0.0f, 0.0f)));
// 		verts.push_back(Vertex_PCU(Vec3(c1 * radius, s1 * radius, -halfLength), color, uvs));
// 		verts.push_back(Vertex_PCU(Vec3(c0 * radius, s0 * radius, -halfLength), color, uvs));
// 		
// 		verts.push_back(Vertex_PCU(Vec3(c0 * radius, s0 * radius, halfLength), color, uvs));
// 		verts.push_back(Vertex_PCU(Vec3(c0 * radius, s0 * radius, -halfLength), color, uvs));
// 		verts.push_back(Vertex_PCU(Vec3(c1 * radius, s1 * radius, -halfLength), color, uvs));
// 		
// 		verts.push_back(Vertex_PCU(Vec3(c0 * radius, s0 * radius, halfLength), color, uvs));
// 		verts.push_back(Vertex_PCU(Vec3(c1 * radius, s1 * radius, -halfLength), color, uvs));
// 		verts.push_back(Vertex_PCU(Vec3(c1 * radius, s1 * radius, halfLength), color, uvs));
// 
// 		cAngDeg = nAngDeg; // unused
// 		c0 = c1;
// 		s0 = s1;
// 		nAngDeg += incDeg;
// 		Clamp(nAngDeg, 0.0f, 360.0f);
// 		c1 = CosDegrees(nAngDeg);
// 		s1 = SinDegrees(nAngDeg);
// 	}
// }


// void AddVertsForSphere(std::vector<Vertex_PCU>& verts, Vec3 const& center, float radius, int slices, int stacks, const Rgba8& color)
// {
// 	UNUSED(center); // centered on the origin right now
// 	float uIncrement = 1.0f / static_cast<float>(slices);
// 	float vIncrement = 1.0f / static_cast<float>(stacks);
// 	float sliceDegrees = 360.0f * uIncrement;
// 	float stackDegrees = 180.0f * vIncrement;
// 
// 	float longAngle = 0.0f;
// 	float latAngle = -90.f;
// 	float uCoord = 0.0f;
// 	float vCoord = 0.0f;
// 
// 	Vertex_PCU quad[4];
// 	for (int index = 0; index < 4; index++)
// 	{
// 		quad[index].m_color = color;
// 	}
// 
// 	for (int slice = 0; slice < slices; slice++)
// 	{
// 		float c0yaw = CosDegrees(longAngle);
// 		float c1yaw = CosDegrees(longAngle + sliceDegrees);
// 		float s0yaw = SinDegrees(longAngle);
// 		float s1yaw = SinDegrees(longAngle + sliceDegrees);
// 		vCoord = 0.0f; // reset every loop
// 		latAngle = -90.f;
// 
// 		for (int stack = 0; stack < stacks; stack++)
// 		{
// 			// construct the quad for the patch as CCW
// 			float c0pitch = CosDegrees(latAngle);
// 			float c1pitch = CosDegrees(latAngle + stackDegrees);
// 			float s0pitch = SinDegrees(latAngle);
// 			float s1pitch = SinDegrees(latAngle + stackDegrees);
// 
// 			quad[0].m_position = Vec3(c0pitch * c0yaw, c0pitch * s0yaw, -s0pitch); // lower left
// 			quad[1].m_position = Vec3(c0pitch * c1yaw, c0pitch * s1yaw, -s0pitch); // lower right
// 			quad[2].m_position = Vec3(c1pitch * c1yaw, c1pitch * s1yaw, -s1pitch); // upper right
// 			quad[3].m_position = Vec3(c1pitch * c0yaw, c1pitch * s0yaw, -s1pitch); // upper left
// 			for (int index = 0; index < 4; index++)
// 			{
// 				quad[index].m_position.ClampAll(-1.0f, 1.0f);
// 				quad[index].m_position.x *= radius;
// 				quad[index].m_position.y *= radius;
// 				quad[index].m_position.z *= radius;
// 			}
// 
// 			quad[0].m_uvTexCoords = Vec2(uCoord, vCoord);
// 			quad[1].m_uvTexCoords = Vec2(uCoord + uIncrement, vCoord);
// 			quad[2].m_uvTexCoords = Vec2(uCoord + uIncrement, vCoord + vIncrement);
// 			quad[3].m_uvTexCoords = Vec2(uCoord, vCoord + vIncrement);
// 
// 			// push the CCW triangles for the quad
// 			//			AddVertsForQuad3D(localVerts, ul, ll, lr, ur, color, AABB2(Vec2(ub, 0.0f), Vec2(ut, 1.0f)));
// 			verts.push_back(quad[0]);
// 			verts.push_back(quad[1]);
// 			verts.push_back(quad[2]);
// 
// 			verts.push_back(quad[0]);
// 			verts.push_back(quad[2]);
// 			verts.push_back(quad[3]);
// 
// 			latAngle += stackDegrees;
// 			vCoord += vIncrement;
// 		}
// 		longAngle += sliceDegrees;
// 		uCoord += uIncrement;
// 	}
// }

void AddVertsForSphere(std::vector<Vertex_PCU>& verts, Vec3 const& center, EulerAngles orientation, float radius, int slices, int stacks, const Rgba8& color)
{
	std::vector<Vertex_PCU> localVerts;
	float uIncrement = 1.0f / static_cast<float>(slices);
	float vIncrement = 1.0f / static_cast<float>(stacks);
	float sliceDegrees = 360.0f * uIncrement;
	float stackDegrees = 180.0f * vIncrement;

	float longAngle = 0.0f;
	float latAngle = -90.f;
	float uCoord = 0.0f;
	float vCoord = 0.0f;

	Vec3 lr;
	Vec3 ur;
	Vec3 ll;
	Vec3 ul;

	for (int slice = 0; slice < slices; slice++)
	{
		float c0yaw = CosDegrees(longAngle);
		float c1yaw = CosDegrees(longAngle + sliceDegrees);
		float s0yaw = SinDegrees(longAngle);
		float s1yaw = SinDegrees(longAngle + sliceDegrees);
		vCoord = 0.0f; // reset every loop
		latAngle = -90.f;

		for (int stack = 0; stack < stacks; stack++)
		{
			// construct the quad for the patch as CCW
			float c0pitch = CosDegrees(latAngle);
			float c1pitch = CosDegrees(latAngle + stackDegrees);
			float s0pitch = SinDegrees(latAngle);
			float s1pitch = SinDegrees(latAngle + stackDegrees);

			ll = Vec3(c0pitch * c0yaw * radius, c0pitch * s0yaw * radius, s0pitch * radius); // lower left
			lr = Vec3(c0pitch * c1yaw * radius, c0pitch * s1yaw * radius, s0pitch * radius); // lower right
			ur = Vec3(c1pitch * c1yaw * radius, c1pitch * s1yaw * radius, s1pitch * radius); // upper right
			ul = Vec3(c1pitch * c0yaw * radius, c1pitch * s0yaw * radius, s1pitch * radius); // upper left

			// push the CCW triangles for the quad
			AddVertsForQuad3D(localVerts, ul, ll, lr, ur, color, AABB2(Vec2(uCoord, vCoord), Vec2(uCoord + uIncrement, vCoord + vIncrement)));

			latAngle += stackDegrees;
			vCoord += vIncrement;
		}
		longAngle += sliceDegrees;
		uCoord += uIncrement;
	}

	Mat44 orient = orientation.GetAsMatrix_XFwd_YLeft_ZUp();
	orient.Orthonormalize_XFwd_YLeft_ZUp();
	Mat44 trans;
	trans.SetTranslation3D(center);
	trans.Append(orient);
	TransformVertexArray3D(localVerts, trans);
	for (int index = 0; index < localVerts.size(); index++)
	{
		verts.push_back(localVerts[index]);
	}
}

void AddVertsForCone(std::vector<Vertex_PCU>& verts, Vec3 const& bottomCenter, Vec3 const& topCenter, float radius /*= 0.2f*/, const Rgba8& color /*= Rgba8::WHITE*/, int edges /*= 16*/)
{
	float length = Vec3(topCenter - bottomCenter).GetLength();
	Vec3 position = bottomCenter + (topCenter - bottomCenter) * 0.5f;
	EulerAngles orientation(topCenter - bottomCenter);
	orientation.m_pitchDegrees += 90.0f; // Z pitch fix
	AddVertsForCone(verts, position, orientation, length, radius, color, edges);
}

void AddVertsForCone(std::vector<Vertex_PCU>& verts, Vec3 const& position, EulerAngles const& orientation, float length, float radius, const Rgba8& color /*= Rgba8::WHITE*/, int edges /*= 16*/)
{
	float halfLength = length * 0.5f; // use half the length for centered on origin
	Vec2 uvs = Vec2(0.0f, 0.0f);
	float incDeg = 360.0f / static_cast<float>(edges);

	float cAngDeg = 0.0f;
	float nAngDeg = incDeg;
	float c0 = CosDegrees(cAngDeg);
	float s0 = SinDegrees(cAngDeg);
	float c1 = CosDegrees(nAngDeg);
	float s1 = SinDegrees(nAngDeg);
	std::vector<Vertex_PCU> localVerts;

	for (int index = 0; index < edges; index++)
	{
		// create corner vertices for quad3
		Vec3 ul(c0 * radius, s0 * radius, halfLength);
		Vec3 ll(c0 * radius, s0 * radius, -halfLength);
		Vec3 lr(c1 * radius, s1 * radius, -halfLength);
		Vec3 ur(c1 * radius, s1 * radius, halfLength);

		// create matching uv vertices for quad3 assuming wrapping texture
		float llu = RangeMapClamped(c0, -1.0f, 1.0f, 0.0f, 1.0f);
		float llv = RangeMapClamped(s0, -1.0f, 1.0f, 0.0f, 1.0f);
		float uru = RangeMapClamped(c1, -1.0f, 1.0f, 0.0f, 1.0f);
		float urv = RangeMapClamped(s1, -1.0f, 1.0f, 0.0f, 1.0f);

		// create top and bottom triangles facing outward from cylinder
		localVerts.push_back(Vertex_PCU(Vec3(0.0f, 0.0f, halfLength), color, Vec2(0.5f, 0.5f)));
		localVerts.push_back(Vertex_PCU(ll, color, Vec2(llu, llv)));
		localVerts.push_back(Vertex_PCU(lr, color, Vec2(uru, urv)));

		localVerts.push_back(Vertex_PCU(Vec3(0.0f, 0.0f, -halfLength), color, Vec2(0.5f, 0.5f)));
		localVerts.push_back(Vertex_PCU(lr, color, Vec2(uru, 1.0f - urv)));
		localVerts.push_back(Vertex_PCU(ll, color, Vec2(llu, 1.0f - llv)));

// 		float ub = RangeMapClamped(cAngDeg, 0.0f, 360.0f, -0.5f, 0.5f);
// 		float ut = RangeMapClamped(nAngDeg, 0.0f, 360.0f, -0.5f, 0.5f);
// 		AddVertsForQuad3D(localVerts, ul, ll, lr, ur, color, AABB2(Vec2(ub, 0.0f), Vec2(ut, 1.0f)));

		cAngDeg = nAngDeg;
		c0 = c1;
		s0 = s1;
		nAngDeg += incDeg;
		c1 = CosDegrees(nAngDeg);
		s1 = SinDegrees(nAngDeg);
	}

	// model matrix
	Mat44 orient = orientation.GetAsMatrix_XFwd_YLeft_ZUp();
	orient.Orthonormalize_XFwd_YLeft_ZUp();
	Mat44 trans;
	trans.SetTranslation3D(position);
	trans.Append(orient);
	TransformVertexArray3D(localVerts, trans);
	for (int index = 0; index < localVerts.size(); index++)
	{
		verts.push_back(localVerts[index]);
	}
}

void AddVertsForArrow3D(std::vector<Vertex_PCU>& verts, Vec3 const& tailPos, Vec3 const& tipPos, float arrowSize, float lineThickness, Rgba8 const& color)
{
	Vec3 EtoS(tipPos - tailPos);
	float ESlength = EtoS.GetLength();
	float ESinverse = 1.0f / ESlength;
	float shaft;
	float arrow;
	if (ESlength < 2.0f  * arrowSize)
	{
		shaft = ESlength * 0.5f; // half length allocated to shaft and arrow head
		arrow = ESlength * 0.5f;
	}
	else
	{
		shaft = ESlength - arrowSize; // shaft + 0.5 unit arrow head
		arrow = arrowSize;
	}

	EulerAngles orientation(EtoS);
	orientation.m_pitchDegrees += 90.0f; // Z pitch fix
	AddVertsForCylinder3D(verts, tailPos + EtoS * (shaft * ESinverse * 0.5f), orientation, shaft, lineThickness, color);
	AddVertsForCone(verts, tailPos + EtoS * shaft * ESinverse, tipPos, 2.0f * lineThickness, color);
}

void AddVertsForLine3D(std::vector<Vertex_PCU>& verts, Vec3 const& tailPos, Vec3 const& tipPos, float lineThickness, Rgba8 const& color)
{
	Vec3 EtoS(tipPos - tailPos);
	float ESlength = EtoS.GetLength();
	EulerAngles orientation(EtoS);
	orientation.m_pitchDegrees += 90.0f; // Z pitch fix
	AddVertsForCylinder3D(verts, tailPos + EtoS * 0.5f, orientation, ESlength, lineThickness, color);
}