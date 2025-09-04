#pragma once

#include <map>
#include <string>
#include <functional>

#include "Request.hpp"
#include "Response.hpp"

class Router {
  public:
    Router();
    ~Router();

    using Handler = std::function<void(const Request&, Response&)>;

    // router register
    void addRoute(const std::string& method, const std::string& path, Handler handler);

    // helpers
    void get(const std::string& path, Handler handler);

    void handleRequest(const Request& req, Response& res) const;

  private:
    const Handler* findHandler(const std::string& method, const std::string& path) const;

    std::map<std::string, std::map<std::string, Handler>> _routes;
};
