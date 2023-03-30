#pragma once
#include "Engine/Math/Vec2.hpp"

struct FloatRange
{
public: // NOTE: this is one of the few cases where we break both the "m_" naming rule AND the avoid-public-members rule
	float m_min = 0.f;
	float m_max = 0.f;

	static FloatRange const ZERO;
	static FloatRange const ONE;
	static FloatRange const ZERO_TO_ONE;

	~FloatRange();
	FloatRange();
	FloatRange(const FloatRange& copyFrom);							// copy constructor (from another vec2)
	explicit FloatRange(float m_min, float m_max);		// explicit constructor (from x, y)
	explicit FloatRange(Vec2 range);		// explicit constructor (from x, y)

	void		operator=(const FloatRange& copyFrom);				// vec2 = vec2
	bool		operator==(const FloatRange& compare) const;		// vec2 == vec2
	bool		operator!=(const FloatRange& compare) const;		// vec2 != vec2

	bool IsOnRange( float const value ) const;
	bool IsOverlappingWith( FloatRange const range ) const;
};