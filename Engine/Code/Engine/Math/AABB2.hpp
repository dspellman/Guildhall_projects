#pragma once
#include "Engine/Math/Vec2.hpp"
#include <string>
#include <vector>

struct AABB2
{
public:
	static AABB2 const UNIT_BOX;
	static AABB2 const ZERO_TO_ONE;

	Vec2 m_mins;
	Vec2 m_maxs;

	AABB2();
	~AABB2();
	AABB2( AABB2 const& copyFrom );
	explicit AABB2( Vec2 const& mins, Vec2 const& maxs );
	explicit AABB2( float minx, float miny, float maxx, float maxy );

	// Accessors
	bool IsPointInside(Vec2 const point) const;
	Vec2 GetCenter() const;
	Vec2 GetDimensions() const;
	Vec2 GetNearestPoint( Vec2 const& referencePoint ) const;
	Vec2 GetPointAtUV( Vec2 const& uv)  const;
	Vec2 GetUVForPoint( Vec2 const& point ) const;
	std::string ToString();
	void DrawHollow();

	// Mutators
	void Translate( Vec2 const& translation );
	void SetCenter( Vec2 const& newCenter );
	void SetDimensions( Vec2 const& newDimensions );
	void StretchToIncludePoint( Vec2 const& point );
};