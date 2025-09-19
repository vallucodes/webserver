/**
 * @file Response.cpp
 * @brief Implementation of HTTP Response class
 */

#include "Response.hpp"

/**
 * @brief Default constructor
 */
Response::Response() {
  _status = "";
}

/**
 * @brief Destructor
 */
Response::~Response() {}

/**
 * @brief Get message type identifier
 * @return String "Response"
 */
std::string Response::getMessageType() const {
    return "Response";
}

/**
 * @brief Set HTTP status line
 * @param status Status line string
 */
void Response::setStatus(const std::string& status) {
  _status = status;
}

/**
 * @brief Get HTTP status line
 * @return Status line string
 */
std::string_view Response::getStatus() const {
  return _status;
}

/**
 * @brief Set HTTP header
 * @param key Header name
 * @param value Header value
 */
void Response::setHeaders(const std::string& key, const std::string& value) {
    _headers[key] = value;
}

/**
 * @brief Print response to console for debugging
 */
void Response::print() const {
    std::cout << "[DEBUG]:=== HTTP Response ===\n";
    std::cout << "Status: " << _status << "\n";

    std::cout << "Headers:\n";
    for (const auto& pair : _headers) {
        std::cout << pair.first << ": " << pair.second << "\n";
    }

    // Body output is commented out to prevent large content from cluttering console
    // Uncomment the following line when debugging specific response body content
    std::cout << "Body: Uncommented for debugging in Response.cpp" << std::endl;
    // std::cout << "Body: " << getBody() << std::endl; // getBody() inherited from AMessage
    std::cout << "[DEBUG]: ===  End of HTTP Response ===\n" << std::endl;
}
