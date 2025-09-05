#include "Cluster.hpp"
#include "Server.hpp"

void	Cluster::config() {
	srand(time(0));

	std::string addr1 = "127.0.0.1";
	std::string addr2 = "127.0.0.1";
	int port1 = 1024 + rand() % (10000 - 1024 + 1);
	int port2 = 1024 + rand() % (10000 - 1024 + 1);

	_addresses.push_back({inet_addr(addr1.c_str()), port1});
	_addresses.push_back({inet_addr(addr2.c_str()), port2});
}

void	Cluster::create() {
	std::cout << "Initializing servers...\n";
	for (const std::pair<uint32_t, int>& entry : _addresses)
	{
		Server serv(entry.first, entry.second);
		serv.create();
		_fds.push_back({serv.getFd(), POLLIN, 0});
		_server_fds.insert(serv.getFd());
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
					handleClientData(i);
			}
			else
				checkForTimeouts();
		}
	}
}

void	Cluster::handleNewClient(size_t i) {
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
}

void	Cluster::handleClientData(size_t& i) {
	char buffer[1024];
	int bytes = recv(_fds[i].fd, buffer, sizeof(buffer), 0);
	if (bytes <= 0)
		dropClient(i, CLIENT_DISCONNECT);
	else
		processReceivedData(i, buffer, bytes);
}

void	Cluster::processReceivedData(size_t& i, const char* buffer, int bytes) {
	_client_buffers[_fds[i].fd].buffer.append(buffer, bytes);
	_client_buffers[_fds[i].fd].start = std::chrono::high_resolution_clock::now();
	ClientBuffer& client_data = _client_buffers[_fds[i].fd];

	if (requestComplete(client_data.buffer, client_data.status)) {	// check if request is fully received
		send(_fds[i].fd, buffer, bytes, 0);							// call here the parser in future. Send now is just sending back same message to client
		_client_buffers.erase(_fds[i].fd);
		_client_buffers[_fds[i].fd].start = {};
	}

	if (client_data.status == false) {		// flag of invalid request is set
		// send response that payload is too large, 413
		dropClient(i, CLIENT_MALFORMED_REQUEST);
	}
}

void	Cluster::dropClient(size_t& i, const std::string& msg) {
	std::cout << "Client " << _fds[i].fd << msg;
	close (_fds[i].fd);
	_client_buffers.erase(_fds[i].fd);
	_fds.erase(_fds.begin() + i);
	--i;
}

void	Cluster::checkForTimeouts() {
	auto now = std::chrono::high_resolution_clock::now();
	for (size_t i = 0; i < _fds.size(); ++i) {
		if (isServerSocket(_fds[i].fd, getServerFds()))
			continue ;
		if (_client_buffers[_fds[i].fd].start == std::chrono::high_resolution_clock::time_point{})
			continue ;
		auto elapsed = now - _client_buffers[_fds[i].fd].start;
		auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
		// std::cout << "Elapsed time: " << elapsed_ms << "Buffer size: " << _client_buffers[_fds[i].fd].buffer.size() << std::endl;
		if (elapsed_ms > TIME_OUT_REQUEST && _client_buffers[_fds[i].fd].buffer.size() > 0)
			dropClient(i, CLIENT_TIMEOUT);
	}
}

const	std::vector<std::pair<uint32_t, int>>& Cluster::getAddresses() const {  //move this to Config.hpp
	return _addresses;
}

const	std::set<int>& Cluster::getServerFds() const {
	return _server_fds;
}
