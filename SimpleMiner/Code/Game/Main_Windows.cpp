#define WIN32_LEAN_AND_MEAN		// Always #define this before #including <windows.h>
#include <windows.h>			// #include this (massive, platform-specific) header in very few places
#include <math.h>
#include <cassert>
#include <crtdbg.h>
#include <stdio.h>
#include "App.hpp"

App* g_theApp = nullptr; // not in App.cpp...

//-----------------------------------------------------------------------------------------------
// #SD1ToDo: Move this useful macro to a more central place, e.g. Engine/Core/EngineCommon.hpp
//
#define UNUSED(x) (void)(x);

//-----------------------------------------------------------------------------------------------
// #SD1ToDo: Move these constants to Game/GameCommon.hpp or elsewhere
// 
constexpr float CLIENT_ASPECT = 2.0f; // We are requesting a 2:1 aspect window area

//-----------------------------------------------------------------------------------------------
// #SD1ToDo: Move each of these items to its proper place, once that place is established
// 
const char* APP_NAME = "SimpleMiner";	// ...becomes ??? (Change this per project!)

//-----------------------------------------------------------------------------------------------
int WINAPI WinMain( HINSTANCE applicationInstanceHandle, HINSTANCE, LPSTR commandLineString, int )
{
	UNUSED( applicationInstanceHandle );
	UNUSED( commandLineString );

	g_theApp = new App();
	g_theApp->Startup();
	g_theApp->Run();
	g_theApp->Shutdown();
	delete g_theApp;
	g_theApp = nullptr;

	return 0;
}


