#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Vec2.hpp"

#define _USE_MATH_DEFINES
#include <math.h>
#include "Engine/Core/StringUtils.hpp"
#include <string>
#include "IntVec2.hpp"
#include "Vec3.hpp"

// convenience constants to avoid division and remember relationships
constexpr float degreesToRadians = static_cast<float>(M_PI) / 180.0f;
constexpr float radiansToDegrees = 180.0f / static_cast<float>(M_PI);

Vec2 const Vec2::ZERO = Vec2(0.0f, 0.0f);
Vec2 const Vec2::ONE = Vec2(1.0f, 1.0f);

//-----------------------------------------------------------------------------------------------
Vec2::Vec2( const Vec2& copy )
	: x( copy.x )
	, y( copy.y )
{
}


//-----------------------------------------------------------------------------------------------
Vec2::Vec2( float initialX, float initialY )
	: x(initialX)
	, y(initialY)
{
}


Vec2::Vec2( IntVec2& iv2 )
{
	x = static_cast<float>(iv2.x);
	y = static_cast<float>(iv2.y);
}

Vec2::Vec2( const Vec3 v )
{
	x = v.x;
	y = v.y;
}

Vec2 const Vec2::MakeFromPolarRadians(float orientationRadians, float length /*= 1.0f*/)
{
	// EXCEPTION length of zero is a problem
	return Vec2(cosf(orientationRadians) * length, sinf(orientationRadians) * length);
}

Vec2 const Vec2::MakeFromPolarDegrees(float orientationDegrees, float length /*= 1.0f*/)
{
	// EXCEPTION length of zero is a problem
	return Vec2(CosDegrees(orientationDegrees) * length, SinDegrees(orientationDegrees) * length);
}

// linear interpolate on 3D line defined by two Vec3 points
Vec2 Vec2::Lerp(Vec2 const& a, Vec2 const& b, float fraction)
{
	Vec2 point;
	point.x = a.x * (1.0f - fraction) + b.x * fraction;
	point.y = a.y * (1.0f - fraction) + b.y * fraction;
	return point;
}

//-----------------------------------------------------------------------------------------------
const Vec2 Vec2::operator + ( const Vec2& vecToAdd ) const
{
	return Vec2( x + vecToAdd.x, y + vecToAdd.y );
}


//-----------------------------------------------------------------------------------------------
const Vec2 Vec2::operator - ( const Vec2& vecToSubtract ) const
{
	return Vec2( x - vecToSubtract.x, y - vecToSubtract.y );
}


//------------------------------------------------------------------------------------------------
const Vec2 Vec2::operator-() const
{
	return Vec2( -x, -y );
}


float Vec2::GetLength() const
{
	return sqrtf(GetLengthSquared());
}

float Vec2::GetLengthSquared() const
{
	return (x * x + y * y);
}

float Vec2::GetOrientationRadians() const
{
	return atan2f(y, x);
}

float Vec2::GetOrientationDegrees() const
{
	return atan2f(y, x) * radiansToDegrees;
}

Vec2 const Vec2::GetRotated90Degrees() const
{
	return Vec2(-y, x);
}

Vec2 const Vec2::GetRotatedMinus90Degrees() const
{
	return Vec2(y, -x );
}

Vec2 const Vec2::GetRotatedRadians(float deltaRadians) const
{
	float length = GetLength();
	float theta = atan2f(y, x);
	theta += deltaRadians;
	return Vec2(length * cosf(theta), length * sinf(theta));
}

Vec2 const Vec2::GetRotatedDegrees(float deltaDegrees) const
{
	float length = GetLength();
	float theta = atan2f(y, x);
	theta += deltaDegrees * degreesToRadians;
	return Vec2(length * cosf(theta), length * sinf(theta));
}

Vec2 const Vec2::GetClamped(float maxLength) const
{
	float length = GetLength();
	length = (length > maxLength) ? maxLength / length : 1.0f;
	return Vec2(length * x, length * y);
}

Vec2 const Vec2::GetNormalized() const
{
	float length = GetLength();
	if (length == 0.0f)
	{
		return Vec2::ZERO;
	}
	return Vec2(x / length, y / length);
}

Vec2 Vec2::GetReflected(Vec2 const& impactSurfaceNormal) const
{
	Vec2 throwAway(x, y);
	throwAway -= 2.0f * DotProduct2D(throwAway, impactSurfaceNormal) * impactSurfaceNormal;
	return throwAway;
}

void Vec2::SetOrientationRadians(float newOrientationRadians)
{
	float length = GetLength();
	x = length * cosf(newOrientationRadians);
	y = length * sinf(newOrientationRadians);
}

void Vec2::SetOrientationDegrees(float newOrientationDegrees)
{
	float newOrientationRadians = ConvertDegreesToRadians(newOrientationDegrees);
	SetOrientationRadians(newOrientationRadians);
}

void Vec2::SetPolarRadians(float newOrientationRadians, float newLength)
{
	x = newLength * cosf(newOrientationRadians);
	y = newLength * sinf(newOrientationRadians);

}

void Vec2::SetPolarDegrees(float newOrientationDegrees, float newLength)
{
	float newOrientationRadians = ConvertDegreesToRadians(newOrientationDegrees);
	SetPolarRadians(newOrientationRadians, newLength);
}

void Vec2::Rotate90Degrees()
{
	float temp = y;
	y = x;
	x = -temp;
}

void Vec2::RotateMinus90Degrees()
{
	float temp = y;
	y = -x;
	x = temp;
}

void Vec2::RotateRadians(float deltaRadians)
{
	float length = GetLength();
	float theta = atan2f(y, x);
	theta += deltaRadians;
 	x = length * cosf(theta);
 	y = length * sinf(theta);
}

void Vec2::RotateDegrees(float deltaDegrees)
{
	RotateRadians(ConvertDegreesToRadians(deltaDegrees));
}

void Vec2::SetLength(float newLength)
{
	Normalize();
	x *= newLength;
	y *= newLength;
}

void Vec2::ClampLength(float newLength)
{
	SetLength((GetLength() > newLength) ? newLength : GetLength());
}

void Vec2::Normalize()
{
	float length = GetLength();
	if (length == 0.0f)
	{
		x = 0.0f;
		y = 0.0f;
	}
	else
	{
		x /= length;
		y /= length;
	}
}

float Vec2::NormalizeAndGetPreviousLength()
{
	float oldLength = GetLength();
	Normalize();
	return oldLength;
}

void Vec2::Reflect(Vec2 const& impactSurfaceNormal)
{
	Vec2 throwAway(x, y);
	throwAway -= 2.0f * DotProduct2D(throwAway, impactSurfaceNormal) * impactSurfaceNormal;
	x = throwAway.x;
	y = throwAway.y;
}

bool Vec2::SetFromText(const char* text)
{
	Strings strings = SplitStringOnDelimiter(text, ',');
	if (strings.size() == 2)
	{
		x = static_cast<float>(atof(strings[0].c_str()));
		y = static_cast<float>(atof(strings[1].c_str()));
		return true;
	}
	return false;
}

std::string Vec2::ToString()
{
	std::string content = "(";
	content += std::to_string(x);
	content += ",";
	content += std::to_string(y);
	content += ")";
	return content;
}

//-----------------------------------------------------------------------------------------------
const Vec2 Vec2::operator*( float uniformScale ) const
{
	return Vec2( x * uniformScale, y * uniformScale);
}


//------------------------------------------------------------------------------------------------
const Vec2 Vec2::operator*( const Vec2& vecToMultiply ) const
{
	return Vec2( x * vecToMultiply.x, y * vecToMultiply.y);
}


//-----------------------------------------------------------------------------------------------
const Vec2 Vec2::operator/( float inverseScale ) const
{
	return Vec2( x / inverseScale, y / inverseScale);
}


//-----------------------------------------------------------------------------------------------
void Vec2::operator+=( const Vec2& vecToAdd )
{
	x += vecToAdd.x;
	y += vecToAdd.y;
}


//-----------------------------------------------------------------------------------------------
void Vec2::operator-=( const Vec2& vecToSubtract )
{
	x -= vecToSubtract.x;
	y -= vecToSubtract.y;
}


//-----------------------------------------------------------------------------------------------
void Vec2::operator*=( const float uniformScale )
{
	x *= uniformScale;
	y *= uniformScale;
}


//-----------------------------------------------------------------------------------------------
void Vec2::operator/=( const float uniformDivisor )
{
	x /= uniformDivisor;
	y /= uniformDivisor;
}


//-----------------------------------------------------------------------------------------------
void Vec2::operator=( const Vec2& copyFrom )
{
	x = copyFrom.x;
	y = copyFrom.y;
}


//-----------------------------------------------------------------------------------------------
const Vec2 operator*( float uniformScale, const Vec2& vecToScale )
{
	return Vec2(uniformScale * vecToScale.x, uniformScale * vecToScale.y);
}


//-----------------------------------------------------------------------------------------------
bool Vec2::operator==( const Vec2& compare ) const
{
	return compare.x == x && compare.y == y;
}


//-----------------------------------------------------------------------------------------------
bool Vec2::operator!=( const Vec2& compare ) const
{
	return compare.x != x || compare.y != y;
}

