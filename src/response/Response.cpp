/**
 * @file Response.cpp
 * @brief Implementation of HTTP Response class
 */

#include "Response.hpp"

/** Default constructor */
Response::Response() {
  _status = "";
}

/** Destructor */
Response::~Response() {}

/** Get message type identifier */
std::string Response::getMessageType() const {
    return "Response";
}

/** Set HTTP status line */
void Response::setStatus(const std::string& status) {
  _status = status;
}

/** Get HTTP status line */
std::string_view Response::getStatus() const {
  return _status;
}

/** Set HTTP header */
void Response::setHeaders(const std::string& key, const std::string& value) {
    _headers[key] = value;
}

/** Print response to console for debugging */
void Response::print() const {
    std::cout << "=== HTTP Response ===\n";
    std::cout << "Status     : " << _status << "\n";

    std::cout << "Headers    :\n";
    for (const auto& pair : _headers) {
        std::cout << pair.first << ": " << pair.second << "\n";
    }

    // Uncomment the following line when debugging specific response body content
    std::cout << "Body: Uncommented for debugging in Response.cpp" << std::endl;
    // std::cout << "Body: " << getBody() << std::endl; // getBody() inherited from AMessage
    std::cout << "===  End of HTTP Response ===\n" << std::endl;
}
