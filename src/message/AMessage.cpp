#include "AMessage.hpp"

AMessage::AMessage()
    : _method(), _path(), _body(), _httpVersion(), _headers()
{}

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
std::string_view AMessage::getHeaders( const std::string& key) const{
    auto it = _headers.find(key);
    if (it != _headers.end())
        return ( it->second);
    return "";
}

std::string_view AMessage::getHttpVersion() const {
    return _httpVersion;
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

void AMessage::setHeaders(const std::string& key, const std::string& value) {
    _headers[key] = value;
}

void AMessage::setHttpVersion(const std::string& httpVersion){
    _httpVersion = httpVersion;
}