#include "Engine/Renderer/Window.hpp"
#include "Engine/InputSystem/InputSystem.hpp"
#define WIN32_LEAN_AND_MEAN		// Always #define this before #including <windows.h>
#include <windows.h>
//#include "Game/GameCommon.hpp"

Window* Window::s_mainWindow = nullptr;
bool focus = false;

//-----------------------------------------------------------------------------------------------
// Handles Windows (Win32) messages/events; i.e. the OS is trying to tell us something happened.
// This function is called by Windows whenever we ask it for notifications

LRESULT CALLBACK WindowsMessageHandlingProcedure(HWND windowHandle, UINT wmMessageCode, WPARAM wParam, LPARAM lParam)
{
	Window* windowContext = Window::GetWindowContext();
	InputSystem* input = windowContext->GetConfig().m_inputSystem;

	switch (wmMessageCode)
	{
		case WM_ACTIVATE:
			focus = (wParam > 0);
			break;

		case WM_CLOSE:
		{
			ERROR_RECOVERABLE("WM_CLOSE not handled");
			return 0;
		}

		case WM_CHAR:
		{
			int characterCode = static_cast<int>(wParam);
			if (input->HandleCharInput(characterCode))
			{
				return 0;
			}
			break;
		}

		// Raw physical keyboard "key-was-just-depressed" event (case-insensitive, not translated)
		case WM_KEYDOWN:
		{
			unsigned char asKey = (unsigned char)wParam;
			if (input)
			{
				bool wasConsumed;
				wasConsumed = input->HandleKeyPressed(asKey);
				if (wasConsumed)
				{
					return 0;
				}
			}
			break;
		}

		// Raw physical keyboard "key-was-just-released" event (case-insensitive, not translated)
		case WM_KEYUP:
		{
			unsigned char asKey = (unsigned char)wParam;
			if (input)
			{
				return input->HandleKeyReleased(asKey);
			}
			break;
		}

		// Treat mouse clicks as special case keys
		case WM_LBUTTONDOWN:
		{
			unsigned char keyCode = KEYCODE_LEFT_MOUSE;
			bool wasConsumed = false;
			if (input)
			{
				wasConsumed = input->HandleKeyPressed(keyCode);
				if (wasConsumed)
				{
					return 0; // "Consumes" this message (tells Windows "okay, we handled it")
					break;
				}
			}
			break;
		}

		// Treat mouse clicks as special case keys
		case WM_LBUTTONUP:
		{
			unsigned char keyCode = KEYCODE_LEFT_MOUSE;
			bool wasConsumed = false;
			if (input)
			{
				wasConsumed = input->HandleKeyReleased(keyCode);
				if (wasConsumed)
				{
					return 0; // "Consumes" this message (tells Windows "okay, we handled it")
					break;
				}
			}
			break;
		}

		case WM_RBUTTONDOWN:
		{
			unsigned char keyCode = KEYCODE_RIGHT_MOUSE;
			bool wasConsumed = false;
			if (input)
			{
				wasConsumed = input->HandleKeyPressed(keyCode);
				if (wasConsumed)
				{
					return 0; // "Consumes" this message (tells Windows "okay, we handled it")
					break;
				}
			}
			break;
		}
		case WM_RBUTTONUP:
		{
			unsigned char keyCode = KEYCODE_RIGHT_MOUSE;
			bool wasConsumed = false;
			if (input)
			{
				wasConsumed = input->HandleKeyReleased(keyCode);
				if (wasConsumed)
				{
					return 0; // "Consumes" this message (tells Windows "okay, we handled it")
					break;
				}
			}
			break;
		}
		case WM_MOUSEWHEEL:
		{
			unsigned char keyCode;
			bool wasConsumed = false;
			short zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
			if (input)
			{
				if (zDelta > 0)
				{
					keyCode = KEYCODE_WHEEL_UP;
					wasConsumed = input->HandleKeyPressed(keyCode);
				}
				if (zDelta < 0)
				{
					keyCode = KEYCODE_WHEEL_DOWN;
					wasConsumed = input->HandleKeyPressed(keyCode);
				}
				if (wasConsumed)
				{
					return 0; // "Consumes" this message (tells Windows "okay, we handled it")
					break;
				}
			}
			break;
		}
	}
	// Send back to Windows any unhandled/unconsumed messages we want other apps to see (e.g. play/pause in music apps, etc.)
	return DefWindowProc(windowHandle, wmMessageCode, wParam, lParam);
}

Window::Window(WindowConfig const& config)
	: m_config( config )
{
	s_mainWindow = this;
}

Window::~Window()
{

}

void Window::Startup()
{
	CreateOSWindow();
}

void Window::BeginFrame()
{
	RunMessagePump();
}

void Window::EndFrame()
{

}

void Window::Shutdown()
{

}

WindowConfig const& Window::GetConfig() const
{
	return m_config;
}

Window* Window::GetWindowContext()
{
	return s_mainWindow;
}

// Returns the mouse cursor position in normalized (U,V) coordinates relative to the interior 
// (client area) of our window. Reports numbers outside [0,1) if the mouse cursor is outside.
Vec2 Window::GetNormalizedCursorPos() const
{
	HWND windowHandle = HWND(GetOSWindowHandle());
	POINT cursorCoords;
	RECT clientRect;
	::GetCursorPos(&cursorCoords); // in screen coordinates, (0,0) top-left
	::ScreenToClient(windowHandle, &cursorCoords); // relative to the window interior 
	::GetClientRect( windowHandle, &clientRect ); // size of window interior (0,0 to width,height)
	float cursorX = float(cursorCoords.x) / float(clientRect.right); // normalized x position 
	float cursorY = float(cursorCoords.y) / float(clientRect.bottom); // normalized y position 
	return Vec2( cursorX, 1.f - cursorY ); // We want (0,0) in the bottom-left
}
	
void Window::CreateOSWindow()
{
	HMODULE applicationInstanceHandle = ::GetModuleHandle( NULL );
	// Define a window style/class
	WNDCLASSEX windowClassDescription;
	memset(&windowClassDescription, 0, sizeof(windowClassDescription));
	windowClassDescription.cbSize = sizeof(windowClassDescription);
	windowClassDescription.style = CS_OWNDC; // Redraw on move, request own Display Context
	windowClassDescription.lpfnWndProc = static_cast<WNDPROC>(WindowsMessageHandlingProcedure); // Register our Windows message-handling function
	windowClassDescription.hInstance = GetModuleHandle(NULL);
	windowClassDescription.hIcon = NULL;
	windowClassDescription.hCursor = NULL;
	windowClassDescription.lpszClassName = TEXT("Simple Window Class");
	RegisterClassEx(&windowClassDescription);

	// #SD1ToDo: Add support for full screen mode (requires different window style flags than windowed mode)
	const DWORD windowStyleFlags = WS_CAPTION | WS_BORDER | WS_SYSMENU | WS_OVERLAPPED;
	const DWORD windowStyleExFlags = WS_EX_APPWINDOW;

	// Get desktop rect, dimensions, aspect
	RECT desktopRect;
	m_desktopWindowHandle = GetDesktopWindow();
	GetClientRect((HWND)m_desktopWindowHandle, &desktopRect);
	float desktopWidth = (float)(desktopRect.right - desktopRect.left);
	float desktopHeight = (float)(desktopRect.bottom - desktopRect.top);
	float desktopAspect = desktopWidth / desktopHeight;

	// Calculate maximum client size (as some % of desktop size)
	constexpr float maxClientFractionOfDesktop = 0.90f;
	float clientWidth = desktopWidth * maxClientFractionOfDesktop;
	float clientHeight = desktopHeight * maxClientFractionOfDesktop;
	if (m_config.m_clientAspect > desktopAspect)
	{
		// Client window has a wider aspect than desktop; shrink client height to match its width
		clientHeight = clientWidth / m_config.m_clientAspect;
	}
	else
	{
		// Client window has a taller aspect than desktop; shrink client width to match its height
		clientWidth = clientHeight * m_config.m_clientAspect;
	}

	// Try to create an icon
//	HICON hIcon = LoadIcon(applicationInstanceHandle, MAKEINTRESOURCE(IDI_ICON1));

	// Calculate client rect bounds by centering the client area
	float clientMarginX = 0.5f * (desktopWidth - clientWidth);
	float clientMarginY = 0.5f * (desktopHeight - clientHeight);
	RECT clientRect;
	clientRect.left = (int)clientMarginX;
	clientRect.right = clientRect.left + (int)clientWidth;
	clientRect.top = (int)clientMarginY;
	clientRect.bottom = clientRect.top + (int)clientHeight;
	m_clientWidth = (int)clientWidth; // save client drawing area dimensions
	m_clientHeight = (int)clientHeight;

	// Calculate the outer dimensions of the physical window, including frame
	RECT windowRect = clientRect;
	AdjustWindowRectEx(&windowRect, windowStyleFlags, FALSE, windowStyleExFlags);

	WCHAR windowTitle[1024];
	MultiByteToWideChar(GetACP(), 0, m_config.m_windowTitle.c_str(), -1, windowTitle, sizeof(windowTitle) / sizeof(windowTitle[0]));
	HWND hWnd = CreateWindowEx(
		windowStyleExFlags,
		windowClassDescription.lpszClassName,
		windowTitle,
		windowStyleFlags,
		windowRect.left,
		windowRect.top,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		NULL,
		NULL,
		(HINSTANCE)applicationInstanceHandle,
		NULL);

	m_osWindowHandle = hWnd;

	::ShowWindow(hWnd, SW_SHOW);
	::SetForegroundWindow(hWnd);
	::SetFocus(hWnd);

	HCURSOR cursor = ::LoadCursor(NULL, IDC_ARROW);
	::SetCursor(cursor);
}

//-----------------------------------------------------------------------------------------------
// Processes all Windows messages (WM_xxx) for this app that have queued up since last frame.
// For each message in the queue, our WindowsMessageHandlingProcedure (or "WinProc") function
//	is called, telling us what happened (key up/down, minimized/restored, gained/lost focus, etc.)
//
// #SD1ToDo: We will move this function to a more appropriate place later on...
//
void Window::RunMessagePump()
{
	MSG queuedMessage;
	for (;; )
	{
		const BOOL wasMessagePresent = PeekMessage(&queuedMessage, NULL, 0, 0, PM_REMOVE);
		if (!wasMessagePresent)
		{
			break;
		}

		TranslateMessage(&queuedMessage);
		DispatchMessage(&queuedMessage); // This tells Windows to call our "WindowsMessageHandlingProcedure" (a.k.a. "WinProc") function
	}
}

void* Window::GetHwnd() const
{
	return m_osWindowHandle;
//	return m_desktopWindowHandle;
}

IntVec2 Window::GetClientDimensions() const
{
	return IntVec2(m_clientWidth, m_clientHeight);
}

bool Window::HasFocus() const
{
//	return focus;
	return ((HWND)(GetHwnd()) == ::GetActiveWindow());
}

std::string Window::CopyClipboardToString()
{
	std::string pasteText = {};
	HGLOBAL   hglb;
	LPTSTR    lptstr;
	HWND hwnd = (HWND)(GetHwnd());

	if (!IsClipboardFormatAvailable(CF_TEXT))
	{
		return pasteText;
	}

	if (!OpenClipboard(hwnd))
	{
		return pasteText;
	}

	hglb = GetClipboardData(CF_TEXT);
	if (hglb != NULL)
	{
		lptstr = (LPTSTR)GlobalLock(hglb);
		if (lptstr != NULL)
		{
			pasteText = std::string((char const*)lptstr);
			GlobalUnlock(hglb);
		}
	}
	CloseClipboard();
	return pasteText;
}

bool Window::CopyStringToClipboard(std::string& text)
{
	LPTSTR  lptstrCopy;
	HGLOBAL hglbCopy;
	size_t length = text.size();

	if (!OpenClipboard((HWND)(GetHwnd())))
	{
		return false;
	}

	EmptyClipboard();	
	hglbCopy = GlobalAlloc(GMEM_MOVEABLE, (length + 1) * sizeof(TCHAR));
	if (hglbCopy == NULL)
	{
		CloseClipboard();
		return false;
	}

	// Lock the handle and copy the text to the buffer. 
	lptstrCopy = (LPTSTR)GlobalLock(hglbCopy);
	memcpy(lptstrCopy, text.data(), length * sizeof(TCHAR));
	lptstrCopy[length] = (TCHAR)0;    // null terminator character 
	GlobalUnlock(hglbCopy);

	// Place the handle on the clipboard. 
	SetClipboardData(CF_TEXT, hglbCopy);
	return true;
}