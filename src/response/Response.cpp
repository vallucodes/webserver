#include "Response.hpp"

// setup default values
Response::Response() {
  _status = "200 OK"; // ? what is the default status ?
  _headers = "";
  _body = "";
}

Response::~Response() {}

void Response::setStatus(std::string status) {
  _status = status;
}

void Response::setHeaders(std::string headers) {
  _headers = headers;
}

// should be template
void Response::setBody(std::string body) {
  _body = body;
}
