#include "Engine\Renderer\Camera.hpp"

Camera::Camera()
{
};

Camera::~Camera()
{
};

void Camera::SetOrthoView(Vec2 const& bottomLeft, Vec2 const& topRight, float near /*= 0.0f*/, float far /*= 1.0f*/)
//void Camera::SetOrthoView(Vec2 const& bottomLeft, Vec2 const& topRight)
{
	m_bottomLeft = bottomLeft;
	m_topRight = topRight;
	m_near = near;
	m_far = far;
	m_mode = CameraMode::ORTHOGRAPHIC;
};

void Camera::SetPerspectiveView(float aspect, float fov, float near, float far)
{
	// zNear cannot be 0.0f
	m_aspect = aspect;
	m_fovDegrees = fov;
	m_near = (near == 0.0f ? 0.1f : near);
	m_far = far;
	m_mode = CameraMode::PERSPECTIVE;
}

void Camera::SetOrbitalCamera(bool isOrbitalCamera)
{
	m_orbital = isOrbitalCamera;
}

Vec2 Camera::GetOrthoBottomLeft() const
{
	return m_bottomLeft;
};

Vec2 Camera::GetOrthoTopRight() const
{
	return m_topRight;
};

AABB2 Camera::GetViewport() const
{
	return m_viewport;
}

void Camera::SetViewport(const AABB2& viewport)
{
	m_viewport = viewport;
}

void Camera::Translate2d(Vec2 shakeOffset)
{
	m_bottomLeft += shakeOffset;
	m_topRight += shakeOffset;
}

Mat44 Camera::GetOrthoMatrix() const
{
	return Mat44::CreateOrthoProjection(m_bottomLeft.x, m_topRight.x, m_bottomLeft.y, m_topRight.y, 0.0f, 1.0f);
}

Mat44 Camera::GetPerspectiveMatrix() const
{
	return Mat44::CreatePerspectiveProjection(m_fovDegrees, m_aspect, m_near, m_far);
}

Mat44 Camera::GetOrthoOrProjectionMatrix() const
{
	Mat44 projection;
	if (m_mode == CameraMode::ORTHOGRAPHIC)
	{
		projection= GetOrthoMatrix();
	}
	else // PERSPECTIVE
	{
		projection = GetPerspectiveMatrix(); // the default at least works
	}
	projection.Append(GetRenderMatrix());
	return projection;
}

void Camera::SetPostion(Vec3 position)
{
	m_position = position;
}

Vec3 Camera::GetPosition() const
{
	return m_position;
}

void Camera::SetFocus(Vec3 focus)
{
	m_focus = focus;
}

Vec3 Camera::GetFocus() const
{
	return m_focus;
}

void Camera::SetOrientation(EulerAngles orientation)
{
	m_orientation = orientation;
}

EulerAngles Camera::GetOrientation() const
{
	return m_orientation;
}

Mat44 Camera::GetViewMatrix() const
{
	Mat44 view = m_orientation.GetAsMatrix_XFwd_YLeft_ZUp();
	view.Orthonormalize_XFwd_YLeft_ZUp();
	Mat44 trans;
	trans.SetTranslation3D(m_position);
	// camera that orbits the origin
	if (m_orbital)
	{
		Mat44 focus;
		focus.SetTranslation3D(m_focus);
		focus.Append(view);
		focus.Append(trans);
		return focus.GetOrthonormalInverse();
// 		view.Append(trans);
// 		return view.GetOrthonormalInverse();
	}
	// Normal camera position and orientation
	trans.Append(view);
	return trans.GetOrthonormalInverse();
}

void Camera::SetRenderTransform(Vec3 const& iBasis, Vec3 const& jBasis, Vec3 const& kBasis)
{
	m_RenderTransform = Mat44(iBasis, jBasis, kBasis, Vec3(0.0f, 0.0f, 0.0f));
}

Mat44 Camera::GetRenderMatrix() const
{
	return m_RenderTransform;
}
