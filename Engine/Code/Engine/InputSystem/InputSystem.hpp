#pragma once
#include "Engine/InputSystem/XboxController.hpp"

extern unsigned char const KEYCODE_F1;
extern unsigned char const KEYCODE_F2;
extern unsigned char const KEYCODE_F3;
extern unsigned char const KEYCODE_F4;
extern unsigned char const KEYCODE_F5;
extern unsigned char const KEYCODE_F6;
extern unsigned char const KEYCODE_F7;
extern unsigned char const KEYCODE_F8;
extern unsigned char const KEYCODE_F9;
extern unsigned char const KEYCODE_F10;
extern unsigned char const KEYCODE_F11;
extern unsigned char const KEYCODE_F12;
extern unsigned char const KEYCODE_ESC;
extern unsigned char const KEYCODE_TILDE;
extern unsigned char const KEYCODE_UPARROW;
extern unsigned char const KEYCODE_DOWNARROW;
extern unsigned char const KEYCODE_LEFTARROW;
extern unsigned char const KEYCODE_RIGHTARROW;
extern unsigned char const KEYCODE_SPACE;
extern unsigned char const KEYCODE_ENTER;
extern unsigned char const KEYCODE_LEFT_MOUSE;
extern unsigned char const KEYCODE_RIGHT_MOUSE;
extern unsigned char const KEYCODE_LEFT_SHIFT;
extern unsigned char const KEYCODE_RIGHT_SHIFT;
extern unsigned char const KEYCODE_WHEEL_UP;
extern unsigned char const KEYCODE_WHEEL_DOWN;

constexpr int NUM_KEYCODES = 256;
constexpr int NUM_XBOX_CONTROLLERS = 4;

struct InputSystemConfig
{

};

class InputSystem
{
public:	
	InputSystem(InputSystemConfig config);
	void Startup();
	void BeginFrame();
	void EndFrame();
	void Shutdown();

	bool HandleCharInput( int charCode );
	void ConsumeKeyJustPressed( unsigned char keyCode );

	bool WasKeyJustPressed(unsigned char keyCode);
	bool WasKeyJustReleased(unsigned char keyCode);
	bool IsKeyDown(unsigned char keyCode);
	bool HandleKeyPressed(unsigned char keyCode);
	bool HandleKeyReleased(unsigned char keyCode);
	XboxController const& GetController(int controllerID);

	void SetMouseMode(bool hidden, bool clipped, bool relative);
	Vec2 GetMouseClientPosition();
	Vec2 GetMouseClientDelta();
	Vec2 GetClientRectangle();

	void PrintMouse(IntVec2 mousePos, const char* const unit);
	KeyButtonState m_keyStates[NUM_KEYCODES];
	XboxController m_controller[NUM_XBOX_CONTROLLERS];
private:
	InputSystemConfig m_config;
	bool m_mouseHidden = false;
	bool m_mouseClipped = false;
	bool m_mouseRelative = false;
public:
	IntVec2 m_clientCenter = IntVec2::ZERO;
	IntVec2 m_mouseCurr = IntVec2::ZERO;
	IntVec2 m_mousePrev = IntVec2::ZERO;
	IntVec2 m_delta = IntVec2::ZERO;
};
