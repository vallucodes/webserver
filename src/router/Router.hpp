/**
 * @file Router.hpp
 * @brief HTTP Router component for handling route mappings and request dispatching
 *
 * This header file defines the Router class which is responsible for:
 * - Managing HTTP route mappings organized by server name, HTTP method, and path
 * - Routing incoming HTTP requests to appropriate handler functions
 * - Providing utility functions for error response generation
 * - Supporting multiple server configurations with different route sets
 *
 * The router uses a hierarchical map structure for efficient route lookup:
 * server_name -> path -> method -> handler function
 */

#pragma once

#include <map>
#include <string>
#include <string_view>
#include <functional>

#include "../request/Request.hpp"
#include "../response/Response.hpp"
#include "../server/Server.hpp"

/**
 * @namespace http
 * @brief Contains HTTP status codes and status messages as constants
 *
 * This namespace provides standardized HTTP status codes and their corresponding
 * status messages used throughout the web server for consistent response formatting.
 */
namespace http {

    // HTTP Status Messages
    const std::string STATUS_OK_200 = "200 OK";
    const std::string STATUS_MOVED_PERMANENTLY_301 = "301 Moved Permanently";
    const std::string STATUS_FOUND_302 = "302 Found";
    const std::string STATUS_NOT_FOUND_404 = "404 Not Found";
    const std::string STATUS_METHOD_NOT_ALLOWED_405 = "405 Method Not Allowed";
    const std::string STATUS_BAD_REQUEST_400 = "400 Bad Request";
    const std::string STATUS_PAYLOAD_TOO_LARGE_413 = "413 Payload Too Large";
    const std::string STATUS_INTERNAL_SERVER_ERROR_500 = "500 Internal Server Error";

    // HTTP Status Codes
    const int OK_200 = 200;
    const int MOVED_PERMANENTLY_301 = 301;
    const int FOUND_302 = 302;
    const int NOT_FOUND_404 = 404;
    const int METHOD_NOT_ALLOWED_405 = 405;
    const int BAD_REQUEST_400 = 400;
    const int PAYLOAD_TOO_LARGE_413 = 413;
    const int INTERNAL_SERVER_ERROR_500 = 500;

}

/**
 * @namespace error_page
 * @brief Contains file paths to HTML error pages served for different HTTP errors
 *
 * This namespace centralizes the paths to custom error page templates.
 * Each constant represents the file path to an HTML template that will be
 * served when the corresponding HTTP error occurs.
 */
namespace error_page {
  const std::string ERROR_PAGE_NOT_FOUND_404 = "www/errors/not_found_404.html";
  const std::string ERROR_PAGE_METHOD_NOT_ALLOWED_405 = "www/errors/method_not_allowed_405.html";
  const std::string ERROR_PAGE_BAD_REQUEST_400 = "www/errors/internal_server_error_500.html"; // Using generic error page
  const std::string ERROR_PAGE_PAYLOAD_TOO_LARGE_413 = "www/errors/internal_server_error_500.html"; // Using generic error page
  const std::string ERROR_PAGE_INTERNAL_SERVER_ERROR_500 = "www/errors/internal_server_error_500.html";
}

/**
 * @namespace page
 * @brief Contains default file paths and constants for web server content
 *
 * This namespace defines standard paths and filenames used by the web server
 * for serving default content, index pages, and upload-related pages.
 */
namespace page {
  const std::string WWW = "www";
  const std::string ROOT_HTML = "/";
  const std::string INDEX_HTML_PATH = "/index.html";
  const std::string INDEX_HTML = "www/index.html";
  const std::string UPLOAD_HTML = "www/upload.html";
  const std::string UPLOAD_ERROR_HTML = "www/upload_error.html";
  const std::string UPLOAD_SUCCESS_HTML = "www/upload_success.html";
}

/**
 * @class Router
 * @brief Main router class that manages HTTP route mappings and handles request dispatching
 *
 * The Router class is the core component responsible for:
 * - Storing route mappings organized by server name, HTTP method, and path
 * - Matching incoming requests to appropriate handler functions
 * - Providing a clean interface for registering new routes
 * - Supporting multiple server configurations simultaneously
 *
 * Routes are stored in a hierarchical map structure for efficient lookup:
 * std::map<std::string, std::map<std::string, std::map<std::string, Handler>>>
 * This allows O(log n) lookup time for route matching.
 */
class Router {
  public:
    /**
     * @brief Default constructor for Router
     */
    Router();

    /**
     * @brief Destructor for Router
     */
    ~Router();

    /**
     * @brief Type alias for route handler functions
     *
     * Handler functions take three parameters:
     * - const Request&: The incoming HTTP request object
     * - Response&: The response object to be populated
     * - const Location*: Pointer to the matched location configuration (can be nullptr)
     */
    using Handler = std::function<void(const Request&, Response&, const Location*)>;

    /**
     * @brief Initialize the router with routes based on server configurations
     * @param configs Vector of Server objects containing route configurations
     *
     * This method processes all server configurations and registers appropriate
     * routes for each server. It sets up default routes and any custom routes
     * defined in the server configuration.
     */
    void setupRouter(const std::vector<Server>& configs);

    /**
     * @brief Debug method to list all registered routes
     *
     * This method outputs all registered routes to the console for debugging
     * and verification purposes. It displays the server name, HTTP method,
     * path, and handler information for each route.
     */
    void listRoutes() const;

    /**
     * @brief Register a new route with specific HTTP method and path for a server
     * @param server_name Name of the server this route belongs to
     * @param method HTTP method (GET, POST, DELETE, etc.)
     * @param path URL path pattern to match
     * @param handler Function to handle requests for this route
     *
     * This method adds a new route mapping to the internal route storage.
     * Routes are organized hierarchically by server name, then path, then method.
     */
    void addRoute(std::string_view server_name, std::string_view method, std::string_view path, Handler handler);

    /**
     * @brief Process an incoming HTTP request and route it to appropriate handler
     * @param server The server configuration for this request
     * @param req The incoming HTTP request object
     * @param res The response object to be populated by the handler
     *
     * This is the main entry point for request processing. It finds the appropriate
     * handler based on the server name, HTTP method, and request path, then
     * invokes the handler function with the request, response, and location data.
     */
    void handleRequest(const Server& server, const Request& req, Response& res) const;

    /**
     * @brief Generate HTML content for default error pages
     * @param status HTTP status code (e.g., 404, 500)
     * @return HTML content string for the error page
     *
     * This static method generates basic HTML error pages for common HTTP
     * status codes when custom error page files are not available.
     */
    static std::string getDefaultErrorPage(int status);

  private:
    /**
     * @brief Find the handler function for a given server, method and path
     * @param server_name Name of the server to search routes for
     * @param method HTTP method to match (GET, POST, etc.)
     * @param path URL path to match against registered routes
     * @return Pointer to handler function or nullptr if no matching route found
     *
     * This method performs a hierarchical lookup in the route storage:
     * 1. Find server by name
     * 2. Find path within that server
     * 3. Find method within that path
     * Returns nullptr if any level of the hierarchy is not found.
     */
    const Handler* findHandler(const std::string& server_name, const std::string& method, const std::string& path) const;

    /**
     * @brief Find the matching location configuration for a given server and path
     * @param server The server configuration object to search in
     * @param path URL path to match against location patterns
     * @return Pointer to matching location or nullptr if not found
     *
     * This method searches through the server's location configurations to find
     * the most specific location that matches the given path. It handles pattern
     * matching and precedence rules for location blocks.
     */
    const Location* findLocation(const Server& server, const std::string& path) const;

    /**
     * @brief Internal storage for route mappings organized hierarchically
     *
     * This nested map structure provides efficient O(log n) lookup time:
     * - First level: server name (string)
     * - Second level: URL path (string)
     * - Third level: HTTP method (string)
     * - Value: Handler function (std::function)
     *
     * Structure: std::map<server_name, std::map<path, std::map<method, Handler>>>
     */
    std::map<std::string, std::map<std::string, std::map<std::string, Handler>>> _routes;
};

/**
 * @brief Utility functions for error handling and response generation
 */

/**
 * @brief Set up a complete error response with appropriate status, headers, and body
 * @param res Response object to configure with error details
 * @param status HTTP status code for the error (e.g., 404, 500)
 *
 * This utility function configures a Response object with:
 * - Appropriate HTTP status code and status message
 * - Standard error headers (Content-Type, Content-Length)
 * - HTML error page content (either custom or default)
 * - Proper response formatting
 *
 * Used throughout the web server when errors need to be returned to clients.
 */
void setErrorResponse(Response& res, int status);
