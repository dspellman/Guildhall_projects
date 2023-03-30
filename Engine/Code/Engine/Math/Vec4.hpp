#pragma once

struct Vec4
{
public: // NOTE: this is one of the few cases where we break both the "m_" naming rule AND the avoid-public-members rule
	float x = 0.f;
	float y = 0.f;
	float z = 0.f;
	float w = 0.0f;

	Vec4();
	Vec4(float _x, float _y, float _z, float _w);
	~Vec4();

	// Accessors
	float GetLength() const;
	float GetLengthXY() const;
	float GetLengthSquared() const;
	float GetLengthXYSquared() const;
	float GetAngleAboutZRadians() const;
	float GetAngleAboutZDegrees() const;
	Vec4 const GetRotatedAboutZRadians(float deltaRadians)  const;
	Vec4 const GetRotatedAboutZDegrees(float deltaRadians)  const;
	Vec4 const GetClamped(float maxLength) const;
	Vec4 const GetNormalized() const;

	// Operators (const)
	bool		operator==(const Vec4& compare) const;		// Vec4 == Vec4
	bool		operator!=(const Vec4& compare) const;		// Vec4 != Vec4
	const Vec4	operator+(const Vec4& vecToAdd) const;		// Vec4 + Vec4
	const Vec4	operator-(const Vec4& vecToSubtract) const;	// Vec4 - Vec4
	const Vec4	operator-() const;							// -Vec4, i.e. "unary negation"
	const Vec4	operator*(float uniformScale) const;		// Vec4 * float
	const Vec4	operator*(const Vec4& vecToMultiply) const;	// Vec4 * Vec4
	const Vec4	operator/(float inverseScale) const;		// Vec4 / float

	// Operators (self-mutating / non-const)
	void		operator+=(const Vec4& vecToAdd);			// Vec4 += Vec4
	void		operator-=(const Vec4& vecToSubtract);		// Vec4 -= Vec4
	void		operator*=(const float uniformScale);		// Vec4 *= float
	void		operator/=(const float uniformDivisor);		// Vec4 /= float
	void		operator=(const Vec4& copyFrom);			// Vec4 = Vec4

	// Standalone "friend" functions that are conceptually, but not actually, part of Vec4::
	friend const Vec4 operator*(float uniformScale, const Vec4& vecToScale);	// float * Vec4
};