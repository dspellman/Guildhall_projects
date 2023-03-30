#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Capsule2.hpp"
#include "Engine/Math/LineSegment2.hpp"
#include "Engine/Math/OBB2.hpp"
#include "Engine/Math/Vec4.hpp"
#include "AABB3.hpp"

struct Vec2;
struct Vec3;
struct Rgba8;

// utility function
double Pi();
float Degrees0to360(float degrees);

// Trig Functions
float ConvertDegreesToRadians(float degrees);
float ConvertRadiansToDegrees(float radians);
float CosDegrees(float degrees);
float SinDegrees(float degrees);
float Atan2Degrees(float y, float x);
float GetShortestAngularDispDegrees(float fromDegrees, float toDegrees);
float GetTurnedTowardDegrees(float fromDegrees, float towardDegrees, float maxDeltaDegrees);

// Distance Functions
float GetDistance2D(Vec2 const& positionA, Vec2 const& positionB);
float GetDistanceSquared2D(Vec2 const& positionA, Vec2 const& positionB);
float GetDistance3D(Vec3 const& positionA, Vec3 const& positionB);
float GetDistanceSquared3D(Vec3 const& positionA, Vec3 const& positionB);
float GetDistanceXY3D(Vec3 const& positionA, Vec3 const& positionB);
float GetDistanceXYSquared3D(Vec3 const& positionA, Vec3 const& positionB);
int  GetTaxicabDistance2D( IntVec2 const& pointA, IntVec2 const& pointB );
float DotProduct2D( Vec2 const& a, Vec2 const& b );
float DotProduct3D( Vec3 const& a, Vec3 const& b );
float DotProduct4D( Vec4 const& a, Vec4 const& b );
float CrossProduct2D( Vec2 const& a, Vec2 const& b );
Vec3 CrossProduct3D( Vec3 const& a, Vec3 const& b);
float GetProjectedLength3D(Vec3 const& vectorToProject, Vec3 const& vectorToProjectOnto);
Vec3 const GetProjectedOnto3D(Vec3 const& vectorToProject, Vec3 const& vectorToProjectOnto);
float GetProjectedLength2D(Vec2 const& vectorToProject, Vec2 const& vectorToProjectOnto);
Vec2 const GetProjectedOnto2D( Vec2 const& vectorToProject, Vec2 const& vectorToProjectOnto );
float GetAngleDegreesBetweenVectors2D( Vec2 const& a, Vec2 const& b );

// L e r p and clamp utilities
float ClampZeroToOne( float value);
float Clamp( float value, float min, float max );
int Clamp(int value, int min, int max);
float GetRangeClamped(float value, float inLow, float inHigh, float outLow, float outHigh);
float RangeMapClamped(float value, float inLow, float inHigh, float outLow, float outHigh);
float RangeMap(float value, float inLow, float inHigh, float outLow, float outHigh);
int RangeMap(int value, int inLow, int inHigh, int outLow, int outHigh);
float GetFractionWithin(float value, float inLow, float inHigh);
float GetFractionWithin(int value, int inLow, int inHigh);
float Interpolate(float value, float outLow, float outHigh);
int Interpolate(int outLow, int outHigh, float value);
int RoundDownToInt(float value);
float NormalizeByte( unsigned char byteValue );
unsigned char DenormalizeByte( float zeroToOne );

// Bezier Curves
float ComputeCubicBezier1D( float A, float B, float C, float D, float t );
float ComputeQuinticBezier1D( float A, float B, float C, float D, float E, float F, float t );

// Geometry Functions
bool IsPointInsideDisc2D( Vec2 const& point, Vec2 const& discCenter, float discRadius );
bool DoDiscsOverlap(Vec2 const& centerA, float radiusA, Vec2 const& centerB, float radiusB);
bool DoSpheresOverlap(Vec3 const& centerA, float radiusA, Vec3 const& centerB, float radiusB);
bool IsPointInsideOrientedSector2D(Vec2 const& point, Vec2 const& sectorTip, float sectorForwardDegrees, float sectorApertureDegrees, float sectorRadius);
bool IsPointInsideDirectedSector2D(Vec2 const& point, Vec2 const& sectorTip, Vec2 const& sectorForwardNormal, float sectorApertureDegrees, float sectorRadius);
bool PushDiscOutOfPoint2D( Vec2& mobileDiscCenter, float discRadius, Vec2 const& fixedPoint );
bool PushDiscOutOfDisc2D(Vec2& mobileDiscCenter, float mobileRadius, Vec2 const& fixedDiscCenter, float fixedRadius);
bool PushDiscsOutOfEachOther2D(Vec2& aCenter, float aRadius, Vec2& bCenter, float bRadius);
bool PushDiscOutOfAABB2D( Vec2& mobileDiscCenter, float discRadius, AABB2 const& fixedBox );

// Transform Functions
void TransformPosition2D(Vec2& posToTransform, float uniformScaleXY, float rotationDegreesAboutZ, Vec2 const& translationXY);
void TransformPosition2D(Vec2& posToTransform, Vec2 const& iBasis, Vec2 const& jBasis, Vec2 const& translationXY);
void TransformPositionXY3D(Vec3& positionToTransform, float scaleXY, float zRotationDegrees, Vec2 const& translationXY);
void TransformPositionXY3D(Vec3& posToTransform, Vec2 const& iBasis, Vec2 const& jBasis, Vec2 const& translationXY);

// other functions
float GetClamped( float value, float minValue, float maxValue );
Vec2 GetMovedTowardPoint(Vec2 fromPoint, Vec2 towardPoint, float maxTranslation);
float GetMovedTowardScalar(float from, float to, float maxTranslation);

//bool  IsPointInsideDisc2D(Vec2 const& point, Vec2 const& discCenter, float discRadius);
bool  IsPointInsideAABB2D(Vec2 const& point, AABB2 const& box);
bool  IsPointInsideAABB3D(Vec3 const& point, AABB3 const& box);
bool  IsPointInsideCapsule2D(Vec2 const& point, Capsule2 const& capsule);
bool  IsPointInsideCapsule2D(Vec2 const& point, Vec2 const& boneStart, Vec2 const& boneEnd, float radius);
bool  IsPointInsideOBB2D(Vec2 const& point, OBB2 const& orientedSox);

// bool  IsPointInsideOrientedSector2D(Vec2 const& point, Vec2 const& sectorTip, float sectorForwardDegrees, float sectorApertureDegrees);
// bool  IsPointInsideDirectedSector2D(Vec2 const& point, Vec2 const& sectorTip, Vec2 const& sectorforwardNormal, float sectorApertureDegrees);
bool  DoDiscsOverlap2D(Vec2 const& centerA, float radiusA, Vec2 const& centerB, float radiusB);
bool  DoSpheresOverlap3D(Vec3 const& centerA, float radiusA, Vec3 const& centerB, float radiusB);
bool  DoZcylindersOverlap(Vec3 const& centerA, float heightA, float radiusA, Vec3 const& centerB, float heightB, float radiusB);
bool  DoAABB3BoxesOverlap(AABB3 const& boundsA, AABB3 const& boundsB);
bool  DoesSphereOverlapAABB3(Vec3 const& center, float radius, AABB3 const& box);
bool  DoesSphereOverlapZCylinder(Vec3 const& center, float radius, Vec3 const& Zcenter, float Zheight, float Zradius);
bool  DoesZCylinderOverlapAABB3(Vec3 const& center, float height, float radius, AABB3 const& box);
bool  DoesAABB3OverlapZCylinder(AABB3 const& box, Vec3 const& center, float height, float radius);

Vec2 const GetNearestPointOnDisc2D(Vec2 const& referencePoint, Vec2 const& discCenter, float discRadius);
Vec2 const GetNearestPointOnAABB2D(Vec2 const& referencepos, AABB2 box);
Vec2 const GetNearestPointOnInfiniteline2D(Vec2 const& referencePos, LineSegment2 const& infiniteLine);
Vec2 const GetNearestPointOnInfiniteline2D(Vec2 const& referencePos, Vec2 const& pointOnLine, Vec2 const& anotherPointOnLine);
Vec2 const GetNearestPointOnLineSegment2D(Vec2 const& referencePos, LineSegment2 const& lineSegment);
Vec2 const GetNearestPointOnLineSegment2D(Vec2 const& referencepos, Vec2 const& lineSegStart, Vec2 const& lineSegEnd);
Vec2 const GetNearestPointOnCapsule2D(Vec2 const& referencepos, Capsule2 const& capsule);
Vec2 const GetNearestPointOnCapsule2D(Vec2 const& referencePos, Vec2 const& boneStart, Vec2 const& boneEnd, float radius);
Vec2 const GetNearestPointOnOBB2D(Vec2 const& referencePos, OBB2 const& orientedBox);

Vec3 const GetNearestPointOnAABB3D(Vec3 const& referencepos, AABB3 const& box);
Vec3 const GetNearestPointOnSphere(Vec3 const& referencePoint, Vec3 const& center, float radius);
Vec3 const GetNearestPointOnCylinder(Vec3 const& referencepos, Vec3 const& center, float height, float radius);