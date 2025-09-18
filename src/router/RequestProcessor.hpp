/**
 * @file RequestProcessor.hpp
 * @brief Request processing logic separated from Router for better organization
 *
 * This header file defines the RequestProcessor class which encapsulates
 * the complex request processing logic. It provides a clean separation
 * of concerns by handling request validation, processing, and response
 * generation independently from the Router's routing logic.
 *
 * The RequestProcessor is responsible for:
 * - Request validation and sanitization
 * - Path normalization and security checks
 * - Handler execution with proper error handling
 * - Fallback mechanisms (static file serving, error responses)
 * - Debug logging and monitoring
 *
 * Design Benefits:
 * - Single Responsibility: Focused on request processing only
 * - Testability: Can be unit tested independently
 * - Maintainability: Changes to processing logic don't affect routing
 * - Reusability: Can be used by different routing components
 * - Extensibility: Easy to add new processing features
 */

#pragma once

#include <string>
#include <functional>
#include "../request/Request.hpp"
#include "../response/Response.hpp"
#include "../server/Server.hpp"
#include "HttpConstants.hpp"

/**
 * @brief Forward declaration for Location struct
 */
struct Location;

/**
 * @brief Handler function type alias
 */
using Handler = std::function<void(const Request&, Response&, const Location*)>;

/**
 * @class RequestProcessor
 * @brief Processes HTTP requests and coordinates with handlers
 *
 * The RequestProcessor class handles the complete request processing pipeline
 * from receiving a raw request to generating an appropriate response. It works
 * closely with the Router to provide a clean separation between routing logic
 * and request processing logic.
 *
 * Key Responsibilities:
 * - Request validation and preprocessing
 * - Path normalization and security validation
 * - Handler lookup coordination
 * - Response generation and error handling
 * - Debug logging and performance monitoring
 *
 * Usage Pattern:
 * 1. Router receives request and determines handler
 * 2. Router delegates to RequestProcessor for execution
 * 3. RequestProcessor handles all processing logic
 * 4. RequestProcessor returns processed response to Router
 */
class RequestProcessor {
  public:
    /**
     * @brief Default constructor
     */
    RequestProcessor();

    /**
     * @brief Destructor
     */
    ~RequestProcessor();

    /**
     * @brief Process a complete HTTP request with routing information
     * @param server The server configuration for this request
     * @param req The incoming HTTP request
     * @param handler The matched handler function (can be nullptr)
     * @param res The response object to populate
     *
     * This is the main entry point for request processing. It orchestrates
     * the entire request processing pipeline including validation, handler
     * execution, and fallback mechanisms.
     */
    void processRequest(const Server& server, const Request& req,
                       const Handler* handler, Response& res) const;

    /**
     * @brief Validate and preprocess a request path
     * @param path The raw request path to validate
     * @return true if path is valid, false otherwise
     *
     * Performs security validation on request paths including:
     * - Path traversal attack prevention
     * - Null byte injection detection
     * - Invalid character filtering
     * - Length limits enforcement
     */
    bool validatePath(const std::string& path) const;

    /**
     * @brief Normalize a request path for processing
     * @param path The path to normalize (modified in-place)
     *
     * Normalizes paths by:
     * - Removing query parameters (?key=value)
     * - Removing URL fragments (#anchor)
     * - Converting relative paths to absolute
     * - Cleaning up duplicate slashes
     */
    void normalizePath(std::string& path) const;

  private:
    /**
     * @brief Execute a handler function with error handling
     * @param handler The handler to execute
     * @param server Server configuration
     * @param req The request object
     * @param res The response object
     * @param path The normalized request path
     * @return true if execution successful, false if error occurred
     */
    bool executeHandlerSafely(const Handler* handler, const Server& server,
                             const Request& req, Response& res,
                             const std::string& path) const;

    /**
     * @brief Attempt to serve a request as a static file
     * @param req The request object
     * @param res The response object
     * @param method The HTTP method
     * @return true if static file served successfully, false otherwise
     */
    bool tryServeAsStaticFile(const Request& req, Response& res,
                             const std::string& method) const;

    /**
     * @brief Generate appropriate error response
     * @param res Response object to populate
     * @param status HTTP status code
     * @param message Optional error message for logging
     */
    void generateErrorResponse(Response& res, int status,
                              const std::string& message = "") const;

    /**
     * @brief Log request processing information for debugging
     * @param req The request being processed
     * @param stage The processing stage (validation, routing, execution, etc.)
     * @param details Additional details about the processing stage
     */
    void logRequestProcessing(const Request& req, const std::string& stage,
                             const std::string& details = "") const;

    // Private member variables could be added here for configuration,
    // caching, or performance monitoring if needed in the future
};
