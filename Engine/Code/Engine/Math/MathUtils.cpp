#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/Rgba8.hpp"
#define _USE_MATH_DEFINES
#include <math.h>

// convenience constants to avoid division and remember relationships
constexpr float degreesToRadians = static_cast<float>(M_PI) / 180.0f; 
constexpr float radiansToDegrees = 180.0f / static_cast<float>(M_PI);

// utility function
double Pi()
{
	return M_PI;
}

float Degrees0to360(float degrees)
{
	while (degrees > 360.0f)
	{
		degrees -= 360.0f;
	}	
	while (degrees < 0.0f)
	{
		degrees += 360.0f;
	}
	return degrees;
}

//Trigonometry Functions
float ConvertDegreesToRadians(float degrees)
{
	return degrees * degreesToRadians;
}

float ConvertRadiansToDegrees(float radians)
{
	return radians * radiansToDegrees;
}

float CosDegrees(float degrees)
{
	return cosf(ConvertDegreesToRadians(degrees));
}

float SinDegrees(float degrees)
{
	return sinf(ConvertDegreesToRadians(degrees));
}

float Atan2Degrees(float yDegrees, float xDegrees)
{
	return ConvertRadiansToDegrees(atan2f(yDegrees, xDegrees));
}

// returns displacement required to move to final orientation
float GetShortestAngularDispDegrees(float fromDegrees, float toDegrees)
{
	float displacement = toDegrees - fromDegrees;
	while (displacement > 180.0f)
		displacement -= 360.0f;
	while (displacement < -180.0f)
		displacement += 360.0f;
	return displacement;
}

// returns new orientation in degrees
float GetTurnedTowardDegrees(float fromDegrees, float towardDegrees, float maxDeltaDegrees)
{
	maxDeltaDegrees = fabsf(maxDeltaDegrees); // by convention we will use the magnitude
	float displacement = GetShortestAngularDispDegrees(fromDegrees, towardDegrees);
	if (fabsf(displacement) < maxDeltaDegrees)
		return towardDegrees;
	if (displacement < 0)
		return fromDegrees - maxDeltaDegrees;
	else
		return fromDegrees + maxDeltaDegrees;
}

// Distance Functions
float GetDistance2D(Vec2 const& positionA, Vec2 const& positionB)
{
	return sqrtf(GetDistanceSquared2D(positionA, positionB));
}

float GetDistanceSquared2D(Vec2 const& positionA, Vec2 const& positionB)
{
	Vec2 magnitude = positionB - positionA;
	return magnitude.x * magnitude.x + magnitude.y * magnitude.y;
}

float GetDistance3D(Vec3 const& positionA, Vec3 const& positionB)
{
	return sqrtf(GetDistanceSquared3D(positionA, positionB));
}

float GetDistanceSquared3D(Vec3 const& positionA, Vec3 const& positionB)
{
	Vec3 magnitude = positionB - positionA;
	return magnitude.x * magnitude.x + magnitude.y * magnitude.y + magnitude.z * magnitude.z;
}

float GetDistanceXY3D(Vec3 const& positionA, Vec3 const& positionB)
{
	return sqrtf(GetDistanceXYSquared3D(positionA, positionB));
}

float GetDistanceXYSquared3D(Vec3 const& positionA, Vec3 const& positionB)
{
	Vec3 magnitude = positionB - positionA;
	return magnitude.x * magnitude.x + magnitude.y * magnitude.y;
}

int GetTaxicabDistance2D(IntVec2 const& pointA, IntVec2 const& pointB)
{
	return abs(pointA.x - pointB.x) + abs(pointA.y - pointB.y);
}

float DotProduct2D(Vec2 const& a, Vec2 const& b)
{
	return a.x * b.x + a.y * b.y;
}

float DotProduct3D(Vec3 const& a, Vec3 const& b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

float DotProduct4D(Vec4 const& a, Vec4 const& b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

float CrossProduct2D(Vec2 const& a, Vec2 const& b)
{
	return a.x * b.y - a.y * b.x; // the area of the parallelogram
}

Vec3 CrossProduct3D(Vec3 const& a, Vec3 const& b)
{
	Vec3 xprod;
	xprod.x = a.y * b.z - a.z * b.y;
	xprod.y = a.z * b.x - a.x * b.z;
	xprod.z = a.x * b.y - a.y * b.x;
	return xprod;
}

float GetProjectedLength3D(Vec3 const& vectorToProject, Vec3 const& vectorToProjectOnto)
{
	return DotProduct3D(vectorToProject, vectorToProjectOnto.GetNormalized());
}

Vec3 const GetProjectedOnto3D(Vec3 const& vectorToProject, Vec3 const& vectorToProjectOnto)
{
	Vec3 projection;
	projection = GetProjectedLength3D(vectorToProject, vectorToProjectOnto) * vectorToProjectOnto.GetNormalized();
	return projection;
}

float GetProjectedLength2D(Vec2 const& vectorToProject, Vec2 const& vectorToProjectOnto)
{
	return DotProduct2D(vectorToProject, vectorToProjectOnto.GetNormalized());
}

Vec2 const GetProjectedOnto2D(Vec2 const& vectorToProject, Vec2 const& vectorToProjectOnto)
{
	Vec2 projection;
	projection = GetProjectedLength2D(vectorToProject, vectorToProjectOnto) * vectorToProjectOnto.GetNormalized();
	return projection;
}

float GetAngleDegreesBetweenVectors2D(Vec2 const& a, Vec2 const& b)
{
	float angle = acosf(DotProduct2D(a.GetNormalized(), b.GetNormalized()));
	return angle * radiansToDegrees;
}

float ClampZeroToOne(float value)
{
	if (value < 0.0f)
	{
		return 0.0f;
	}
	if (value > 1.0f)
	{
		return 1.0f;
	}
	return value;
}

float Clamp(float value, float min, float max)
{
	if (value < min)
	{
		return min;
	}
	if (value > max)
	{
		return max;
	}
	return value;
}

int Clamp(int value, int min, int max)
{
	if (value < min)
	{
		return min;
	}
	if (value > max)
	{
		return max;
	}
	return value;
}

// Geometry Functions
bool DoDiscsOverlap(Vec2 const& centerA, float radiusA, Vec2 const& centerB, float radiusB)
{
	float radiusAB = radiusA + radiusB;
	return GetDistanceSquared2D(centerA, centerB) <  radiusAB * radiusAB ? true : false;
}

bool DoSpheresOverlap(Vec3 const& centerA, float radiusA, Vec3 const& centerB, float radiusB)
{
	float radiusAB = radiusA + radiusB;
	return GetDistanceSquared3D(centerA, centerB) < radiusAB * radiusAB ? true : false;
}

Vec2 const GetNearestPointOnDisc2D(Vec2 const& referencePoint, Vec2 const& discCenter, float discRadius)
{
	Vec2 CP = referencePoint - discCenter;
	if (CP.GetLength() < discRadius)
		return referencePoint;
	return discCenter + CP.GetNormalized() * discRadius;
}

bool IsPointInsideOrientedSector2D(Vec2 const& point, Vec2 const& sectorTip, float sectorForwardDegrees, float sectorApertureDegrees, float sectorRadius)
{
	if (!IsPointInsideDisc2D(point, sectorTip, sectorRadius))
		return false; // fast elimination
	Vec2 CP = point - sectorTip;
	float theta = CP.GetOrientationDegrees();
	if (fabsf(GetShortestAngularDispDegrees(theta, sectorForwardDegrees)) < sectorApertureDegrees * 0.5f)
		return true;
	return false;
}

bool IsPointInsideDirectedSector2D(Vec2 const& point, Vec2 const& sectorTip, Vec2 const& sectorForwardNormal, float sectorApertureDegrees, float sectorRadius)
{
	if (!IsPointInsideDisc2D(point, sectorTip, sectorRadius))
		return false; // fast elimination
	Vec2 CP = point - sectorTip;
	if (fabsf(GetAngleDegreesBetweenVectors2D(CP, sectorForwardNormal)) < sectorApertureDegrees * 0.5f)
		return true;
	return false;
}

bool DoDiscsOverlap2D(Vec2 const& centerA, float radiusA, Vec2 const& centerB, float radiusB)
{
	return DoDiscsOverlap(centerA, radiusA, centerB, radiusB);
}

bool DoSpheresOverlap3D(Vec3 const& centerA, float radiusA, Vec3 const& centerB, float radiusB)
{
	return DoSpheresOverlap(centerA, radiusA, centerB, radiusB);
}

bool DoesSphereOverlapAABB3(Vec3 const& center, float radius, AABB3 const& box)
{
	Vec3 nearestOnBox = GetNearestPointOnAABB3D(center, box);
	return (nearestOnBox - center).GetLengthSquared() < radius * radius;
}

bool DoesSphereOverlapZCylinder(Vec3 const& center, float radius, Vec3 const& Zcenter, float Zheight, float Zradius)
{
	Vec3 nearestOnCylinder = GetNearestPointOnCylinder(center, Zcenter, Zheight, Zradius);
	return (nearestOnCylinder - center).GetLengthSquared() < radius * radius;
}

bool DoesZCylinderOverlapAABB3(Vec3 const& center, float height, float radius, AABB3 const& box)
{
	// Test vertical separation first
	// test cylinder above box
	if (center.z - 0.5f * height > box.m_maxs.z)
	{
		return false;
	}
	// test box above cylinder
	if (box.m_mins.z > center.z + 0.5f * height)
	{
		return false;
	}
	// test overlap in XY plane if the Z range overlaps
	Vec2 center2D(center.x, center.y);
	Vec2 nearestOnBox = GetNearestPointOnAABB2D(center2D, AABB2(box.m_mins.x, box.m_mins.y, box.m_maxs.x, box.m_maxs.y));
	return IsPointInsideDisc2D(nearestOnBox, center2D, radius);
}

bool DoesAABB3OverlapZCylinder(AABB3 const& box, Vec3 const& center, float height, float radius)
{
	return DoesZCylinderOverlapAABB3(center, height, radius, box);
}

bool DoZcylindersOverlap(Vec3 const& centerA, float heightA, float radiusA, Vec3 const& centerB, float heightB, float radiusB)
{
	// test overlap in XY plane
	if (DoDiscsOverlap2D(Vec2(centerA.x, centerA.y), radiusA, Vec2(centerB.x, centerB.y), radiusB) == false)
	{
		return false;
	}
	// test A above B
	if (centerA.z - 0.5f * heightA > centerB.z + 0.5f * heightB)
	{
		return false;
	}
	// test B above A
	if (centerB.z - 0.5f * heightB > centerA.z + 0.5f * heightA)
	{
		return false;
	}
	// cylinders overlap
	return true;
}

bool DoAABB3BoxesOverlap(AABB3 const& boundsA, AABB3 const& boundsB)
{
	// check x-axis overlap
	if (boundsA.m_mins.x > boundsB.m_maxs.x)
	{
		return false;
	}
	if (boundsB.m_mins.x > boundsA.m_maxs.x)
	{
		return false;
	}
	// check y-axis overlap
	if (boundsA.m_mins.y > boundsB.m_maxs.y)
	{
		return false;
	}
	if (boundsB.m_mins.y > boundsA.m_maxs.y)
	{
		return false;
	}
	// check z-axis overlap
	if (boundsA.m_mins.z > boundsB.m_maxs.z)
	{
		return false;
	}
	if (boundsB.m_mins.z > boundsA.m_maxs.z)
	{
		return false;
	}
	// boxes overlap
	return true;
}

Vec2 const GetNearestPointOnAABB2D(Vec2 const& referencepos, AABB2 box)
{
	return box.GetNearestPoint(referencepos);
}

Vec2 const GetNearestPointOnInfiniteline2D(Vec2 const& referencePos, LineSegment2 const& infiniteLine)
{
	Vec2 startToEnd = infiniteLine.m_endPos - infiniteLine.m_startPos;
	Vec2 startToPoint = referencePos - infiniteLine.m_startPos;
	return infiniteLine.m_startPos + GetProjectedOnto2D(startToPoint, startToEnd); // projected onto line segment
}

Vec2 const GetNearestPointOnInfiniteline2D(Vec2 const& referencePos, Vec2 const& pointOnLine, Vec2 const& anotherPointOnLine)
{
	Vec2 startToEnd = anotherPointOnLine - pointOnLine;
	Vec2 startToPoint = referencePos - pointOnLine;
	return pointOnLine + GetProjectedOnto2D(startToPoint, startToEnd); // projected onto line segment
}

Vec2 const GetNearestPointOnLineSegment2D(Vec2 const& referencePos, LineSegment2 const& lineSegment)
{
	return lineSegment.GetNearestPoint(referencePos);
}

Vec2 const GetNearestPointOnLineSegment2D(Vec2 const& referencepos, Vec2 const& lineSegStart, Vec2 const& lineSegEnd)
{
	Vec2 startToEnd = lineSegEnd - lineSegStart;
	Vec2 endToPoint = referencepos - lineSegEnd;
	Vec2 startToPoint = referencepos - lineSegStart;

	if (DotProduct2D(startToEnd, endToPoint) >= 0.0f)
	{
		return lineSegEnd;
	}
	if (DotProduct2D(startToEnd, startToPoint) <= 0.0f)
	{
		return lineSegStart;
	}
	return lineSegStart + GetProjectedOnto2D(startToPoint, startToEnd); // projected onto line segment
}

Vec2 const GetNearestPointOnCapsule2D(Vec2 const& referencepos, Capsule2 const& capsule)
{
	if (IsPointInsideCapsule2D(referencepos, capsule))
	{
		return referencepos;
	}
	Vec2 nearestPos = GetNearestPointOnLineSegment2D(referencepos, capsule.m_startPos, capsule.m_endPos);
	Vec2 nearestPosToPoint = referencepos - nearestPos;
	nearestPosToPoint.Normalize(); // get the unit vector
	nearestPosToPoint *= capsule.m_radius; // extend radius distance to reach the capsule border
	return nearestPos + nearestPosToPoint;
}

Vec2 const GetNearestPointOnCapsule2D(Vec2 const& referencePos, Vec2 const& boneStart, Vec2 const& boneEnd, float radius)
{
	if (IsPointInsideCapsule2D(referencePos, boneStart, boneEnd, radius))
	{
		return referencePos;
	}
	Vec2 nearestPos = GetNearestPointOnLineSegment2D(referencePos, boneStart, boneEnd);
	Vec2 nearestPosToPoint = referencePos - nearestPos;
	nearestPosToPoint.Normalize(); // get the unit vector
	nearestPosToPoint *= radius; // extend radius distance to reach the capsule border
	return nearestPos + nearestPosToPoint;
}

Vec2 const GetNearestPointOnOBB2D(Vec2 const& referencePos, OBB2 const& orientedBox)
{
	if (IsPointInsideOBB2D(referencePos, orientedBox))
	{
		return referencePos;
	}
	Vec2 localPos = orientedBox.GetLocalPosForWorldPos(referencePos);
	Vec2 nearestPos(GetClamped(localPos.x, -orientedBox.m_halfDimensions.x, orientedBox.m_halfDimensions.x),
					GetClamped(localPos.y, -orientedBox.m_halfDimensions.y, orientedBox.m_halfDimensions.y));
	Vec2 worldPos = orientedBox.GetWorldPosForLocalPos(nearestPos);
	return worldPos;
}

bool IsPointInsideAABB3D(Vec3 const& point, AABB3 const& box)
{
	return box.IsPointInside(point);
}

Vec3 const GetNearestPointOnAABB3D(Vec3 const& referencepos, AABB3 const& box)
{
	return box.GetNearestPoint(referencepos);
}

Vec3 const GetNearestPointOnCylinder(Vec3 const& referencepos, Vec3 const& center, float height, float radius)
{
	Vec2 perimeter = GetNearestPointOnDisc2D(Vec2(referencepos.x, referencepos.y), Vec2(center.x, center.y), radius);
	return Vec3(perimeter.x, perimeter.y, Clamp(referencepos.z, center.z - height * 0.5f, center.z + height * 0.5f));
}

Vec3 const GetNearestPointOnSphere(Vec3 const& referencePoint, Vec3 const& center, float radius)
{
	Vec3 CP = referencePoint - center;
	return center + CP.GetNormalized() * radius;
}

bool PushDiscOutOfPoint2D(Vec2& mobileDiscCenter, float discRadius, Vec2 const& fixedPoint)
{
	Vec2 AB = fixedPoint - mobileDiscCenter;
	float overlap = discRadius - AB.GetLength();
	if (overlap <= 0.0f)
		return false;  // already don't overlap
	mobileDiscCenter -= AB.GetNormalized() * overlap;
	return true;
}

bool PushDiscOutOfDisc2D(Vec2& mobileDiscCenter, float mobileRadius, Vec2 const& fixedDiscCenter, float fixedRadius)
{
	Vec2 AB = fixedDiscCenter - mobileDiscCenter;
	float overlap = mobileRadius + fixedRadius - AB.GetLength();
	if (overlap <= 0.0f)
		return false;  // already don't overlap
	mobileDiscCenter -= AB.GetNormalized() * overlap;
	return true;
}

bool PushDiscsOutOfEachOther2D(Vec2& aCenter, float aRadius, Vec2& bCenter, float bRadius)
{
	Vec2 AB = bCenter - aCenter;
	float overlap = aRadius + bRadius - AB.GetLength();
	if (overlap <= 0.0f)
		return false;  // already don't overlap
	bCenter += AB.GetNormalized() * overlap * 0.5;
	aCenter -= AB.GetNormalized() * overlap * 0.5;
	return true;
}

bool PushDiscOutOfAABB2D(Vec2& mobileDiscCenter, float discRadius, AABB2 const& fixedBox)
{
	Vec2 boxPoint = fixedBox.GetNearestPoint(mobileDiscCenter);
	Vec2 CB = boxPoint - mobileDiscCenter;
	if (CB.GetLength() > discRadius)
		return false;

	return PushDiscOutOfPoint2D(mobileDiscCenter, discRadius, boxPoint);
}

// Transform functions
void TransformPosition2D(Vec2& posToTransform, float uniformScaleXY, float rotationDegreesAboutZ, Vec2 const& translationXY)
{
	// Scale first
	posToTransform.x *= uniformScaleXY;
	posToTransform.y *= uniformScaleXY;

	// Rotate second 
	float theta = atan2f(posToTransform.y, posToTransform.x);
	theta +=  degreesToRadians * rotationDegreesAboutZ; // convert to radians and rotate
	float R = sqrtf(posToTransform.x * posToTransform.x + posToTransform.y * posToTransform.y);
	posToTransform.x = R * cosf(theta);
	posToTransform.y = R * sinf(theta);

	// Translate third
	posToTransform.x += translationXY.x;
	posToTransform.y += translationXY.y;
}

void TransformPosition2D(Vec2& posToTransform, Vec2 const& iBasis, Vec2 const& jBasis, Vec2 const& translationXY)
{
	Vec2 throwAway(posToTransform.x, posToTransform.y);
	throwAway = translationXY + posToTransform.x * iBasis + posToTransform.y * jBasis;
	posToTransform.x = throwAway.x;
	posToTransform.y = throwAway.y;
}

void TransformPositionXY3D(Vec3& positionToTransform, float scaleXY, float zRotationDegrees, Vec2 const& translationXY)
{
	// Scale first
	positionToTransform.x *= scaleXY;
	positionToTransform.y *= scaleXY;

	// Rotate second 
	float theta = atan2f(positionToTransform.y, positionToTransform.x);
	theta += degreesToRadians * zRotationDegrees; // convert to radians and rotate
	float R = sqrtf(positionToTransform.x * positionToTransform.x + positionToTransform.y * positionToTransform.y);
	positionToTransform.x = R * cosf(theta);
	positionToTransform.y = R * sinf(theta);

	// Translate third
	positionToTransform.x += translationXY.x;
	positionToTransform.y += translationXY.y;
}

void TransformPositionXY3D(Vec3& posToTransform, Vec2 const& iBasis, Vec2 const& jBasis, Vec2 const& translationXY)
{
	Vec2 throwAway(posToTransform.x, posToTransform.y);
	throwAway = translationXY + posToTransform.x * iBasis + posToTransform.y * jBasis;
	posToTransform.x = throwAway.x;
	posToTransform.y = throwAway.y;
}

float GetRangeClamped(float value, float inLow, float inHigh, float outLow, float outHigh)
{
	float result = RangeMap(value, inLow, inHigh, outLow, outHigh);
	if (outLow < outHigh)
	{
		result = (result < outLow) ? outLow : result;
		result = (result > outHigh) ? outHigh : result;
	}
	else // range reversed, so test must be too
	{
		result = (result > outLow) ? outLow : result;
		result = (result < outHigh) ? outHigh : result;
	}
	return result;
}

float RangeMapClamped(float value, float inLow, float inHigh, float outLow, float outHigh)
{
	float result = RangeMap(value, inLow, inHigh, outLow, outHigh);
	if (outLow < outHigh)
	{
		result = (result < outLow) ? outLow : result;
		result = (result > outHigh) ? outHigh : result;
	}
	else // range reversed, so test must be too
	{
		result = (result > outLow) ? outLow : result;
		result = (result < outHigh) ? outHigh : result;
	}
	return result;
}

float RangeMap(float value, float inLow, float inHigh, float outLow, float outHigh)
{
	float fraction = GetFractionWithin(value, inLow, inHigh);
	return Interpolate(outLow, outHigh, fraction);
}

int RangeMap(int value, int inLow, int inHigh, int outLow, int outHigh)
{
	float fraction = GetFractionWithin(value, inLow, inHigh);
	return Interpolate(outLow, outHigh, fraction);
}

float GetFractionWithin(float value, float inLow, float inHigh)
{
	if (inLow == inHigh)
		return 0.5f; // compromise to avoid endpoint anomalies

	return (value - inLow) / (inHigh - inLow);
}

float GetFractionWithin(int value, int inLow, int inHigh)
{
	if (inLow == inHigh)
		return 0.5f; // compromise to avoid endpoint anomalies

	return (float)(value - inLow) / (float)(inHigh - inLow);
}

float Interpolate(float outLow, float outHigh, float value)
{
	//return outLow + value * (outHigh - outLow);
	// return parametric version of value
	return (1.0f - value) * outLow + value * outHigh;
}

int Interpolate(int outLow, int outHigh, float value)
{
	//return outLow + value * (outHigh - outLow);
	// return parametric version of value
	return (int)((1.0f - value) * (float)outLow + value * (float)outHigh);
}

int RoundDownToInt(float value)
{
	return static_cast<int>(floorf(value));
}

float NormalizeByte(unsigned char byteValue)
{
	if (byteValue == 255)
		return 1.0f;
	return static_cast<float>(byteValue) / 256.0f;
}

unsigned char DenormalizeByte(float zeroToOne)
{
	if (zeroToOne == 1.0f)
		return 255;
	return static_cast<unsigned char>(zeroToOne * 256.0f);
}

float ComputeCubicBezier1D(float A, float B, float C, float D, float t)
{
	float AB = Interpolate(A, B, t);
	float BC = Interpolate(B, C, t);
	float CD = Interpolate(C, D, t);

	float ABC = Interpolate(AB, BC, t);
	float BCD = Interpolate(BC, CD, t);

	float ABCD = Interpolate(ABC, BCD, t);
	return ABCD;
}

float ComputeQuinticBezier1D(float A, float B, float C, float D, float E, float F, float t)
{
	float AB = Interpolate(A, B, t);
	float BC = Interpolate(B, C, t);
	float CD = Interpolate(C, D, t);
	float DE = Interpolate(D, E, t);
	float EF = Interpolate(E, F, t);

	float ABC = Interpolate(AB, BC, t);
	float BCD = Interpolate(BC, CD, t);
	float CDE = Interpolate(CD, DE, t);
	float DEF = Interpolate(DE, EF, t);

	float ABCD = Interpolate(ABC, BCD, t);
	float BCDE = Interpolate(BCD, CDE, t);
	float CDEF = Interpolate(CDE, DEF, t);

	float ABCDE = Interpolate(ABCD, BCDE, t);
	float BCDEF = Interpolate(BCDE, CDEF, t);

	float ABCDEF = Interpolate(ABCDE, BCDEF, t);
	return ABCDEF;
}

// bool IsPointInsideDisc2D(Vec2 const& point, Vec2 const& discCenter, float discRadius)
// {
// 	float distanceSquared = (point.x - discCenter.x) * (point.x - discCenter.x) + (point.y - discCenter.y) * (point.y - discCenter.y);
// 	if (distanceSquared < discRadius * discRadius)
// 		return true;
// 	return false;
// }

float GetClamped(float value, float minValue, float maxValue)
{
	if (value < minValue)
		return minValue;
	if (value > maxValue)
		return maxValue;
	return value;
}

Vec2 GetMovedTowardPoint(Vec2 fromPoint, Vec2 towardPoint, float maxTranslation)
{
	maxTranslation = fabsf(maxTranslation); // by convention the movement is positive
	Vec2 direction = towardPoint - fromPoint;
	if (direction.GetLength() < maxTranslation)
	{
		return towardPoint; // return actual point to get equality
	}
	else
	{
		direction.Normalize();
		return fromPoint + direction * maxTranslation;
	}
}

float GetMovedTowardScalar(float from, float to, float maxTranslation)
{
	maxTranslation = fabsf(maxTranslation); // by convention the movement is positive
	float movement = to - from;
	if (fabsf(movement) < maxTranslation)
	{
		return to;
	}
	else
	{
		if (movement > 0.0f)
			return from + maxTranslation;
		else
			return from - maxTranslation;
	}
}

bool IsPointInsideDisc2D(Vec2 const& point, Vec2 const& discCenter, float discRadius)
{
	Vec2 centerToPoint = discCenter - point;
	if (centerToPoint.GetLengthSquared() < discRadius * discRadius)
	{
		return true;
	}
	return false;
}

bool IsPointInsideAABB2D(Vec2 const& point, AABB2 const& box)
{
	Vec2 nearestPoint = GetNearestPointOnAABB2D(point, box);
	if (nearestPoint == point)
	{
		return true;
	}
	return false;
}

bool IsPointInsideCapsule2D(Vec2 const& point, Capsule2 const& capsule)
{
	Vec2 nearestPos = GetNearestPointOnLineSegment2D(point, capsule.m_startPos, capsule.m_endPos);
	if (GetDistanceSquared2D(nearestPos, point) > capsule.m_radius * capsule.m_radius)
	{
		return false;
	}
	return true;
}

bool IsPointInsideCapsule2D(Vec2 const& point, Vec2 const& boneStart, Vec2 const& boneEnd, float radius)
{
	Vec2 nearestPos = GetNearestPointOnLineSegment2D(point, boneStart, boneEnd);
	if (GetDistanceSquared2D(nearestPos, point) > radius * radius)
	{
		return false;
	}
	return true;
}

bool IsPointInsideOBB2D(Vec2 const& point, OBB2 const& orientedBox)
{
	Vec2 localPos = orientedBox.GetLocalPosForWorldPos(point);
	if (localPos.x < -orientedBox.m_halfDimensions.x || localPos.x > orientedBox.m_halfDimensions.x ||
		localPos.y < -orientedBox.m_halfDimensions.y || localPos.y > orientedBox.m_halfDimensions.y)
	{
		return false;
	}
	return true;
}
