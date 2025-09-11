#include "Router.hpp"
#include "handlers/Handlers.hpp"

// using namespace http;

Router::Router() {}
Router::~Router() {}

void Router::setupRouter() {
    addRoute("GET", "/", getStaticPage);
	addRoute("GET", "/index.html", getStaticPage);
    
	addRoute("GET", "/imgs/lhaas.png", getStaticFile);
	addRoute("GET", "/imgs/vlopatin.png", getStaticFile);
	addRoute("GET", "/imgs/imunaev-.png", getStaticFile);
}

void Router::addRoute(std::string_view method, std::string_view path, Handler handler) {
    _routes[std::string(path)][std::string(method)] = std::move(handler);
}

// void Router::get(std::string_view path, Handler handler) {
//     addRoute("GET", path, std::move(handler));
// }

// void Router::post(std::string_view path, Handler handler) {
//     addRoute("POST", path, std::move(handler));
// }

// void Router::del(std::string_view path, Handler handler) {
//     addRoute("DELETE", path, std::move(handler));
// }

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

std::string Router::getDefaultErrorPage(int status) {
    switch (status) {
        case http::NOT_FOUND_404:
            return readFileToString(error_page::ERROR_PAGE_NOT_FOUND_404);
        case http::METHOD_NOT_ALLOWED_405:
            return readFileToString(error_page::ERROR_PAGE_METHOD_NOT_ALLOWED_405);
        case http::INTERNAL_SERVER_ERROR_500:
            return readFileToString(error_page::ERROR_PAGE_INTERNAL_SERVER_ERROR_500);
        default:
            return readFileToString(error_page::ERROR_PAGE_INTERNAL_SERVER_ERROR_500);
    }
}

void setErrorResponse(Response& res, int status){
    if (status == http::NOT_FOUND_404) {
        res.setStatus(http::STATUS_NOT_FOUND_404);
    } else if (status == http::METHOD_NOT_ALLOWED_405) {
        res.setStatus(http::STATUS_METHOD_NOT_ALLOWED_405);
    } else if (status == http::INTERNAL_SERVER_ERROR_500) {
        res.setStatus(http::STATUS_INTERNAL_SERVER_ERROR_500);
    }
    res.setHeader("Content-Type", "text/html");
    res.setHeader("Content-Length", std::to_string(Router::getDefaultErrorPage(status).length()));
    res.setHeader("Connection", "close");
    res.setBody(Router::getDefaultErrorPage(status));
}

void Router::handleRequest(const Request& req, Response& res) const {
    std::string_view method_view = req.getMethod();
    std::string_view path_view = req.getPath();

    std::string method(method_view);
    std::string path(path_view);

    if (const Handler* h = findHandler(method, path)) {
        try {
            (*h)(req, res);
        } catch (...) {
            setErrorResponse(res, http::INTERNAL_SERVER_ERROR_500);
        }
        return;
    }

    // If handler not found, check if path exists
    auto path_it = _routes.find(path);
    if (path_it != _routes.end()) {
        // Path exists but method not allowed
        setErrorResponse(res, http::METHOD_NOT_ALLOWED_405);
    } else {
        // Path not found
        setErrorResponse(res, http::NOT_FOUND_404);
    }
}


// Handler createHandler(const std::string& method, const std::string& path, const LocationConfig& loc) {
//     if (!loc.root.empty() && method == "GET") {
//         return [root = loc.root](const Request& req, Response& res) {
//             res.setStatus(HTTP_STATUS_OK_200);
//             res.setBody("Serving GET from " + root + req.uri);
//         };
//     } else if (!loc.cgi_path.empty() && method == "POST") {
//         return [cgiPath = loc.cgi_path](const Request& req, Response& res) {
//             res.setStatus(HTTP_STATUS_OK_200);
//             res.setBody("CGI POST response");
//         };
//     }
//     return [](const Request&, Response& res) {
//         res.setStatus(HTTP_STATUS_NOT_FOUND_404);
//         res.setHeader("Content-Type", "text/html");
//         res.setHeader("Content-Length", std::to_string(getDefaultErrorPage(HTTP_STATUS_NOT_FOUND_404).length()));
//         res.setHeader("Connection", "close");
//         res.setBody(getDefaultErrorPage(HTTP_STATUS_NOT_FOUND_404));
//     };
// }



// TODO: Define LocationConfig type and implement handler creation logic
// Router::Handler testHandler;

// Router::Handler createHandler() {
//     Router::Handler testHandler;
//     return Router::getStaticPage();
// }

// testHandler = createHandler();




