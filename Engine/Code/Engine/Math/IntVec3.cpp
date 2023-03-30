#include "Engine/Math/IntVec3.hpp"
#define _USE_MATH_DEFINES
#include <math.h>
#include "../Core/StringUtils.hpp"

// convenience constants to avoid division and remember relationships
constexpr float degreesToRadians = static_cast<float>(M_PI) / 180.0f;
constexpr float radiansToDegrees = 180.0f / static_cast<float>(M_PI);

IntVec3 const IntVec3::ZERO = IntVec3(0, 0, 0);
IntVec3 const IntVec3::ONE = IntVec3(1, 1, 1);

IntVec3::IntVec3(IntVec3 const& copyFrom)
{
	x = copyFrom.x;
	y = copyFrom.y;
	z = copyFrom.z;
}

IntVec3::IntVec3(int initialX, int initialY, int initialZ)
{
	x = initialX;
	y = initialY;
	z = initialZ;
}

float IntVec3::GetLength() const
{
	return sqrtf (static_cast<float>(GetLengthSquared()));
}

int IntVec3::GetLengthSquared() const
{
	return x * x + y * y + z * z;
}

int IntVec3::GetTaxicabLength() const
{
	return abs(x) + abs(y) + abs(z);
}

IntVec3 const IntVec3::GetRotated90Degrees() const
{
	return IntVec3(-y, x, z);
}

IntVec3 const IntVec3::GetRotatedMinus90Degrees() const
{
	return IntVec3(y, -x, z);
}

float IntVec3::GetOrientationDegrees() const
{
	return atan2f(static_cast<float>(y), static_cast<float>(x)) * radiansToDegrees;
}

float IntVec3::GetOrientationRadians() const
{
	return atan2f(static_cast<float>(y), static_cast<float>(x));
}

void IntVec3::Rotate90Degrees()
{
	int temp = y;
	y = x;
	x = -temp;
}

void IntVec3::RotateMinus90Degrees()
{
	int temp = y;
	y = -x;
	x = temp;
}

EulerAngles IntVec3::GetEulerAngles()
{
	return EulerAngles((float)x, (float)y, (float)z);
}

bool IntVec3::SetFromText(const char* text)
{
	Strings strings = SplitStringOnDelimiter(text, ',');
	if (strings.size() == 2)
	{
		x = atoi(strings[0].c_str());
		y = atoi(strings[1].c_str());
		z = atoi(strings[2].c_str());
		return true;
	}
	return false;
}

const IntVec3 IntVec3::operator+(const IntVec3& vecToAdd) const
{
	return IntVec3(x + vecToAdd.x, y + vecToAdd.y, z + vecToAdd.z);
}

const IntVec3 IntVec3::operator-(const IntVec3& vecToSubtract) const
{
	return IntVec3(x - vecToSubtract.x, y - vecToSubtract.y, z - vecToSubtract.z);
}

bool IntVec3::operator!=(const IntVec3& compare) const
{
	return x != compare.x || y != compare.y || z != compare.z;
}

bool IntVec3::operator==(const IntVec3& compare) const
{
	return !(*this != compare);
}
