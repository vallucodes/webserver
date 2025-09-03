#pragma once
#include <vector>
#include <poll.h>
#include "webserv.hpp"

class Server {

	private:
	public:
		int server_fd;
		std::vector<pollfd> fds;
		
		void	create();
		void	run();
};
