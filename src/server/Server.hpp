#pragma once

#include "webserv.hpp"

class Server {

	private:
	public:
		int server_fd;
		void	create();
		void	run();
};
