#pragma once

#include <map>
#include <string>
#include <string_view>
#include <functional>

#include "../request/Request.hpp"
#include "../response/Response.hpp"

// HTTP constants namespace
namespace http {

    // HTTP Status Messages
    const std::string STATUS_OK_200 = "200 OK";
    const std::string STATUS_NOT_FOUND_404 = "404 Not Found";
    const std::string STATUS_METHOD_NOT_ALLOWED_405 = "405 Method Not Allowed";
    const std::string STATUS_INTERNAL_SERVER_ERROR_500 = "500 Internal Server Error";

    // HTTP Status Codes
    const int OK_200 = 200;
    const int NOT_FOUND_404 = 404;
    const int METHOD_NOT_ALLOWED_405 = 405;
    const int INTERNAL_SERVER_ERROR_500 = 500;

}

// Error page file paths namespace
namespace error_page {
  const std::string ERROR_PAGE_NOT_FOUND_404 = "www/errors/not_found_404.html";
  const std::string ERROR_PAGE_METHOD_NOT_ALLOWED_405 = "www/errors/method_not_allowed_405.html";
  const std::string ERROR_PAGE_INTERNAL_SERVER_ERROR_500 = "www/errors/internal_server_error_500.html";

}
// Default file paths namespace
namespace page {

  const std::string WWW = "www";
  const std::string ROOT_HTML = "/";
  const std::string INDEX_HTML_PATH = "/index.html";
  const std::string INDEX_HTML = "www/index.html";
  const std::string UPLOAD_HTML = "www/upload.html";
  const std::string UPLOAD_ERROR_HTML = "www/upload_error.html";
  const std::string UPLOAD_SUCCESS_HTML = "www/upload_success.html";
}
// Router class - Manages HTTP route mappings and request handling (global, no namespace)
class Router {
  public:
    Router();
    ~Router();

    // Type alias for route handler functions
    using Handler = std::function<void(const Request&, Response&)>;

    // void setupRouter(someConfigData& data);
    // Initialize the router with default routes for static files and upload endpoints
    void setupRouter();

    // Debug method to list all routes
    void listRoutes() const;

    // Register a new route with specific HTTP method and path
    // @param method HTTP method (GET, POST, etc.)
    // @param path URL path to match
    // @param handler Function to handle requests for this route
    void addRoute(std::string_view method, std::string_view path, Handler handler);

    // Process an incoming HTTP request and route it to appropriate handler
    // @param req The incoming HTTP request
    // @param res The response object to populate
    void handleRequest(const Request& req, Response& res) const;

    // Default error page generator
    // @param status HTTP status code
    // @return HTML content for the error page
    static std::string getDefaultErrorPage(int status);

  private:
    // Find the handler function for a given method and path
    // @param method HTTP method to match
    // @param path URL path to match
    // @return Pointer to handler function or nullptr if not found
    const Handler* findHandler(const std::string& method, const std::string& path) const;

    // Internal storage for route mappings
    std::map<std::string, std::map<std::string, Handler>> _routes;
};

// Utility functions for error handling

// Set up a complete error response with appropriate status, headers, and body
// @param res Response object to configure
// @param status HTTP status code for the error
void setErrorResponse(Response& res, int status);
