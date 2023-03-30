#include "Engine/Network/WinCommon.hpp"
#include "Engine/Network/TCPConnection.hpp"
#include <stdint.h>
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

TCPConnection::TCPConnection()
{

}

bool TCPConnection::Connect(NetAddress const& addr)
{
	sockaddr_in ipv4;
	UINT uintAddr = 0; // 127.0.0.1 -> 0x7f000001
	UNUSED(uintAddr);

	ipv4.sin_family = AF_INET;
	ipv4.sin_addr.S_un.S_addr = ::htonl(addr.address);
	ipv4.sin_port = ::htons(addr.port);

	// create the socket
	SOCKET mySocket = ::socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	if (mySocket == INVALID_SOCKET)
	{
		return false;
	}

	// blocks until success or failure
	int result = ::connect( mySocket, (sockaddr*) &ipv4, (int) sizeof(ipv4));
	if (result == SOCKET_ERROR)
	{
		int error = ::WSAGetLastError(); // gets the actual error
		UNUSED(error);
		::closesocket(mySocket);
		return false;
	}

	SetBlocking(false);

	m_handle = mySocket;
	m_address = addr;
	return true;
}

size_t TCPConnection::Send(void const* data, size_t const dataSize)
{
	if (IsClosed())
	{
		return 0;
	}
	int bytesSent = ::send(m_handle, (char const*) data, (int) dataSize, 0);
	if (bytesSent > 0)
	{
		GUARANTEE_OR_DIE( bytesSent == dataSize, "sent not right");
		return bytesSent;
	}
	else
	{
		if (bytesSent == 0)
		{
			// special case - socket was closed cleanly
			Close();
			return 0;
		}
		else
		{
			// socket error later
			Close();
			return 0;
		}
	}
}

// send should always match, receive almost never will
size_t TCPConnection::Receive(void* buffer, size_t maxBytesToRead)
{
	int bytesRead = ::recv(m_handle, (char*)buffer, (int)maxBytesToRead, 0);
	if (bytesRead == 0) 
	{
		Close(); // clean close
		return 0;
	}
	else if (bytesRead > 0) 
	{
		return (size_t)bytesRead;
	}
	else 
	{
		CheckForFatalError();
		return 0;
	}
}

void TCPConnection::SendString(std::string const& str)
{
	Send(str.c_str(), str.size() + 1);
}