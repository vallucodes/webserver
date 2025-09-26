#pragma once

#include "../../response/Response.hpp"

// Forward declarations
struct Location;
class Server;
class Request;

namespace router {
namespace utils {

bool isValidLocationServer(Response& res, const Location* location, const Server* server, const Request& req);
bool isValidPath(const std::string_view& path, Response& res, const Request& req);
bool isFileExistsAndExecutable(const std::string& filePath, Response& res, const Request& req);

} // namespace utils
} // namespace router
