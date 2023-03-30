#include "Engine/Network/WinCommon.hpp"
#include "Engine/Network/NetSystem.hpp"
#include "Engine/Core/EngineCommon.hpp"

NetSystem::NetSystem(NetConfig const& config)
{
	UNUSED(config);
}

NetSystem::~NetSystem()
{

}

void NetSystem::Startup()
{
	// window specific - linux just enabled by default
	WORD version = MAKEWORD(2, 2);
	WSADATA data;

	int error = ::WSAStartup(version, &data);
	GUARANTEE_OR_DIE(error == 0, "Failed to initialize.");
}

void NetSystem::Shutdown()
{
	::WSACleanup(); // or in destructor?
}

void NetSystem::BeginFrame()
{

}

void NetSystem::EndFrame()
{

}
