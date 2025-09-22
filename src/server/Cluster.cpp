#include "../../inc/webserv.hpp"
#include "Cluster.hpp"
#include "devHelpers.hpp"
#include "../config/Config.hpp"
#include "../parser/Parser.hpp"
#include "../request/Request.hpp"
#include "../router/Router.hpp"

void	Cluster::config(const std::string& config_file) {

	Config config;

	config.validate(config_file);
	_configs = config.parse(config_file);
	// printAllConfigs(_configs);
	if (_configs.size() == 0)
		throw std::runtime_error("Error: config file doesnt have any server"); // maybe this will be caught already in parsing
	groupConfigs();
	// printAllConfigGroups(_listener_groups);

	_max_clients = 100;

	// ASK ILIA to del this shit
	_router.setupRouter();
}

void	Cluster::groupConfigs() {
	for (auto& config : _configs) {
		if (_listener_groups.empty()) {
			createGroup(config);
			continue ;
		}
		uint32_t IP_conf = config.getAddress();
		int port_conf = config.getPort();

		bool added = false;
		for (auto& group : _listener_groups) {
			uint32_t IP_group =group.default_config->getAddress();
			int	port_group = group.default_config->getPort();

			if (IP_group == IP_conf && port_group == port_conf) {
				group.configs.push_back(config);
				added = true;
			}
		}
		if (!added)
			createGroup(config);
	}
}

void	Cluster::createGroup(const Server& conf) {
	ListenerGroup new_group;

	new_group.fd = -1;
	new_group.configs.push_back(conf);
	new_group.default_config = &conf;

	_listener_groups.push_back(new_group);
}

void	Cluster::create() {
	std::cout << "Initializing servers...\n";
	for (auto& group : _listener_groups)
	{
		Server serv = *group.default_config;
		int fd = serv.create();
		group.fd = fd;
		_fds.push_back({fd, POLLIN | POLLOUT, 0});
		_server_fds.insert(fd);
		_servers[fd] = &group;
	}
}

void	Cluster::run() {
	while (true)
	{
		if (poll(_fds.data(), _fds.size(), TIME_OUT_POLL) < 0)
			throw std::runtime_error("Error: poll");

		for (size_t i = 0; i < _fds.size(); ++i) {
			if (_fds[i].revents & POLLIN) {						// check if there is data to read related to fd
				if (isServerSocket(_fds[i].fd, getServerFds()))	// check if fd is server or client
					handleNewClient(i);
				else
					handleClientInData(i); // if client sends data before getting response, drop him for malformed request
			}
			if (_fds[i].revents & POLLOUT)
				sendPendingData(i);
		}
		checkForTimeouts();
	}
}

void	Cluster::handleNewClient(size_t i) {
	if (_fds.size() >= _max_clients) {
		// std::cout << "Server busy: can't accept new clients\n";
		return ;
	}
	sockaddr_in client_addr{};
	socklen_t addrlen = sizeof(client_addr);
	int client_fd = accept(_fds[i].fd, (sockaddr*)&client_addr, &addrlen); // 2nd argument: collect clients IP and port. 3rd argument tells size of the buffer of second argument
	if (client_fd < 0)
		throw std::runtime_error("Error: accept");

	setSocketToNonBlockingMode(client_fd);

	std::cout << "New client connected: "
			<< inet_ntoa(client_addr.sin_addr) << ":"
			<< ntohs(client_addr.sin_port) << ". Assigned fd: "
			<< client_fd << "\n";
	_fds.push_back({client_fd, POLLIN, 0});
	_clients[client_fd] = _servers[_fds[i].fd];
}

void	Cluster::handleClientInData(size_t& i) {
	char buffer[4096];
	int bytes = recv(_fds[i].fd, buffer, sizeof(buffer), 0);
	if (bytes <= 0)
		dropClient(i, CLIENT_DISCONNECT);
	else {
		// std::cout << "START----------------------------------------------------------\n";
		// std::cout << "bytes: " << bytes << std::endl;
		// std::cout << "buffer received (hex): ";
		// for (int j = 0; j < bytes; ++j) {
		// 	printf("%02X ", (unsigned char)buffer[j]);
		// }
		// std::cout << "\nEND----------------------------------------------------------\n";
		processReceivedData(i, buffer, bytes);
	}
}

std::string	headersToString(const std::unordered_map<std::string, std::vector<std::string>>& headers) {
	std::string result;
	for (const auto& pair : headers) {
		const std::string& key = pair.first;
		const std::vector<std::string>& values = pair.second;
		for (const auto& value : values) {
			result += key + ": " + value + "\r\n";
		}
	}
	return result;
}

// Convert Response object to HTTP string
std::string	responseToString(const Response& res) {
	std::string responseStr = "HTTP/1.1 " + std::string(res.getStatus()) + "\r\n";
	responseStr += headersToString(res.getAllHeaders());
		responseStr += "\r\n";
	responseStr += std::string(res.getBody());
	return responseStr;
}

void	Cluster::processReceivedData(size_t& i, const char* buffer, int bytes) {
	ClientRequestState& client_state = _client_buffers[_fds[i].fd];
	client_state.buffer.append(buffer, bytes);
	client_state.receive_start = std::chrono::high_resolution_clock::now();

	while (requestComplete(client_state)) {
		buildRequest(client_state);
		// call here the parser in future.
		Server conf = findRelevantConfig(_fds[i].fd, client_state.clean_buffer);
		// printServerConfig(conf); // this is simulating what will be sent to parser later
		Parser parse;
		Request req = parse.parseRequest(client_state.request, client_state.kick_me);
		Response res;
		// Handle the request using the router
		_router.handleRequest(req, res); // correct

		//res.print(); //DEBUG PRINT

		// Convert response to HTTP string format
		client_state.response = responseToString(res);

		if (client_state.buffer.empty())
			client_state.receive_start = {};
		else
		{
			// std::cout << "clock started\n";
			client_state.receive_start = std::chrono::high_resolution_clock::now();
		}

		_fds[i].events |= POLLOUT;			// start to listen if client is ready to receive response
		client_state.send_start = std::chrono::high_resolution_clock::now(); //this should be moved to the response part of code
		client_state.waiting_response = true;
	}

	if (client_state.data_validity == false) {
		// send response that payload is too large, 413
		dropClient(i, CLIENT_MALFORMED_REQUEST);
	}
}

void	Cluster::sendPendingData(size_t& i) {
	// --- Send minimal HTTP response ---
	ClientRequestState& client_state = _client_buffers[_fds[i].fd];
	// std::cout << "in sendPendingData(). Response: \n" << client_state.response << std::endl;
	if (!client_state.response.size())
		return ;

	if (client_state.waiting_response == true) {
		std::string response = popResponseChunk(client_state);
		std::cout << RED << "Sending response to: " << _fds[i].fd << RESET << std::endl;
		ssize_t sent = send(_fds[i].fd, response.c_str(), response.size(), 0);
		if (sent >= 0 && client_state.response.empty()) {
			// std::cout << "Response fully sent" << std::endl;
			_fds[i].events &= ~POLLOUT;
			client_state.send_start = std::chrono::high_resolution_clock::time_point{};
			client_state.waiting_response = false;
			// Ilia added this
			// Close connection after sending response (HTTP/1.0 style)
			// or infinity loop
			// dropClient(i, " - Response sent, closing connection");
		}
		else if (sent < 0)
			dropClient(i, CLIENT_SEND_ERROR);
		if (client_state.kick_me) {
			dropClient(i, CLIENT_CLOSE_CONNECTION);
		}
	}
}

void	Cluster::dropClient(size_t& i, const std::string& msg) {
	std::cout << CYAN << "Client " << _fds[i].fd << msg << RESET;
	close (_fds[i].fd);
	_client_buffers.erase(_fds[i].fd);
	_fds.erase(_fds.begin() + i);
	--i;
	// std::cout << "Currently active clients: " << _fds.size() - getServerFds().size() << "\n";
}

void	Cluster::checkForTimeouts() {
	auto now = std::chrono::high_resolution_clock::now();
	for (size_t i = 0; i < _fds.size(); ++i) {
		if (isServerSocket(_fds[i].fd, getServerFds()))
			continue ;
		if (_client_buffers[_fds[i].fd].receive_start != std::chrono::high_resolution_clock::time_point{}) {
			auto elapsed = now - _client_buffers[_fds[i].fd].receive_start;
			auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
			// std::cout << "Timeout checker request:" << elapsed_ms << std::endl;
			if (elapsed_ms > TIME_OUT_REQUEST && _client_buffers[_fds[i].fd].buffer.size() > 0)
				dropClient(i, CLIENT_TIMEOUT);
		}

		if (_client_buffers[_fds[i].fd].send_start != std::chrono::high_resolution_clock::time_point{}) {
			auto elapsed = now - _client_buffers[_fds[i].fd].send_start;
			auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
			// std::cout << "Timeout checker response:" << elapsed_ms << std::endl;
			if (elapsed_ms > TIME_OUT_RESPONSE && _client_buffers[_fds[i].fd].response.size() > 0)
				dropClient(i, CLIENT_TIMEOUT);
		}
	}
}

const Server&	Cluster::findRelevantConfig(int client_fd, const std::string& buffer) {
	std::smatch		match;
	ListenerGroup*	conf = _clients[client_fd];
	size_t			header_end = findHeader(buffer);
	std::string		header = buffer.substr(0, header_end);

	std::regex	re("Host:\\s*([^:\\s]+)"); // add optional case of port :8080 to be ignored if its there
	if (!std::regex_search(header, match, re))
		return *conf->default_config;

	std::string host = match[1];
	for (auto& conf : conf->configs) {
		if (conf.getName() == host)
			return conf;
	}
	return *conf->default_config;
}

const	std::set<int>& Cluster::getServerFds() const {
	return _server_fds;
}
