#include "AMessage.hpp"

AMessage::AMessage(void) {
    _method = "";
    _path = "";
    _body = "";
}

AMessage::~AMessage(void) {}

std::string_view AMessage::getMethod() const {
    return _method;
}

std::string_view AMessage::getPath() const {
    return _path;
}

std::string_view AMessage::getBody() const {
    return _body;
}

void AMessage::setMethod(const std::string& method) {
    _method = method;
}

void AMessage::setPath(const std::string& path) {
    _path = path;
}

void AMessage::setBody(const std::string& body) {
    _body = body;
}
