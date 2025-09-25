#pragma once

#include "../../response/Response.hpp"

// Forward declarations
struct Location;
class Server;

namespace router {
namespace utils {

bool isValidLocationServer(Response& res, const Location* location, const Server* server);
bool isValidPath(const std::string_view& path, Response& res);
bool isFileExistsAndExecutable(const std::string& filePath, Response& res);

} // namespace utils
} // namespace router
