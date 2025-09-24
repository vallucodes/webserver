#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include <iostream>
#include <string>

#include <sys/socket.h>
#include <arpa/inet.h>

#include <signal.h>
#include <unistd.h>

// Utility function declarations
std::string readFileToString(const std::string& filename);

// Utility function implementation
#include <fstream>
#include <sstream>

inline std::string readFileToString(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
        throw std::runtime_error("Could not open file: " + filename);
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

#define TIME_OUT_POLL		100
#define TIME_OUT_REQUEST	5000
#define TIME_OUT_RESPONSE	5000
#define MAX_BUFFER_SIZE		10000000
#define MAX_BODY_SIZE		10000000
#define MAX_HEADER_SIZE		8192
#ifndef MAX_RESPONSE_SIZE
# define MAX_RESPONSE_SIZE	100000 // Default value for non-test builds
#endif

#endif // WEBSERV_HPP
