#include "HelperFunctions.hpp"

bool	isServerSocket(int fd, const std::set<int>& server_fds) {
	std::set<int>::iterator it = server_fds.find(fd);
	if (it != server_fds.end())
		return true;
	return false;
}

bool	requestComplete(const std::string& buffer, bool& status) {
	// std::cout << "Buffer to be parsed currently: " << std::endl;
	// std::cout << buffer << std::endl;
	if (buffer.size() > 2097152) {
		status = false;
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
	if (std::regex_search(buffer, match, std::regex(R"(Content-Length:\s*(\d+)\r?\n)"))) {
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
			status = false;
			return false;
		}
	}
	else if (body_curr_len > 0) {
		// std::cout << "Body received after no length given" << std::endl;
		status = false;
		return false;
	}
	else {
		return false;
	}
}
