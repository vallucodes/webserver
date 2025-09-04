#pragma once

#include <vector>
#include <poll.h>
#include <map>
#include <set>
#include <chrono>

#include "webserv.hpp"
#include "HelperFunctions.hpp"

class Cluster {

	private:
		std::vector<std::pair<uint32_t, int>> _addresses;  //move this to Config.hpp

		std::vector<pollfd>	_fds;				// store here all servers sockets fd and every connected cliends fd
		std::set<int>		_server_fds;		// only servers fds

		struct ClientBuffer {
			std::chrono::time_point<std::chrono::high_resolution_clock>	start;
			std::string	buffer;
			bool		status = 1;
		};
		std::map<int, ClientBuffer>	_client_buffers;	// storing client related reuqest, and bool is 1:valid, 0 invalid

	public:

		void	config();
		void	create();
		void	run();

		const std::vector<std::pair<uint32_t, int>>& getAddresses() const; //move this to Config.hpp
		const std::set<int>& getServerFds() const;

		void	checkForTimeouts();
};
