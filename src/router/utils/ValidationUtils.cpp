#include "ValidationUtils.hpp"
#include "../../server/Server.hpp"
#include "../../request/Request.hpp"
#include "../HttpConstants.hpp"
#include "HttpResponseBuilder.hpp"
#include <filesystem> // for std::filesystem::directory_iterator, std::filesystem::path, std::filesystem::exists, std::filesystem::is_directory, std::filesystem::is_regular_file, std::filesystem::create_directories, std::filesystem::remove, std::filesystem::file_size, std::filesystem::last_write_time

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
bool isValidLocationServer(Response& res, const Location* location, const Server* server, const Request& req) {
  if (!location) {
    router::utils::HttpResponseBuilder::setErrorResponse(res, http::NOT_FOUND_404, req);
    return false;
  }
  if (location->cgi_path.empty()) {
    router::utils::HttpResponseBuilder::setErrorResponse(res, http::INTERNAL_SERVER_ERROR_500, req);
    return false;
  }
  if (location->cgi_ext.empty()) {
    router::utils::HttpResponseBuilder::setErrorResponse(res, http::INTERNAL_SERVER_ERROR_500, req);
    return false;
  }
  if (!server) {
    router::utils::HttpResponseBuilder::setErrorResponse(res, http::INTERNAL_SERVER_ERROR_500, req);
    return false;
  }
  if (server->getRoot().empty()) {
    router::utils::HttpResponseBuilder::setErrorResponse(res, http::INTERNAL_SERVER_ERROR_500, req);
    return false;
  }
  return true;
}

bool isValidPath(const std::string_view& path, Response& res, const Request& req) {
  if (path.empty()) {
    router::utils::HttpResponseBuilder::setErrorResponse(res, http::BAD_REQUEST_400, req);
    return false;
  }
  return true;
}

bool isFileExistsAndExecutable(const std::string& filePath, Response& res, const Request& req) {
  if (filePath.empty()) {
    router::utils::HttpResponseBuilder::setErrorResponse(res, http::BAD_REQUEST_400, req);
    return false;
  }

  // Check if file exists and is executable
  if (!std::filesystem::exists(filePath)) {
    router::utils::HttpResponseBuilder::setErrorResponse(res, http::NOT_FOUND_404, req);
    return false;
  }

  return true;
}
} // namespace utils
} // namespace router
