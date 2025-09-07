#include "Request.hpp"

Request::Request(void) {}

Request::~Request(void) {}

std::string Request::getMessageType() const {
    return "Request";
}
