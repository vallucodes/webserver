#pragma once
#include <vector>
#include <poll.h>
#include "webserv.hpp"

class Server {

	private:
	public:
		int server_fd;
		std::vector<pollfd> fds; // store here server socked fd and every connected cliends fd

		void	create();
		void	run();
};
