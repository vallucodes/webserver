#include "Server.hpp"
#include "HelperFunctions.hpp"

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

void	Server::setAddress(uint32_t address) {
	_address = address;
}

void	Server::setPort(int port) {
	_port = port;
}

void	Server::setMaxBodySize(int max_body_size) {
	_client_max_body_size = max_body_size;
}

void	Server::setName(const std::string& name) {
	_name = name;
}

uint32_t	Server::getAddress() const {
	return _address;
}

int	Server::getPort() const {
	return _port;
}

int	Server::getMaxBodySize() const {
	return _client_max_body_size;
}

const std::string&	Server::getName() const {
	return _name;
}
