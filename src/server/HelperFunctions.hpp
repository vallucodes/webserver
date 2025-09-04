#pragma once
#include <regex>
#include "Cluster.hpp"

bool	isSocketFd(int fd, const std::set<int>& server_fds);
bool	requestComplete(const std::string& buffer, bool& status);
