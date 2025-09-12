#include "Cluster.hpp"
#include "devHelpers.hpp"
#include "../config/Config.hpp"
#include <arpa/inet.h>

void	Cluster::config(const std::string& config_file) {

	Config config;

	_configs = config.parse(config_file);
	// printAllConfigs(_configs);
	if (_configs.size() == 0)
		throw std::runtime_error("Error: config file doesnt have any server"); // maybe this will be caught already in parsing
	groupConfigs();
	// printAllConfigGroups(_listener_groups);

	_max_clients = 100;
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
	else
		processReceivedData(i, buffer, bytes);
}

#include <fstream>
std::string readFileToString(const std::string& filename) {
	std::ifstream file(filename);
	if (!file) {
		throw std::runtime_error("Could not open file: " + filename);
	}

	std::ostringstream buffer;
	buffer << file.rdbuf();  // read whole file into buffer
	return buffer.str();
}

void	Cluster::processReceivedData(size_t& i, const char* buffer, int bytes) {
	_client_buffers[_fds[i].fd].buffer.append(buffer, bytes);
	_client_buffers[_fds[i].fd].receive_start = std::chrono::high_resolution_clock::now();
	ClientRequestState& client_state = _client_buffers[_fds[i].fd];

	if (requestComplete(client_state.buffer, client_state.data_validity)) {
		// call here the parser in future. Send now is just sending back same message to client
		Server conf = findRelevantConfig(_fds[i].fd, _client_buffers[_fds[i].fd].buffer);
		printServerConfig(conf); // this is simulating what will be sent to parser later


		std::string body = readFileToString("www/index.html");
		client_state.response =
			"HTTP/1.1 200 OK\r\n"
			"Content-Type: text/html; charset=UTF-8\r\n"
			"Content-Length: " + std::to_string(body.size()) + "\r\n"
			"\r\n";

		client_state.response.append(body);
		client_state.buffer.clear();
		client_state.receive_start = {};
		_fds[i].events |= POLLOUT;
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
	if (!client_state.response.size())
		return ;

	if (client_state.waiting_response == true) {
		std::cout << "Sending data to: " << _fds[i].fd << std::endl;
		ssize_t sent = send(_fds[i].fd, client_state.response.c_str(), client_state.response.size(), 0); // this needs to be chuncked in future
		if (sent >= 0) {
			client_state.response.clear(); // response fully sent
			_fds[i].events &= ~POLLOUT;
			client_state.send_start = std::chrono::high_resolution_clock::time_point{};
			client_state.waiting_response = false;
		}
		else if (sent < 0)
			dropClient(i, CLIENT_SEND_ERROR);
	}
}

void	Cluster::dropClient(size_t& i, const std::string& msg) {
	std::cout << "Client " << _fds[i].fd << msg;
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
		if (_client_buffers[_fds[i].fd].receive_start == std::chrono::high_resolution_clock::time_point{} &&
		_client_buffers[_fds[i].fd].send_start == std::chrono::high_resolution_clock::time_point{})
			continue ;

		auto elapsed = now - _client_buffers[_fds[i].fd].receive_start;
		auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
		// std::cout << "Timeout checker request:" << elapsed_ms << std::endl;
		if (elapsed_ms > TIME_OUT_REQUEST && _client_buffers[_fds[i].fd].buffer.size() > 0)
			dropClient(i, CLIENT_TIMEOUT);

		elapsed = now - _client_buffers[_fds[i].fd].send_start;
		elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
		// std::cout << "Timeout checker response:" << elapsed_ms << std::endl;
		if (elapsed_ms > TIME_OUT_RESPONSE && _client_buffers[_fds[i].fd].response.size() > 0)
			dropClient(i, CLIENT_TIMEOUT);
	}
}

const Server&	Cluster::findRelevantConfig(int client_fd, std::string&	buffer) {
	std::smatch		match;
	ListenerGroup*	conf = _clients[client_fd];
	size_t			header_end = findHeader(buffer);
	std::string		header = buffer.substr(0, header_end);

	std::regex	re("Host:\\s+(\\S+)"); // add optional case of port :8080 to be ignored if its there
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
