#pragma once
#include <stdlib.h>
#include "FloatRange.hpp"

class RandomNumberGenerator
{
public:
	int RollRandomIntLessThan(int maxNotInclusive);
	int RollRandomIntInRange( int minInclusive, int maxInclusive);
	float RollRandomFloatZeroToOne();
	float RollRandomFloatInRange(float minInclusive, float maxInclusive);
	float RollRandomFloatInRange(FloatRange range);
	Vec2 GenerateRandomUnitVector2D();
	Vec3 GenerateRandomUnitVector3D();
};

extern RandomNumberGenerator random;