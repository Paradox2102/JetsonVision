#include "DataServer.hpp"

#include <unistd.h>

#include <sstream>

#include <vector>

void DataServer::bufferData(char header, const std::vector<int>& args)
{
	std::ostringstream stream;

	stream << header;
	for (auto arg : args)
	{
		stream << ' ' << arg;
	}
	stream << std::endl;

	buffer = stream.str();
}

void DataServer::readData(int)
{
	// TODO: Handle commands from client (robot) 
}

void DataServer::writeData(int fd)
{
	::write(fd, buffer.data(), buffer.size());
}
