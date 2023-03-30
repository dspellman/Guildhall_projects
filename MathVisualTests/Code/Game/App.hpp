#pragma once
//#include "Game/Game.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Renderer/Camera.hpp"

class Game;

// Class controlling rendering in Starship application
class App
{
public:
	Game* m_theGame = nullptr;

	void Run();
private:
	double m_lastFrameTime = 0.0; // start of last frame update

public:
	App();
	~App();
	void Startup();
	void Shutdown();
	void RunFrame();

	// functions called by Windows message handler
	bool IsQuitting() const;
	bool HandleQuitRequested();

private:
	// functions specific to rendering
	void BeginFrame();
	void Update(float deltaSeconds);
	void Render() const;
	void EndFrame();
};

extern App* g_theApp;