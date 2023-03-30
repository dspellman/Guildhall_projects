// Engine/Net/Net.hpp
#pragma comment(lib, "ws2_32.lib") // winsock libraries

struct NetConfig
{

};

class Network
{
public:
	Network(NetConfig const& config);
	~Network();

	void Startup();
	void Shutdown();

	void BeginFrame();
	void EndFrame();
};
