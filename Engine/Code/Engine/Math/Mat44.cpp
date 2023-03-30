#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Core/EngineCommon.hpp"

// Construction
Mat44::Mat44(Vec2 const& iBasis2D, Vec2 const& jBasis2D, Vec2 const& translation2D)
{
	m_values[Ix] = iBasis2D.x;	m_values[Jx] = jBasis2D.x;	m_values[Kx] = 0.0f;		m_values[Tx] = translation2D.x;
	m_values[Iy] = iBasis2D.y;	m_values[Jy] = jBasis2D.y;	m_values[Ky] = 0.0f;		m_values[Ty] = translation2D.y;
	m_values[Iz] = 0.0f;		m_values[Jz] = 0.0f;		m_values[Kz] = 1.0f;		m_values[Tz] = 0.0f;
	m_values[Iw] = 0.0f;		m_values[Jw] = 0.0f;		m_values[Kw] = 0.0f;		m_values[Tw] = 1.0f;
}

Mat44::Mat44(Vec3 const& iBasis3D, Vec3 const& jBasis3D, Vec3 const& kBasis3D, Vec3 const& translation3D)
{
	m_values[Ix] = iBasis3D.x;	m_values[Jx] = jBasis3D.x;	m_values[Kx] = kBasis3D.x;	m_values[Tx] = translation3D.x;
	m_values[Iy] = iBasis3D.y;	m_values[Jy] = jBasis3D.y;	m_values[Ky] = kBasis3D.y;	m_values[Ty] = translation3D.y;
	m_values[Iz] = iBasis3D.z;	m_values[Jz] = jBasis3D.z;	m_values[Kz] = kBasis3D.z;	m_values[Tz] = translation3D.z;
	m_values[Iw] = 0.0f;		m_values[Jw] = 0.0f;		m_values[Kw] = 0.0f;		m_values[Tw] = 1.0f;
}

Mat44::Mat44(Vec4 const& iBasis4D, Vec4 const& jBasis4D, Vec4 const& kBasis4D, Vec4 const& translation4D)
{
	m_values[Ix] = iBasis4D.x;		m_values[Jx] = jBasis4D.x;		m_values[Kx] = kBasis4D.x;		m_values[Tx] = translation4D.x;
	m_values[Iy] = iBasis4D.y;		m_values[Jy] = jBasis4D.y;		m_values[Ky] = kBasis4D.y;		m_values[Ty] = translation4D.y;
	m_values[Iz] = iBasis4D.z;		m_values[Jz] = jBasis4D.z;		m_values[Kz] = kBasis4D.z;		m_values[Tz] = translation4D.z;
	m_values[Iw] = iBasis4D.w;		m_values[Jw] = jBasis4D.w;		m_values[Kw] = kBasis4D.w;		m_values[Tw] = translation4D.w;
}

Mat44::Mat44(float const* sixteenValuesBasisMajor)
{
	float const*& svbm = sixteenValuesBasisMajor;
	m_values[Ix] = svbm[0];		m_values[Jx] = svbm[4];		m_values[Kx] = svbm[8];		m_values[Tx] = svbm[12];
	m_values[Iy] = svbm[1];		m_values[Jy] = svbm[5];		m_values[Ky] = svbm[9];		m_values[Ty] = svbm[13];
	m_values[Iz] = svbm[2];		m_values[Jz] = svbm[6];		m_values[Kz] = svbm[10];	m_values[Tz] = svbm[14];
	m_values[Iw] = svbm[3];		m_values[Jw] = svbm[7];		m_values[Kw] = svbm[11];	m_values[Tw] = svbm[15];
}

Mat44::Mat44()
{
	m_values[Ix] = 1.0f;		m_values[Jx] = 0.0f;		m_values[Kx] = 0.0f;		m_values[Tx] = 0.0f;
	m_values[Iy] = 0.0f;		m_values[Jy] = 1.0f;		m_values[Ky] = 0.0f;		m_values[Ty] = 0.0f;
	m_values[Iz] = 0.0f;		m_values[Jz] = 0.0f;		m_values[Kz] = 1.0f;		m_values[Tz] = 0.0f;
	m_values[Iw] = 0.0f;		m_values[Jw] = 0.0f;		m_values[Kw] = 0.0f;		m_values[Tw ]= 1.0f;
}

// Translation
Mat44 const Mat44::CreateTranslation2D(Vec2 const& translationXY)
{
	Mat44 xlat2d = Mat44();
	xlat2d.m_values[Tx] = translationXY.x;
	xlat2d.m_values[Ty] = translationXY.y;
	return xlat2d;
}

Mat44 const Mat44::CreateTranslation3D(Vec3 const& translationXYZ)
{
	Mat44 xlat3d = Mat44();
	xlat3d.m_values[Tx] = translationXYZ.x;
	xlat3d.m_values[Ty] = translationXYZ.y;	
	xlat3d.m_values[Tz] = translationXYZ.z;
	return xlat3d;
}

// Scaling
Mat44 const Mat44::CreateUniformScale2D(float uniformScaleXY)
{
	Mat44 scaled = Mat44();
	scaled.m_values[Ix] = uniformScaleXY;	
	scaled.m_values[Jy] = uniformScaleXY;
	return scaled;
}

Mat44 const Mat44::CreateUniformScale3D(float uniformScaleXYZ)
{
	Mat44 scaled = Mat44();
	scaled.m_values[Ix] = uniformScaleXYZ;
	scaled.m_values[Jy] = uniformScaleXYZ;
	scaled.m_values[Kz] = uniformScaleXYZ;
	return scaled;
}

Mat44 const Mat44::CreateNonUniformScale2D(Vec2 const& nonuniformScaleXY)
{
	Mat44 scaled = Mat44();
	scaled.m_values[Ix] = nonuniformScaleXY.x;
	scaled.m_values[Jy] = nonuniformScaleXY.y;
	return scaled;
}

Mat44 const Mat44::CreateNonUniformScale3D(Vec3 const& nonUniformScaleXYZ)
{
	Mat44 scaled = Mat44();
	scaled.m_values[Ix] = nonUniformScaleXYZ.x;
	scaled.m_values[Jy] = nonUniformScaleXYZ.y;
	scaled.m_values[Kz] = nonUniformScaleXYZ.z;
	return scaled;
}

// Rotation
Mat44 const Mat44::CreateZRotationDegrees(float rotationDegreesAboutZ)
{
	float cos = CosDegrees(rotationDegreesAboutZ);
	float sin = SinDegrees(rotationDegreesAboutZ);
	Mat44 rotated;
	rotated.m_values[Ix] = cos;		rotated.m_values[Jx] = -sin;	rotated.m_values[Kx] = 0.0f;		rotated.m_values[Tx] = 0.0f;
	rotated.m_values[Iy] = sin;		rotated.m_values[Jy] = cos;		rotated.m_values[Ky] = 0.0f;		rotated.m_values[Ty] = 0.0f;
	rotated.m_values[Iz] = 0.0f;	rotated.m_values[Jz] = 0.0f;	rotated.m_values[Kz] = 1.0f;		rotated.m_values[Tz] = 0.0f;
	rotated.m_values[Iw] = 0.0f;	rotated.m_values[Jw] = 0.0f;	rotated.m_values[Kw] = 0.0f;		rotated.m_values[Tw] = 1.0f;
	return rotated;
}

Mat44 const Mat44::CreateXRotationDegrees(float rotationDegreesAboutX)
{
	float cos = CosDegrees(rotationDegreesAboutX);
	float sin = SinDegrees(rotationDegreesAboutX);
	Mat44 rotated;
	rotated.m_values[Ix] = 1.0f;	rotated.m_values[Jx] = 0.0f;	rotated.m_values[Kx] = 0.0f;	rotated.m_values[Tx] = 0.0f;
	rotated.m_values[Iy] = 0.0f;	rotated.m_values[Jy] = cos;		rotated.m_values[Ky] = -sin;	rotated.m_values[Ty] = 0.0f;
	rotated.m_values[Iz] = 0.0f;	rotated.m_values[Jz] = sin;		rotated.m_values[Kz] = cos;		rotated.m_values[Tz] = 0.0f;
	rotated.m_values[Iw] = 0.0f;	rotated.m_values[Jw] = 0.0f;	rotated.m_values[Kw] = 0.0f;	rotated.m_values[Tw] = 1.0f;
	return rotated;
}

Mat44 const Mat44::CreateYRotationDegrees(float rotationDegreesAboutY)
{
	float cos = CosDegrees(rotationDegreesAboutY);
	float sin = SinDegrees(rotationDegreesAboutY);
	Mat44 rotated;
	rotated.m_values[Ix] = cos;		rotated.m_values[Jx] = 0.0f;	rotated.m_values[Kx] = sin;		rotated.m_values[Tx] = 0.0f;
	rotated.m_values[Iy] = 0.0f;	rotated.m_values[Jy] = 1.0f;	rotated.m_values[Ky] = 0.0f;	rotated.m_values[Ty] = 0.0f;
	rotated.m_values[Iz] = -sin;	rotated.m_values[Jz] = 0.0f;	rotated.m_values[Kz] = cos;		rotated.m_values[Tz] = 0.0f;
	rotated.m_values[Iw] = 0.0f;	rotated.m_values[Jw] = 0.0f;	rotated.m_values[Kw] = 0.0f;	rotated.m_values[Tw] = 1.0f;
	return rotated;
} 

Vec2 const Mat44::TransformVectorQuantity2D(Vec2 const& vectorQuantityXY) const
{
	float results[2];
	// row 1
	results[0] = (m_values[Ix] * vectorQuantityXY.x) + (m_values[Jx] * vectorQuantityXY.y); // + (m_values[Kx] * appendThis.m_values[Iz]) + (m_values[Tx] * appendThis.m_values[Iw]); // column 1
	results[1] = (m_values[Iy] * vectorQuantityXY.x) + (m_values[Jy] * vectorQuantityXY.y); // + (m_values[Ky] * appendThis.m_values[Iz]) + (m_values[Ty] * appendThis.m_values[Iw]); // column 2

	return Vec2(results[0], results[1]);
}

Vec3 const Mat44::TransformVectorQuantity3D(Vec3 const& vectorQuantityXYZ) const
{
	float results[3];
	// row 1
	results[0] = (m_values[Ix] * vectorQuantityXYZ.x) + (m_values[Jx] * vectorQuantityXYZ.y) + (m_values[Kx] * vectorQuantityXYZ.z); // + (m_values[Tx] * appendThis.m_values[Iw]); // column 1
	results[1] = (m_values[Iy] * vectorQuantityXYZ.x) + (m_values[Jy] * vectorQuantityXYZ.y) + (m_values[Ky] * vectorQuantityXYZ.z); // + (m_values[Ty] * appendThis.m_values[Iw]); // column 2
	results[2] = (m_values[Iz] * vectorQuantityXYZ.x) + (m_values[Jz] * vectorQuantityXYZ.y) + (m_values[Kz] * vectorQuantityXYZ.z); // + (m_values[Tz] * appendThis.m_values[Iw]); // column 3

	return Vec3(results[0], results[1], results[2]);
}

Vec2 const Mat44::TransformPosition2D(Vec2 const& positionXY) const
{
	float results[2];
	// row 1
	results[0] = (m_values[Ix] * positionXY.x) + (m_values[Jx] * positionXY.y) + (m_values[Tx]); // column 1
	results[1] = (m_values[Iy] * positionXY.x) + (m_values[Jy] * positionXY.y) + (m_values[Ty]); // column 2

	return Vec2(results[0], results[1]);
}

Vec3 const Mat44::TransformPosition3D(Vec3 const& position3D) const
{
	float results[3];
	// row 1
	results[0] = (m_values[Ix] * position3D.x) + (m_values[Jx] * position3D.y) + (m_values[Kx] * position3D.z) + (m_values[Tx]); // column 1
	results[1] = (m_values[Iy] * position3D.x) + (m_values[Jy] * position3D.y) + (m_values[Ky] * position3D.z) + (m_values[Ty]); // column 2
	results[2] = (m_values[Iz] * position3D.x) + (m_values[Jz] * position3D.y) + (m_values[Kz] * position3D.z) + (m_values[Tz]); // column 3

	return Vec3(results[0], results[1], results[2]);
}

Vec4 const Mat44::TransformHomogeneous3D(Vec4 const& homogeneousPoint3D) const
{
	float results[4];
	// row 1
	results[0] = (m_values[Ix] * homogeneousPoint3D.x) + (m_values[Jx] * homogeneousPoint3D.y) + (m_values[Kx] * homogeneousPoint3D.z) + (m_values[Tx] * homogeneousPoint3D.w); // column 1
	results[1] = (m_values[Iy] * homogeneousPoint3D.x) + (m_values[Jy] * homogeneousPoint3D.y) + (m_values[Ky] * homogeneousPoint3D.z) + (m_values[Ty] * homogeneousPoint3D.w); // column 2
	results[2] = (m_values[Iz] * homogeneousPoint3D.x) + (m_values[Jz] * homogeneousPoint3D.y) + (m_values[Kz] * homogeneousPoint3D.z) + (m_values[Tz] * homogeneousPoint3D.w); // column 3
	results[3] = (m_values[Iw] * homogeneousPoint3D.x) + (m_values[Jw] * homogeneousPoint3D.y) + (m_values[Kw] * homogeneousPoint3D.z) + (m_values[Tw] * homogeneousPoint3D.w); // column 4

	return Vec4(results[0], results[1], results[2], results[3]);
}

float* Mat44::GetAsFloatArray()
{
	return m_values;
}

float const* Mat44::GetAsFloatArray() const
{
	return m_values;
}

Vec2 const Mat44::GetIBasis2D() const
{
	return Vec2(m_values[Ix], m_values[Iy]);
}

Vec2 const Mat44::GetJBasis2D() const
{
	return Vec2(m_values[Jx], m_values[Jy]);
}

Vec2 const Mat44::GetTranslation2D() const
{
	return Vec2(m_values[Tx], m_values[Ty]);
}

Vec3 const Mat44::GetIBasis3D() const
{
	return Vec3(m_values[Ix], m_values[Iy], m_values[Iz]);
}

Vec3 const Mat44::GetJBasis3D() const
{
	return Vec3(m_values[Jx], m_values[Jy], m_values[Jz]);
}

Vec3 const Mat44::GetKBasis3D() const
{
	return Vec3(m_values[Kx], m_values[Ky], m_values[Kz]);
}

Vec3 const Mat44::GetTranslation3D() const
{
	return Vec3(m_values[Tx], m_values[Ty], m_values[Tz]);
}

Vec4 const Mat44::GetIBasis4D() const
{
	return Vec4(m_values[Ix], m_values[Iy], m_values[Iz], m_values[Iw]);
}

Vec4 const Mat44::GetJBasis4D() const
{
	return Vec4(m_values[Jx], m_values[Jy], m_values[Jz], m_values[Jw]);
}

Vec4 const Mat44::GetKBasis4D() const
{
	return Vec4(m_values[Kx], m_values[Ky], m_values[Kz], m_values[Kw]);
}

Vec4 const Mat44::GetTranslation4D() const
{
	return Vec4(m_values[Tx], m_values[Ty], m_values[Tz], m_values[Tw]);
}

void Mat44::SetTranslation2D(Vec2 const& translationXY)
{
	m_values[Tx] = translationXY.x;
	m_values[Ty] = translationXY.y;
	m_values[Tz] = 0.0f;
	m_values[Tw] = 1.0f;
}

void Mat44::SetTranslation3D(Vec3 const& translationXYZ)
{
	m_values[Tx] = translationXYZ.x;
	m_values[Ty] = translationXYZ.y;
	m_values[Tz] = translationXYZ.z;
	m_values[Tw] = 1.0f;
}

// set basis
// Sets z=0, w=0 for i & j; does not modify k or t
void Mat44::SetIJ2D(Vec2 const& iBasis2D, Vec2 const& jBasis2D)
{
	m_values[Ix] = iBasis2D.x; m_values[Jx] = jBasis2D.x;
	m_values[Iy] = iBasis2D.y; m_values[Jy] = jBasis2D.y;
	m_values[Iz] = 0.0f;	   m_values[Jz] = 0.0f;
	m_values[Iw] = 0.0f;	   m_values[Jw] = 0.0f;
}

// Sets z=0, w=0 for i,j,t; does not modify k
void Mat44::SetIJT2D(Vec2 const& iBasis2D, Vec2 const& jBasis2D, Vec2 const& translationXY)
{
	m_values[Ix] = iBasis2D.x; m_values[Jx] = jBasis2D.x; m_values[Tx] = translationXY.x;
	m_values[Iy] = iBasis2D.y; m_values[Jy] = jBasis2D.y; m_values[Ty] = translationXY.y;
	m_values[Iz] = 0.0f;	   m_values[Jz] = 0.0f;		  m_values[Tz] = 0.0f;
	m_values[Iw] = 0.0f;	   m_values[Jw] = 0.0f;		  m_values[Tw] = 1.0f;
}

// Sets w=0 for i,j,k
void Mat44::SetIJK3D(Vec3 const& iBasis3D, Vec3 const& jBasis3D, Vec3 const& kBasis3D)
{
	m_values[Ix] = iBasis3D.x; m_values[Jx] = jBasis3D.x; m_values[Kx] = kBasis3D.x;
	m_values[Iy] = iBasis3D.y; m_values[Jy] = jBasis3D.y; m_values[Ky] = kBasis3D.y;
	m_values[Iz] = iBasis3D.z; m_values[Jz] = jBasis3D.z; m_values[Kz] = kBasis3D.z;
	m_values[Iw] = 0.0f;	   m_values[Jw] = 0.0f;		  m_values[Kw] = 0.0f;
}

// Sets w=0 for i,j,k,t
void Mat44::SetIJKT3D(Vec3 const& iBasis3D, Vec3 const& jBasis3D, Vec3 const& kBasis3D, Vec3 const& translationXYZ)
{
	m_values[Ix] = iBasis3D.x; m_values[Jx] = jBasis3D.x; m_values[Kx] = kBasis3D.x; m_values[Tx] = translationXYZ.x;
	m_values[Iy] = iBasis3D.y; m_values[Jy] = jBasis3D.y; m_values[Ky] = kBasis3D.y; m_values[Ty] = translationXYZ.y;
	m_values[Iz] = iBasis3D.z; m_values[Jz] = jBasis3D.z; m_values[Kz] = kBasis3D.z; m_values[Tz] = translationXYZ.z;
	m_values[Iw] = 0.0f;	   m_values[Jw] = 0.0f;		  m_values[Kw] =0.0f;		 m_values[Tw] = 1.0f;
}

void Mat44::SetIJKT4D(Vec4 const& iBasis4D, Vec4 const& jBasis4D, Vec4 const& kBasis4D, Vec4 const& translation4D)
{
	m_values[Ix] = iBasis4D.x; m_values[Jx] = jBasis4D.x; m_values[Kx] = kBasis4D.x; m_values[Tx] = translation4D.x;
	m_values[Iy] = iBasis4D.y; m_values[Jy] = jBasis4D.y; m_values[Ky] = kBasis4D.y; m_values[Ty] = translation4D.y;
	m_values[Iz] = iBasis4D.z; m_values[Jz] = jBasis4D.z; m_values[Kz] = kBasis4D.z; m_values[Tz] = translation4D.z;
	m_values[Iw] = iBasis4D.w; m_values[Jw] = jBasis4D.w; m_values[Kw] = kBasis4D.w; m_values[Tw] = translation4D.w;
}

// Multiplication operations
void Mat44::Append(Mat44 const& appendThis)
{
	float results[16];

	//	multiply on right in column notation / on left in row notation
	// row 1
	results[0] =  (m_values[Ix] * appendThis.m_values[Ix]) + (m_values[Jx] * appendThis.m_values[Iy]) + (m_values[Kx] * appendThis.m_values[Iz]) + (m_values[Tx] * appendThis.m_values[Iw]); // column 1
	results[1] =  (m_values[Iy] * appendThis.m_values[Ix]) + (m_values[Jy] * appendThis.m_values[Iy]) + (m_values[Ky] * appendThis.m_values[Iz]) + (m_values[Ty] * appendThis.m_values[Iw]); // column 2
	results[2] =  (m_values[Iz] * appendThis.m_values[Ix]) + (m_values[Jz] * appendThis.m_values[Iy]) + (m_values[Kz] * appendThis.m_values[Iz]) + (m_values[Tz] * appendThis.m_values[Iw]); // column 3
	results[3] =  (m_values[Iw] * appendThis.m_values[Ix]) + (m_values[Jw] * appendThis.m_values[Iy]) + (m_values[Kw] * appendThis.m_values[Iz]) + (m_values[Tw] * appendThis.m_values[Iw]); // column 4

	// row 2
	results[4] =  (m_values[Ix] * appendThis.m_values[Jx]) + (m_values[Jx] * appendThis.m_values[Jy]) + (m_values[Kx] * appendThis.m_values[Jz]) + (m_values[Tx] * appendThis.m_values[Jw]);
	results[5] =  (m_values[Iy] * appendThis.m_values[Jx]) + (m_values[Jy] * appendThis.m_values[Jy]) + (m_values[Ky] * appendThis.m_values[Jz]) + (m_values[Ty] * appendThis.m_values[Jw]);
	results[6] =  (m_values[Iz] * appendThis.m_values[Jx]) + (m_values[Jz] * appendThis.m_values[Jy]) + (m_values[Kz] * appendThis.m_values[Jz]) + (m_values[Tz] * appendThis.m_values[Jw]);
	results[7] =  (m_values[Iw] * appendThis.m_values[Jx]) + (m_values[Jw] * appendThis.m_values[Jy]) + (m_values[Kw] * appendThis.m_values[Jz]) + (m_values[Tw] * appendThis.m_values[Jw]);
	
	// row 3
	results[8] =  (m_values[Ix] * appendThis.m_values[Kx]) + (m_values[Jx] * appendThis.m_values[Ky]) + (m_values[Kx] * appendThis.m_values[Kz]) + (m_values[Tx] * appendThis.m_values[Kw]);
	results[9] =  (m_values[Iy] * appendThis.m_values[Kx]) + (m_values[Jy] * appendThis.m_values[Ky]) + (m_values[Ky] * appendThis.m_values[Kz]) + (m_values[Ty] * appendThis.m_values[Kw]);
	results[10] = (m_values[Iz] * appendThis.m_values[Kx]) + (m_values[Jz] * appendThis.m_values[Ky]) + (m_values[Kz] * appendThis.m_values[Kz]) + (m_values[Tz] * appendThis.m_values[Kw]);
	results[11] = (m_values[Iw] * appendThis.m_values[Kx]) + (m_values[Jw] * appendThis.m_values[Ky]) + (m_values[Kw] * appendThis.m_values[Kz]) + (m_values[Tw] * appendThis.m_values[Kw]);
	
	// row 4
	results[12] = (m_values[Ix] * appendThis.m_values[Tx]) + (m_values[Jx] * appendThis.m_values[Ty]) + (m_values[Kx] * appendThis.m_values[Tz]) + (m_values[Tx] * appendThis.m_values[Tw]);
	results[13] = (m_values[Iy] * appendThis.m_values[Tx]) + (m_values[Jy] * appendThis.m_values[Ty]) + (m_values[Ky] * appendThis.m_values[Tz]) + (m_values[Ty] * appendThis.m_values[Tw]);
	results[14] = (m_values[Iz] * appendThis.m_values[Tx]) + (m_values[Jz] * appendThis.m_values[Ty]) + (m_values[Kz] * appendThis.m_values[Tz]) + (m_values[Tz] * appendThis.m_values[Tw]);
	results[15] = (m_values[Iw] * appendThis.m_values[Tx]) + (m_values[Jw] * appendThis.m_values[Ty]) + (m_values[Kw] * appendThis.m_values[Tz]) + (m_values[Tw] * appendThis.m_values[Tw]);

	for (int index = 0; index < 16; index++)
	{
		m_values[index] = results[index];
	}
}

void Mat44::AppendZRotation(float degreesRotationAboutZ)
{
	Mat44 rotator = CreateZRotationDegrees(degreesRotationAboutZ);
	Append(rotator);
}

void Mat44::AppendYRotation(float degreesRotationAboutY)
{
	Mat44 rotator = CreateYRotationDegrees(degreesRotationAboutY);
	Append(rotator);
}

void Mat44::AppendXRotation(float degreesRotationAboutX)
{
	Mat44 rotator = CreateXRotationDegrees(degreesRotationAboutX);
	Append(rotator);
}

void Mat44::AppendTranslation2D(Vec2 const& translationXY)
{
	Mat44 temp = CreateTranslation2D(translationXY);
	Append(temp);
}

void Mat44::AppendTranslation3D(Vec3 const& translationXYZ)
{
	Mat44 temp = CreateTranslation3D(translationXYZ);
	Append(temp);
}

void Mat44::AppendScaleUniform2D(float uniformScaleXY)
{
	Mat44 temp = CreateUniformScale2D(uniformScaleXY);
	Append(temp);
}

void Mat44::AppendScaleUniform3D(float uniformScaleXYZ)
{
	Mat44 temp = CreateUniformScale3D(uniformScaleXYZ);
	Append(temp);
}

void Mat44::AppendScaleNonUniform2D(Vec2 const& nonUniformScaleXY)
{
	Mat44 temp = CreateNonUniformScale2D(nonUniformScaleXY);
	Append(temp);
}

void Mat44::AppendScaleNonUniform3D(Vec3 const& nonUniformScaleXYZ)
{
	Mat44 temp = CreateNonUniformScale3D(nonUniformScaleXYZ);
	Append(temp);
}

// orthonormal stuff
Mat44 const Mat44::CreateOrthoProjection(float left, float right, float bottom, float top, float zNear, float zFar)
{
	Mat44 ortho;

	float range = 1.0f / (right - left);
	ortho.m_values[Ix] = 2.0f * range;
	ortho.m_values[Tx] = -(left + right) * range;

	range = 1.0f / (top - bottom);
	ortho.m_values[Jy] = 2.0f * range;
	ortho.m_values[Ty] = -(bottom + top) * range;

	range = 1.0f / (zFar - zNear);
	ortho.m_values[Kz] = range;
	ortho.m_values[Tz] = -zNear * range;

	return ortho;
}

Mat44 const Mat44::CreatePerspectiveProjection(float fovYDegrees, float aspect, float zNear, float zFar)
{
	Mat44 proj;
	float thetaDegrees = fovYDegrees * 0.5f;
	float Sy = CosDegrees(thetaDegrees) / SinDegrees(thetaDegrees);
	float Sx = Sy / aspect;
	float Sz = zFar / (zFar - zNear);
	float tz = (zNear * zFar) / (zNear - zFar);

	proj.m_values[Jy] = Sy;
	proj.m_values[Ix] = Sx;
	proj.m_values[Kw] = 1.0f;
	proj.m_values[Kz] = Sz;
	proj.m_values[Tz] = tz;
	proj.m_values[Tw] = 0.0f;

	return proj;
}

void Mat44::Swap(int a, int b)
{
	float temp = m_values[a];
	m_values[a] = m_values[b];
	m_values[b] = temp;
}

void Mat44::Transpose()
{
	Swap(Iy, Jx);
	Swap(Iz, Kx);
	Swap(Iw, Tx);
	Swap(Jz, Ky);
	Swap(Jw, Ty);
	Swap(Kw, Tz);
}

Mat44 const Mat44::GetOrthonormalInverse() const
{
	Mat44 R;
	R.m_values[Ix] = m_values[Ix];		R.m_values[Jx] = m_values[Iy];		R.m_values[Kx] = m_values[Iz];
	R.m_values[Iy] = m_values[Jx];		R.m_values[Jy] = m_values[Jy];		R.m_values[Ky] = m_values[Jz];
	R.m_values[Iz] = m_values[Kx];		R.m_values[Jz] = m_values[Ky];		R.m_values[Kz] = m_values[Kz];

	Mat44 T;
	T.m_values[Tx] = -m_values[Tx];
	T.m_values[Ty] = -m_values[Ty];
	T.m_values[Tz] = -m_values[Tz];

	R.Append(T);
	return R;
}

void Mat44::Orthonormalize_XFwd_YLeft_ZUp()
{
	Vec3 I;
	I.x = m_values[Ix];
	I.y = m_values[Iy];
	I.z = m_values[Iz];
	I.Normalize();
	m_values[Ix] = I.x;
	m_values[Iy] = I.y;
	m_values[Iz] = I.z;

	Vec3 K;
	K.x = m_values[Kx];
	K.y = m_values[Ky];
	K.z = m_values[Kz];
	K -= GetProjectedOnto3D(K, I);
	K.Normalize();
	m_values[Kx] = K.x;
	m_values[Ky] = K.y;
	m_values[Kz] = K.z;

	Vec3 J;
	J.x = m_values[Jx];
	J.y = m_values[Jy];
	J.z = m_values[Jz];
	J -= GetProjectedOnto3D(J, K);
	J -= GetProjectedOnto3D(J, I);
	J.Normalize();
	m_values[Jx] = J.x;
	m_values[Jy] = J.y;
	m_values[Jz] = J.z;
}				  

