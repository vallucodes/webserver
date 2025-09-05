#pragma once
#include <regex>
#include <fcntl.h>
#include <sys/resource.h>
#include "Cluster.hpp"

bool		isServerSocket(int fd, const std::set<int>& server_fds);
bool		requestComplete(const std::string& buffer, bool& status);
void		setSocketToNonBlockingMode(int fd);
uint64_t	getMaxClients();
