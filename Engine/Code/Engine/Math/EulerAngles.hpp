#pragma once
#include <Engine/Math/Mat44.hpp>

struct EulerAngles
{
	static EulerAngles const ZERO;

public:
	float m_yawDegrees = 0.f;
	float m_pitchDegrees = 0.f;
	float m_rollDegrees = 0.f;

public:
	EulerAngles() = default;
	EulerAngles(Vec3 direction);
	EulerAngles(float yawDegrees, float pitchDegrees, float rollDegrees);
	void GetAsVectors_XFwd_YLeft_ZUp(Vec3& out_forwardIBasis, Vec3& out_leftJBasis, Vec3& out_upKBasis ) const; 
	Vec3 GetForwardNormal() const;
	Mat44 GetAsMatrix_XFwd_YLeft_ZUp() const;
	const EulerAngles operator*(float scale) const;
	void operator+=(const EulerAngles& eulerAngle);
	const EulerAngles operator+(const EulerAngles& eulerAngle) const;
	const EulerAngles operator-(const EulerAngles& eulerAngle) const;

	static Vec3 TestInversion(Vec3 test);

	friend const EulerAngles operator*(float scale, const EulerAngles& vecToScale);	// float * EulerAngles
};
