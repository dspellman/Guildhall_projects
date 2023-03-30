#include "Engine/InputSystem/AnalogJoystick.hpp"
#define _USE_MATH_DEFINES
#include <math.h>
#include "../Math/MathUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

Vec2 AnalogJoystick::GetPosition() const
{
	return m_correctedPosition;
}

float AnalogJoystick::GetMagnitude() const
{
	return m_correctedPosition.GetLength();
}

float AnalogJoystick::GetOrientationDegrees() const
{
	return m_correctedPosition.GetOrientationDegrees();
}

Vec2 AnalogJoystick::GetRawUncorrectedPosition() const
{
	return m_rawPosition;
}

float AnalogJoystick::GetInnerDeadZoneFraction() const
{
	return m_innerDeadZoneFraction;
}

float AnalogJoystick::GetOuterDeadZoneFraction() const
{
	return m_outerDeadZoneFraction;
}

void AnalogJoystick::Reset()
{
	// center the joystick
	m_rawPosition.x = 0.0f;
	m_rawPosition.y = 0.0f;
	m_correctedPosition.x = 0.0f;
	m_correctedPosition.y = 0.0f;
}

void AnalogJoystick::SetDeadZoneThresholds(float normalizedInnerDeadZone, float normalizedOuterDeadZone)
{
	m_innerDeadZoneFraction = normalizedInnerDeadZone;
	m_outerDeadZoneFraction = normalizedOuterDeadZone;
}

void AnalogJoystick::UpdatePosition(float rawNormalizedX, float rawNormalizedY)
{
	m_rawPosition.x = rawNormalizedX;
	m_rawPosition.y = rawNormalizedY;

	float radius = GetRangeClamped(m_rawPosition.GetLength(), m_innerDeadZoneFraction, m_outerDeadZoneFraction, 0.0f, 1.0f);
	float theta = atan2f(m_rawPosition.y, m_rawPosition.x);

	m_correctedPosition.x = radius * cosf(theta);
	m_correctedPosition.y = radius * sinf(theta);
}

void AnalogJoystick::PrintDebugPosition()
{
	if (m_correctedPosition.GetLength() > 0.1)
	{
		DebuggerPrintf("Joystick is [%f, %f]\n", m_correctedPosition.x, m_correctedPosition.y);
	}
}
