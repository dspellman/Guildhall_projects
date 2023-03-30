#pragma once

struct IntVec2
{
public:
	int x = 0;
	int y = 0;

	static IntVec2 const ZERO;
	static IntVec2 const ONE;

public:
	~IntVec2() {}
	IntVec2() {}
	IntVec2( IntVec2 const& copyFrom );
	explicit IntVec2( int initialX, int initialY );

	float GetLength() const;
	int GetLengthSquared() const;
	int GetTaxicabLength() const;
	IntVec2 const GetRotated90Degrees() const;
	IntVec2 const GetRotatedMinus90Degrees() const;
	float GetOrientationDegrees() const;
	float GetOrientationRadians() const;

	void Rotate90Degrees();
	void RotateMinus90Degrees();

	bool SetFromText( const char* text );

	bool		operator!=(const IntVec2& compare) const;		// IntVec2 != IntVec2
	bool		operator==(const IntVec2& compare) const;		// IntVec2 != IntVec2
	const IntVec2	operator+(const IntVec2& vecToAdd) const;		// IntVec2 + IntVec2
	const IntVec2	operator-(const IntVec2& vecToSubtract) const;	// IntVec2 - IntVec2

	friend bool operator< (IntVec2 const& a, IntVec2 const& b)
	{
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