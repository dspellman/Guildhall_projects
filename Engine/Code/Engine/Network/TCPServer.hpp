#pragma once
//#include <stdint.h>
#include "Engine/Network/TCPSocket.hpp"
#include "Engine/Network/TCPConnection.hpp"

class TCPServer : public TCPSocket
{
public:
	bool Host(uint16_t service, uint32_t backlog = 16);
	TCPConnection* Accept();
	bool Listen(uint16_t port, uint32_t maxQueuedConnections);
};