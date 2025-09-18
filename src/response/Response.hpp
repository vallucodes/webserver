/**
 * @file Response.hpp
 * @brief HTTP Response class for building and managing HTTP responses
 *
 * This header file defines the Response class which represents an HTTP response
 * message. It provides methods for setting status codes, headers, and response
 * body content, following the HTTP/1.1 specification.
 *
 * The Response class is a core component of the web server's request-response
 * cycle, working together with the Request class to handle HTTP communication.
 * It inherits from AMessage to provide common message handling functionality
 * while adding HTTP-specific response features.
 *
 * Key Features:
 * - HTTP status line management (status codes and reason phrases)
 * - HTTP header management with case-insensitive header names
 * - Response body handling for various content types
 * - Integration with the Router and Handler system
 * - Support for different response types (static files, dynamic content, errors)
 *
 * HTTP Response Structure:
 * ┌─────────────────────────────────────────────────────────────┐
 * │ HTTP/1.1 200 OK                                            │ Status Line
 * │ Content-Type: text/html                                    │ Headers
 * │ Content-Length: 123                                        │
 * │ Connection: close                                          │
 * │                                                             │
 * │ <html><body>Hello World!</body></html>                     │ Body
 * └─────────────────────────────────────────────────────────────┘
 *
 * Usage in Request-Response Flow:
 * 1. Router receives incoming request
 * 2. Router matches request to appropriate handler
 * 3. Handler populates Response object with appropriate content
 * 4. Server sends Response back to client
 * 5. Response object manages HTTP formatting and transmission
 */

#pragma once

#include <string>
#include <iostream>
#include <map>
#include "../message/AMessage.hpp"

/**
 * @class Response
 * @brief Represents an HTTP response message with status, headers, and body
 *
 * The Response class encapsulates all components of an HTTP response:
 * - Status line (HTTP version, status code, reason phrase)
 * - Headers (Content-Type, Content-Length, etc.)
 * - Body (HTML, JSON, binary data, etc.)
 *
 * It provides a clean interface for handlers to build responses and ensures
 * proper HTTP formatting when the response is sent to clients.
 *
 * Inheritance:
 * - Inherits from AMessage for common message handling functionality
 * - Adds HTTP-specific response features and status management
 * - Maintains compatibility with the broader message system
 *
 * Thread Safety:
 * - Response objects are typically used within a single request context
 * - No internal synchronization mechanisms (single-threaded usage assumed)
 * - Safe to use in multi-threaded environments if properly scoped
 */
class Response: public AMessage {
  public:
    /**
     * @brief Default constructor for Response
     *
     * Initializes an empty response with default values.
     * Status is empty, no headers are set, and body is empty.
     */
    Response();

    /**
     * @brief Destructor for Response
     *
     * Cleans up response resources. Since Response uses standard library
     * containers, no special cleanup is required.
     */
    ~Response();

    /**
     * @brief Get the message type identifier
     * @return String identifying this as a response message
     *
     * Overrides AMessage::getMessageType() to return "Response".
     * Used for message type identification in the broader message system.
     */
    virtual std::string getMessageType() const override;

    /**
     * @brief Get the HTTP status line
     * @return Status line string (e.g., "200 OK", "404 Not Found")
     *
     * Returns the current HTTP status line set for this response.
     * If no status has been set, returns an empty string.
     */
    std::string_view getStatus() const;

    /**
     * @brief Set the HTTP status line
     * @param status Status line string (e.g., "200 OK", "404 Not Found")
     *
     * Sets the HTTP status line for the response. This should include
     * both the status code and reason phrase as per HTTP specification.
     *
     * Common status lines:
     * - "200 OK" - Successful request
     * - "404 Not Found" - Resource not found
     * - "500 Internal Server Error" - Server error
     * - "302 Found" - Temporary redirect
     */
    void setStatus(const std::string& status);

    /**
     * @brief Set an HTTP header
     * @param key Header name (case-insensitive)
     * @param value Header value
     *
     * Sets an HTTP header for the response. Header names are stored
     * in a case-insensitive manner but preserve the original casing
     * for output formatting.
     *
     * Common headers set by handlers:
     * - Content-Type: MIME type of response body
     * - Content-Length: Size of response body in bytes
     * - Connection: Connection handling ("close", "keep-alive")
     * - Location: Redirect target URL
     * - Cache-Control: Caching directives
     */
    virtual void setHeaders(const std::string& key, const std::string& value) override;

    /**
     * @brief Print the complete response to console
     *
     * Outputs the full HTTP response including status line, all headers,
     * and body content to stdout. Used for debugging and logging purposes.
     *
     * Output format matches HTTP/1.1 specification for easy reading and
     * verification of response content.
     */
    void print() const;

  private:
    /**
     * @brief HTTP status line storage
     *
     * Stores the complete status line (e.g., "200 OK", "404 Not Found").
     * This includes both the numeric status code and the reason phrase.
     */
    std::string _status;

    /**
     * @brief HTTP headers storage
     *
     * Map storing HTTP headers as key-value pairs.
     * Keys are header names, values are header content.
     * Header names are handled in a case-insensitive manner for lookups
     * but preserve original casing for proper HTTP formatting.
     */
    std::map<std::string, std::string> _headers;
};
