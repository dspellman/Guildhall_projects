#pragma once
#include "Engine/Math/Vec2.hpp"

struct IntRange
{
public: // NOTE: this is one of the few cases where we break both the "m_" naming rule AND the avoid-public-members rule
	float m_min = 0.f;
	float m_max = 0.f;

	static IntRange const ZERO;
	static IntRange const ONE;
	static IntRange const ZERO_TO_ONE;

	~IntRange();
	IntRange();
	IntRange(const IntRange& copyFrom);							// copy constructor (from another vec2)
	explicit IntRange(float m_min, float m_max);		// explicit constructor (from x, y)
	explicit IntRange(Vec2 range);		// explicit constructor (from x, y)

	void		operator=(const IntRange& copyFrom);				// vec2 = vec2
	bool		operator==(const IntRange& compare) const;		// vec2 == vec2
	bool		operator!=(const IntRange& compare) const;		// vec2 != vec2

	bool IsOnRange( float const value ) const;
	bool IsOverlappingWith( IntRange const range ) const;
};