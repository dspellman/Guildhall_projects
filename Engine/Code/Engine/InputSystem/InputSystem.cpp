#include <stdio.h>
#include <stdlib.h>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h> // must #include Windows.h before #including Xinput.h
#include <Xinput.h> // include the Xinput API header file (interface)
#pragma comment( lib, "xinput9_1_0" ) // Link in the xinput.lib static library // #Eiserloh: Xinput 1_4 doesn't work in Windows 7; use version 9_1_0 explicitly for broadest compatibility

#include "Engine/InputSystem/XboxController.hpp"
#include "Engine/InputSystem/AnalogJoystick.hpp"
#include "Engine/InputSystem/KeyButtonState.hpp"
#include "Engine/InputSystem/InputSystem.hpp"
#include "../Core/EventSystem.hpp"
#include "../Core/EngineCommon.hpp"
#include "Game/GameCommon.hpp"
#include "../Renderer/Window.hpp"

unsigned char const KEYCODE_F1 = VK_F1;
unsigned char const KEYCODE_F2 = VK_F2;
unsigned char const KEYCODE_F3 = VK_F3;
unsigned char const KEYCODE_F4 = VK_F4;
unsigned char const KEYCODE_F5 = VK_F5;
unsigned char const KEYCODE_F6 = VK_F6;
unsigned char const KEYCODE_F7 = VK_F7;
unsigned char const KEYCODE_F8 = VK_F8;
unsigned char const KEYCODE_F9 = VK_F9;
unsigned char const KEYCODE_F10 = VK_F10;
unsigned char const KEYCODE_F11 = VK_F11;
unsigned char const KEYCODE_F12 = VK_F12;
unsigned char const KEYCODE_ESC = VK_ESCAPE;
unsigned char const KEYCODE_TILDE = 0xC0;
unsigned char const KEYCODE_UPARROW = VK_UP;
unsigned char const KEYCODE_DOWNARROW = VK_DOWN;
unsigned char const KEYCODE_LEFTARROW = VK_LEFT;
unsigned char const KEYCODE_RIGHTARROW = VK_RIGHT;
unsigned char const KEYCODE_SPACE = VK_SPACE;
unsigned char const KEYCODE_ENTER = VK_RETURN;
unsigned char const KEYCODE_HOME = VK_HOME;
unsigned char const KEYCODE_END = VK_END;
unsigned char const KEYCODE_DELETE = VK_DELETE;
unsigned char const KEYCODE_BACK = VK_BACK;
unsigned char const KEYCODE_LEFT_MOUSE = VK_LBUTTON;
unsigned char const KEYCODE_RIGHT_MOUSE = VK_RBUTTON;
unsigned char const KEYCODE_LEFT_SHIFT = VK_SHIFT;
unsigned char const KEYCODE_RIGHT_SHIFT = VK_SHIFT;
// the following codes using unassigned virtual key values
unsigned char const KEYCODE_WHEEL_UP = 0x0E;
unsigned char const KEYCODE_WHEEL_DOWN = 0x0F;

InputSystem::InputSystem(InputSystemConfig config)
	: m_config( config )
{

}

void InputSystem::Startup()
{
	// reset everything to neutral state
	for (int key = 0; key < NUM_KEYCODES; key++)
	{
		m_keyStates[key].isKeyPressed = false;
		m_keyStates[key].wasKeyPressed = false;
	}

	for (int controller = 0; controller < NUM_XBOX_CONTROLLERS; controller++)
	{
		m_controller[controller].Reset();
	}
}

void InputSystem::BeginFrame()
{
	// update controllers at start of frame to have fresh values during frame
	for (int controller = 0; controller < NUM_XBOX_CONTROLLERS; controller++)
		m_controller[controller].Update();

	// do some mouse processing
	if (g_systemClock.IsPaused() || g_theConsole->IsOpen() || !m_mouseRelative || !m_mouseClipped || !Window::GetWindowContext()->HasFocus())
	{
		m_delta = IntVec2::ZERO;
	}
	else
	{
		POINT mousePos;
		GetCursorPos(&mousePos);
		m_mouseCurr = IntVec2(mousePos.x, mousePos.y);

		m_delta = IntVec2(m_mouseCurr.x - m_mousePrev.x, m_mouseCurr.y - m_mousePrev.y);

		SetCursorPos(m_clientCenter.x, m_clientCenter.y); // absolute screen coordinates
		GetCursorPos(&mousePos);
		m_mousePrev = IntVec2(mousePos.x, mousePos.y);
	}
}

void InputSystem::EndFrame()
{
	// transfer key states after windows updates during frame
	for (int key = 0; key < NUM_KEYCODES; key++)
	{
		m_keyStates[key].wasKeyPressed = m_keyStates[key].isKeyPressed;  
	}
	m_keyStates[KEYCODE_WHEEL_UP].isKeyPressed = false;
	m_keyStates[KEYCODE_WHEEL_DOWN].isKeyPressed = false;
}

void InputSystem::Shutdown()
{

}

bool InputSystem::HandleCharInput(int charCode)
{
	if (!g_theConsole || g_theConsole->IsOpen() == false)
		return false;
	EventArgs args;
	std::string value = std::to_string(charCode);
	args.SetValue("key", value);
	FireEvent("WM_CHAR", args);
	if (charCode != KEYCODE_TILDE)
	{
		ConsumeKeyJustPressed(static_cast<unsigned char>(charCode));
	}
	return true;
}

void InputSystem::ConsumeKeyJustPressed(unsigned char keyCode)
{
	m_keyStates[keyCode].wasKeyPressed = false;
	m_keyStates[keyCode].isKeyPressed = false;
}

bool InputSystem::WasKeyJustPressed(unsigned char keyCode)
{
	return m_keyStates[keyCode].isKeyPressed && !m_keyStates[keyCode].wasKeyPressed;
}

bool InputSystem::WasKeyJustReleased(unsigned char keyCode)
{
	return !m_keyStates[keyCode].isKeyPressed && m_keyStates[keyCode].wasKeyPressed;
}

bool InputSystem::IsKeyDown(unsigned char keyCode)
{
	return m_keyStates[keyCode].isKeyPressed;
}

bool InputSystem::HandleKeyPressed(unsigned char keyCode)
{
	m_keyStates[keyCode].isKeyPressed = true;
	if (!g_theConsole || g_theConsole->IsOpen() == false)
		return true;
	EventArgs args;
	std::string value = std::to_string(keyCode);
	args.SetValue("key", value);
	FireEvent("WM_KEYDOWN", args);
	if (keyCode != KEYCODE_TILDE)
	{
		ConsumeKeyJustPressed(keyCode);
	}
	return true; // right now this always succeeds
}

bool InputSystem::HandleKeyReleased(unsigned char keyCode)
{
	m_keyStates[keyCode].isKeyPressed = false;
	if (!g_theConsole || g_theConsole->IsOpen() == false)
		return true;
	EventArgs args;
	std::string value = std::to_string(keyCode);
	args.SetValue("key", value);
	FireEvent("WM_KEYUP", args);
	if (keyCode != KEYCODE_TILDE)
	{
		ConsumeKeyJustPressed(keyCode);
	}
	return true; // right now this always succeeds
}

XboxController const& InputSystem::GetController(int controllerID)
{
	return m_controller[controllerID];
}

void InputSystem::SetMouseMode(bool hidden, bool clipped, bool relative)
{
	// handle change in visibility
	if (hidden and !m_mouseHidden)
	{
		int result;
		do 
		{
			result = ShowCursor( FALSE );
		} 
		while (result >= 0);
	}

	if (!hidden and m_mouseHidden)
	{
		int result;
		do
		{
			result = ShowCursor(TRUE);
		}
		while (result < 0);
	}

	// handle clipping
	HWND hWnd = GetForegroundWindow();
	RECT client;
	GetClientRect(hWnd, &client);

	POINT corner;
	corner.x = 0;
	corner.y = 0;
	ClientToScreen(hWnd, &corner);

	if (clipped || relative)
	{
		// set clipping region using absolute screen coordinates
		RECT clip;
		clip.left = client.left + corner.x;
		clip.right = client.right + corner.x;
		clip.top = client.top + corner.y;
		clip.bottom = client.bottom + corner.y;
		ClipCursor(&clip);
	}
	else
	{
		ClipCursor(nullptr);
	}

	// handle relative
	if (relative)
	{
		// set mouse position based on client coordinates, but GetCursorPos gives screen coords and needs conversion
		m_clientCenter = IntVec2(static_cast<int>((client.left + client.right) / 2 + corner.x), static_cast<int>((client.top + client.bottom) / 2 + corner.y));
		m_mouseCurr = m_clientCenter;
		m_mousePrev = m_mouseCurr;
		SetCursorPos(m_mouseCurr.x, m_mouseCurr.y); // absolute screen coordinates
	}

	// save parameters
	m_mouseHidden = hidden;
	m_mouseClipped = clipped;
	m_mouseRelative = relative;
}

Vec2 InputSystem::GetMouseClientPosition()
{
	return Vec2(m_mouseCurr);
}

Vec2 InputSystem::GetMouseClientDelta()
{
//	PrintMouse(m_delta, " delta");
	return Vec2(m_delta);
}

Vec2 InputSystem::GetClientRectangle()
{
	HWND hWnd = GetForegroundWindow();
	RECT client;
	GetClientRect(hWnd, &client);
	return Vec2(static_cast<float>(client.right - client.left), static_cast<float>(client.bottom - client.top));
}

void InputSystem::PrintMouse(IntVec2 mousePos, const char* const unit)
{
if (mousePos.x == 0)
	return;
	std::string str;
	char buffer[80];
	sprintf_s(buffer, "[%i, %i]", mousePos.x, mousePos.y);
	std::string sbuf = buffer;
	sbuf += unit;
	EventArgs args;
	args.SetValue("Mouse", sbuf);
	FireEvent("test", args);
}