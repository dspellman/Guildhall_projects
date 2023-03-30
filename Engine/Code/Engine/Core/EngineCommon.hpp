#pragma once
#include <vector>
#include <string>

//-----------------------------------------------------------------------------------------------
// #SD1ToDo: Move this useful macro to a more central place, e.g. Engine/Core/EngineCommon.hpp
//
#define UNUSED(x) (void)(x);
#pragma warning (disable : 26812)

#include "Engine/Core/ErrorWarningAssert.hpp"
#include "StringUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/Vertex_PNCU.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Renderer/SimpleTriangleFont.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/IntVec3.hpp"
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/DevConsole.hpp"

extern NamedStrings g_gameConfigBlackboard;
extern DevConsole* g_theConsole;
extern EventSystem* g_theEventSystem;

// convenience templates for vector operations
template < typename T > void push(T* object, std::vector<T*>& objVector)
{
	for (int index = 0; index < objVector.size(); index++)
	{
		if (objVector[index] == nullptr)
		{
			objVector[index] = object;
			return;
		}
	}
	objVector.push_back(object);
}

template < typename T > void destroy(std::vector<T*>& objVector)
{
	for (int index = 0; index < objVector.size(); index++)
	{
		if (objVector[index])
		{
			delete objVector[index];
			objVector[index] = nullptr;
		}
	}
	objVector.clear();
}
