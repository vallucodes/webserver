#pragma once

#include <regex>
#include <fcntl.h>
#include <sys/resource.h>
#include "Cluster.hpp"

struct ClientRequestState;

bool		isServerSocket(int fd, const std::set<int>& server_fds);
void		setSocketToNonBlockingMode(int fd);
void		checkNameRepitition(const std::vector<Server> configs, const Server config);
uint64_t	getMaxClients();
size_t		findHeader(const std::string& buffer);

bool	requestComplete(ClientRequestState& client_state);
bool	decodeChunkedBody(ClientRequestState& client_state);
int		isChunkedBodyComplete(ClientRequestState& client_state, size_t header_end);
bool	isRequestBodyComplete(ClientRequestState& client_state, size_t header_end);

std::string		popResponseChunk(ClientRequestState& client_state);
void			buildRequest(ClientRequestState& client_state);
