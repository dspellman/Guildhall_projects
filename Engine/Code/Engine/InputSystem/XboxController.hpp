#pragma once
#include "Engine/InputSystem/AnalogJoystick.hpp" 
#include "Engine/InputSystem/KeyButtonState.hpp" 

constexpr int NUM_JOYSTICKS = 2;

enum XboxButtonID
{
	DPAD_UP,
	DPAD_DOWN,
	DPAD_LEFT,
	DPAD_RIGHT,
	START,
	BACK,
	LEFT_THUMB,
	RIGHT_THUMB,
	LEFT_SHOULDER,
	RIGHT_SHOULDER,
	A,
	B,
	X,
	Y,
	COUNT
};

class XboxController
{
	friend class InputSystem;

private:
	bool m_isConnected = false;
	int m_id = 0;
	float m_leftTrigger = 0.0f; // normalized to [0,1]
	float m_RightTrigger = 0.0f;
	KeyButtonState m_button[static_cast<int>(XboxButtonID::COUNT)];
	unsigned short m_buttonMask[static_cast<int>(XboxButtonID::COUNT)];
	AnalogJoystick m_leftStick;
	AnalogJoystick m_rightStick;

public:
	XboxController();
	~XboxController();
	bool IsConnected() const;
	int GetControllerID() const;

	bool IsButtonPressed(XboxButtonID buttonID) const;
	bool WasButtonJustPressed(XboxButtonID buttonID) const;
	bool WasButtonJustReleased(XboxButtonID buttonID) const;

	AnalogJoystick const& GetLeftStick() const;
	AnalogJoystick const& GetRightStick() const;
	float GetLeftTrigger() const;
	float GetRightTrigger() const;
	KeyButtonState const& GetButton(XboxButtonID buttonID) const;

private:
	void Update();
	void Reset();
	void UpdateJoystick(AnalogJoystick& out_joystick, short rowX, short rowY);
	void UpdateButton(XboxButtonID buttonID, unsigned short buttonFlags, unsigned short bitMask);
};