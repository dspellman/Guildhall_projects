#pragma once
#include <stdint.h>
#include <minwindef.h>
#include <string>
#include <vector>

class NetAddress
{
public:
	uint32_t address = 0;
	uint16_t port = 0;

public:
	std::string ToString();
	bool IsLocal() const;

	static std::vector<NetAddress> GetAllInternal(uint16_t port);
	static NetAddress GetLoopBack(uint16_t port);
	static NetAddress GetLocal(uint16_t port);
	static NetAddress FromString(std::string const& str);
};

