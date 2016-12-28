#pragma once

#include <netdb.h>
#include <poll.h>
#include <sys/socket.h>

#include <vector>

class Server
{
	int sockfd;
	std::vector<pollfd> clients;

	static const addrinfo hints;

public:
	Server(const char* port, int backlog = SOMAXCONN);
	virtual ~Server();

	Server(const Server&) = delete;
	Server& operator=(const Server&) = delete;

	void run(int timeout = -1);

protected:
	virtual void readData(int fd) = 0;
	virtual void writeData(int fd) = 0;

	static int check(int res);
};
