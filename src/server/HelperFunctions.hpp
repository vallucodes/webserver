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

bool	requestComplete(ClientRequestState& client_state, std::string& buffer);
bool	decodeChunkedBody(std::string& buffer, ClientRequestState& client_state);
int		isChunkedBodyComplete(ClientRequestState& client_state, std::string& buffer, size_t header_end);
bool	isRequestBodyComplete(ClientRequestState& client_state, const std::string& buffer, size_t header_end);

std::string		popResponseChunk(ClientRequestState& client_state);
void			buildRequest(ClientRequestState& client_state, std::string& buffer);
