#pragma once

// Engine/Platform/WinCommon.hpp
#define WIN32_LEAN_AND_MEAN		// Always #define this before #including <windows.h>
//#define NOMINMAX 
#include <windows.h>			// #include this (massive, platform-specific) header in very few places
#include <WinSock2.h>