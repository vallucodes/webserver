#include "Request.hpp"

Request::Request() {
    _method = "GET";
    _path = "/";
    _body = "";
    _status = "200 OK";
}

Request::~Request() {}

std::string Request::method() const {
    return _method;
}

std::string Request::path() const {
    return _path;
}

std::string Request::body() const {
    return _body;
}

std::string Request::status() const {
    return _status;
}

void Request::setMethod(const std::string& method) {
    _method = method;
}

void Request::setPath(const std::string& path) {
    _path = path;
}

void Request::setBody(const std::string& body) {
    _body = body;
}

void Request::setStatus(const std::string& status) {
    _status = status;
}
