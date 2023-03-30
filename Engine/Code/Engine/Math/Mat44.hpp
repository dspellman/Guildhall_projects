#pragma once

struct Vec2;
struct Vec3;
struct Vec4;

//	
// A 4x4 Homogeneous 2D/3D transformation matrix, stored basis-major in memory (Ix�Iy,Iz�Iw�Jx,Jy...). 
//
// Note: we specifically DO NOT provide an operator* overload, since doing so would require a
// decision to commit the Mat44 to only work correctly with EITHER column-major or row-major style 
// style notation. They demand two different ways of writing operator*, and in order to implement 
// an operator*, we are forced to make a notational committment. This is certainly ambiguous to the 
// reader and, at the very least, potentially confusing. Instead, we prefer to use method names,
// such as "Append", which are more neutral (e.g. multiply a new matrix "on the right in column-
// notation / on the left in row-notation".
//

struct Mat44
{
	enum { Ix, Iy, Iz, Iw, Jx, Jy, Jz, Jw, Kx, Ky, Kz, Kw, Tx, Ty, Tz, Tw }; // index nicknames, [0] through [15]
	float m_values[16]; // stored in "basis major" order (Ix1IylIz�IwIJx...) - translation in [12,13,14]
	
	Mat44(); // Default constructor is IDENTITY matrix! 
	explicit Mat44( Vec2 const& iBasis2D, Vec2 const& jBasis2D, Vec2 const& translation2D );
	explicit Mat44( Vec3 const& iBasis3D, Vec3 const& jBasis3D, Vec3 const& kBasis3D, Vec3 const& translation3D );
	explicit Mat44( Vec4 const& iBasis4D, Vec4 const& jBasis4D, Vec4 const& kBasis4D, Vec4 const& translation4D );
	explicit Mat44(float const* sixteenValuesBasisMajor);	

	static Mat44 const CreateTranslation2D(Vec2 const& translationXY);
	static Mat44 const CreateTranslation3D(Vec3 const& translationXYZ);
	static Mat44 const CreateUniformScale2D(float uniformScaleXY);
	static Mat44 const CreateUniformScale3D(float uniformScaleXYZ);
	static Mat44 const CreateNonUniformScale2D(Vec2 const& nonuniformScaleXY);
	static Mat44 const CreateNonUniformScale3D(Vec3 const& nonUniformScaleXYZ);
	static Mat44 const CreateZRotationDegrees(float rotationDegreesAboutZ);
	static Mat44 const CreateYRotationDegrees(float rotationDegreesAboutY);
	static Mat44 const CreateXRotationDegrees(float rotationDegreesAboutX);

	Vec2 const TransformVectorQuantity2D(Vec2 const& vectorQuantityXY) const; 
	Vec3 const TransformVectorQuantity3D(Vec3 const& vectorQuantityXYZ) const;
	Vec2 const TransformPosition2D(Vec2 const& positionXY) const; 
	Vec3 const TransformPosition3D(Vec3 const& position3D) const; 
	Vec4 const TransformHomogeneous3D(Vec4 const& homogeneousPoint3D) const;

	float*       GetAsFloatArray();	// non-const (mutable) version
	float const* GetAsFloatArray() const;	// const version, used only when Mat44
	Vec2 const	 GetIBasis2D() const; 
	Vec2 const	 GetJBasis2D() const; 
	Vec2 const	 GetTranslation2D() const;
	Vec3 const	 GetIBasis3D() const; 
	Vec3 const	 GetJBasis3D() const; 
	Vec3 const	 GetKBasis3D() const; 
	Vec3 const	 GetTranslation3D() const; 
	Vec4 const	 GetIBasis4D() const; 
	Vec4 const	 GetJBasis4D() const; 
	Vec4 const	 GetKBasis4D() const; 
	Vec4 const	 GetTranslation4D() const;

	void SetTranslation2D(Vec2 const& translationXY);	// Sets translationZ = 0, translationW = 1
	void SetTranslation3D(Vec3 const& translationXYZ);	// Sets translation W = 1
	void SetIJ2D(Vec2 const& iBasis2D, Vec2 const& jBasis2D); // Sets z=0, w=0 for i & j; does not modify k or t
	void SetIJT2D(Vec2 const& iBasis2D, Vec2 const& jBasis2D, Vec2 const& translationXY); // Sets z=0, w=0 for i,j,t; does not modify k 
	void SetIJK3D( Vec3 const& iBasis3D, Vec3 const& jBasis3D, Vec3 const& kBasis3D );	// Sets w=0 for i,j,k
	void SetIJKT3D(Vec3 const& iBasis3D, Vec3 const& jBasis3D, Vec3 const& kBasis3D, Vec3 const& translationXYZ); // Sets w=0 for i,j,k,t 
	void SetIJKT4D( Vec4 const& iBasis4D, Vec4 const& jBasis4D, Vec4 const& kBasis4D, Vec4 const& translation4D );
		
	void Append(Mat44 const& appendThis);							//	multiply on right in column notation / on left in row notation
	void AppendZRotation(float degreesRotationAboutZ);				// same as appending (*= in column notation) a z - rotation matrix
	void AppendYRotation(float degreesRotationAboutY);				//same as appending (*= in column notation) a y - rotation matrix
	void AppendXRotation(float degreesRotationAboutX);				// same as appending (*= in column notation) an x - rotation matrix
	void AppendTranslation2D(Vec2 const& translationXY);			// same as appending (*= in column notation) a translation matrix
	void AppendTranslation3D(Vec3 const& translationXYZ);			// same as appending (*= in column notation) a translation matrix
	void AppendScaleUniform2D(float uniformScaleXY);				// K and T bases should remain unaffected
	void AppendScaleUniform3D(float uniformScaleXYZ);				// translation should remain unaffected
	void AppendScaleNonUniform2D(Vec2 const& nonUniformScaleXY);	// K and T bases should remain unaffected
	void AppendScaleNonUniform3D(Vec3 const& nonUniformScaleXYZ);	// translation should remain unaffected      

	static Mat44 const    CreateOrthoProjection(float left, float right, float bottom, float top, float zNear, float zFar);
	static Mat44 const    CreatePerspectiveProjection(float fovYDegrees, float aspect, float zNear, float zFar);
	void Swap(int a, int b);
	void                  Transpose();                     // Swaps columns with rows
	Mat44 const           GetOrthonormalInverse() const;   // Only works for orthonormal affine matrices
	void                  Orthonormalize_XFwd_YLeft_ZUp(); // Forward is canonical, Up is secondary, Left tertiary
};
