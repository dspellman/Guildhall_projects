#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/Vec3.hpp"
#include "../Math/AABB2.hpp"

enum CameraMode
{
	ORTHOGRAPHIC,
	PERSPECTIVE
};

class Camera
{
private:
	Vec2 m_bottomLeft;
	Vec2 m_topRight;
	float m_aspect = 1.0f;
	float m_fovDegrees = 90.0f;
	float m_near = 0.0f;
	float m_far = 1.0f;
	CameraMode m_mode = CameraMode::PERSPECTIVE;
	bool m_orbital = false;
	Vec3 m_position = Vec3::ZERO;
	Vec3 m_focus = Vec3::ZERO;
	EulerAngles m_orientation = EulerAngles::ZERO;
	Mat44 m_RenderTransform; // the render basis to change coordinate axis from game basis to DirectX basis
	AABB2 m_viewport = AABB2::ZERO_TO_ONE;
	Vec3		m_renderI = Vec3(1.0f, 0.0f, 0.0f);
	Vec3		m_renderJ = Vec3(0.0f, 1.0f, 0.0f);
	Vec3		m_renderK = Vec3(0.0f, 0.0f, 1.0f);

public:
	Camera();
	~Camera();

	void SetOrthoView(Vec2 const& bottomLeft, Vec2 const& topRight, float near = 0.0f, float far = 1.0f);

	//void SetOrthoView(Vec2 const& bottomLeft, Vec2 const& topRight);
	void SetPerspectiveView( float aspect, float fov, float near, float far );
	void SetOrbitalCamera(bool isOrbitalCamera);
	Vec2 GetOrthoBottomLeft() const;
	Vec2 GetOrthoTopRight() const;

	AABB2 GetViewport() const;
	void SetViewport(const AABB2& viewport);

	void Translate2d(Vec2 shakeOffset);
	Mat44 GetOrthoOrProjectionMatrix() const;
	void SetPostion(Vec3 position);
	Vec3 GetPosition() const;
	void SetFocus(Vec3 position);
	Vec3 GetFocus() const;
	void SetOrientation(EulerAngles orientation);
	EulerAngles GetOrientation() const;
	Mat44 GetViewMatrix() const;
	void SetRenderTransform( Vec3 const& iBasis, Vec3 const& jBasis, Vec3 const& kBasis );
	Mat44 GetRenderMatrix() const;

protected:
	Mat44 GetOrthoMatrix() const;
	Mat44 GetPerspectiveMatrix() const;
};