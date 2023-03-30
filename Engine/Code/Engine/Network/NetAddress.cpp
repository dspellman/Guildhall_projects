#include "Engine/Network/WinCommon.hpp"
#include "Engine/Network/NetAddress.hpp"
#include "Engine/Core/StringUtils.hpp"
#include <inaddr.h>
#include <ws2tcpip.h>
#include "Engine/Core/EngineCommon.hpp"

NetAddress NetAddress::GetLoopBack(uint16_t portLocal)
{
	NetAddress addr;
	addr.address = 0x7f000001; // 127 0 0 1
	addr.port = portLocal;
	return addr;
}

//---------------------------------------------------------
// NetAddress:GetAllInternal()
//---------------------------------------------------------
// we want print an address other can join
// hosting 0.0.0.0 (INADDR_ANY)
std::vector<NetAddress> NetAddress::GetAllInternal(uint16_t portLocal)
{
	std::vector<NetAddress> results;

	char hostName[256];
	int result = gethostname(hostName, sizeof(hostName));
	if (result == SOCKET_ERROR) {
		return results;
	}
	// set a breakpoint - you'll your system name

	addrinfo hints;
	::memset(&hints, 0, sizeof(hints)); // zeros out memory
										  // addrinfo hints {};

	hints.ai_family = AF_INET; // only return IPv4 addresses
	hints.ai_socktype = SOCK_STREAM; // only TCP addresses

									 // hints.ai_flags = AI_PASSIVE; // only open sockets

	addrinfo* addresses = nullptr;
	int status = ::getaddrinfo(
		hostName,   // host we're resolving
		nullptr,    // optional service (http, ftp...)
		&hints,     // filter
		&addresses  // out addresses
	);

	if (status != 0) {
		return results;
	}

	// this is a linked list, 
	addrinfo* iter = addresses;
	while (iter != nullptr) {

		NetAddress addr;
		sockaddr_in* ipv4 = (sockaddr_in*)iter->ai_addr;

		addr.address = ::ntohl(ipv4->sin_addr.S_un.S_addr);
		addr.port = portLocal;
		results.push_back(addr);

		iter = iter->ai_next;
	}

	// clean up first
	::freeaddrinfo(addresses);

	return results;
}

//---------------------------------------------------------
// NetAddress::ToString()
//---------------------------------------------------------

// "a.b.c.d:port"
std::string NetAddress::ToString()
{
	return Stringf( "%u.%u.%u.%u:%u", 
		(address & 0xff000000) >> 24,
		(address & 0x00ff0000) >> 16, 
		(address & 0x0000ff00) >> 8,
		(address & 0x000000ff) >> 0,
		port
	);

	/*
	uint32 newAddr = ::htonl(address); 
	byte* bytes = (bytes*) &newAddr; 
	return Stringf( "%u.%u.%u.%u:%u", 
	bytes[0], bytes[1], bytes[2], bytes[3], 
	port ); 
	*/
}

bool NetAddress::IsLocal() const
{
	return false; // TEST DEBUG
}

NetAddress NetAddress::GetLocal(uint16_t port)
{
	UNUSED(port);
	NetAddress addr;
	return addr;
}

NetAddress NetAddress::FromString(std::string const& str)
{
	Strings parts = SplitStringOnDelimiter(str, ':');
	IN_ADDR addr;
	int result = ::inet_pton(AF_INET, parts[0].c_str(), &addr);
	if (result == SOCKET_ERROR)
	{
		return NetAddress(); // this will be 0.0.0.0:0
	}

	uint16_t port = (uint16_t)::atoi(parts[1].c_str());
	NetAddress address;
	address.address = ::ntohl(addr.S_un.S_addr);
	address.port = port;
	return address;
}
