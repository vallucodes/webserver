/**
 * @file Router.hpp
 * @brief Simplified HTTP Router for route mappings
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
 * @brief Manages HTTP route mappings with simplified logic
 */
class Router {
public:
  Router();
  ~Router();

  /** Handler function type: (Request, Response, Server) -> void */
  using Handler = std::function<void(const Request&, Response&, const Server&)>;

  /** Initialize router with server configurations */
  void setupRouter(const std::vector<Server>& configs);

  /** Process HTTP request and route to handler */
  void handleRequest(const Server& server, const Request& req, Response& res) const;

  /** Debug: list all registered routes */
  void listRoutes() const;

private:
  /** Register a new route */
  void addRoute(int server_id, std::string_view method, std::string_view path, Handler handler);

  /** Find handler for server/method/path */
  const Handler* findHandler(int server_id, const std::string& method, const std::string& path) const;

  /** Find matching location configuration */
  const Location* findLocation(const Server& server, const std::string& path) const;

  /** Route storage: server id → path → HTTP method → Handler */
  std::map<int, std::map<std::string, std::map<std::string, Handler>>> _routes;

  /** Request processor for handling requests */
  RequestProcessor _requestProcessor;
};
