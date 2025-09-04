#include "Response.hpp"

Response::Response() {
  _status = "200 OK"; // Default status
  _body = "";
}

Response::~Response() {}

void Response::setStatus(const std::string& status) {
  _status = status;
}

void Response::setBody(const std::string& body) {
  _body = body;
}

const std::string& Response::getStatus() const {
  return _status;
}

const std::string& Response::getBody() const {
  return _body;
}
