#include "Engine/Math/EulerAngles.hpp"
#include "MathUtils.hpp"

EulerAngles const EulerAngles::ZERO = EulerAngles(0.0f, 0.0f, 0.0f);

EulerAngles::EulerAngles(float yawDegrees, float pitchDegrees, float rollDegrees)
{
	m_yawDegrees = yawDegrees;
	m_pitchDegrees = pitchDegrees;
	m_rollDegrees = rollDegrees;
}

EulerAngles::EulerAngles(Vec3 direction)
{
	float length = direction.GetLength();
	m_yawDegrees = Atan2Degrees(direction.y, direction.x);
	m_pitchDegrees = ConvertRadiansToDegrees(-asinf(direction.z / length));
	m_rollDegrees = 0.0f; // default is no roll for pure direction vector
}

void EulerAngles::GetAsVectors_XFwd_YLeft_ZUp(Vec3& out_forwardIBasis, Vec3& out_leftJBasis, Vec3& out_upKBasis) const
{
	float cy = CosDegrees(m_yawDegrees);
	float sy = SinDegrees(m_yawDegrees);
	float cp = CosDegrees(m_pitchDegrees);
	float sp = SinDegrees(m_pitchDegrees);
	float cr = CosDegrees(m_rollDegrees);
	float sr = SinDegrees(m_rollDegrees);

	out_forwardIBasis.x = cy * cp;
	out_forwardIBasis.y = sy * cp;
	out_forwardIBasis.z = -sp;

	out_leftJBasis.x = (cy * sp * sr) - (sy * cr);
	out_leftJBasis.y = (sy * sp * sr) + (cy * cr);
	out_leftJBasis.z = cp * sr;

	out_upKBasis.x = (cy * sp * cr) + (sy * sr);
	out_upKBasis.y = (sy * sp * cr) - (cy * sr);
	out_upKBasis.z = cp * cr;
}

Vec3 EulerAngles::GetForwardNormal() const
{
	Vec3 i, j, k;
	GetAsVectors_XFwd_YLeft_ZUp(i, j, k);
	return i;
}

Mat44 EulerAngles::GetAsMatrix_XFwd_YLeft_ZUp() const
{
	Vec3 i;
	Vec3 j;
	Vec3 k;
	GetAsVectors_XFwd_YLeft_ZUp(i, j, k);
	return Mat44(i, j, k, Vec3(0.0f, 0.0f, 0.0f));
}

const EulerAngles EulerAngles::operator*(float scale) const
{
	return EulerAngles(m_yawDegrees * scale, m_pitchDegrees * scale, m_rollDegrees * scale);
}

void EulerAngles::operator+=(const EulerAngles& eulerAngle)
{
	m_yawDegrees += eulerAngle.m_yawDegrees;
	m_pitchDegrees += eulerAngle.m_pitchDegrees;
	m_rollDegrees += eulerAngle.m_rollDegrees;
}

const EulerAngles EulerAngles::operator+(const EulerAngles& eulerAngle) const
{
	return EulerAngles(m_yawDegrees + eulerAngle.m_yawDegrees, m_pitchDegrees + eulerAngle.m_pitchDegrees, m_rollDegrees + eulerAngle.m_rollDegrees);
}

const EulerAngles EulerAngles::operator-(const EulerAngles& eulerAngle) const
{
	return EulerAngles(m_yawDegrees - eulerAngle.m_yawDegrees, m_pitchDegrees - eulerAngle.m_pitchDegrees, m_rollDegrees - eulerAngle.m_rollDegrees);
}

Vec3 EulerAngles::TestInversion(Vec3 test)
{
	EulerAngles temp(test);
	Mat44 matrix = temp.GetAsMatrix_XFwd_YLeft_ZUp();
	Mat44 inverse = matrix.GetOrthonormalInverse();
	Vec3 xform = inverse.TransformPosition3D(test);
	return xform;
}

const EulerAngles operator*(float scale, const EulerAngles& eulerAngle)
{
	return EulerAngles(scale * eulerAngle.m_yawDegrees, scale * eulerAngle.m_pitchDegrees, scale * eulerAngle.m_rollDegrees);
}
