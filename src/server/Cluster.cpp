#include "Cluster.hpp"
#include "dev/devHelpers.hpp"
#include "../config/Config.hpp"
#include "../parser/Parser.hpp"
#include "../request/Request.hpp"
#include "../router/Router.hpp"
#include "../router/utils/HttpResponseBuilder.hpp"
#include "../response/Response.hpp"

void	Cluster::config(const std::string& config_file) {
	Config config;

	signal(SIGINT, handleSigTerminate);
	signal(SIGTERM, handleSigTerminate);

	config.validate(config_file);
	_configs = config.parse(config_file);
	printAllConfigs(_configs);
	validateConfigs();

	_max_clients = getMaxClients();
	_router.setupRouter(_configs);
}

void	Cluster::validateConfigs() {
	if (_configs.size() == 0)
		throw std::runtime_error("Error: Config: no server in config file");
	std::set<int> ports;
	for (auto& config : _configs)
		if (!ports.insert(config.getPort()).second)
			throw std::runtime_error("Error: Config: repeated ports");
}

void	Cluster::create() {
	std::cout << CYAN << time_now() << "	Initializing servers...\n" << RESET;
	for (Server& conf : _configs)
	{
		int fd = conf.create();
		_fds.push_back({fd, POLLIN | POLLOUT, 0});
		_server_fds.insert(fd);
		_servers[fd] = &conf;
	}
}

void	Cluster::run() {
	while (signal_to_terminate == false)
	{
		if (poll(_fds.data(), _fds.size(), TIME_OUT_POLL) < 0) {
			if (errno == EINTR)
				continue ;
			throw std::runtime_error("Error: poll()");
		}

		for (size_t i = 0; i < _fds.size(); ++i) {
			if (_fds[i].revents & POLLIN) {
				if (isServerSocket(_fds[i].fd, getServerFds()))
					handleNewClient(i);
				else
					handleClientInData(i);
			}
			if (_fds[i].revents & POLLOUT)
				sendPendingData(i);
		}
		checkForTimeouts();
	}
}

void	Cluster::handleNewClient(size_t i) {
	if (_fds.size() >= _max_clients)
		return ;

	sockaddr_in client_addr{};
	socklen_t addrlen = sizeof(client_addr);
	int client_fd = accept(_fds[i].fd, (sockaddr*)&client_addr, &addrlen);
	if (client_fd < 0)
		throw std::runtime_error("Error: accept()");

	setSocketToNonBlockingMode(client_fd);

	std::cout << CYAN
			<< time_now()
			<< "	New client connected"
			<< ". Assigned socket: "
			<< client_fd << "\n"
			<< RESET;

	_fds.push_back({client_fd, POLLIN, 0});
	_client_buffers[client_fd].config = _servers[_fds[i].fd];
	_client_buffers[client_fd].max_body_size = _servers[_fds[i].fd]->getMaxBodySize();
}

void	Cluster::handleClientInData(size_t& i) {
	char buffer[4096];
	int bytes = recv(_fds[i].fd, buffer, sizeof(buffer), 0);
	if (bytes <= 0)
		dropClient(i, CLIENT_DISCONNECT);
	else
		processReceivedData(i, buffer, bytes);
}

void	Cluster::prepareResponse(ClientRequestState& client_state, Request& req, int i) {
	Response res;
	_router.handleRequest(*client_state.config, req, res);
	client_state.response = responseToString(res);
	_fds[i].events |= POLLOUT;
	client_state.send_start = std::chrono::high_resolution_clock::now();
	client_state.waiting_response = true;
}

void	Cluster::processReceivedData(size_t& i, const char* buffer, int bytes) {
	ClientRequestState& client_state = _client_buffers[_fds[i].fd];
	client_state.buffer.append(buffer, bytes);
	client_state.receive_start = std::chrono::high_resolution_clock::now();

	while (requestComplete(client_state)) {
		client_state.request = client_state.clean_buffer.substr(0, client_state.request_size);
		// const Server& conf = findRelevantConfig(_fds[i].fd, client_state.clean_buffer);
		Parser parse;
		Request req = parse.parseRequest(client_state.request, client_state.kick_me, false);
		prepareResponse(client_state, req, i);
		setTimer(client_state);
	}

	if (client_state.data_validity == false) {
		// const Server& conf = findRelevantConfig(_fds[i].fd, client_state.clean_buffer);
		Parser parse;
		Request req = parse.parseRequest("400 Bad Request", client_state.kick_me, false);
		prepareResponse(client_state, req, i);
		client_state.kick_me = true;
	}
}

void	Cluster::sendPendingData(size_t& i) {
	ClientRequestState& client_state = _client_buffers[_fds[i].fd];
	if (!client_state.response.size())
		return ;

	if (client_state.waiting_response == true) {
		std::string response = popResponseChunk(client_state);
		std::cout << RED << time_now() << "	Sending response to client " << _fds[i].fd << RESET << std::endl;
		ssize_t sent = send(_fds[i].fd, response.c_str(), response.size(), 0);
		if (sent >= 0 && client_state.response.empty()) {
			_fds[i].events &= ~POLLOUT;
			client_state.send_start = std::chrono::high_resolution_clock::time_point{};
			client_state.waiting_response = false;
		}
		else if (sent < 0) // maybe check here for -1, check eval sheet
			dropClient(i, CLIENT_SEND_ERROR);
		if (client_state.kick_me) {
			dropClient(i, CLIENT_CLOSE_CONNECTION);
		}
	}
}

void	Cluster::dropClient(size_t& i, const std::string& msg) {
	std::cout << CYAN << time_now() << "	Client " << _fds[i].fd << msg << RESET;
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
		if (_client_buffers[_fds[i].fd].receive_start != std::chrono::high_resolution_clock::time_point{}) {
			auto elapsed = now - _client_buffers[_fds[i].fd].receive_start;
			auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
			if (elapsed_ms > TIME_OUT_REQUEST && _client_buffers[_fds[i].fd].buffer.size() > 0){
        send408Response(i);
				dropClient(i, CLIENT_TIMEOUT);
			}
		}

		if (_client_buffers[_fds[i].fd].send_start != std::chrono::high_resolution_clock::time_point{}) {
			auto elapsed = now - _client_buffers[_fds[i].fd].send_start;
			auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
			if (elapsed_ms > TIME_OUT_RESPONSE && _client_buffers[_fds[i].fd].response.size() > 0)
				dropClient(i, CLIENT_TIMEOUT);
		}
	}
}

// ILIA Added this function to send 408 Request Timeout response
void	Cluster::send408Response(size_t i) {
	// Create a minimal Request object for the 408 response
	Request req;
	req.setHttpVersion("HTTP/1.1");
	req.setMethod("GET");
	req.setPath("/");
	req.setHeaders("host", "localhost");

	// Create Response object
	Response res;

	// Use HttpResponseBuilder to create 408 response
	router::utils::HttpResponseBuilder::setErrorResponse(res, http::REQUEST_TIMEOUT_408, req);

	// Convert response to HTTP string format
	std::string responseStr = responseToString(res);

	// Send the 408 response immediately
	std::cout << RED << time_now() << "	Sending 408 Request Timeout to client " << _fds[i].fd << RESET << std::endl;
	ssize_t sent = send(_fds[i].fd, responseStr.c_str(), responseStr.size(), 0);
	if (sent < 0) {
		std::cout << "Failed to send 408 response" << std::endl;
	}
}
// end of ILIA Added this function to send 408 Request Timeout response

const std::set<int>&	Cluster::getServerFds() const {
	return _server_fds;
}

Cluster::~Cluster() {
	for (const pollfd& fd : _fds)
		close(fd.fd);
}
