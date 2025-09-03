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
		if (poll(_fds.data(), _fds.size(), 0) < 0)
			throw std::runtime_error("Error: poll");

		for (size_t i = 0; i < _fds.size(); ++i) {
			if (_fds[i].revents & POLLIN) {								// check if there is data to read related to fd
				if (isSocketFd(_fds[i].fd, getServerFds())) {			// check if fd is server or client
					// new client
					sockaddr_in client_addr{};
					socklen_t addrlen = sizeof(client_addr);
					int client_fd = accept(_fds[i].fd, (sockaddr*)&client_addr, &addrlen); // 2nd argument: collect clients IP and port. 3rd argument tells size of the buffer of second argument
					if (client_fd >= 0) {
						std::cout << "New client connected: "
								<< inet_ntoa(client_addr.sin_addr) << ":"
								<< ntohs(client_addr.sin_port) << ". Assigned fd: "
								<< client_fd << "\n";
						_fds.push_back({client_fd, POLLIN, 0});
					}
					else
						throw std::runtime_error("Error: accept");
				}
				else {
					// Existing client sending data
					char buffer[1024];
					int bytes = recv(_fds[i].fd, buffer, sizeof(buffer), 0);
					if (bytes <= 0) {
						std::cout << "Client disconnected\n";
						close (_fds[i].fd);
						_fds.erase(_fds.begin() + i);
						--i;
					}
					else
					{
						_client_buffers[_fds[i].fd].append(buffer, bytes);
						if (requestComplete(_client_buffers[_fds[i].fd]))
						{
							send(_fds[i].fd, buffer, bytes, 0);			// call here the parser
							_client_buffers.erase(_fds[i].fd);
						}
					}
				}
			}
		}
	}
}

const std::vector<std::pair<uint32_t, int>>& Cluster::getAddresses() const {  //move this to Config.hpp
	return _addresses;
}

const std::set<int>& Cluster::getServerFds() const {
	return _server_fds;
}
