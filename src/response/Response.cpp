#include "Response.hpp"

Response::Response() {
  _status = "";
}

Response::~Response() {}

std::string Response::getMessageType() const {
    return "Response";
}

void Response::setStatus(const std::string& status) {
  _status = status;
}

std::string_view Response::getStatus() const {
  return _status;
}

void Response::setHeaders(const std::string& key, const std::string& value) {
    _headers[key] = value;
}

void Response::print() const {
    std::cout << "=== HTTP Response ===\n";
    std::cout << "Status: " << _status << "\n";

    std::cout << "Headers:\n";
    for (const auto& pair : _headers) {
        std::cout << pair.first << ": " << pair.second << "\n";
    }

    std::cout << "Body: Uncommented for debugging in Response.cpp\n" << std::endl;
    std::cout << "Body:" << _body << std::endl;
    std::cout << "===================\n" << std::endl;
}
