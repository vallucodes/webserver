#pragma once

#include <map>
#include <string>
#include <string_view>
#include <functional>

#include "Request.hpp"
#include "Response.hpp"

// Usage for parsing config file:
// if (directive == "location" && !method.empty()) {
//   router.addRoute(method, path, [path, method](const Request& req, Response& res) {
//       (void)req; (void)path; (void)method;
//       res.setStatus("200 OK");
//       res.setBody("Handler for " + method + " on " + std::string(path));
//   });


class Router {
  public:
    Router();
    ~Router();

    // Type alias for route handler functions
    using Handler = std::function<void(const Request&, Response&)>;

    // Register a new route with specific HTTP method and path
    void addRoute(std::string_view method, std::string_view path, Handler handler);

    // Register a new routes
    void get(std::string_view path, Handler handler);
    void post(std::string_view path, Handler handler);
    void del(std::string_view path, Handler handler);
    void handleRequest(const Request& req, Response& res) const;

  private:
    // Find the handler function for a given method and path
    const Handler* findHandler(std::string_view method, std::string_view path) const;

    // Default error page generator (moved to class as static for encapsulation)
    static std::string getDefaultErrorPage(int status);

    // Internal storage for route mappings
    //
    // Example:
    // _routes:
    //   "/api":
    //     "GET" -> Handler (lambda for GET on /api)
    //     "POST" -> Handler (lambda for POST on /api)
    //   "/static":
    //     "DELETE" -> Handler (lambda for DELETE on /static)
    std::map<std::string, std::map<std::string, Handler>> _routes;
};
