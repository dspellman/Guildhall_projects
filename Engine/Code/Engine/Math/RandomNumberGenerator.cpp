#include "Engine/Math/RandomNumberGenerator.hpp"
#include <stdlib.h>
#include "Vec3.hpp"

int RandomNumberGenerator::RollRandomIntLessThan(int maxNotInclusive)
{
	return rand() % maxNotInclusive; // slightly biased
}

int RandomNumberGenerator::RollRandomIntInRange(int minInclusive, int maxInclusive)
{
	return minInclusive + rand() % (maxInclusive + 1 - minInclusive); // one more to be inclusive
}

float RandomNumberGenerator::RollRandomFloatZeroToOne()
{
	return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
}

float RandomNumberGenerator::RollRandomFloatInRange(float minInclusive, float maxInclusive)
{
	return minInclusive + RollRandomFloatZeroToOne() * (maxInclusive - minInclusive);
}

float RandomNumberGenerator::RollRandomFloatInRange(FloatRange range)
{
	return RollRandomFloatInRange(range.m_min, range.m_max);
}

Vec2 RandomNumberGenerator::GenerateRandomUnitVector2D()
{
	Vec2 unitVector = Vec2::ONE;

	unitVector.x = RollRandomFloatZeroToOne();
	unitVector.y = RollRandomFloatZeroToOne();

	unitVector.x *= RollRandomFloatZeroToOne() < 0.5f ? -1.0f : 1.0f;
	unitVector.y *= RollRandomFloatZeroToOne() < 0.5f ? -1.0f : 1.0f;

	unitVector.Normalize();
	return unitVector;
}

Vec3 RandomNumberGenerator::GenerateRandomUnitVector3D()
{
	Vec3 unitVector = Vec3::ONE;
	unitVector.x = RollRandomFloatZeroToOne();
	unitVector.y = RollRandomFloatZeroToOne();
	unitVector.z = RollRandomFloatZeroToOne();

	unitVector.x *= RollRandomFloatZeroToOne() < 0.5f ? -1.0f : 1.0f;
	unitVector.y *= RollRandomFloatZeroToOne() < 0.5f ? -1.0f : 1.0f;
	unitVector.z *= RollRandomFloatZeroToOne() < 0.5f ? -1.0f : 1.0f;

	unitVector.Normalize();
	return unitVector;
}