#include "Engine/Math/RaycastUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "FloatRange.hpp"

RaycastResult2D RaycastVsLineSegment2D(Vec2 startPos, Vec2 fwdNormal, float maxDist, Vec2 lineStart, Vec2 lineEnd)
{
	// set up default return for no intersection
	RaycastResult2D raycastResult;
	raycastResult.m_didImpact = false;
	raycastResult.m_impactDist = maxDist;
	raycastResult.m_impactPos = startPos + fwdNormal * maxDist;
	raycastResult.m_impactNormal = fwdNormal; // this should not matter
	raycastResult.m_rayFwdNormal = fwdNormal;
	raycastResult.m_rayStartPos = startPos;
	raycastResult.m_rayMaxLength = maxDist;

	Vec2 S = lineStart;
	Vec2 E = lineEnd;
	Vec2 SE = E - S;
	Vec2 i = fwdNormal;
	Vec2 j(-i.y, i.x);
	Vec2 R = startPos;
	float M = maxDist;

	Vec2 RE = E - R;
	float REj = DotProduct2D(RE, j);
	float REi = DotProduct2D(RE, i);
	Vec2 RS = S - R;
	float RSj = DotProduct2D(RS, j);
	float RSi = DotProduct2D(RS, i);

	// early exit tests
	if (REj * RSj > 0.0f)
	{
		return raycastResult;
	}

	if (REi < 0.0f && RSi < 0.0f)
	{
		return raycastResult;
	}

	if (REi > M && RSi > M)
	{
		return raycastResult;
	}

	float fraction = RSj / (RSj - REj);
	Vec2 I = S + fraction * SE;
	Vec2 RI = I - R;
	float impactDistance = DotProduct2D(RI, i);

	if (impactDistance < 0.0f || impactDistance > maxDist)
	{
		return raycastResult;
	}

	// apparent intersection at impact distance
	raycastResult.m_didImpact = true;
	//raycastResult.m_fraction = fraction;
	raycastResult.m_impactDist = impactDistance;
	raycastResult.m_impactPos = I; //S + impactDistance * i;  THIS CODE WAS IN THE NOTES
	raycastResult.m_impactNormal = SE.GetRotated90Degrees().GetNormalized();
	if (RSj > 0.0f)
	{
		raycastResult.m_impactNormal *= -1.0f;
	}
	return raycastResult;
}

// circle raycast //////////////////////////////////////////////////////////////////////////////////////////////
RaycastResult2D RaycastVsDisc2D(Vec2 startPos, Vec2 fwdNormal, float maxDist, Vec2 discCenter, float discRadius)
{
	RaycastResult2D raycastResult;
	Vec2 SC = discCenter - startPos;
	Vec2 i = fwdNormal; 
	Vec2 j = Vec2(-i.y, i.x);
	float SCi = DotProduct2D(SC, i);
	float SCj = DotProduct2D(SC, j);

	// set up default return for no intersection
	raycastResult.m_didImpact = false;
	raycastResult.m_impactDist = maxDist;
	raycastResult.m_impactPos = startPos + fwdNormal * maxDist;
	raycastResult.m_impactNormal = fwdNormal; // this should not matter
	raycastResult.m_rayFwdNormal = fwdNormal;
	raycastResult.m_rayStartPos = startPos;
	raycastResult.m_rayMaxLength = maxDist;

	if (SCj > discRadius || SCj < -discRadius)
	{
		return raycastResult; // did not hit this disc
	}

	if (SCi > maxDist + discRadius)
	{
		return raycastResult; // did not hit this disc
	}

	if (SCi < -discRadius)
	{
		return raycastResult; // did not hit this disc
	}

	// test if the ray starts in a disc
	if (IsPointInsideDisc2D(startPos, discCenter, discRadius))
	{
		raycastResult.m_didImpact = true;
		raycastResult.m_impactDist = 0.0f; // (discCenter - startPos).GetLength(); // this is artificial to sort which disc is "nearest" to the ray
		raycastResult.m_impactPos = startPos;
		raycastResult.m_impactNormal = Vec2(-fwdNormal.x, -fwdNormal.y);
		raycastResult.m_rayFwdNormal = fwdNormal;
		raycastResult.m_rayStartPos = startPos;
		raycastResult.m_rayMaxLength = maxDist;
		return raycastResult;
	}

	float a = sqrtf(discRadius * discRadius - SCj * SCj);
	float impactDist = SCi - a;
	if (impactDist < 0 || impactDist > maxDist)
	{
		return raycastResult; // before or after ray
	}

	// ray intersects disc, so return hit
	Vec2 impactPos = startPos + i * impactDist;
	raycastResult.m_didImpact = true;
	raycastResult.m_impactDist = impactDist;
	raycastResult.m_impactPos = impactPos;
	raycastResult.m_impactNormal = (impactPos - discCenter).GetNormalized();
	raycastResult.m_rayFwdNormal = fwdNormal;
	raycastResult.m_rayStartPos = startPos;
	raycastResult.m_rayMaxLength = maxDist;
	return raycastResult;
}

float min(float tx1, float tx2)
{
	return (tx1 < tx2) ? tx1 : tx2;
}

float max(float tx1, float tx2)
{
	return (tx1 > tx2) ? tx1 : tx2;
}

bool intersection(AABB2 b, Vec2 s, Vec2 n) 
{
	float tx1 = (b.m_mins.x - s.x) / n.x;
	float tx2 = (b.m_maxs.x - s.x) / n.x;

	float tmin = min(tx1, tx2);
	float tmax = max(tx1, tx2);

	float ty1 = (b.m_mins.y - s.y) / n.y;
	float ty2 = (b.m_maxs.y - s.y) / n.y;

	tmin = max(tmin, min(ty1, ty2));
	tmax = min(tmax, max(ty1, ty2));

	return tmax >= tmin;
}

// AABB2 raycast ////////////////////////////////////////////////////////////////////////////
RaycastResult2D RaycastVsAABB2D(Vec2 startPos, Vec2 fwdNormal, float maxDist, AABB2 bounds)
{
	RaycastResult2D raycastResult;

	// set up default return for no intersection
	raycastResult.m_didImpact = false;
	raycastResult.m_impactDist = maxDist;
	raycastResult.m_impactPos = startPos + fwdNormal * maxDist;
	raycastResult.m_impactNormal = fwdNormal; // this should not matter
	raycastResult.m_rayFwdNormal = fwdNormal;
	raycastResult.m_rayStartPos = startPos;
	raycastResult.m_rayMaxLength = maxDist;

	if (IsPointInsideAABB2D(startPos, bounds) == true)
	{
		raycastResult.m_didImpact = true;
		raycastResult.m_impactDist = 0.0f; // (discCenter - startPos).GetLength(); // this is artificial to sort which disc is "nearest" to the ray
		raycastResult.m_impactPos = startPos;
		raycastResult.m_impactNormal = Vec2(-fwdNormal.x, -fwdNormal.y);
		return raycastResult;
	}

	// if the ray is vertical or horizontal, we need a special test
	if (fwdNormal.y == 0.0f)
	{
		if (startPos.y < bounds.m_mins.y || startPos.y > bounds.m_maxs.y)
			return raycastResult;
		if (startPos.x + fwdNormal.x * maxDist >= bounds.m_mins.x)
		{
			raycastResult.m_didImpact = true;
			raycastResult.m_impactDist = bounds.m_mins.x - startPos.x;
			raycastResult.m_impactPos = Vec2(bounds.m_mins.x, startPos.y);
			raycastResult.m_impactNormal = Vec2(-1.0f, 0.0f);
			return raycastResult;
		}
		if (startPos.x + fwdNormal.x * maxDist <= bounds.m_maxs.x)
		{
			raycastResult.m_didImpact = true;
			raycastResult.m_impactDist = startPos.x - bounds.m_maxs.x;
			raycastResult.m_impactPos = Vec2(bounds.m_maxs.x, startPos.y);
			raycastResult.m_impactNormal = Vec2(1.0f, 0.0f);
			return raycastResult;
		}
	}
	
	if (fwdNormal.x == 0.0f)
	{
		if (startPos.x < bounds.m_mins.x || startPos.x > bounds.m_maxs.x)
			return raycastResult;
		if (startPos.y + fwdNormal.y * maxDist >= bounds.m_mins.y)
		{
			raycastResult.m_didImpact = true;
			raycastResult.m_impactDist = bounds.m_mins.y - startPos.y;
			raycastResult.m_impactPos = Vec2(startPos.x, bounds.m_mins.y);
			raycastResult.m_impactNormal = Vec2(0.0f, -1.0f);
			return raycastResult;
		}
		if (startPos.y + fwdNormal.y * maxDist <= bounds.m_maxs.y)
		{
			raycastResult.m_didImpact = true;
			raycastResult.m_impactDist = startPos.y - bounds.m_maxs.y;
			raycastResult.m_impactPos = Vec2(startPos.x, bounds.m_maxs.y);
			raycastResult.m_impactNormal = Vec2(0.0f, 1.0f);
			return raycastResult;
		}
	}

	// otherwise do the regular slope tests
	float iInverse = 1.0f / (fwdNormal.x * maxDist);
	float left = (bounds.m_mins.x - startPos.x) * iInverse;
	left = left < 0.0f ? 2.0f : left;
	float right = (bounds.m_maxs.x - startPos.x) * iInverse;
	right = right < 0.0f ? 2.0f : right;

	float jInverse = 1.0f / (fwdNormal.y * maxDist);
	float bottom = (bounds.m_mins.y - startPos.y) * jInverse;
	bottom = bottom < 0.0f ? 2.0f : bottom;
	float top = (bounds.m_maxs.y - startPos.y) * jInverse;
	top = top < 0.0f ? 2.0f : top;

	Vec2 impactNormali = fwdNormal;
	Vec2 impactNormalj = fwdNormal;

	float ti = 2.0f; // sentinel value greater than 1.0f
	float tj = 2.0f;

	if (left < right)
	{
		ti = left;
		impactNormali = Vec2(-1.0f, 0.0f);
	}
	else
	{
		ti = right;
		impactNormali = Vec2(1.0f, 0.0f);
	}
	if (FloatRange(bounds.m_mins.y, bounds.m_maxs.y).IsOnRange(startPos.y + fwdNormal.y * maxDist * ti) == false)
	{
		ti = 2.0f;
	}

	if (bottom < top)
	{
		tj = bottom;
		impactNormalj = Vec2(0.0f, -1.0f);
	}
	else
	{
		tj = top;
		impactNormalj = Vec2(0.0f, 1.0f);
	}
	if (FloatRange(bounds.m_mins.x, bounds.m_maxs.x).IsOnRange(startPos.x + fwdNormal.x * maxDist * tj) == false)
	{
		tj = 2.0f;
	}

	// hit if t is in range
	float t;
	Vec2 impactNormal;
	if (ti < tj)
	{
		t = ti;
		impactNormal = impactNormali;
	}
	else
	{
		t = tj;
		impactNormal = impactNormalj;
	}
	if (FloatRange::ZERO_TO_ONE.IsOnRange(t))
	{
		raycastResult.m_didImpact = true;
		raycastResult.m_impactDist = t * maxDist;
		raycastResult.m_impactPos = startPos + t * fwdNormal * maxDist;
		raycastResult.m_impactNormal = impactNormal;
		return raycastResult;
	}

	// did not hit so return default
	return raycastResult;
}

RaycastResult2D RaycastVsOBB2D(Vec2 startPos, Vec2 fwdNormal, float maxDist, OBB2 bounds)
{
	// set up default return for no intersection
	RaycastResult2D raycastResult;
	raycastResult.m_didImpact = false;
	raycastResult.m_impactDist = maxDist;
	raycastResult.m_impactPos = startPos + fwdNormal * maxDist;
	raycastResult.m_impactNormal = fwdNormal; // this should not matter
	raycastResult.m_rayFwdNormal = fwdNormal;
	raycastResult.m_rayStartPos = startPos;
	raycastResult.m_rayMaxLength = maxDist;

	if (IsPointInsideOBB2D(startPos, bounds) == true)
	{
		raycastResult.m_didImpact = true;
		raycastResult.m_impactDist = 0.0f; // (discCenter - startPos).GetLength(); // this is artificial to sort which disc is "nearest" to the ray
		raycastResult.m_impactPos = startPos;
		raycastResult.m_impactNormal = Vec2(-fwdNormal.x, -fwdNormal.y);
		return raycastResult;
	}

	Vec2 S = startPos;
	Vec2 E = S + fwdNormal * maxDist;
	Vec2 P = bounds.m_center - startPos;
	Vec2 i = bounds.m_iBasisNormal;
	Vec2 j(-i.y, i.x);

	// calculate scaling factors
	float scaleX = 1.0f / DotProduct2D(fwdNormal, i);
	float scaleY = 1.0f / DotProduct2D(fwdNormal, j);

	// calculate intersection points on each side scaled to t
	float left = (DotProduct2D(P, i) - bounds.m_halfDimensions.x) * scaleX;
	float right = (DotProduct2D(P, i) + bounds.m_halfDimensions.x) * scaleX;
	float bottom = (DotProduct2D(P, j) - bounds.m_halfDimensions.y) * scaleY;
	float top = (DotProduct2D(P, j) + bounds.m_halfDimensions.y) * scaleY;

	// find the closest intersection point t
	float tmin = max(min(left, right), min(bottom, top));
	float tmax = min(max(left, right), max(bottom, top));

	if (tmin < 0.0f && tmax < 0.0f)
	{
			return raycastResult; // box is behind ray
	}

	if (tmin > maxDist && tmax > maxDist)
	{
		return raycastResult; // box is beyond ray
	}

	if (tmin > tmax)
	{
		return raycastResult;
	}

	// choose the correct normal
	if (tmin == left)
	{
		raycastResult.m_impactNormal = -i;
	}
	if (tmin == right)
	{
		raycastResult.m_impactNormal = i;
	}
	if (tmin == bottom)
	{
		raycastResult.m_impactNormal = -j;
	}
	if (tmin == top)
	{
		raycastResult.m_impactNormal = j;
	}

	raycastResult.m_didImpact = true;
	raycastResult.m_impactDist = tmin;
	raycastResult.m_impactPos = startPos + tmin * fwdNormal;
	return raycastResult;
}

// 3D sphere raycast ///////////////////////////////////////////////////////////////////////////////////
RaycastResult3D RaycastVsSphere(Vec3 startPos, Vec3 fwdNormal, float maxDist, Vec3 center, float radius)
{
	RaycastResult3D	raycastResult;
	Vec3 endPos = startPos + fwdNormal * maxDist;

	// set up default for no hit
	raycastResult.m_didImpact = false;
	raycastResult.m_impactDist = maxDist;
	raycastResult.m_impactPos = endPos;
	raycastResult.m_impactNormal = fwdNormal; // this should not matter
	raycastResult.m_rayFwdNormal = fwdNormal;
	raycastResult.m_rayStartPos = startPos;
	raycastResult.m_rayMaxLength = maxDist;

	Vec3 SC = center - startPos;
	float sci = DotProduct3D(SC, fwdNormal);
	// test if impact point is beyond ray distance
	if (sci > maxDist + radius)
	{
		return raycastResult;
	}

	// test if sphere is behind the ray
	if (sci < -radius)
	{
		return raycastResult;
	}

	// test whether raycast starts inside sphere
	if (SC.GetLengthSquared() < radius * radius)
	{
		raycastResult.m_didImpact = true;
		raycastResult.m_impactDist = 0.0f;
		raycastResult.m_impactPos = startPos;
		raycastResult.m_impactNormal = Vec3(-fwdNormal.x, -fwdNormal.y, -fwdNormal.z);
		return raycastResult;
	}

	Vec3 SCi = GetProjectedOnto3D(SC, fwdNormal);
	Vec3 SCj = SC - SCi;
	// test whether the ray is outside the radius of the sphere
	if (radius * radius < SCj.GetLengthSquared())
	{
		return raycastResult;
	}

	float a = sqrtf(radius * radius - SCj.GetLengthSquared());
	float distance = SCi.GetLength() - a;
	// test whether the ray reaches the sphere
	if (distance < 0.0f || distance > maxDist)
	{
		return raycastResult;
	}

	Vec3 impactPos = startPos + distance * fwdNormal;

	raycastResult.m_didImpact = true;
	raycastResult.m_impactDist = distance;
	raycastResult.m_impactPos = impactPos;
	raycastResult.m_impactNormal = (impactPos - center).GetNormalized();
	return raycastResult;
}

//////////////////////////////////////////////////////////////////////////////
// cylinder center is middle of cylinder (+/- 0.5f * length above/below plane)
RaycastResult3D RaycastVsZCylinder(Vec3 start, Vec3 fwdNormal, float maxDist, Vec3 center, float length, float radius)
{
	RaycastResult3D	raycastResult;
	Vec3 end = start + fwdNormal * maxDist;
	Vec2 center2D(center.x, center.y);
	Vec2 flattened(start.x, start.y);

	// set up default for no hit
	raycastResult.m_didImpact = false;
	raycastResult.m_impactDist = maxDist;
	raycastResult.m_impactPos = end;
	raycastResult.m_impactNormal = fwdNormal; // this should not matter
	raycastResult.m_rayFwdNormal = fwdNormal;
	raycastResult.m_rayStartPos = start;
	raycastResult.m_rayMaxLength = maxDist;

	// Step 0: test if the start is inside the cylinder
	if ((flattened - center2D).GetLengthSquared() < radius * radius)
	{
		if (start.z < center.z + 0.5f * length && start.z > center.z - 0.5f * length)
		{
			raycastResult.m_didImpact = true;
			raycastResult.m_impactDist = 0.0f;
			raycastResult.m_impactPos = start;
			raycastResult.m_impactNormal = Vec3(-fwdNormal.x, -fwdNormal.y, -fwdNormal.z);
			return raycastResult;
		}
	}

	// Step 1: test for intersection with column
	Vec2 normal2D(fwdNormal.x, fwdNormal.y);
	float maxDist2D = normal2D.GetLength() * maxDist;
	normal2D.Normalize();
	RaycastResult2D circleResult = RaycastVsDisc2D(flattened, normal2D, maxDist2D, Vec2(center.x, center.y), radius);
	if (circleResult.m_didImpact == false)
	{
		return raycastResult; // this is our unqualified early exit point
	}

	// test column hit for inside z-range (+/- 0.5f * length)
	float t = circleResult.m_impactDist / maxDist2D;
	Vec3 midHit = start + t * fwdNormal * maxDist;
	if ((midHit.z <= center.z + 0.5f * length) && (midHit.z >= center.z - 0.5f * length))
	{
		// column intersection is within z-axis range of cylinder
		raycastResult.m_didImpact = true;
		raycastResult.m_impactDist = t * maxDist;
		raycastResult.m_impactPos = midHit;
		raycastResult.m_impactNormal = Vec3(circleResult.m_impactNormal.x, circleResult.m_impactNormal.y, 0.0f);
		return raycastResult;
	}

	// Step 2: test top down intersection
	if (fwdNormal.z < 0.0f) // ray aimed down
	{
		t = (center.z + 0.5f * length - start.z) / (end.z - start.z);
		if (t >= 0.0 && t <= 1.0f)
		{
			Vec3 topHit = start + t * fwdNormal * maxDist;
			if ((topHit.x - center.x) * (topHit.x - center.x) + (topHit.y - center.y) * (topHit.y - center.y) < radius * radius)
			{
				// ray intersects disc, so return hit
				raycastResult.m_didImpact = true;
				raycastResult.m_impactDist = t * maxDist;
				raycastResult.m_impactPos = topHit;
				raycastResult.m_impactNormal = Vec3(0.0f, 0.0f, 1.0f);
				return raycastResult;
			}
		}
	}
	
	// Step 3:  test bottom up intersection
	if (fwdNormal.z > 0.0f) // ray aimed up
	{
		t = (center.z - 0.5f * length - start.z) / (end.z - start.z);
		if (t >= 0.0 && t <= 1.0f)
		{
			Vec3 bottomHit = start + t * fwdNormal * maxDist;
			if ((bottomHit.x - center.x) * (bottomHit.x - center.x) + (bottomHit.y - center.y) * (bottomHit.y - center.y) < radius * radius)
			{
				// ray intersects disc, so return hit
				raycastResult.m_didImpact = true;
				raycastResult.m_impactDist = t * maxDist;
				raycastResult.m_impactPos = bottomHit;
				raycastResult.m_impactNormal = Vec3(0.0f, 0.0f, -1.0f);
				return raycastResult;
			}
		}
	}

	// no hit found so return default
	return raycastResult;
}

// AABB3 raycast ////////////////////////////////////////////////////////////////////////////
RaycastResult3D RaycastVsAABB3D(Vec3 startPos, Vec3 fwdNormal, float maxDist, AABB3 bounds)
{
	RaycastResult3D raycastResult;

	// set up default return for no intersection
	raycastResult.m_didImpact = false;
	raycastResult.m_impactDist = maxDist;
	raycastResult.m_impactPos = startPos + fwdNormal * maxDist;
	raycastResult.m_impactNormal = fwdNormal; // this should not matter
	raycastResult.m_rayFwdNormal = fwdNormal;
	raycastResult.m_rayStartPos = startPos;
	raycastResult.m_rayMaxLength = maxDist;

	// Step 0: test if the start is inside the box
	if (IsPointInsideAABB3D(startPos, bounds) == true)
	{
		raycastResult.m_didImpact = true;
		raycastResult.m_impactDist = 0.0f; // (discCenter - startPos).GetLength(); // this is artificial to sort which disc is "nearest" to the ray
		raycastResult.m_impactPos = startPos;
		raycastResult.m_impactNormal = Vec3(-fwdNormal.x, -fwdNormal.y, -fwdNormal.z);
		return raycastResult;
	}

	// Step 1: test for intersection with column
	Vec2 flattened(startPos.x, startPos.y);
	Vec2 normal2D(fwdNormal.x, fwdNormal.y);
	float maxDist2D = normal2D.GetLength() * maxDist;
	normal2D.Normalize();

	RaycastResult2D squareResult = RaycastVsAABB2D(flattened, normal2D, maxDist2D, AABB2(bounds.m_mins.x, bounds.m_mins.y, bounds.m_maxs.x, bounds.m_maxs.y));
	if (squareResult.m_didImpact == false)
	{
		return raycastResult; // this is our unqualified early exit point
	}

	// test column hit for inside z-range (+/- 0.5f * length)
	float t = squareResult.m_impactDist / maxDist2D;
	Vec3 midHit = startPos + t * fwdNormal * maxDist;
	if ((midHit.z <= bounds.m_maxs.z) && (midHit.z >= bounds.m_mins.z))
	{
		// column intersection is within z-axis range of box
		raycastResult.m_didImpact = true;
		raycastResult.m_impactDist = t * maxDist;
		raycastResult.m_impactPos = midHit;
		raycastResult.m_impactNormal = Vec3(squareResult.m_impactNormal.x, squareResult.m_impactNormal.y, 0.0f);
		return raycastResult;
	}

	// variables for testing top and bottom hits
	Vec3 endPos = startPos + fwdNormal * maxDist;
	FloatRange xRange(bounds.m_mins.x, bounds.m_maxs.x);
	FloatRange yRange(bounds.m_mins.y, bounds.m_maxs.y);

	// Step 2: test top down intersection
	if (fwdNormal.z < 0.0f) // ray aimed down
	{
		t = (bounds.m_maxs.z - startPos.z) / (endPos.z - startPos.z);
		if (t >= 0.0 && t <= 1.0f)
		{
			Vec3 topHit = startPos + t * fwdNormal * maxDist;
			if (xRange.IsOnRange(topHit.x) && yRange.IsOnRange(topHit.y))
			{
				// ray intersects disc, so return hit
				raycastResult.m_didImpact = true;
				raycastResult.m_impactDist = t * maxDist;
				raycastResult.m_impactPos = topHit;
				raycastResult.m_impactNormal = Vec3(0.0f, 0.0f, 1.0f);
				return raycastResult;
			}
		}
	}
	
	// Step 3:  test bottom up intersection
	if (fwdNormal.z > 0.0f) // ray aimed up
	{
		t = (bounds.m_mins.z - startPos.z) / (endPos.z - startPos.z);
		if (t >= 0.0 && t <= 1.0f)
		{
			Vec3 bottomHit = startPos + t * fwdNormal * maxDist;
			if (xRange.IsOnRange(bottomHit.x) && yRange.IsOnRange(bottomHit.y))
			{
				// ray intersects disc, so return hit
				raycastResult.m_didImpact = true;
				raycastResult.m_impactDist = t * maxDist;
				raycastResult.m_impactPos = bottomHit;
				raycastResult.m_impactNormal = Vec3(0.0f, 0.0f, -1.0f);
				return raycastResult;
			}
		}
	}

	// no hit found so return default
	return raycastResult;
}


