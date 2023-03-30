#include "Engine/Math/IntVec2.hpp"
#define _USE_MATH_DEFINES
#include <math.h>
#include "../Core/StringUtils.hpp"

// convenience constants to avoid division and remember relationships
constexpr float degreesToRadians = static_cast<float>(M_PI) / 180.0f;
constexpr float radiansToDegrees = 180.0f / static_cast<float>(M_PI);

IntVec2 const IntVec2::ZERO = IntVec2(0, 0);
IntVec2 const IntVec2::ONE = IntVec2(1, 1);

IntVec2::IntVec2(IntVec2 const& copyFrom)
{
	x = copyFrom.x;
	y = copyFrom.y;
}

IntVec2::IntVec2(int initialX, int initialY)
{
	x = initialX;
	y = initialY;
}

float IntVec2::GetLength() const
{
	return sqrtf (static_cast<float>(GetLengthSquared()));
}

int IntVec2::GetLengthSquared() const
{
	return x * x + y * y;
}

int IntVec2::GetTaxicabLength() const
{
	return abs(x) + abs(y);
}

IntVec2 const IntVec2::GetRotated90Degrees() const
{
	return IntVec2(-y, x);
}

IntVec2 const IntVec2::GetRotatedMinus90Degrees() const
{
	return IntVec2(y, -x );
}

float IntVec2::GetOrientationDegrees() const
{
	return atan2f(static_cast<float>(y), static_cast<float>(x)) * radiansToDegrees;
}

float IntVec2::GetOrientationRadians() const
{
	return atan2f(static_cast<float>(y), static_cast<float>(x));
}

void IntVec2::Rotate90Degrees()
{
	int temp = y;
	y = x;
	x = -temp;
}

void IntVec2::RotateMinus90Degrees()
{
	int temp = y;
	y = -x;
	x = temp;
}

bool IntVec2::SetFromText(const char* text)
{
	Strings strings = SplitStringOnDelimiter(text, ',');
	if (strings.size() == 2)
	{
		x = atoi(strings[0].c_str());
		y = atoi(strings[1].c_str());
		return true;
	}
	return false;
}

const IntVec2 IntVec2::operator+(const IntVec2& vecToAdd) const
{
	return IntVec2(x + vecToAdd.x, y + vecToAdd.y);
}

const IntVec2 IntVec2::operator-(const IntVec2& vecToSubtract) const
{
	return IntVec2(x - vecToSubtract.x, y - vecToSubtract.y);
}

bool IntVec2::operator!=(const IntVec2& compare) const
{
	return x != compare.x || y != compare.y;
}

bool IntVec2::operator==(const IntVec2& compare) const
{
	return x == compare.x && y == compare.y;
}
