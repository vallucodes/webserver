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

  /** Handler function type */
  using Handler = std::function<void(const Request&, Response&, const Server&)>;

  /** Initialize router with server configs */
  void setupRouter(const std::vector<Server>& configs);

  /** Process HTTP request */
  void handleRequest(const Server& server, const Request& req, Response& res) const;

  /** List all registered routes */
  void listRoutes() const;

private:
  /** Register route */
  void addRoute(int server_id, std::string_view method, std::string_view path, Handler handler);

  /** Find handler */
  const Handler* findHandler(int server_id, const std::string& method, const std::string& path) const;

  /** Find matching location */
  const Location* findLocation(const Server& server, const std::string& path) const;

  /** Route storage: server_id → path → method → handler */
  std::map<int, std::map<std::string, std::map<std::string, Handler>>> _routes;

  /** Request processor */
  RequestProcessor _requestProcessor;
};
