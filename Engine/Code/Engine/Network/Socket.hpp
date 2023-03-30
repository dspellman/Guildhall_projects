#pragma once
#include <stdint.h>
#include "Engine/Network/NetAddress.hpp"
//#include <vadefs.h>

typedef uintptr_t SocketHandle;

class Socket
{
public:
	~Socket();
	Socket();
	bool Bind();
	void Close();
	bool IsClosed() const;
	void SetBlocking(bool isBlocking);
	bool CheckForFatalError();
//protected:
public:
	SocketHandle m_handle;
	NetAddress m_address;
};