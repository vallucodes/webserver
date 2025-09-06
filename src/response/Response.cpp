#include "Response.hpp"

Response::Response() {
  _status = "200 OK"; // Default status
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
