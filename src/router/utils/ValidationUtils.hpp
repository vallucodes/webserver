#pragma once

#include "../../response/Response.hpp"

// Forward declarations
struct Location;
class Server;

namespace router {
namespace utils {

bool isValidateLocationServer(Response& res, const Location* location, const Server* server);

} // namespace utils
} // namespace router
