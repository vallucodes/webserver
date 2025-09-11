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
namespace page_file {
  const std::string INDEX_HTML = "www/index.html";
}
// Router class - Manages HTTP route mappings and request handling (global, no namespace)
class Router {
  public:
    Router();
    ~Router();

    // Type alias for route handler functions
    using Handler = std::function<void(const Request&, Response&)>;

    // void setupRouter(someConfigData& data);
    void setupRouter();

    // Register a new route with specific HTTP method and path
    void addRoute(std::string_view method, std::string_view path, Handler handler);

    // Helper methods for registering routes
    // void get(std::string_view path, Handler handler);
    // void post(std::string_view path, Handler handler);
    // void del(std::string_view path, Handler handler);

    // Process an incoming HTTP request and route it to appropriate handler
    void handleRequest(const Request& req, Response& res) const;

    // Default error page generator
    static std::string getDefaultErrorPage(int status);

  private:
    // Find the handler function for a given method and path
    const Handler* findHandler(const std::string& method, const std::string& path) const;

    // Internal storage for route mappings
    std::map<std::string, std::map<std::string, Handler>> _routes;
};

// Utility functions for error handling
void setErrorResponse(Response& res, int status);
