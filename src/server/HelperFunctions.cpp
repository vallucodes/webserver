#include "HelperFunctions.hpp"

bool isSocketFd(int fd, const std::set<int>& server_fds) {
	std::set<int>::iterator it = server_fds.find(fd);
	if (it != server_fds.end())
		return true;
	return false;
}
