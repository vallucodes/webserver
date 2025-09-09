#include "Server.hpp"
#include "HelperFunctions.hpp"

Server::Server(uint32_t address, int port) : _address(address), _port(port) {}

int	Server::create() {
	int fd = socket(AF_INET, SOCK_STREAM, 0); // create TCP socket that can talk over IPv4.
	if (fd < 0)
		throw std::runtime_error("Error: Socket creation");

	setSocketToNonBlockingMode(fd);

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;			// Use internet protocol IPv4
	addr.sin_port = htons(_port);		// set port to listen to
	addr.sin_addr.s_addr = _address;	// listen to specified address

	if (bind(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {	// bind IP address to an existing socket
		close (fd);
		throw std::runtime_error("Error: bind");
	}
	if (listen(fd, SOMAXCONN) < 0)		//give socket ability to receive connections
		throw std::runtime_error("Error: listen");

	struct in_addr inaddr;
	inaddr.s_addr = _address;
	std::cout << "Server created: Host[" << inet_ntoa(inaddr) << "] Port:[" << _port << "]\n";
	return fd;
}
