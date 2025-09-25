#include "ValidationUtils.hpp"
#include "../../server/Server.hpp"
#include "../HttpConstants.hpp"
#include "HttpResponseBuilder.hpp"

namespace router {
namespace utils {

bool isValidateLocationServer(Response& res, const Location* location, const Server* server) {
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

} // namespace utils
} // namespace router
