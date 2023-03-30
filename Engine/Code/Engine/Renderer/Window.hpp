#pragma once
#include "Engine/Core/EngineCommon.hpp"

class InputSystem;

struct WindowConfig
{
	InputSystem* m_inputSystem = nullptr;
	std::string m_windowTitle = "Untitled App";
	float m_clientAspect = 2.0f;
	bool m_isFullscreen = false;
};

class Window
{
public:
	Window( WindowConfig const& config );
	~Window();

	void Startup();
	void BeginFrame();
	void EndFrame();
	void Shutdown();

	WindowConfig const& GetConfig() const;
	void* GetOSWindowHandle() const { return m_osWindowHandle; }
	static Window* GetWindowContext();
	Vec2 GetNormalizedCursorPos() const;
	void* GetHwnd() const;
	IntVec2 GetClientDimensions() const;
	bool HasFocus() const;
	std::string CopyClipboardToString();
	bool CopyStringToClipboard(std::string& text);

protected:
	void CreateOSWindow();
	void RunMessagePump();

protected:
	WindowConfig m_config;
	void* m_osWindowHandle = nullptr;
	static Window* s_mainWindow;  // refactor later to support multiple windows
private:
	void* m_desktopWindowHandle;
	int m_clientWidth = 0;
	int m_clientHeight = 0;
};