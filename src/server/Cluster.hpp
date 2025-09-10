#pragma once

#include <vector>
#include <poll.h>
#include <map>
#include <set>
#include <chrono>
#include <iostream>

#include "webserv.hpp"
#include "Server.hpp"
#include "HelperFunctions.hpp"

class Cluster {

	private:
		uint64_t				_max_clients;
		std::vector<pollfd>		_fds;				// servers and clients fds
		std::set<int>			_server_fds;		// only servers fds
		std::vector<Server>		_configs;			// parsed configs
		std::map<int, Server*>	_servers;			// fd and related config to sent it later to parser

		struct ClientRequestState {
			std::chrono::time_point<std::chrono::high_resolution_clock>	receive_start {};
			std::chrono::time_point<std::chrono::high_resolution_clock>	send_start {};
			std::string	buffer;
			std::string	response;
			bool		data_validity = 1;
			bool		waiting_response = 0; // later can be removed if check that response field has anything, send that
		};
		std::map<int, ClientRequestState>	_client_buffers;	// storing client related reuqest, and bool is 1:valid, 0 invalid

		void	handleNewClient(size_t i);
		void	handleClientInData(size_t& i);
		void	sendPendingData(size_t& i);
		void	dropClient(size_t& i, const std::string& msg);
		void	processReceivedData(size_t& i, const char* buffer, int bytes);
		void	checkForTimeouts();

	public:

		void	config(const std::string& config);
		void	create();
		void	run();

		const std::set<int>&	getServerFds() const;
};
