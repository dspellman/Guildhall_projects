#include "Engine/Math/MathUtils.hpp"
#include "Engine\Math\Vec4.hpp"

#define _USE_MATH_DEFINES
#include <math.h>

// convenience constants to avoid division and remember relationships
constexpr float degreesToRadians = static_cast<float>(M_PI) / 180.0f;
constexpr float radiansToDegrees = 180.0f / static_cast<float>(M_PI);

Vec4::Vec4()
{

}

Vec4::Vec4(float _x, float _y, float _z, float _w)
	: x(_x), y(_y), z(_z), w(_w)
{

}

Vec4::~Vec4()
{

}

float Vec4::GetLength() const
{
	return sqrtf(GetLengthSquared());
}

float Vec4::GetLengthXY() const
{
	return sqrtf(GetLengthXYSquared());
}

float Vec4::GetLengthSquared() const
{
	return (x * x + y * y + z * z);
}

float Vec4::GetLengthXYSquared() const
{
	return (x * x + y * y);
}

float Vec4::GetAngleAboutZRadians() const
{
	return atan2f(y, x);
}

float Vec4::GetAngleAboutZDegrees() const
{
	return atan2f(y, x) * radiansToDegrees;
}

Vec4 const Vec4::GetRotatedAboutZRadians(float deltaRadians) const
{
	float length = GetLengthXY();
	float theta = atan2f(y, x);
	theta += deltaRadians;
	return Vec4(length * cosf(theta), length * sinf(theta), z, w);
}

Vec4 const Vec4::GetRotatedAboutZDegrees(float deltaDegrees) const
{
	float length = GetLengthXY();
	float theta = atan2f(y, x);
	theta += deltaDegrees * degreesToRadians;
	return Vec4(length * cosf(theta), length * sinf(theta), z, w);
}

Vec4 const Vec4::GetClamped(float maxLength) const
{
	float length = GetLength();
	length = (length > maxLength) ? maxLength / length : 1.0f;
	return Vec4(length * x, length * y, length * z, length * w);
}

Vec4 const Vec4::GetNormalized() const
{
	float length = GetLength();
	return Vec4(x / length, y / length, z / length, w / length);
}

//-----------------------------------------------------------------------------------------------
const Vec4 Vec4::operator + (const Vec4& vecToAdd) const
{
	return Vec4(x + vecToAdd.x, y + vecToAdd.y, z + vecToAdd.z, vecToAdd.w + w);
}

const Vec4 Vec4::operator-(const Vec4& vecToSubtract) const
{
	return Vec4(x - vecToSubtract.x, y - vecToSubtract.y, z - vecToSubtract.z, w - vecToSubtract.w);
}

//------------------------------------------------------------------------------------------------
const Vec4 Vec4::operator-() const
{
	return Vec4(-x, -y, -z, -w);
}

//-----------------------------------------------------------------------------------------------
const Vec4 Vec4::operator*(float uniformScale) const
{
	return Vec4(x * uniformScale, y * uniformScale, z * uniformScale, w * uniformScale);
}


//------------------------------------------------------------------------------------------------
const Vec4 Vec4::operator*(const Vec4& vecToMultiply) const
{
	return Vec4(x * vecToMultiply.x, y * vecToMultiply.y, z * vecToMultiply.z, w * vecToMultiply.w);
}


//-----------------------------------------------------------------------------------------------
const Vec4 Vec4::operator/(float inverseScale) const
{
	return Vec4(x / inverseScale, y / inverseScale, z / inverseScale, w / inverseScale);
}


//-----------------------------------------------------------------------------------------------
void Vec4::operator+=(const Vec4& vecToAdd)
{
	x += vecToAdd.x;
	y += vecToAdd.y;
	z += vecToAdd.z;
}


//-----------------------------------------------------------------------------------------------
void Vec4::operator-=(const Vec4& vecToSubtract)
{
	x -= vecToSubtract.x;
	y -= vecToSubtract.y;
	z -= vecToSubtract.z;
}


//-----------------------------------------------------------------------------------------------
void Vec4::operator*=(const float uniformScale)
{
	x *= uniformScale;
	y *= uniformScale;
	z *= uniformScale;
}


//-----------------------------------------------------------------------------------------------
void Vec4::operator/=(const float uniformDivisor)
{
	x /= uniformDivisor;
	y /= uniformDivisor;
	z /= uniformDivisor;
}


//-----------------------------------------------------------------------------------------------
void Vec4::operator=(const Vec4& copyFrom)
{
	x = copyFrom.x;
	y = copyFrom.y;
	z = copyFrom.z;
}


//-----------------------------------------------------------------------------------------------
const Vec4 operator*(float uniformScale, const Vec4& vecToScale)
{
	return Vec4(uniformScale * vecToScale.x, uniformScale * vecToScale.y, uniformScale * vecToScale.z, uniformScale * vecToScale.w);
}


//-----------------------------------------------------------------------------------------------
bool Vec4::operator==(const Vec4& compare) const
{
	return compare.x == x && compare.y == y && compare.z == z && compare.w == w;
}


//-----------------------------------------------------------------------------------------------
bool Vec4::operator!=(const Vec4& compare) const
{
	return compare.x != x || compare.y != y || compare.z != z || compare.w != w;
}
