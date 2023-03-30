#pragma once
#include "EulerAngles.hpp"

struct IntVec3
{
public:
	int x = 0;
	int y = 0;
	int z = 0;

	static IntVec3 const ZERO;
	static IntVec3 const ONE;

public:
	~IntVec3() {}
	IntVec3() {}
	IntVec3( IntVec3 const& copyFrom );
	explicit IntVec3( int initialX, int initialY, int initialZ );

	float GetLength() const;
	int GetLengthSquared() const;
	int GetTaxicabLength() const;
	IntVec3 const GetRotated90Degrees() const;
	IntVec3 const GetRotatedMinus90Degrees() const;
	float GetOrientationDegrees() const;
	float GetOrientationRadians() const;

	void Rotate90Degrees();
	void RotateMinus90Degrees();

	EulerAngles GetEulerAngles();
	bool SetFromText(const char* text);

	bool			operator!=(const IntVec3& compare) const;		// IntVec3 != IntVec3
	bool			operator==(const IntVec3& compare) const;
	const IntVec3	operator+(const IntVec3& vecToAdd) const;		// IntVec3 + IntVec3
	const IntVec3	operator-(const IntVec3& vecToSubtract) const;	// IntVec3 - IntVec3

	friend bool operator< (IntVec3 const& a, IntVec3 const& b)
	{
		if (a.z < b.z)
		{
			return true;
		}
		if (a.z > b.z)
		{
			return false;
		}
		if (a.y < b.y)
		{
			return true;
		}
		if (a.y > b.y)
		{
			return false;
		}
		return (a.x < b.x);
	}
};