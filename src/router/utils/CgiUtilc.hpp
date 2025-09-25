#pragma once

#include <string>
#include <vector>

struct Location;

namespace router {
namespace utils {

bool isCgiScriptWithLocation(const std::string& filename, const Location* location);

} // namespace utils
} // namespace router
