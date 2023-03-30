#include "Engine/Math/MathUtils.hpp"
#include "Engine\Math\Vec3.hpp"

#define _USE_MATH_DEFINES
#include <math.h>

// convenience constants to avoid division and remember relationships
constexpr float degreesToRadians = static_cast<float>(M_PI) / 180.0f;
constexpr float radiansToDegrees = 180.0f / static_cast<float>(M_PI);

Vec3 const Vec3::ZERO = Vec3(0.0f, 0.0f, 0.0f);
Vec3 const Vec3::ONE = Vec3(1.0f, 1.0f, 0.0f);

Vec3::Vec3()
{

}

Vec3::Vec3(int ix, int iy, int iz)
{
	x = float(ix);
	y = float(iy);
	z = float(iz);
}

Vec3::Vec3(float _x, float _y, float _z)
	: x(_x), y(_y), z(_z)
{

}

Vec3::Vec3(const Vec2 xy)
{
	x = xy.x;
	y = xy.y;
	z = 0.0f;
}

Vec3::~Vec3()
{

}

float Vec3::GetLength() const
{
	return sqrtf(GetLengthSquared());
}

float Vec3::GetLengthXY() const
{
	return sqrtf(GetLengthXYSquared());
}

float Vec3::GetLengthSquared() const
{
	return (x * x + y * y + z * z);
}

float Vec3::GetLengthXYSquared() const
{
	return (x * x + y * y);
}

float Vec3::GetAngleAboutZRadians() const
{
	return atan2f(y, x);
}

float Vec3::GetAngleAboutZDegrees() const
{
	return atan2f(y, x) * radiansToDegrees;
}

Vec3 const Vec3::GetRotatedAboutZRadians(float deltaRadians) const
{
	float length = GetLengthXY();
	float theta = atan2f(y, x);
	theta += deltaRadians;
	return Vec3(length * cosf(theta), length * sinf(theta), z);
}

Vec3 const Vec3::GetRotatedAboutZDegrees(float deltaDegrees) const
{
	float length = GetLengthXY();
	float theta = atan2f(y, x);
	theta += deltaDegrees * degreesToRadians;
	return Vec3(length * cosf(theta), length * sinf(theta), z);
}

Vec3 const Vec3::GetClamped(float maxLength) const
{
	float length = GetLength();
	length = (length > maxLength) ? maxLength / length : 1.0f;
	return Vec3(length * x, length * y, length * z);
}

Vec3 const Vec3::GetNormalized() const
{
	float length = GetLength();
	if (length == 0.0f)
	{
		return Vec3::ZERO;
	}
	return Vec3(x / length, y / length, z / length);
}

void Vec3::Normalize()
{
	float length = GetLength();
	if (length == 0.0f)
	{
		x = 0.0f;
		y = 0.0f;
		z = 0.0f;
	}
	else
	{
		float inverse = 1.0f / length;
		x *= inverse;
		y *= inverse;
		z *= inverse;
	}
}

void Vec3::ClampAll(float lowerBound, float upperbound)
{
	x = Clamp(x, lowerBound, upperbound);
	y = Clamp(y, lowerBound, upperbound);
	z = Clamp(z, lowerBound, upperbound);
}

void Vec3::Reflect(Vec3 const& impactSurfaceNormal)
{
	Vec3 throwAway(x, y, z);
	throwAway -= 2.0f * DotProduct3D(throwAway, impactSurfaceNormal) * impactSurfaceNormal;
	x = throwAway.x;
	y = throwAway.y;
	z = throwAway.z;
}

EulerAngles Vec3::GetEulerAngles() const
{
	return EulerAngles(Vec3(x, y, z));
}

Vec2 Vec3::XY() const
{
	return Vec2(x, y);
}

// linear interpolate on 3D line defined by two Vec3 points
Vec3 Vec3::Lerp(Vec3 const& a, Vec3 const& b, float fraction)
{
	Vec3 point;
	point.x = a.x * (1.0f - fraction) + b.x * fraction;
	point.y = a.y * (1.0f - fraction) + b.y * fraction;
	point.z = a.z * (1.0f - fraction) + b.z * fraction;
	return point;
}

//-----------------------------------------------------------------------------------------------
const Vec3 Vec3::operator + (const Vec3& vecToAdd) const
{
	return Vec3(x + vecToAdd.x, y + vecToAdd.y, z + vecToAdd.z);
}

const Vec3 Vec3::operator-(const Vec3& vecToSubtract) const
{
	return Vec3(x - vecToSubtract.x, y - vecToSubtract.y, z - vecToSubtract.z);
}

//------------------------------------------------------------------------------------------------
const Vec3 Vec3::operator-() const
{
	return Vec3(-x, -y, -z);
}

//-----------------------------------------------------------------------------------------------
const Vec3 Vec3::operator*(float uniformScale) const
{
	return Vec3(x * uniformScale, y * uniformScale, z * uniformScale);
}


//------------------------------------------------------------------------------------------------
const Vec3 Vec3::operator*(const Vec3& vecToMultiply) const
{
	return Vec3(x * vecToMultiply.x, y * vecToMultiply.y, z * vecToMultiply.z);
}


//-----------------------------------------------------------------------------------------------
const Vec3 Vec3::operator/(float inverseScale) const
{
	return Vec3(x / inverseScale, y / inverseScale, z / inverseScale);
}


//-----------------------------------------------------------------------------------------------
void Vec3::operator+=(const Vec3& vecToAdd)
{
	x += vecToAdd.x;
	y += vecToAdd.y;
	z += vecToAdd.z;
}


//-----------------------------------------------------------------------------------------------
void Vec3::operator-=(const Vec3& vecToSubtract)
{
	x -= vecToSubtract.x;
	y -= vecToSubtract.y;
	z -= vecToSubtract.z;
}


//-----------------------------------------------------------------------------------------------
void Vec3::operator*=(const float uniformScale)
{
	x *= uniformScale;
	y *= uniformScale;
	z *= uniformScale;
}


//-----------------------------------------------------------------------------------------------
void Vec3::operator/=(const float uniformDivisor)
{
	x /= uniformDivisor;
	y /= uniformDivisor;
	z /= uniformDivisor;
}


//-----------------------------------------------------------------------------------------------
void Vec3::operator=(const Vec3& copyFrom)
{
	x = copyFrom.x;
	y = copyFrom.y;
	z = copyFrom.z;
}


//-----------------------------------------------------------------------------------------------
const Vec3 operator*(float uniformScale, const Vec3& vecToScale)
{
	return Vec3(uniformScale * vecToScale.x, uniformScale * vecToScale.y, uniformScale * vecToScale.z);
}


//-----------------------------------------------------------------------------------------------
bool Vec3::operator==(const Vec3& compare) const
{
	return compare.x == x && compare.y == y && compare.z == z;
}


//-----------------------------------------------------------------------------------------------
bool Vec3::operator!=(const Vec3& compare) const
{
	return compare.x != x || compare.y != y || compare.z != z;
}
