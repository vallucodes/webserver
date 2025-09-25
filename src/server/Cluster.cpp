#include "Cluster.hpp"

void	Cluster::config(const std::string& config_file) {
	Config config;

	signal(SIGINT, handleSigTerminate);
	signal(SIGTERM, handleSigTerminate);

	config.validate(config_file);
	_configs = config.parse(config_file);
	groupConfigs();

	_max_clients = getMaxClients();
	_router.setupRouter(_configs);
}

void	Cluster::groupConfigs() {
	if (_configs.size() == 0)
		throw std::runtime_error("Error: config file doesnt have any server");
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
			int port_group = group.default_config->getPort();

			if (IP_group == IP_conf && port_group == port_conf) {
				checkNameRepitition(group.configs, config);
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
	std::cout << CYAN << time_now() << "	Initializing servers...\n" << RESET;
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
		throw std::runtime_error("Error: accept");

	setSocketToNonBlockingMode(client_fd);

	std::cout << CYAN
			<< time_now()
			<< "	New client connected"
			<< ". Assigned socket: "
			<< client_fd << "\n"
			<< RESET;

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

void	Cluster::prepareResponse(ClientRequestState& client_state, const Server& conf, Request& req, int i) {
	Response res;
	_router.handleRequest(conf, req, res);
	client_state.response = responseToString(res);
	_fds[i].events |= POLLOUT;
	client_state.send_start = std::chrono::high_resolution_clock::now();
	client_state.waiting_response = true;
}

void	Cluster::processReceivedData(size_t& i, const char* buffer, int bytes) {
	ClientRequestState& client_state = _client_buffers[_fds[i].fd];
	client_state.buffer.append(buffer, bytes);
	client_state.receive_start = std::chrono::high_resolution_clock::now();

	while (requestComplete(client_state, _fds[i].fd, this)) {
		client_state.request = client_state.clean_buffer.substr(0, client_state.request_size);
		const Server& conf = findRelevantConfig(_fds[i].fd, client_state.clean_buffer);
		Parser parse;
		Request req = parse.parseRequest(client_state.request, client_state.kick_me, false);
		prepareResponse(client_state, conf, req, i);
		setTimer(client_state);
	}

	if (client_state.data_validity == false) {
		const Server& conf = findRelevantConfig(_fds[i].fd, client_state.clean_buffer);
		Parser parse;
		Request req = parse.parseRequest("400 Bad Request", client_state.kick_me, false);
		prepareResponse(client_state, conf, req, i);
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
		else if (sent < 0)
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
			if (elapsed_ms > TIME_OUT_REQUEST && _client_buffers[_fds[i].fd].buffer.size() > 0)
				dropClient(i, CLIENT_TIMEOUT);
		}

		if (_client_buffers[_fds[i].fd].send_start != std::chrono::high_resolution_clock::time_point{}) {
			auto elapsed = now - _client_buffers[_fds[i].fd].send_start;
			auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
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

	std::regex	re("Host:\\s*([^:\\s]+)");
	if (!std::regex_search(header, match, re))
		return *conf->default_config;

	std::string host = match[1];
	for (auto& conf : conf->configs) {
		if (conf.getName() == host)
			return conf;
	}
	return *conf->default_config;
}

const std::set<int>&	Cluster::getServerFds() const {
	return _server_fds;
}

Cluster::~Cluster() {
	for (const pollfd& fd : _fds)
		close(fd.fd);
}
