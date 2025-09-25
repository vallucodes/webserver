#pragma once

#include <string>
#include <vector>

#include "../../request/Request.hpp"
#include "../../server/Server.hpp"

struct Location;

namespace router {
namespace utils {

bool isCgiScriptWithLocation(const std::string& filename, const Location* location);
bool isChunked(const Request& req);
std::string parseChunkedRequestBody(const std::string& body);
std::vector<std::string> setupCgiEnvironment(const Request& req, const std::string& scriptPath, const std::string& scriptName, const Server& server);

} // namespace utils
} // namespace router
