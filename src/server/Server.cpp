#include "Server.hpp"

Server::Server(uint32_t address, int port) : _fd(-1), _address(address), _port(port) {}

void	Server::create() {
	_fd = socket(AF_INET, SOCK_STREAM, 0); // create TCP socket that can talk over IPv4.
	if (_fd < 0)
		throw std::runtime_error("Error: Socket creation");

	srand(time(0));
	// int port = 1024 + rand() % (10000 - 1024 + 1);

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;			// Use internet protocol IPv4
	addr.sin_port = htons(_port);		// set port to listen to
	addr.sin_addr.s_addr = _address;	// listen to all availabe interfaces

	if (bind(_fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {	// bind IP address to a existing socket
		close (_fd);
		throw std::runtime_error("Error: bind");
	}
	if (listen(_fd, SOMAXCONN) < 0)		//give socket ability to receive connections
		throw std::runtime_error("Error: listen");

	struct in_addr inaddr;
	inaddr.s_addr = _address;
	std::cout << "Server created: Host[" << inet_ntoa(inaddr) << "] Port:[" << _port << "]\n";
}

int		Server::getFd() {
	return _fd;
}
