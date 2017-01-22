#include "Server.hpp"

#include <netdb.h>
#include <poll.h>
#include <sys/socket.h>

#include <unistd.h> // close

#include <system_error>
#include <stdexcept>

const addrinfo Server::hints
{
	AI_PASSIVE | AI_NUMERICSERV,
	AF_UNSPEC,
	SOCK_STREAM,
	0
};

Server::Server(const char* port, int backlog)
{
	addrinfo* info;
	int ret = ::getaddrinfo(nullptr, port, &hints, &info);
	if (ret != 0)
	{
		throw std::runtime_error(gai_strerror(ret));
	}

	// Only use first result
	sockfd = check(socket(info->ai_family, info->ai_socktype, info->ai_protocol));

	static const int yes = 1;
	::setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes);

	check(::bind(sockfd, info->ai_addr, info->ai_addrlen));
	freeaddrinfo(info);

	// Listening socket is first "client"
	check(::listen(sockfd, backlog));
	clients.push_back({ sockfd, POLLIN });
}

Server::~Server()
{
	for (pollfd client : clients)
	{
		close(client.fd);
	}
}

void Server::run(int timeout)
{
	check(poll(clients.data(), clients.size(), timeout));

	if (clients.front().revents & POLLIN)
	{
		int client = check(::accept(sockfd, nullptr, nullptr));

		auto it = ++clients.begin();
		for (; it != clients.end(); ++it)
		{
			if (it->fd < 0)
			{
				it->fd = client;
				break;
			}
		}

		if (it == clients.end())
		{
			clients.push_back({ client, POLLIN | POLLOUT });
		}
	}

	for (auto it = ++clients.begin(); it != clients.end(); ++it)
	{
		// TODO: Reading data sent after hang up
		if (it->revents & POLLHUP)
		{
			::close(it->fd);
			it->fd = -1;
			continue;
		}

		if (it->revents & POLLIN)
		{
			readData(it->fd);
		}

		if (it->revents & POLLOUT)
		{
			writeData(it->fd);
		}
	}
}

int Server::check(int ret)
{
	if (ret == -1)
	{
		throw std::system_error(errno, std::system_category());
	}

	return ret;
}
