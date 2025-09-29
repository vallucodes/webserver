/**
 * @file Router.cpp
 * @brief HTTP Router implementation
 */

#include "../../inc/webserv.hpp"
#include "Router.hpp"
#include "utils/HttpResponseBuilder.hpp"
#include "utils/StringUtils.hpp"
#include "HttpConstants.hpp"
#include "handlers/Handlers.hpp"
#include <algorithm> // for std::find

using namespace router::utils;

Router::Router() {}

Router::~Router() {}

// ========================= ROUTER SETUP =========================

/** Initialize router with server configs */
void Router::setupRouter(const std::vector<Server>& configs) {
  _routes.clear();

  for (size_t i = 0; i < configs.size(); ++i) {
    const Server& server = configs[i];
    std::string server_root = server.getRoot();

    for (const auto& location : server.getLocations()) {
      std::string location_path = location.location;

      for (const auto& method : location.allowed_methods) {
        Handler handler;

        if (!location.return_url.empty()) {
          handler = [&server](const Request& req, Response& res, const Server& /* srv */) {
            redirect(req, res, server);
          };
        } else if (!location.cgi_path.empty() && !location.cgi_ext.empty()) {
          handler = [&server](const Request& req, Response& res, const Server& /* srv */) {
            cgi(req, res, server);
          };
        } else if (method == http::POST && !location.upload_path.empty()) {
          handler = [&server](const Request& req, Response& res, const Server& /* srv */) {
            post(req, res, server);
          };
        } else if (method == http::DELETE && !location.upload_path.empty()) {
          handler = [&server](const Request& req, Response& res, const Server& /* srv */) {
            del(req, res, server);
          };
        } else {
          handler = [&server](const Request& req, Response& res, const Server& /* srv */) {
            get(req, res, server);
          };
        }

        addRoute(server.getId(), method, location_path, handler);
      }
    }
  }
  listRoutes();
}

// ========================= ROUTES REGISTRATION =========================

/** Register route */
void Router::addRoute(int server_id, std::string_view method, std::string_view path, Handler handler) {
  _routes[server_id][std::string(path)][std::string(method)] = std::move(handler);
}

// ========================= REQUEST HANDLING =========================

/** Find handler */
const Router::Handler* Router::findHandler(int server_id, const std::string& method, const std::string& path) const {
  auto server_it = _routes.find(server_id);
  if (server_it == _routes.end()) {
    return nullptr;
  }

  const auto& server_routes = server_it->second;

  // Try exact path match first
  auto path_it = server_routes.find(path);
  if (path_it != server_routes.end()) {
    auto method_it = path_it->second.find(method);
    if (method_it != path_it->second.end()) {
      return &method_it->second;
    }
  }

  // Find best advanced match
  const Handler* best_handler = nullptr;
  size_t best_match_length = 0;
  bool is_extension_match = false;

  for (const auto& route_pair : server_routes) {
    const std::string& route_path = route_pair.first;

    auto method_it = route_pair.second.find(method);
    if (method_it == route_pair.second.end()) {
      continue;
    }

    // Extension-based matching
    if (!route_path.empty() && route_path[0] == '.' && path.length() > route_path.length()) {
      if (path.substr(path.length() - route_path.length()) == route_path) {
        if (!is_extension_match || route_path.length() > best_match_length) {
          best_handler = &method_it->second;
          best_match_length = route_path.length();
          is_extension_match = true;
        }
      }
    }
    // Prefix-based matching
    else if (!route_path.empty() &&
             path.length() >= route_path.length() &&
             path.substr(0, route_path.length()) == route_path) {

      bool is_valid_prefix_match = false;

      if (path.length() == route_path.length()) {
        is_valid_prefix_match = true;
      }
      else if (path[route_path.length()] == '/') {
        is_valid_prefix_match = true;
      }
      else if (route_path == "/" && path.length() > 1) {
        is_valid_prefix_match = false;
      }

      if (is_valid_prefix_match) {
        if (!is_extension_match && route_path.length() > best_match_length) {
          best_handler = &method_it->second;
          best_match_length = route_path.length();
        }
      }
    }
  }

  return best_handler;
}


/** Find matching location */
const Location* Router::findLocation(const Server& server, const std::string& path) const {
  const auto& locations = server.getLocations();
  const Location* best_match = nullptr;
  size_t best_match_length = 0;

  for (const auto& location : locations) {
    const std::string& location_path = location.location;

    // Exact match
    if (location_path == path) {
      return &location;
    }

    // Prefix match
    if (path.length() > location_path.length() &&
        path.substr(0, location_path.length()) == location_path &&
        (location_path.back() == '/' || path[location_path.length()] == '/')) {
      if (location_path.length() > best_match_length) {
        best_match = &location;
        best_match_length = location_path.length();
      }
    }
  }

  return best_match;
}


/** Process HTTP request */
void Router::handleRequest(const Server& server, const Request& req, Response& res) const {
  if (req.getError()) {
    int statusCode = router::utils::HttpResponseBuilder::parseStatusCodeFromString(std::string(req.getStatus()));
    router::utils::HttpResponseBuilder::setErrorResponse(res, statusCode, req, server);
    return;
  }

  std::string_view method_view = req.getMethod();
  std::string_view path_view = req.getPath();
  std::string method(method_view);
  std::string path(path_view);

  path = router::utils::StringUtils::normalizePath(path);
  const Handler* handler = findHandler(server.getId(), method, path);
  _requestProcessor.processRequest(req, handler, res, server);
}

// ========================= HELPERS =========================

/** List all registered routes */
void Router::listRoutes() const {

  std::cout << "=== Available routes: ===" << std::endl;
  for (const auto& server_pair : _routes) {
    int server_id = server_pair.first;
    std::cout << "Server ID: " << server_id << std::endl;

    for (const auto& path_pair : server_pair.second) {
      const std::string& path = path_pair.first;
      std::cout << "  " << path << " -> ";
      for (const auto& method_pair : path_pair.second) {
        std::cout << method_pair.first << " ";
      }
      std::cout << std::endl;
    }
  }
  std::cout << "=========================\n" << std::endl;
}


