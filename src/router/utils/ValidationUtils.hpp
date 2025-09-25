#pragma once

#include <filesystem> // for std::filesystem::directory_iterator, std::filesystem::path, std::filesystem::exists, std::filesystem::is_directory, std::filesystem::is_regular_file, std::filesystem::create_directories, std::filesystem::remove, std::filesystem::file_size, std::filesystem::last_write_time
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
