#include "Engine/Network/WinCommon.hpp"
#include "Engine/Network/Socket.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Rgba8.hpp"
#include <stdio.h>
#include "../Core/EngineCommon.hpp"

Socket::~Socket()
{
	Close();
}

Socket::Socket()
	: m_handle(INVALID_SOCKET)
{

}

bool Socket::Bind()
{
	ERROR_RECOVERABLE("what are parameters and what does this do?");
	return true;
}

void Socket::Close()
{
	if (!IsClosed())
	{
		::closesocket(m_handle);
		m_handle = INVALID_SOCKET;
	}
}

bool Socket::IsClosed() const
{
	return m_handle == INVALID_SOCKET;
}

//---------------------------------------------------------
// SetBlocking
//---------------------------------------------------------
void Socket::SetBlocking(bool isBlocking)
{
	u_long non_blocking = isBlocking ? 0 : 1;
	::ioctlsocket(m_handle, FIONBIO, &non_blocking);
}

//---------------------------------------------------------
// CheckFatalError() - Send & Receive
//---------------------------------------------------------
bool Socket::CheckForFatalError()
{
	char msg[80];
	int error = WSAGetLastError();
	switch (error) 
	{
	case 0: // no error
	case WSAEWOULDBLOCK:
		return false;

	case WSAECONNRESET:
		sprintf_s(msg, "Network: Socket hit fatal error: 0x%08x", error);
		g_theConsole->AddLine(Rgba8::RED, std::string(msg));
		return true;

	default:
		sprintf_s(msg, "Network: Disconnected due to error: 0x%08x", error);
		g_theConsole->AddLine(Rgba8::RED, std::string(msg));
		Close();
		return true;
	}
}
