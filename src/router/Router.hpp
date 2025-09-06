#pragma once

#include <map>
#include <string>
#include <string_view>
#include <functional>

#include "../response/Response.hpp"

// Simple Request struct (fixed to return const std::string& for efficiency)
struct Request {
    std::string method;
    std::string path;
    std::string_view getMethod() const { return method; }
    std::string_view getPath() const { return path; }
};

// Router class - Manages HTTP route mappings and request handling
class Router {
  public:
    Router();
    ~Router();

    // Type alias for route handler functions
    using Handler = std::function<void(const Request&, Response&)>;

    // Register a new route with specific HTTP method and path
    void addRoute(const std::string& method, const std::string& path, Handler handler);

    // Helper methods for registering routes
    void get(const std::string& path, Handler handler);
    void post(const std::string& path, Handler handler);
    void del(const std::string& path, Handler handler);

    // Process an incoming HTTP request and route it to appropriate handler
    void handleRequest(const Request& req, Response& res) const;

  private:
    // Find the handler function for a given method and path
    const Handler* findHandler(std::string_view method, std::string_view path) const;

    // Default error page generator (moved to class as static for encapsulation)
    static std::string getDefaultErrorPage(int status);

    // Internal storage for route mappings
    std::map<std::string, std::map<std::string, Handler>> _routes;
};
