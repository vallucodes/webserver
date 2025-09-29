#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include <iostream>
#include <string>

#include <sys/socket.h>
#include <arpa/inet.h>

#include <signal.h>
#include <unistd.h>

#define DEFAULT_CONF		"configs/default.conf"

#define TIME_OUT_POLL		100
#define TIME_OUT_REQUEST	5000
#define TIME_OUT_RESPONSE	10000
#define MAX_BUFFER_SIZE		10000000
#define MAX_BODY_SIZE		10000000
#define MAX_HEADER_SIZE		8192
#ifndef MAX_RESPONSE_SIZE
# define MAX_RESPONSE_SIZE	100000 // Default value for non-test builds
#endif

#endif // WEBSERV_HPP
