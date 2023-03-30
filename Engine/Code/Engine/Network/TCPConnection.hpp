#pragma once
#include "TCPSocket.hpp"
#include "NetAddress.hpp"

class TCPConnection : public TCPSocket
{
public:
	TCPConnection();
	bool Connect(NetAddress const& addr);
	size_t Send( void const* data, size_t const dataSize);
	size_t Receive( void* buffer, size_t maxBytesToRead);
	void SendString(std::string const& str);
};
