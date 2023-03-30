// Engine/Net/Net.hpp
#pragma comment(lib, "ws2_32.lib") // winsock libraries

struct NetConfig
{

};

class NetSystem
{
public:
	NetSystem(NetConfig const& config);
	~NetSystem();

	void Startup();
	void Shutdown();

	void BeginFrame();
	void EndFrame();
};
