#pragma once

#include <string>
#include <vector>

#include "../../request/Request.hpp"
#include "../../response/Response.hpp"
#include "../../server/Server.hpp"

struct Location;

namespace router {
namespace utils {

bool isCgiScriptWithLocation(const std::string& filename, const Location* location);
bool isChunked(const Request& req);
bool shouldKeepAlive(const Request& req);
std::string parseChunkedRequestBody(const std::string& body);
std::vector<std::string> setupCgiEnvironment(const Request& req, const std::string& scriptPath, const std::string& scriptName, const Server& server);
std::string generateDirectoryListing(const std::string& dirPath, const std::string& requestPath);
bool handleDirectoryRequest(const std::string& dirPath, const std::string& requestPath, const Location* location, Response& res, const Request& req);
std::string createSuccessMessage(const std::string& filename, const std::string& action);
bool serveStaticFile(const std::string& filePath, Response& res, const Request& req);

} // namespace utils
} // namespace router
