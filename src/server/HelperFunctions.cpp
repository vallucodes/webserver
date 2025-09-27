#include "HelperFunctions.hpp"

volatile sig_atomic_t	signal_to_terminate = false;

void	handleSigTerminate(int sig) {
	signal_to_terminate = sig;
	std::cout << RED << "\n" << time_now() << "	Server closed\n" << RESET;
}

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

std::string	time_now() {
	auto now = std::chrono::system_clock::now();
	std::time_t t = std::chrono::system_clock::to_time_t(now);
	std::tm tm = *std::localtime(&t);

	std::ostringstream oss;
	oss << std::put_time(&tm, "[%Y-%m-%d %H:%M:%S]");
	return oss.str();
}

size_t	findHeader(const std::string& buffer) {
	size_t pos = buffer.find("\r\n\r\n");
	if (pos == std::string::npos)
		return std::string::npos;
	return pos + 4;
}

void	checkNameRepitition(const std::vector<Server> configs, const Server config) {
	for (auto& conf : configs) {
		if (conf.getName() == config.getName())
			throw std::runtime_error("Error: Config: Duplicate server_name, " \
					"each virtual host (IP+port+server_name) must be unique");
	}
}

uint64_t	getMaxClients() {
	struct rlimit rl;
	if (getrlimit(RLIMIT_NOFILE, &rl) != 0)
		throw std::runtime_error("Error: getrlimit()");

	uint64_t reserved_fds = 100;
	int max_clients = rl.rlim_cur - reserved_fds;
	if (max_clients < 2)
		throw std::runtime_error("Error: Not enough fd's available to create a server");
	return max_clients;
}

std::string	headersToString(const std::unordered_map<std::string, std::vector<std::string>>& headers) {
	std::string result;
	for (const auto& pair : headers) {
		const std::string& key = pair.first;
		const std::vector<std::string>& values = pair.second;
		for (const auto& value : values)
			result += key + ": " + value + "\r\n";
	}
	return result;
}

std::string	responseToString(const Response& res) {
	std::string responseStr = "HTTP/1.1 " + std::string(res.getStatus()) + "\r\n";
	responseStr += headersToString(res.getAllHeaders());
		responseStr += "\r\n";
	responseStr += std::string(res.getBody());
	return responseStr;
}

void	setTimer(ClientRequestState& client_state) {
	if (client_state.buffer.empty())
		client_state.receive_start = {};
	else
		client_state.receive_start = std::chrono::high_resolution_clock::now();
}

void	setMaxBodySize(ClientRequestState& client_state, Cluster* cluster, int fd) {
	const Server& conf = cluster->Cluster::findRelevantConfig(fd, client_state.buffer);
	client_state.max_body_size = conf.getMaxBodySize();
}

bool	requestComplete(ClientRequestState& client_state, int fd, Cluster* cluster) {
	if (client_state.buffer.size() > MAX_BUFFER_SIZE) {
		client_state.data_validity = false;
		return false;
	}

	size_t header_end = findHeader(client_state.buffer);
	if (header_end == std::string::npos)
		return false;
	else if (header_end > MAX_HEADER_SIZE) {
		client_state.data_validity = false;
		return false;
	}

	setMaxBodySize(client_state, cluster, fd);

	int status = -1;
	status = isChunkedBodyComplete(client_state, header_end);
	if (status != -1)
		return status;
	status = isRequestBodyComplete(client_state, header_end);
	return status;
}

bool	decodeChunkedBody(ClientRequestState& client_state) {
	std::string result;
	size_t pos = 0;
	size_t header_end = findHeader(client_state.buffer);
	std::string headers = client_state.buffer.substr(0, header_end);
	std::string body = client_state.buffer.substr(header_end);
	bool endReq = false;
	while (pos < body.size()) {
		size_t lineEnd = body.find("\r\n", pos);
		if (lineEnd == std::string::npos) {
			client_state.data_validity = false;
			break ;
		}

		std::string sizeLine = body.substr(pos, lineEnd - pos);
		size_t chunkSize = 0;
		std::istringstream(sizeLine) >> std::hex >> chunkSize;

		if (chunkSize == 0) {
			size_t trailersEnd = body.find("\r\n\r\n", pos);
			if (trailersEnd == std::string::npos) {
				client_state.data_validity = false;
				endReq = false;
				break;
			}
			pos = trailersEnd + 4;
			client_state.buffer = body.substr(pos);
			endReq = true;
			break;
		}

		pos = lineEnd + 2;
		if (pos + chunkSize + 2 > body.size()) {
			client_state.data_validity = false;
			endReq = false;
			break;
		}

		result.append(body.substr(pos, chunkSize));
		pos += chunkSize + 2;
	}

	client_state.clean_buffer = headers + result;
	size_t body_size = client_state.clean_buffer.substr(header_end).size();
	if (body_size > client_state.max_body_size) {
		client_state.data_validity = false;
		endReq = false;
		return endReq;
	}
	client_state.request_size = client_state.clean_buffer.size();
	return (endReq);
}

int	isChunkedBodyComplete(ClientRequestState& client_state, size_t header_end) {
	size_t pos = client_state.buffer.find("\r\nTransfer-Encoding: chunked\r\n");
	if (pos != std::string::npos && pos < header_end)
		return(decodeChunkedBody(client_state));
	return -1;
}

bool	isRequestBodyComplete(ClientRequestState& client_state, size_t header_end) {
	client_state.clean_buffer = client_state.buffer;
	size_t remainder = client_state.clean_buffer.size() - header_end;
	std::smatch match;
	if (std::regex_search(client_state.clean_buffer, match, std::regex(R"(Content-Length:\s*(\d+)\s*\r?\n)"))) {
		size_t body_expected_len = std::stoul(match[1].str());
		if (body_expected_len > client_state.max_body_size) {
			client_state.data_validity = false;
			return false;
		}
		if (remainder >= body_expected_len) {
			client_state.request_size = header_end + body_expected_len;
			client_state.buffer = client_state.clean_buffer.substr(client_state.request_size);
			return true;
		}
		else {
			return false;
		}
	}
	else {
		client_state.request_size = header_end;
		client_state.buffer = client_state.clean_buffer.substr(client_state.request_size);
		return true;
	}
}

std::string	popResponseChunk(ClientRequestState& client_state) {
	std::string response;
	if (client_state.response.size() > MAX_RESPONSE_SIZE) {
		response = client_state.response.substr(0, MAX_RESPONSE_SIZE);
		client_state.response = client_state.response.substr(MAX_RESPONSE_SIZE);
	}
	else {
		response = client_state.response;
		client_state.response.erase();
	}
	return response;
}
