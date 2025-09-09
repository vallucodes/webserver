#include "Request.hpp"

Request::Request(void) {}

Request::~Request(void) {}

std::string Request::getMessageType() const {
    return "Request";
}

void Request::print() const {
    std::cout << "=== HTTP Request ===\n";
    std::cout << "Method: " << _method << "\n";
    std::cout << "Path: " << _path << "\n";
    std::cout << "HTTP Version: " << _httpVersion << "\n";

    std::cout << "Headers:\n";
    for (const auto& [key, value] : _headers) {
        std::cout << "" << key << ": " << value << "\n";
    }

    std::cout << "Body:\n";
    std::cout << _body << "\n";
    std::cout << "==================\n";
}