#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include <string>
#include <vector>

struct AABB3
{
public:
	static AABB3 const UNIT_BOX;
	static AABB3 const ZERO_TO_ONE;

	Vec3 m_mins;
	Vec3 m_maxs;

	AABB3();
	~AABB3();
	AABB3( AABB3 const& copyFrom );
	explicit AABB3( Vec3 const& mins, Vec3 const& maxs );
	explicit AABB3( float minx, float miny, float minz, float maxx, float maxy, float maxz );

	// Accessors
	bool IsPointInside(Vec3 const point) const;
	Vec3 GetCenter() const;
	Vec3 GetDimensions() const;
	Vec3 GetNearestPoint( Vec3 const& referencePoint ) const;
	Vec2 GetPointAtUV( Vec2 const& uv)  const;
	Vec2 GetUVForPoint( Vec2 const& point ) const;
	std::string ToString();
	void DrawHollow();

	// Mutators
	void Translate( Vec3 const& translation );
	void SetCenter( Vec3 const& newCenter );
	void SetDimensions( Vec3 const& newDimensions );
	void StretchToIncludePoint( Vec3 const& point );
};