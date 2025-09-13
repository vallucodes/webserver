#include "../../inc/webserv.hpp"
#include "Cluster.hpp"
#include "Server.hpp"
#include "../router/Router.hpp"
#include "../router/handlers/Handlers.hpp"
#include "../request/Request.hpp"
#include "../response/Response.hpp"
#include "../parser/Parser.hpp"
#include <sstream>


Cluster::Cluster() {}

void	Cluster::config() {
	_max_clients = getMaxClients(); // shouldn allow to run a server if number is way too small
	std::cout << "Max clients: " << _max_clients << std::endl;
	srand(time(0));

	std::string addr1 = "127.0.0.1";
	std::string addr2 = "127.0.0.1";
	int port1 = 1024 + rand() % (10000 - 1024 + 1);
	int port2 = 1024 + rand() % (10000 - 1024 + 1);

	_addresses.push_back({inet_addr(addr1.c_str()), port1});
	_addresses.push_back({inet_addr(addr2.c_str()), port2});

	// ***** Router setup section *****
	// TODO: add here setupRouter function after configuration is parsed
	// _router.setupRouter(someConfigData& data);
	_router.setupRouter(); // dummy hardcoded function
	// ***** end of Router setup section *****
}

void	Cluster::create() {
	std::cout << "Initializing servers...\n";
	for (const std::pair<uint32_t, int>& entry : _addresses)
	{
		Server serv(entry.first, entry.second);
		serv.create();
		_fds.push_back({serv.getFd(), POLLIN | POLLOUT, 0});
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
	std::cout << "Currently active clients: " << _fds.size() - getServerFds().size() << "\n";
	if (client_fd < 0)
		throw std::runtime_error("Error: accept");

	setSocketToNonBlockingMode(client_fd);

	std::cout << "New client connected: "
			<< inet_ntoa(client_addr.sin_addr) << ":"
			<< ntohs(client_addr.sin_port) << ". Assigned fd: "
			<< client_fd << "\n";
	_fds.push_back({client_fd, POLLIN, 0});
}

void	Cluster::handleClientInData(size_t& i) {
	char buffer[1024];
	// char buffer[16384];
	int bytes = recv(_fds[i].fd, buffer, sizeof(buffer), 0);
	if (bytes <= 0)
		dropClient(i, CLIENT_DISCONNECT);
	else
		processReceivedData(i, buffer, bytes);
}

std::string headersToString(const std::unordered_map<std::string, std::vector<std::string>>& headers) {
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
std::string responseToString(const Response& res) {
    std::string responseStr = "HTTP/1.1 " + std::string(res.getStatus()) + "\r\n";
    responseStr += headersToString(res.getAllHeaders());
		responseStr += "\r\n";
    responseStr += std::string(res.getBody());
    return responseStr;
}

void	Cluster::processReceivedData(size_t& i, const char* buffer, int bytes) {
	_client_buffers[_fds[i].fd].buffer.append(buffer, bytes);
	_client_buffers[_fds[i].fd].receive_start = std::chrono::high_resolution_clock::now();
	ClientRequestState& client_state = _client_buffers[_fds[i].fd];

	if (requestComplete(client_state.buffer, client_state.data_validity)) {

		// test fill out request
		Request req;
		Parser	parse;

		// std::cout << "----------\n" << _client_buffers[_fds[i].fd].buffer << "----------\n"  << std::endl;
		req = parse.parseRequest(_client_buffers[_fds[i].fd].buffer);

		Response res;
		// Handle the request using the router
		_router.handleRequest(req, res); // correct

		// Convert response to HTTP string format
		client_state.response = responseToString(res);

		client_state.buffer.clear();
		client_state.receive_start = {};
		_fds[i].events |= POLLOUT;
		client_state.send_start = std::chrono::high_resolution_clock::now(); //this should be moved to the response part of code
		client_state.waiting_response = true;
	}

	if (client_state.data_validity == false) {		// flag of invalid request is set
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
		ssize_t sent = send(_fds[i].fd, client_state.response.c_str(), client_state.response.size(), 0);
		if (sent >= 0) {
			client_state.response.clear(); // response fully sent
			_fds[i].events &= ~POLLOUT;
			// _fds[i].events |= POLLIN;
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
		if (elapsed_ms > TIME_OUT_REQUEST && _client_buffers[_fds[i].fd].buffer.size() > 0)
			dropClient(i, CLIENT_TIMEOUT);

		elapsed = now - _client_buffers[_fds[i].fd].send_start;
		elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
		if (elapsed_ms > TIME_OUT_RESPONSE && _client_buffers[_fds[i].fd].response.size() > 0)
			dropClient(i, CLIENT_TIMEOUT);
	}
}

const	std::vector<std::pair<uint32_t, int>>& Cluster::getAddresses() const {  //move this to Config.hpp
	return _addresses;
}

const	std::set<int>& Cluster::getServerFds() const {
	return _server_fds;
}
