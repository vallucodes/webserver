#pragma once
#include <vector>
#include <poll.h>
#include "webserv.hpp"

class Server {

	private:
		int			_fd;
		uint32_t	_address;
		int			_port;

	public:
		Server(uint32_t address, int port);
		~Server() = default;
		std::vector<pollfd> fds; // store here server socked fd and every connected cliends fd

		void	create();
		int		getFd();
};
