#include "ValidationUtils.hpp"
#include "../../server/Server.hpp"
#include "../HttpConstants.hpp"
#include "HttpResponseBuilder.hpp"

using namespace router::utils;

namespace router {
namespace utils {
/*
    location /cgi-bin {
        allow_methods GET POST
        cgi_path cgi-bin
        cgi_ext .py .js
        index index.html
    }
*/
bool isValidLocationServer(Response& res, const Location* location, const Server* server) {
    if (!location) {
router::utils::HttpResponseBuilder::setErrorResponse(res, http::NOT_FOUND_404);
        return false;
    }
    if (location->cgi_path.empty()) {
router::utils::HttpResponseBuilder::setErrorResponse(res, http::INTERNAL_SERVER_ERROR_500);
        return false;
    }
    if (location->cgi_ext.empty()) {
router::utils::HttpResponseBuilder::setErrorResponse(res, http::INTERNAL_SERVER_ERROR_500);
        return false;
    }
    if (!server) {
router::utils::HttpResponseBuilder::setErrorResponse(res, http::INTERNAL_SERVER_ERROR_500);
        return false;
    }
    if (server->getRoot().empty()) {
router::utils::HttpResponseBuilder::setErrorResponse(res, http::INTERNAL_SERVER_ERROR_500);
        return false;
    }
    return true;
}

bool isValidPath(const std::string_view& path, Response& res) {
    if (path.empty()) {
router::utils::HttpResponseBuilder::setErrorResponse(res, http::BAD_REQUEST_400);
        return false;
    }
    return true;
}

bool isFileExistsAndExecutable(const std::string& filePath, Response& res) {
    if (filePath.empty()) {
router::utils::HttpResponseBuilder::setErrorResponse(res, http::BAD_REQUEST_400);
        return false;
    }

    // Check if file exists and is executable
    if (!std::filesystem::exists(filePath)) {
router::utils::HttpResponseBuilder::setErrorResponse(res, http::NOT_FOUND_404);
        return false;
    }

    return true;
}
} // namespace utils
} // namespace router
