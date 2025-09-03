#pragma once

#include "Cluster.hpp"

bool isSocketFd(int fd, const std::set<int>& server_fds);
