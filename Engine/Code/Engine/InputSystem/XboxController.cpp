#define WIN32_LEAN_AND_MEAN
#include <Windows.h> // must #include Windows.h before #including Xinput.h
#include <Xinput.h> // include the Xinput API header file (interface)
#pragma comment( lib, "xinput9_1_0" ) // Link in the xinput.lib static library // #Eiserloh: Xinput 1_4 doesn't work in Windows 7; use version 9_1_0 explicitly for broadest compatibility

#include "Engine/InputSystem/XboxController.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/MathUtils.hpp"
#define _USE_MATH_DEFINES
#include <math.h>

constexpr float MAX_JOYSTICK = 32767.0f;
constexpr float OUTER_DEAD_ZONE_FRACTION  = 0.95f;
constexpr float TRIGGER_UPPER_DEAD_ZONE = 250.0f;

XboxController::XboxController()
{
	// configure a controller
	m_buttonMask[0] = XINPUT_GAMEPAD_DPAD_UP;
	m_buttonMask[1] = XINPUT_GAMEPAD_DPAD_DOWN;
	m_buttonMask[2] = XINPUT_GAMEPAD_DPAD_LEFT;
	m_buttonMask[3] = XINPUT_GAMEPAD_DPAD_RIGHT;
	m_buttonMask[4] = XINPUT_GAMEPAD_START;
	m_buttonMask[5] = XINPUT_GAMEPAD_BACK;
	m_buttonMask[6] = XINPUT_GAMEPAD_LEFT_THUMB;
	m_buttonMask[7] = XINPUT_GAMEPAD_RIGHT_THUMB;
	m_buttonMask[8] = XINPUT_GAMEPAD_LEFT_SHOULDER;
	m_buttonMask[9] = XINPUT_GAMEPAD_RIGHT_SHOULDER;
	m_buttonMask[10] = XINPUT_GAMEPAD_A;
	m_buttonMask[11] = XINPUT_GAMEPAD_B;
	m_buttonMask[12] = XINPUT_GAMEPAD_X;
	m_buttonMask[13] = XINPUT_GAMEPAD_Y;

	float innerDeadZoneFraction = static_cast<float>(XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) / MAX_JOYSTICK;
	m_leftStick.SetDeadZoneThresholds(innerDeadZoneFraction, OUTER_DEAD_ZONE_FRACTION);
	innerDeadZoneFraction = static_cast<float>(XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) / MAX_JOYSTICK;
	m_rightStick.SetDeadZoneThresholds(innerDeadZoneFraction, OUTER_DEAD_ZONE_FRACTION);

	Reset();
}

XboxController::~XboxController()
{

}

bool XboxController::IsConnected() const
{
	return m_isConnected;
}

int XboxController::GetControllerID() const
{
	return m_id;
}

void XboxController::Update()
{
	// check connection and reset if not connected
	XINPUT_STATE xboxControllerState = {};
	DWORD errorStatus = XInputGetState(m_id, &xboxControllerState);
	if (errorStatus != ERROR_SUCCESS)
	{
		m_isConnected = false;
		Reset();
		return; // leave since nothing to update
	}

	// update state of all controls
	m_isConnected = true;
// 	bool isButtonADown = (xboxControllerState.Gamepad.wButtons & XINPUT_GAMEPAD_A) == XINPUT_GAMEPAD_A;
// 	if (isButtonADown)
// 		DebuggerPrintf("Button A is % s\n", "DOWN");
	for (int button = 0; button < XboxButtonID::COUNT; button++)
	{
		UpdateButton(static_cast<XboxButtonID>(button), xboxControllerState.Gamepad.wButtons, m_buttonMask[button]);
	}

	m_leftTrigger = GetRangeClamped(xboxControllerState.Gamepad.bLeftTrigger, static_cast<float>(XINPUT_GAMEPAD_TRIGGER_THRESHOLD), TRIGGER_UPPER_DEAD_ZONE, 0.0f, 1.0f);
	m_RightTrigger = GetRangeClamped(xboxControllerState.Gamepad.bRightTrigger, static_cast<float>(XINPUT_GAMEPAD_TRIGGER_THRESHOLD), TRIGGER_UPPER_DEAD_ZONE, 0.0f, 1.0f);
	
	UpdateJoystick(m_leftStick, xboxControllerState.Gamepad.sThumbLX, xboxControllerState.Gamepad.sThumbLY);
	UpdateJoystick(m_rightStick, xboxControllerState.Gamepad.sThumbRX, xboxControllerState.Gamepad.sThumbRY);
}

void XboxController::Reset()
{
	// The quick and dirty solution
	for (int button = 0; button < XboxButtonID::COUNT; button++)
	{
		UpdateButton(static_cast<XboxButtonID>(button), (DWORD)0, m_buttonMask[button]);
		UpdateButton(static_cast<XboxButtonID>(button), (DWORD)0, m_buttonMask[button]);
	}
	m_leftTrigger = 0.0f;
	m_RightTrigger = 0.0f;
	m_leftStick.Reset();
	m_rightStick.Reset();
}

void XboxController::UpdateJoystick(AnalogJoystick& out_joystick, short rowX, short rowY)
{
	float normalizedX = RangeMap(static_cast<float>(rowX), -MAX_JOYSTICK, MAX_JOYSTICK, -1.0f, 1.0f); // close enough, ship it
	float normalizedY = RangeMap(static_cast<float>(rowY), -MAX_JOYSTICK, MAX_JOYSTICK, -1.0f, 1.0f);
	out_joystick.UpdatePosition(normalizedX, normalizedY);
}

void XboxController::UpdateButton(XboxButtonID buttonID, unsigned short buttonFlags, unsigned short bitMask)
{
	m_button[buttonID].wasKeyPressed = m_button[buttonID].isKeyPressed; // save old state for last frame
	m_button[buttonID].isKeyPressed = buttonFlags & bitMask; // save current state this frame
}

bool XboxController::IsButtonPressed(XboxButtonID buttonID) const
{
	return m_button[buttonID].isKeyPressed;
}

bool XboxController::WasButtonJustPressed(XboxButtonID buttonID) const
{
	return m_button[buttonID].isKeyPressed && !m_button[buttonID].wasKeyPressed;
}

bool XboxController::WasButtonJustReleased(XboxButtonID buttonID) const
{
	return !m_button[buttonID].isKeyPressed && m_button[buttonID].wasKeyPressed;
}

AnalogJoystick const& XboxController::GetLeftStick() const
{
	return m_leftStick;
}

AnalogJoystick const& XboxController::GetRightStick() const
{
	return m_rightStick;
}

float XboxController::GetLeftTrigger() const
{
	return m_leftTrigger;
}

float XboxController::GetRightTrigger() const
{
	return m_RightTrigger;
}

KeyButtonState const& XboxController::GetButton(XboxButtonID buttonID) const
{
	return m_button[buttonID];
}
