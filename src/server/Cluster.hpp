#pragma once

#include <vector>
#include <poll.h>
#include <map>
#include <set>
#include <chrono>
#include <iostream>

#define RED "\033[1;31m"
#define GREEN "\033[1;32m"
#define CYAN "\033[1;36m"
#define YELLOW "\033[1;33m"
#define RESET "\033[0m"

#include "webserv.hpp"
#include "Server.hpp"
#include "HelperFunctions.hpp"
#include "../router/Router.hpp"

struct ListenerGroup {
	int	fd;
	std::vector<Server>	configs;
	const Server*		default_config;
};

struct ClientRequestState {
	std::chrono::time_point<std::chrono::high_resolution_clock>	receive_start {};
	std::chrono::time_point<std::chrono::high_resolution_clock>	send_start {};
	std::string	buffer;
	std::string	request;
	size_t		request_size;
	std::string	response;
	Server*		config;
	bool		data_validity = 1;
	bool		waiting_response = 0;

};

class Cluster {

	private:
		uint64_t						_max_clients;
		std::vector<pollfd>				_fds;				// servers and clients fds list for poll()
		std::set<int>					_server_fds;		// only servers fds
		std::vector<Server>				_configs;			// parsed configs
		std::vector<ListenerGroup>		_listener_groups;	// group of configs with same IP+port
		std::map<int, ListenerGroup*>	_servers;			// fd of server and related ListenerGroup. Reason to have is to find quickly related ListeningGroup to key
		std::map<int, ListenerGroup*>	_clients;			// fd of client and related config
		Router							_router;			// HTTP router for handling requests


		std::map<int, ClientRequestState>	_client_buffers;	// storing client related reuqest, and bool is 1:valid, 0 invalid

		void			groupConfigs();
		void			createGroup(const Server& conf);
		const Server&	findRelevantConfig(int client_fd, const std::string& buffer);
		void			buildRequest(ClientRequestState& client_state);
		std::string		popResponseChunk(ClientRequestState& client_state);

		// bool			requestComplete(ClientRequestState& client_state);
		// int				isChunkedBodyComplete(std::string& buffer, size_t header_end);
		// bool			isRequestBodyComplete(ClientRequestState& client_state, const std::string& buffer, size_t header_end);
		// void			decodeChunkedBody(std::string& buffer);

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
