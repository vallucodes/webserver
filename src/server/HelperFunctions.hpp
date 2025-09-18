#pragma once

#include <regex>
#include <fcntl.h>
#include <sys/resource.h>
#include "Cluster.hpp"

struct ClientRequestState;

bool		isServerSocket(int fd, const std::set<int>& server_fds);
void		setSocketToNonBlockingMode(int fd);
uint64_t	getMaxClients();
size_t		findHeader(const std::string& buffer);

bool	requestComplete(ClientRequestState& client_state);
void	decodeChunkedBody(std::string& buffer);
int		isChunkedBodyComplete(std::string& buffer, size_t header_end);
bool	isRequestBodyComplete(ClientRequestState& client_state, const std::string& buffer, size_t header_end);
