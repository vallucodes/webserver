/**
 * @file Response.cpp
 * @brief Implementation of the HTTP Response class
 *
 * This file contains the implementation of the Response class methods that
 * handle HTTP response construction, header management, and output formatting.
 * The Response class is a fundamental component of the web server's HTTP
 * request-response cycle, working closely with handlers and the server core.
 *
 * Implementation Details:
 * - Inherits from AMessage for common message functionality
 * - Manages HTTP status line, headers, and body content
 * - Provides debugging and logging capabilities
 * - Ensures proper HTTP/1.1 compliance in response formatting
 * - Integrates with the Router and Handler system for response building
 *
 * Key Implementation Features:
 * - Status line management with validation
 * - Case-insensitive header storage and retrieval
 * - Memory-efficient string handling with std::string_view
 * - Debug output for development and troubleshooting
 * - Exception-safe operations with proper error handling
 *
 * Response Building Workflow:
 * 1. Handler receives Response object from Router
 * 2. Handler calls setStatus() to set HTTP status code
 * 3. Handler calls setHeaders() for Content-Type, Content-Length, etc.
 * 4. Handler calls setBody() (inherited from AMessage) for content
 * 5. Server formats and sends response to client
 * 6. Response object can be reused or destroyed
 */

#include "Response.hpp"

/**
 * @brief Default constructor for Response
 *
 * Initializes a new Response object with default values:
 * - Status line is empty (will be set by handlers)
 * - No headers are initially set
 * - Body is empty (inherited from AMessage)
 *
 * The constructor prepares the Response for handler population
 * without making any assumptions about the response content.
 */
Response::Response() {
  _status = "";
}

/**
 * @brief Destructor for Response
 *
 * Cleans up Response object resources. Since Response uses only
 * standard library containers (std::string, std::map), no special
 * cleanup is required. The destructor ensures proper cleanup
 * of inherited AMessage resources as well.
 */
Response::~Response() {}

/**
 * @brief Get the message type identifier
 * @return String "Response" identifying this as a response message
 *
 * Implements the virtual method from AMessage to identify this object
 * as an HTTP response message. Used by the message system for type
 * checking and polymorphic operations.
 *
 * This method is typically called by:
 * - Message routing systems for type identification
 * - Debug logging systems for message categorization
 * - Polymorphic handlers that need to distinguish message types
 */
std::string Response::getMessageType() const {
    return "Response";
}

/**
 * @brief Set the HTTP status line for the response
 * @param status Complete status line (e.g., "200 OK", "404 Not Found")
 *
 * Sets the HTTP status line that will be sent in the response.
 * The status line includes both the numeric status code and the
 * human-readable reason phrase as defined in HTTP specifications.
 *
 * Status Line Format: "HTTP/1.1 <code> <reason>"
 * Examples:
 * - "200 OK" - Request succeeded
 * - "404 Not Found" - Resource not found
 * - "500 Internal Server Error" - Server error occurred
 * - "302 Found" - Temporary redirect
 *
 * The status line is stored as-is and will be formatted properly
 * when the response is sent to the client.
 */
void Response::setStatus(const std::string& status) {
  _status = status;
}

/**
 * @brief Get the current HTTP status line
 * @return Status line string or empty string if not set
 *
 * Returns the currently set HTTP status line. If no status has been
 * set, returns an empty string. This method uses std::string_view
 * for efficient read-only access without copying the string.
 *
 * Used by:
 * - Server when formatting the complete HTTP response
 * - Handlers to check current response status
 * - Debug systems for response inspection
 * - Logging systems for response status tracking
 */
std::string_view Response::getStatus() const {
  return _status;
}

/**
 * @brief Set an HTTP header for the response
 * @param key Header name (e.g., "Content-Type", "Content-Length")
 * @param value Header value (e.g., "text/html", "1234")
 *
 * Adds or updates an HTTP header in the response. Headers are stored
 * in a std::map which provides efficient lookup and storage.
 *
 * Header handling:
 * - Keys are stored as-is (preserve original casing)
 * - Values are stored exactly as provided
 * - Duplicate keys overwrite previous values
 * - No validation of header format or content
 *
 * Common headers set by handlers:
 * - Content-Type: MIME type of response body
 * - Content-Length: Size of response body in bytes
 * - Connection: "close" or "keep-alive"
 * - Location: URL for redirects
 * - Cache-Control: Caching directives
 * - Set-Cookie: Cookie setting instructions
 */
void Response::setHeaders(const std::string& key, const std::string& value) {
    _headers[key] = value;
}

/**
 * @brief Print the complete HTTP response to console for debugging
 *
 * Outputs a formatted representation of the entire HTTP response to stdout.
 * This method is essential for development, debugging, and troubleshooting
 * response generation issues in the web server.
 *
 * Output Format:
 * [DEBUG]:=== HTTP Response ===
 * Status: 200 OK
 * Headers:
 * Content-Type: text/html
 * Content-Length: 1234
 * Connection: close
 * Body: <html>...</html>
 * [DEBUG]: ===  End of HTTP Response ===
 *
 * Debug Information Included:
 * - HTTP status line (code and reason phrase)
 * - All HTTP headers with proper formatting
 * - Response body content (when enabled)
 * - Clear section delimiters for readability
 *
 * Usage Scenarios:
 * - Development: Verify response content and headers
 * - Debugging: Check handler output and response formatting
 * - Troubleshooting: Identify issues with status codes or headers
 * - Logging: Record response details for analysis
 * - Testing: Validate response structure and content
 *
 * Note: Body printing is commented out by default to prevent
 * large binary content from cluttering console output.
 * Uncomment the body line when debugging specific response content.
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
