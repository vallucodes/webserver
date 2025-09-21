/**
 * @file Router.hpp
 * @brief HTTP Router for route mappings
 */

#pragma once

#include <map>
#include <string>
#include <string_view>
#include <functional>

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

    /** Handler function type: (Request, Response, Location*) -> void */
    using Handler = std::function<void(const Request&, Response&, const Location*)>;

    /** Initialize router with server configurations */
    void setupRouter(const std::vector<Server>& configs);

    /** Debug: list all registered routes */
    void listRoutes() const;

    /** Register a new route */
    void addRoute(std::string_view server_name, std::string_view method, std::string_view path, Handler handler);

    /** Process HTTP request and route to handler */
    void handleRequest(const Server& server, const Request& req, Response& res) const;

    // Now using router::utils::ErrorResponseBuilder::getErrorPageHtml instead

  private:
    /** Find handler for server/method/path */
    const Handler* findHandler(const std::string& server_name, const std::string& method, const std::string& path) const;

    /** Find matching location configuration */
    const Location* findLocation(const Server& server, const std::string& path) const;

    /** Route storage: server name → path → HTTP method → Handler */
    std::map<std::string, std::map<std::string, std::map<std::string, Handler>>> _routes;

    /** Request processor for complex request logic */
    RequestProcessor _requestProcessor;
};

// Now using router::utils::ErrorResponseBuilder::setErrorResponse instead
