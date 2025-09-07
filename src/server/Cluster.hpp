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

		uint64_t			_max_clients;
		std::vector<pollfd>	_fds;				// store here all servers sockets fd and every connected cliends fd
		std::set<int>		_server_fds;		// only servers fds

		struct ClientRequestState {
			std::chrono::time_point<std::chrono::high_resolution_clock>	start {};
			std::string	buffer;
			std::string	response;
			bool		data_validity = 1;
			bool		waiting_response = 0; // later can be removed if check that response field has anything, send that
		};
		std::map<int, ClientRequestState>	_client_buffers;	// storing client related reuqest, and bool is 1:valid, 0 invalid

		void	handleNewClient(size_t i);
		void	handleClientInData(size_t& i);
		void	sendPendingData(size_t i);
		void	dropClient(size_t& i, const std::string& msg);
		void	processReceivedData(size_t& i, const char* buffer, int bytes);
		void	checkForTimeouts();

	public:

		void	config();
		void	create();
		void	run();

		const std::vector<std::pair<uint32_t, int>>&	getAddresses() const; //move this to Config.hpp
		const std::set<int>&	getServerFds() const;
};
