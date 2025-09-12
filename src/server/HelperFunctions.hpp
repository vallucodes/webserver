#pragma once
#include <regex>
#include <fcntl.h>
#include <sys/resource.h>
#include "Cluster.hpp"

bool		isServerSocket(int fd, const std::set<int>& server_fds);
bool		requestComplete(const std::string& buffer, bool& data_validity);
void		setSocketToNonBlockingMode(int fd);
uint64_t	getMaxClients();
size_t		findHeader(const std::string& buffer);
