#pragma once
#include <vector>
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/OBB2.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Capsule2.hpp"
#include "Engine/Math/LineSegment2.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Vertex_PNCU.hpp"

void TransformVertexArrayXY3D( int numVerts, Vertex_PCU* verts, float uniformScaleXY, float rotationDegreesAboutZ, Vec2 const& translationXY );
void TransformVertexArrayXY3D( std::vector < Vertex_PCU>&, Mat44 transformMatrix );
Vec3 GetSurfaceNormal(Vec3 p0, Vec3 p1, Vec3 p2);

void AddVertsForRing2D(std::vector < Vertex_PCU>& verts, Vec2 center, float radius, Rgba8 color, float width, int segments);
void AddVertsForHalfDisc2D(std::vector < Vertex_PCU>& verts, Vec2 const& center, Vec2 const& centerNormal, float radius, Rgba8 const& color);
void AddVertsForCapsule2D(std::vector < Vertex_PCU>& verts, Capsule2 const& capsule, Rgba8 const& color);
void AddVertsForCapsule2D(std::vector < Vertex_PCU>& verts, Vec2 const& boneStart, Vec2 const& boneEnd, float radius, Rgba8 const& color);
void AddVertsForDisc2D(std::vector < Vertex_PCU>& verts, Vec2 const& center, float radius, Rgba8 const& color);
void AddVertsForAABB2D(std::vector < Vertex_PCU>& verts, AABB2 const& bounds, Rgba8 const& color, Vec2 const& uvAtMins = Vec2::ZERO, Vec2 const& uvAtMaxs = Vec2::ONE);
void AddVertsForOBB2D(std::vector < Vertex_PCU>& verts, OBB2 const& box, Rgba8 const& color);
void AddVertsForLineSegnent2D(std::vector < Vertex_PCU>& verts, Vec2 const& start, Vec2 const& end, float thickness, Rgba8 const& color);
void AddVertsForLineSegment2D(std::vector < Vertex_PCU>& verts, LineSegment2 const& lineSegment, float thickness, Rgba8 const& color);
void AddVertsForLineSegnent2DBlunt(std::vector < Vertex_PCU>& verts, Vec2 const& start, Vec2 const& end, float thickness, Rgba8 const& color);
void AddVertsForArrow2D( std::vector<Vertex_PCU>& verts, Vec2 tailPos, Vec2 tipPos, float arrowSize, float lineThickness, Rgba8 color );

void AddVertsForQuad3D(std::vector<Vertex_PCU>& verts, const Vec3& topLeft, const Vec3& bottomLeft, const Vec3& bottomRight, const Vec3& topRight, const Rgba8& color = Rgba8::WHITE, const AABB2& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForQuad3D(std::vector<unsigned int>& indexes, std::vector<Vertex_PCU>& verts, const Vec3& topLeft, const Vec3& bottomLeft, const Vec3& bottomRight, const Vec3& topRight, const Rgba8& color /*= Rgba8::WHITE*/, const AABB2& UVs /*= AABB2::ZERO_TO_ONE*/);
void AddVertsForRoundedQuad3D(std::vector<Vertex_PNCU>& vertexes, const Vec3& topLeft, const Vec3& bottomLeft, const Vec3& bottomRight, const Vec3& topRight, const Rgba8& color = Rgba8::WHITE, const AABB2& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForQuad3D(std::vector<Vertex_PNCU>& verts, const Vec3& topLeft, const Vec3& bottomLeft, const Vec3& bottomRight, const Vec3& topRight, const Rgba8& color = Rgba8::WHITE, const AABB2& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForQuad3D(std::vector<unsigned int>& indexes, std::vector<Vertex_PNCU>& vertexes, const Vec3& topLeft, const Vec3& bottomLeft, const Vec3& bottomRight, const Vec3& topRight, const Rgba8& color = Rgba8::WHITE, const AABB2& UVs = AABB2::ZERO_TO_ONE);

void AddVertsForAABB3D(std::vector<Vertex_PCU>& verts, const AABB3& bounds, const Rgba8& color = Rgba8::WHITE, const AABB2& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForCylinder3D(std::vector<Vertex_PCU>& verts, Vec3 const& position, EulerAngles const& orientation, float length, float radius, const Rgba8& color = Rgba8::WHITE, int edges = 16);
void AddVertsForCylinder3D(std::vector<Vertex_PCU>& verts, Vec3 const& bottomCenter, Vec3 const& topCenter, float radius = 0.1f, const Rgba8& color = Rgba8::WHITE, int edges = 16);
// void AddVertsForZCylinder(std::vector<Vertex_PCU>& verts, Vec3 const& position = Vec3::ZERO, float length = 1.0f, float radius = 0.5f, const Rgba8& color = Rgba8::WHITE, int edges = 16);
void AddVertsForSphere(std::vector<Vertex_PCU>& verts, Vec3 const& center = Vec3::ZERO, EulerAngles orientation = EulerAngles::ZERO, float radius = 0.5f, int slices = 16, int stacks = 8, const Rgba8& color = Rgba8::WHITE);
void AddVertsForCone(std::vector<Vertex_PCU>& verts, Vec3 const& position, EulerAngles const& orientation, float length, float radius, const Rgba8& color = Rgba8::WHITE, int edges = 16);
void AddVertsForCone(std::vector<Vertex_PCU>& verts, Vec3 const& bottomCenter, Vec3 const& topCenter, float radius = 0.2f, const Rgba8& color = Rgba8::WHITE, int edges = 16);
void AddVertsForArrow3D( std::vector<Vertex_PCU>& verts, Vec3 const& tailPos, Vec3 const& tipPos, float arrowSize, float lineThickness, Rgba8 const& color );
void AddVertsForLine3D(std::vector<Vertex_PCU>& verts, Vec3 const& tailPos, Vec3 const& tipPos, float lineThickness, Rgba8 const& color);