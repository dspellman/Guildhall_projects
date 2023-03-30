#include "Engine/Network/WinCommon.hpp"
#include "Engine/Network/TCPServer.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/EngineCommon.hpp"

// TCPServer is a "listen" socket

bool TCPServer::Host(uint16_t service, uint32_t backlog)
{
	NetAddress hostAddress;
	hostAddress.address = INADDR_ANY; // 0 - any IP that refers to my machine(all interfaces)
	hostAddress.port = service;

	// create the socket
	SOCKET mySocket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (mySocket == INVALID_SOCKET) 
	{
		return false;
	}

	// bind the socket (this is a good thing to move to Socket::Bind)
	// binds a socket to a port
	sockaddr_in ipv4;
	ipv4.sin_family = AF_INET;
	ipv4.sin_addr.S_un.S_addr = ::htonl(hostAddress.address);
	ipv4.sin_port = ::htons(hostAddress.port);                // NetInternal.hpp / cpp, NetAddressToSockAddress();

	int result = ::bind(mySocket, (sockaddr*)&ipv4, (int)sizeof(ipv4));
	if (SOCKET_ERROR == result) 
	{
		::closesocket(mySocket);
		return false;
	}

	m_handle = mySocket;
	m_address = hostAddress;

	// now listen
	result = ::listen(m_handle, backlog); // backlog -> how many people can be queued to connect
	if (result == SOCKET_ERROR)
	{
		Close();
	}
	return true;
}

TCPConnection* TCPServer::Accept()
{
	if (IsClosed()) 
	{
		return nullptr;
	}

	sockaddr_storage addr;
	int addrlen = sizeof(addr);
	SocketHandle handle = ::accept(m_handle,
		(sockaddr*)&addr, // address to fill
		&addrlen // in-out, how much space do we have, and how much did we use
	);

	// for now - all your sockets are blocking - this shouldn't happen
	// if non-blocking, you just didn't get a socket
	if (handle == INVALID_SOCKET) 
	{
		return nullptr;
	}

	// make sure it is the right address family (specific to us - we only support ipv4)
	if (addr.ss_family != AF_INET) 
	{
		::closesocket(handle);
		return nullptr;
	}

	// convert to a TCPConnection (is this appropriate in later code?)
	sockaddr_in* ipv4 = (sockaddr_in*)&addr;

	NetAddress netAddress;
	netAddress.address = ::ntohl(ipv4->sin_addr.S_un.S_addr);
	netAddress.port = ::ntohs(ipv4->sin_port);

	TCPConnection* conn = new TCPConnection();
	conn->m_handle = handle;
	conn->m_address = netAddress;

	conn->SetBlocking(false);

	return conn;
}

bool TCPServer::Listen(uint16_t port, uint32_t maxQueuedConnections)
{
	int addressFamily = AF_INET; // ipv4 address family, AF_INET6
	int socketType = SOCK_STREAM; // streaming socket, TCP (SOCK_DGRAM, datagram socket, UDP)
	int protocol = IPPROTO_TCP; // very strictly a TCP socket
	SocketHandle socketHandle = ::socket(addressFamily, socketType, protocol);
	if (socketHandle == INVALID_SOCKET) { // ~0, 0xffff`ffff`ffff`ffff
		return false;
	}

	// Bind the socket - associated my socket with an address
	sockaddr_in ipv4;
	ipv4.sin_family = AF_INET;
	ipv4.sin_addr.S_un.S_addr = INADDR_ANY; // 0
	ipv4.sin_port = port;

	int addressSize = (int)sizeof(ipv4);
	int result = ::bind(socketHandle, (sockaddr*)&ipv4, addressSize);
	if (result == SOCKET_ERROR) {
		::closesocket(socketHandle);
		return false;
	}

	m_handle = socketHandle;
	SetBlocking(false);

	// listen on this socket
	result = ::listen(socketHandle, maxQueuedConnections);
	if (result == SOCKET_ERROR) {
		Close();
		return false;
	}

	return true;
}
