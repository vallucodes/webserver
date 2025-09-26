/**
 * @file Router.hpp
 * @brief HTTP Router for route mappings
 */

#pragma once

#include <map> // for std::map
#include <string> // for std::string
#include <string_view> // for std::string_view
#include <functional> // for std::function

#include "../request/Request.hpp"
#include "../response/Response.hpp"
#include "../server/Server.hpp"
#include "HttpConstants.hpp"
#include "RequestProcessor.hpp"

/**
 * @class Router
 * @brief Manages HTTP route mappings
 */
class Router {
  public:
    Router();
    ~Router();

    /** Handler function type: (Request, Response, Location*, Server) -> void */
    using Handler = std::function<void(const Request&, Response&, const Server&)>;

    /** Initialize router with server configurations */
    void setupRouter(const std::vector<Server>& configs);

    /** Debug: list all registered routes */
    void listRoutes() const;

    /** Register a new route */
    void addRoute(int server_port, std::string_view method, std::string_view path, Handler handler);

    /** Process HTTP request and route to handler */
    void handleRequest(const Server& server, const Request& req, Response& res) const;

    /** Handle request timeout (408) */
    void requestTimeOut();

  private:
    /** Find handler for server/method/path */
    const Handler* findHandler(int server_port, const std::string& method, const std::string& path) const;

    /** Find matching location configuration */
    const Location* findLocation(const Server& server, const std::string& path) const;

    /** Route storage: server port → path → HTTP method → Handler */
    std::map<int, std::map<std::string, std::map<std::string, Handler>>> _routes;

    /** Request processor for complex request logic */
    RequestProcessor _requestProcessor;
};
