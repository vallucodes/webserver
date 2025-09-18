#include "HelperFunctions.hpp"

bool	isServerSocket(int fd, const std::set<int>& server_fds) {
	std::set<int>::iterator it = server_fds.find(fd);
	if (it != server_fds.end())
		return true;
	return false;
}

void	setSocketToNonBlockingMode(int sock) {
	int flags = fcntl(sock, F_GETFL, 0);
	if (flags == -1) {
		close(sock);
		// maybe more cleaning needed here
		throw std::runtime_error("Error: fcntl get flags");
	}
	if (fcntl(sock, F_SETFL, flags | O_NONBLOCK) == -1) {
		close(sock);
		throw std::runtime_error("Error: fcntl set non-blocking");
	}
}

bool	findHostInHeader(const std::string& buffer, size_t header_end) {
	std::regex	re("Host:\\s+\\S+");
	std::smatch	match;
	std::string header = buffer.substr(0, header_end);
	if (std::regex_search(header, match, re))
	{
		// std::cout << "Host found in header" << match[0] << std::endl;
		return true;
	}
	return false;
}

size_t	findHeader(const std::string& buffer) {
	size_t pos = buffer.find("\r\n\r\n");
	if (pos == std::string::npos)
		return std::string::npos;
	return pos + 4;
}



// after parsing config, should be checked that max amount of clients there should be less than max.
// Or handled somehow, this is just temp fix. Its finding systems max fds and using that - 10.
uint64_t	getMaxClients() {
	struct rlimit rl;
	if (getrlimit(RLIMIT_NOFILE, &rl) != 0)
		throw std::runtime_error("Error: getrlimit");

	// Reserve some FDs for standard I/O, listening socket, etc.
	uint64_t reserved_fds = 10;
	uint64_t max_clients = rl.rlim_cur - reserved_fds;
	if (max_clients < 1)
		max_clients = 1;
	return max_clients;
}



