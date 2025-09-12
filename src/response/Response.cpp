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

// Header management
/* void Response::setHeader(const std::string& key, const std::string& value) {
    _headers[key] = value;
} */

/* std::string Response::getHeader(const std::string& key) const {
    auto it = _headers.find(key);
    if (it != _headers.end()) {
        return it->second;
    }
    return "";
}

std::string Response::getAllHeaders() const {
    std::string headers;
    for (const auto& header : _headers) {
        headers += header.first + ": " + header.second + "\r\n";
    }
    return headers;
}
 */