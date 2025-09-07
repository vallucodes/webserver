#include "Router.hpp"

Router::Router() {}
Router::~Router() {}

void Router::addRoute(std::string_view method, std::string_view path, Handler handler) {
    _routes[std::string(path)][std::string(method)] = std::move(handler);
}

void Router::get(std::string_view path, Handler handler) {
    addRoute("GET", path, std::move(handler));
}

void Router::post(std::string_view path, Handler handler) {
    addRoute("POST", path, std::move(handler));
}

void Router::del(std::string_view path, Handler handler) {
    addRoute("DELETE", path, std::move(handler));
}

const Router::Handler* Router::findHandler(std::string_view method, std::string_view path) const {
    auto path_it = _routes.find(std::string(path));
    if (path_it != _routes.end()) {
        auto method_it = path_it->second.find(std::string(method));
        if (method_it != path_it->second.end()) {
            return &method_it->second;
        }
    }
    return nullptr;
}

void Router::handleRequest(const Request& req, Response& res) const {
    std::string_view method = req.getMethod();
    std::string_view path = req.getPath();

    if (const Handler* h = findHandler(method, path)) {
        try {
            (*h)(req, res);
        } catch (...) {
            res.setStatus("500 Internal Server Error");
            res.setBody(getDefaultErrorPage(500));
        }
        return;
    }

    auto path_it = _routes.find(std::string(path));
    if (path_it != _routes.end()) {
        res.setStatus("405 Method Not Allowed");
        res.setBody(getDefaultErrorPage(405));
    } else {
        res.setStatus("404 Not Found");
        res.setBody(getDefaultErrorPage(404));
    }
}

std::string Router::getDefaultErrorPage(int status) {
    switch (status) {
        case 404:
            return "<html><body><h1>404 Not Found</h1><p>The requested resource was not found.</p></body></html>";
        case 405:
            return "<html><body><h1>405 Method Not Allowed</h1><p>Invalid method for this resource.</p></body></html>";
        case 500:
            return "<html><body><h1>500 Internal Server Error</h1><p>Something went wrong on the server.</p></body></html>";
        default:
            return "<html><body><h1>Error</h1><p>An unexpected error occurred.</p></body></html>";
    }
}
