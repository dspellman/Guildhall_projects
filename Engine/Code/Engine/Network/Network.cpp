#include "Engine/Network/WinCommon.hpp"
#include "Engine/Network/Network.hpp"
#include "Engine/Core/EngineCommon.hpp"

Network::Network(NetConfig const& config)
{
	UNUSED(config);
}

Network::~Network()
{

}

void Network::Startup()
{
	// window specific - linux just enabled by default
	WORD version = MAKEWORD(2, 2);
	WSADATA data;

	int error = ::WSAStartup(version, &data);
	GUARANTEE_OR_DIE(error == 0, "Failed to initialize.");
}

void Network::Shutdown()
{
	::WSACleanup();
}

void Network::BeginFrame()
{

}

void Network::EndFrame()
{

}
