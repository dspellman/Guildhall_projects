#include "Engine/Network/WinCommon.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "TCPServer.hpp"
#include "Engine/Network/RemoteDevConsole.hpp"

// We can compile the Mesh out of the engine
#include "Game/EngineBuildPreferences.hpp"
#if !defined( ENGINE_DISABLE_NETWORK )

RemoteDevConsole::~RemoteDevConsole()
{

}

RemoteDevConsole::RemoteDevConsole(RemoteConsoleConfig const& config)
{
	m_console = config.console;
}

void RemoteDevConsole::Startup()
{
	m_state = DISCONNECTED;

	// conn->Connect( NetAddress::FromString("10.9.163.77:3121") ); 

// 	uint16_t port = 3121; // TEST DEBUG?
// 	std::vector<NetAddress> localAddresses = NetAddress::GetAllInternal(port);
// 	UNUSED(localAddresses);
// 
// 	TCPConnection* conn = new TCPConnection();
// 	// where is hostAddr set?
// 	if (conn->Connect(*hostAddr)) {
// 		conn->SetBlocking(false);
// 		conn->SendString("Hello World");
// 
// 		m_connections.push_back(conn);
// 	}
// 	else {
// 		delete conn;
// 	}
// 
// 	uint16_t servicePort = 3121; // 0x0c31
// 	TCPServer* server = new TCPServer();
// 	server->Host(servicePort);
// 	m_server = server; // clarification, RemoteConsole has a server variable
// 
// 	bool hosted = true; // TEST DEBUG where is this set?
// 	if (hosted) {
// 		std::vector<NetAddress> addresses =
// 			NetAddress::GetAllInternal(port);
// 
// 		for (int i = 0; i < addresses.size(); ++i) {
// 			ERROR_RECOVERABLE("print to dev console?");
// 			//Log(addresses[i].ToString().c_str());
// 		}
// 	}
}

void RemoteDevConsole::Shutdown()
{

}

void RemoteDevConsole::Update(float deltaSeconds)
{
	UNUSED(deltaSeconds);
	switch (m_state) 
	{
	case DISCONNECTED:
		UpdateDisconnected();
		break;

	case CLIENT:
		UpdateClient();
		break;

	case HOSTING:
		UpdateHost();
		break;
	}
}

void RemoteDevConsole::ProcessConnections()
{
	constexpr size_t BUFFER_SIZE = 4096;
	uint8_t buffer[BUFFER_SIZE + 1];

	for (int i = 0; i < m_connections.size(); ++i) 
	{
		TCPConnection* cp = m_connections[i];

		size_t recvd = cp->Receive(buffer, BUFFER_SIZE);
		if (recvd > 0) 
		{
			buffer[recvd] = NULL;
			char msg[80];
			sprintf_s(msg, "Received: \"%s\"", buffer);
			g_theConsole->AddLine(Rgba8::GREEN, std::string(msg));
		}

		if (cp->IsClosed()) {
			m_connections.erase(m_connections.begin() + i);
			delete cp;
			--i;
		}
	}
}

//---------------------------------------------------------
// TCPServer (hosting creates a server)
//---------------------------------------------------------
void RemoteDevConsole::AttempToHost(uint16_t port)
{
	TCPServer* server = new TCPServer();
	if (server->Host(port)) 
	{
		server->SetBlocking(false);
		m_server = server;
	}
	else 
	{
		// someone is already hosting on INADDR_ANY on this port
		TCPConnection* conn = new TCPConnection();

		// 127.0.0.1:<port>
		NetAddress loopback = NetAddress::GetLoopBack(port);

		if (conn->Connect(loopback)) 
		{
			conn->SetBlocking(false);
			m_connections.push_back(conn);

			// send some initial data - go for it.
		}
	}
}

bool RemoteDevConsole::TryToHost(uint16_t port)
{
	TCPServer* server = new TCPServer();
	int32_t const maxConnectionsQueued = 32;
	if (server->Listen(port, maxConnectionsQueued)) 
	{
		m_server = server;
		SetState(HOSTING);  // TEST DEBUG what is enum and where is it tested?
		return true;
	}
	else 
	{
		delete server;
		return false;
	}
}

// RC cmd="Help"
// 0, 00, 05, H, e, l, p, 0

// packet
// 00, 08, [payload]
// 00, 08, 00, 00, 05, h, e, l, p, 0

void RemoteDevConsole::SendCommand(int connIdx, std::string const& cmd)
{
	TCPConnection* cp = m_connections[connIdx]; // add error checking

	size_t payloadSize = 1 // isEcho
		+ 2 // cmd size with null
		+ cmd.size() + 1; // cmd

	size_t packetSize = 2  // size of payload
		+ payloadSize; // actual payload

					   //---------------------------------------------------------
					   // manual
	uint8_t buffer[512];
	uint8_t* iter = buffer;

	*((uint16_t*)iter) = ::htons(uint16_t(payloadSize));
	iter += sizeof(uint16_t);

	*iter = 0;
	++iter;

	*((uint16_t*)iter) = ::htons((uint16_t)(cmd.size() + 1));
	iter += sizeof(uint16_t);

	::memcpy(iter, cmd.c_str(), cmd.size() + 1);
	cp->Send(buffer, packetSize);

	//---------------------------------------------------------
	// struct helper
	rdc_header_t header;
	header.payload_size = ::htons(uint16_t(payloadSize));

	rdc_payload_t payload;
	payload.is_echo = false;
	payload.message_size = ::htons((uint16_t)(cmd.size() + 1));

	cp->Send(&header, sizeof(header));
	cp->Send(&payload, sizeof(payload));
	cp->Send(cmd.c_str(), cmd.size() + 1);

	//---------------------------------------------------------
	// stream class helper
// 	MemoryWriter writer(buffer, sizeof(buffer));
// 	writer.SetEndianness(BIG_ENDIAN);
// 
// 	writer.Write<uint16_t>(packetSize);
// 	writer.Write<byte>(0);
// 	writer.Write<uint16_t>((uint16_t)(msg.size() + 1));
// 	writer.WriteBytes(msg.c_str(), msg.size() + 1);
// 
// 	cp->Send(writer.get_buffer(), writer.get_size());
}

// TEST DEBUG where does this get called?
void RemoteDevConsole::UpdateHostState()
{
	// look for new connections
	TCPConnection* conn = m_server->Accept();
	if (conn != nullptr) {
		m_connections.push_back(conn);
	}

	ProcessConnections();
}

bool RemoteDevConsole::SetState(State state)
{
	m_state = state;
	return false;
}

void RemoteDevConsole::RequestHost(uint16_t defaultPort)
{
	m_hostingRequest = true;
	m_hostPort = defaultPort;
}

void RemoteDevConsole::ClearHostRequest()
{
	m_hostingRequest = false;
}

void RemoteDevConsole::RequestJoin(NetAddress localAddr)
{
	m_joiningRequest = true;
	m_hostAddr = localAddr;
}

bool RemoteDevConsole::IsJoinRequested()
{
	return m_joiningRequest;
}

void RemoteDevConsole::ClearJoinRequest()
{
	m_joiningRequest = false;
}

bool RemoteDevConsole::IsHostRequested()
{
	return m_hostingRequest;
}

void RemoteDevConsole::UpdateDisconnected()
{
	if (IsHostRequested()) 
	{
		TCPServer* server = new TCPServer();
		if (server->Host(m_hostPort)) 
		{
// 			PrintAllJoinableAddresses(); // the if (hosted) section
			SetState(HOSTING);
		}
		else 
		{
			delete server;
			ClearHostRequest();
		}
	}
	else if (IsJoinRequested()) 
	{
		// all the code for joining moves here
		// if it succeeds, move to client state
		// else, clear join request
		if (false)
		{
			SetState(CLIENT);
		}
		else
		{
			ClearJoinRequest();
		}
	}
	else 
	{
		switch (m_defaultStateStep) 
		{
		case 0: 
			RequestJoin(NetAddress::FromString("127.0.0.1:3121"));
			break;
		case 1: 
			RequestHost(DEFAULT_PORT); 
			break;
		case 2:
		default: 
			return;
		}
		++m_defaultStateStep;  // seems right?
	}
}

// compared to UpdateHostState()?
void RemoteDevConsole::UpdateHost()
{
	TryToHost(m_hostPort);
	UpdateHostState();
	// check for connections
	// ...

	// check for messages from existing connections
	// ...

	// check for disconnects
	// ...
}

void RemoteDevConsole::UpdateClient()
{
	// check for messages from existing connections
	// ...

	// check for disconnects
	// ...

	// if no connections, disconnect
	// ...
}

#endif // !defined( ENGINE_DISABLE_NETWORK )