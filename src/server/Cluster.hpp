#pragma once

#include <vector>
#include <poll.h>
#include <map>
#include <set>
#include <chrono>
#include <iostream>
#include <errno.h>

#include "webserv.hpp"
#include "Server.hpp"
#include "HelperFunctions.hpp"
#include "dev/devHelpers.hpp"
#include "../router/Router.hpp"
#include "../config/Config.hpp"
#include "../parser/Parser.hpp"
#include "../request/Request.hpp"
#include "../router/Router.hpp"

#define RED "\033[1;31m"
#define GREEN "\033[1;32m"
#define CYAN "\033[1;36m"
#define YELLOW "\033[1;33m"
#define RESET "\033[0m"

#define CLIENT_DISCONNECT			" disconnected.\n"
#define CLIENT_TIMEOUT				" dropped by the server: Timeout.\n"
#define CLIENT_CLOSE_CONNECTION		" dropped by the server: Connection closed.\n"
#define CLIENT_MALFORMED_REQUEST	" dropped by the server: Malformed request.\n"
#define CLIENT_SEND_ERROR			" dropped by the server: send() failed.\n"

extern volatile sig_atomic_t signal_to_terminate;

// struct ListenerGroup {
// 	int	fd;
// 	std::vector<Server>	configs;
// 	const Server*		default_config;
// };

struct ClientRequestState {
	std::chrono::time_point<std::chrono::high_resolution_clock>	receive_start {};
	std::chrono::time_point<std::chrono::high_resolution_clock>	send_start {};
	std::string	clean_buffer;
	std::string	buffer;
	std::string	request;
	size_t		request_size;
	std::string	response;
	Server*		config;
	bool		data_validity = 1;
	bool		waiting_response = 0;
	bool		kick_me = 0;
	size_t		max_body_size = 0;
};

class Cluster {

	private:
		uint64_t						_max_clients;
		std::vector<pollfd>				_fds;				// servers and clients fds list for poll()
		std::set<int>					_server_fds;		// only servers fds
		std::vector<Server>				_configs;			// parsed configs
		// std::vector<ListenerGroup>	_listener_groups;	// groups of configs with same IP+port
		std::map<int, Server*>			_servers;			// fd of server and related ListenerGroup. Reason to have is to find quickly related ListeningGroup to key
		Router							_router;			// HTTP router for handling requests

		std::map<int, ClientRequestState>	_client_buffers;	// storing client related information

		// void	groupConfigs();
		// void	createGroup(const Server& conf);

		void	handleNewClient(size_t i);
		void	handleClientInData(size_t& i);
		void	sendPendingData(size_t& i);
		void	checkForTimeouts();
		void	dropClient(size_t& i, const std::string& msg);
		void	processReceivedData(size_t& i, const char* buffer, int bytes);
		void	prepareResponse(ClientRequestState& client_state, Request& req, int i);

	public:
		~Cluster();

		void	config(const std::string& config);
		void	create();
		void	run();

		// const Server&	findRelevantConfig(int client_fd, const std::string& buffer);

		const std::set<int>&	getServerFds() const;
};
