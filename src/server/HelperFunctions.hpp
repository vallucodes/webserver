#pragma once

#include <regex>
#include <fcntl.h>
#include <sys/resource.h>
#include <iomanip>

#include "Cluster.hpp"
#include "../response/Response.hpp"

struct	ClientRequestState;
class	Cluster;

std::string	time_now();

void		handleSigTerminate(int sig);

bool		isServerSocket(int fd, const std::set<int>& server_fds);
void		setSocketToNonBlockingMode(int fd);
uint64_t	getMaxClients();
size_t		findHeader(const std::string& buffer);
std::string	responseToString(const Response& res);
std::string	headersToString(const std::unordered_map<std::string, std::vector<std::string>>& headers);
void		setTimer(ClientRequestState& client_state);

bool		requestComplete(ClientRequestState& client_state);
bool		decodeChunkedBody(ClientRequestState& client_state);
int			isChunkedBodyComplete(ClientRequestState& client_state, size_t header_end);
bool		isRequestBodyComplete(ClientRequestState& client_state, size_t header_end);

std::string	popResponseChunk(ClientRequestState& client_state);
