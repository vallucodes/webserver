#include "Router.hpp"

// in main.cpp
// Router router;
// router.get("/", [](const Request& req, Response& res) {
//     res.setBody("Hello World!");

// in the server loop
// Request req = parser.parse(socket);
// Response res;
// router.handleRequest(req, res);
// socket.send(res.getgetBody()); ??


Router::Router() {}

Router::~Router() {}

void Router::handleRequest(const Request& req, Response& res) const
{
    const std::string& method = req.method(); // e.g. "GET"
    const std::string& path   = req.path();   // e.g. "/users/123"

    if (const Handler* h = findHandler(method, path)) {
        (*h)(req, res);
        return;
    }

    // Default 404 response if no handler found
    res.setStatus("404 Not Found");
    res.setBody("Route not found");
}

void Router::addRoute(const std::string& method,
                      const std::string& path,
                      Handler handler) {
    _routes[path][method] = std::move(handler);
}

void Router::get(const std::string& path, Handler handler) {
    addRoute("GET", path, std::move(handler));
}

const Router::Handler* Router::findHandler(const std::string& method, const std::string& path) const {
    auto path_it = _routes.find(path);
    if (path_it != _routes.end()) {
        auto method_it = path_it->second.find(method);
        if (method_it != path_it->second.end()) {
            return &method_it->second;
        }
    }
    return nullptr;
}
