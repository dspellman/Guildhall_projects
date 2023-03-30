#include "Game/GameCommon.hpp"
#include "Game/App.hpp"
#include "Game/Game.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/InputSystem/InputSystem.hpp"
#include "Game/PlayerShip.hpp"
#include "Game/Beetle.hpp"
#include "Engine/InputSystem/XboxController.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Renderer/SimpleTriangleFont.hpp"
#include "Engine/Renderer/Window.hpp"
#include "Engine/Core/Clock.hpp"

//App* g_theApp = nullptr; // created and owned by Main_windows.cpp?
Renderer* g_theRenderer = nullptr; // created and owned by the App
InputSystem* g_theInput = nullptr;
AudioSystem* g_theAudio = nullptr;
Window* g_theWindow = nullptr;

void App::Run()
{
	while (!IsQuitting())
	{
		RunFrame();
	}
}

App::App()
{
}

App::~App()
{
	delete m_theGame;
	m_theGame = nullptr;
	delete g_theAudio;
	g_theAudio = nullptr;
	delete g_theConsole;
	g_theConsole = nullptr;
	delete g_theRenderer; // clean up here in case it was not shut down properly
	g_theRenderer = nullptr;
	delete g_theWindow;
	g_theWindow = nullptr;
	delete g_theInput;
	g_theInput = nullptr;
	delete g_theEventSystem;
	g_theEventSystem = nullptr;
};

void App::Startup()
{
	EventSystemConfig eventConfig;
	g_theEventSystem = new EventSystem(eventConfig);

	InputSystemConfig inputConfig;
	g_theInput = new InputSystem(inputConfig);

	WindowConfig windowConfig;
	windowConfig.m_windowTitle = "SD1-A5: Starship Gold (Re-factored)";
	windowConfig.m_clientAspect = 2.0f;
	windowConfig.m_inputSystem = g_theInput;
	g_theWindow = new Window(windowConfig);

	RendererConfig rendererConfig;
	rendererConfig.m_window = g_theWindow;
	g_theRenderer = new Renderer(rendererConfig);

	DevConsoleConfig consoleConfig;
	consoleConfig.theRenderer = g_theRenderer; // this can be null and get updated later
	consoleConfig.consoleFontName = g_gameConfigBlackboard.GetValue("CONSOLE_FONT", "Data/Fonts/MyFixedFont");
	consoleConfig.linesToShow = (g_gameConfigBlackboard.GetValue("CONSOLE_PRINT_LINES", 30));
	g_theConsole = new DevConsole(consoleConfig);

	AudioSystemConfig audioSystemConfig;
	g_theAudio = new AudioSystem(audioSystemConfig);

	g_theEventSystem->Startup();
	g_theInput->Startup();
	g_theWindow->Startup();
	g_theRenderer->Startup();
	g_theConsole->Startup();
	g_theAudio->Startup();

	m_theGame = new Game(); // create an instance that will handle different modes soon
	m_theGame->Startup(); // start up the game when there is a renderer
};

void App::Shutdown()
{
	m_theGame->Shutdown();

	g_theAudio->Shutdown();
	g_theConsole->Shutdown();
	g_theRenderer->Shutdown();
	g_theWindow->Shutdown();
	g_theInput->Shutdown();
	g_theEventSystem->Shutdown();
};

void App::RunFrame()
{
	BeginFrame();
	Update(static_cast<float>(g_systemClock.GetDeltaTime()));
	Render();
	EndFrame();
};

// functions called by Windows message handler
// we could try to set the flag to quit based on position elsewhere (say, in update)
// but doing it here makes the conditions clear rather than obfuscated
bool App::IsQuitting() const 
{ 
	return m_theGame->m_isQuitting; 
};

bool App::HandleQuitRequested()
{
	return true;
}

// functions specific to rendering
void App::BeginFrame()
{
	// Do all BeginFrames() here
	g_systemClock.SystemBeginFrame(); // start the system clock first?
	g_theEventSystem->BeginFrame();
	g_theInput->BeginFrame();
	g_theWindow->BeginFrame();
	g_theRenderer->BeginFrame();
	g_theConsole->BeginFrame();
	g_theAudio->BeginFrame();
};

void App::Update(float deltaSeconds)
{
	// intercept the ESC key if devConsole is open
	if (g_theConsole->IsOpen())
	{
		g_theInput->ConsumeKeyJustPressed(KEYCODE_ESC);
	}
	// there could be other things to update besides the game
	m_theGame->Update(deltaSeconds);
};


void App::Render() const
{
	m_theGame->Render();
};

void App::EndFrame()
{
	g_theAudio->EndFrame();
	g_theConsole->EndFrame();
	g_theRenderer->EndFrame();
	g_theWindow->EndFrame();
	g_theInput->EndFrame();
	g_theEventSystem->EndFrame();
};
