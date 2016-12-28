#pragma once

#include "Server.hpp"

#include <string>
#include <vector>

class DataServer : public Server
{
	std::string buffer;

public:
	using Server::Server;

	void bufferData(char header, const std::vector<int>& args);

protected:
	virtual void readData(int fd) override;
	virtual void writeData(int fd) override;
};
