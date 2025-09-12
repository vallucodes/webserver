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

bool	isRequestBodyComplete(const std::string& buffer, size_t header_end, bool& data_validity) {
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

int	isChunkedBodyComplete(const std::string& buffer, size_t header_end) {
	size_t pos = buffer.find("\r\nTransfer-Encoding: chunked\r\n");
	if (pos != std::string::npos && pos < header_end) // search for body and only after we found the header
	{
		pos = buffer.find("0\r\n\r\n", header_end); // TODO test this
		if (pos == std::string::npos)
			return false;
		else
			return true;
	}
	return -1;
}

bool	requestComplete(const std::string& buffer, bool& data_validity) {
	// std::cout << "Buffer to be parsed currently: " << std::endl;
	// std::cout << buffer << std::endl;
	if (buffer.size() > MAX_BUFFER_SIZE) {
		data_validity = false;
		return false;
	}

	size_t header_end = findHeader(buffer);
	if (header_end == std::string::npos)
		return false;

	// std::cout << "header end detected: " << pos2 << std::endl;

	// if (buffer.size() - header_end > _max_client_body_size) { // check for body exceeding allowed length. Maybe parser will do it.
	// 	data_validity = false;
	// 	return false;
	// }

	int status = -1;
	status = isChunkedBodyComplete(buffer, header_end);
	if (status != -1)
		return status;

	status = isRequestBodyComplete(buffer, header_end, data_validity);
	return status;
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



