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

bool	requestComplete(const std::string& buffer, bool& data_validity) {
	// std::cout << "Buffer to be parsed currently: " << std::endl;
	// std::cout << buffer << std::endl;
	if (buffer.size() > MAX_BUFFER_SIZE) {
		data_validity = false;
		return false;
	}

	size_t header_end = 0;
	size_t pos2 = buffer.find("\r\n\r\n");
	if (pos2 == std::string::npos)
	{
		// std::cout << "Header end not detected" << std::endl;
		return false;
	}
	header_end = pos2 + 4;

	// std::cout << "header end detected: " << std::endl;
	// std::cout << pos2 << std::endl;

	// if (buffer.size() - header_end > _max_client_body_size) {
	// 	data_validity = false;
	// 	return false;
	// }

	size_t pos = buffer.find("\r\nTransfer-Encoding: chunked\r\n");
	if (pos != std::string::npos && pos < header_end) // search for body and only after we found the header
	{
		pos = buffer.find("0\r\n\r\n");
		if (pos == std::string::npos || pos < header_end)
			return false;
		else
			return true;
	}

	size_t body_curr_len = buffer.size() - header_end;
	std::smatch match;
	if (std::regex_search(buffer, match, std::regex(R"(Content-Length:\s*(\d+)\r?\n)"))) { // might be issue that this is in body
		size_t body_expected_len = std::stoul(match[1].str());
		if (body_curr_len == body_expected_len)
		{
			// std::cout << "body received as correct length" << std::endl;
			return true;
		}
		else if (body_curr_len < body_expected_len)
			return false;
		else {
			// std::cout << "body received as too big" << std::endl;
			data_validity = false;
			return false;
		}
	}
	else if (body_curr_len > 0) {
		// std::cout << "Body received after no length given" << std::endl;
		data_validity = false;
		return false;
	}
	else {
		// std::cout << "only header received" << std::endl;
		return true;
	}
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



