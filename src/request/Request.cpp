#include "Request.hpp"

Request::Request(void) : AMessage(), _isError(false) {}

Request::~Request(void) {}

void Request::setStatus(const std::string& status) {
  _status = status;
}

bool Request::getError() const {
    return _isError;
}

void Request::setError(bool val) {
    _isError = val;
}

std::string_view Request::getStatus() const {
  return _status;
}

std::string Request::getMessageType() const {
    return "Request";
}

void Request::print() const {
    std::cout << "=== HTTP Request ===\n";
    std::cout << "Method      : " << _method << "\n";
    std::cout << "Path        : " << _path << "\n";
    std::cout << "HTTP Version: " << _httpVersion << "\n";

    std::cout << "Headers     :\n";
    for (const auto& pair : _headers) {
        const std::string& key = pair.first;
        const std::vector<std::string>& values = pair.second;

        for (const auto& value : values) {
            std::cout << key << ": " << value << "\n";
        }
    }

    // std::cout << "Body:\n" << _body << "\n";
    std::cout << "Body: Uncommented for debugging in Request.cpp" << std::endl;
    std::cout << "===  End of HTTP Request ===\n" << std::endl;

}
