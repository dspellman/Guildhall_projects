#pragma once
#include <vector>
#include "Engine/Network/NetAddress.hpp"

constexpr uint16_t DEFAULT_PORT = 3121;

class DevConsole;
class TCPConnection;
class TCPServer;

enum State
{
	DISCONNECTED,
	HOSTING,
	CLIENT,
};

//---------------------------------------------------------
// RemoteConsole - message format (outgoing options)
//---------------------------------------------------------

// [[header][payload]] [[header][payload]]...

struct rdc_header_t
{
	uint16_t payload_size;
};

#pragma pack(push, 1)
struct rdc_payload_t
{
	uint8_t is_echo; // 0 cmd, 1 echo... other values... user defined?
	uint16_t message_size; // including null
						   // char const* message; 
};
#pragma pack(pop)

constexpr uint16_t REMOTE_CONSOLE_PORT = 3121;   // 0x0C31
											 
struct RemoteConsoleConfig
{
	DevConsole* console = nullptr;
};

class RemoteDevConsole
{
public:
	~RemoteDevConsole();
	RemoteDevConsole(RemoteConsoleConfig const& config);
	void Startup();
	void Shutdown();
	void Update(float deltaSeconds);
	void ProcessConnections();
	void AttempToHost(uint16_t port);
	bool TryToHost(uint16_t port);
	void SendCommand(int connIdx, std::string const& cmd);
	void UpdateHostState();
	void UpdateDisconnected();
	void UpdateHost();
	void UpdateClient();

public:
	NetAddress m_hostAddr;
	DevConsole* m_console = nullptr;
	TCPServer* m_server = nullptr;
	std::vector<TCPConnection*> m_connections;
private:
	bool SetState(State param1);
	void RequestHost(uint16_t defaultPort);
	bool IsHostRequested();
	void ClearHostRequest();
	void RequestJoin(NetAddress localAddr);
	bool IsJoinRequested();
	void ClearJoinRequest();

	State m_state = DISCONNECTED;
	int m_defaultStateStep = 0;
	bool m_hostingRequest = false;
	bool m_joiningRequest = false;
	uint16_t m_hostPort = 3121;
};
