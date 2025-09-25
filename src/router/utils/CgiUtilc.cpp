#include "CgiUtilc.hpp"
#include "../../server/Server.hpp"

namespace router {
namespace utils {

bool isCgiScriptWithLocation(const std::string& filename, const Location* location) {
    if (!location || location->cgi_ext.empty()) {
        return false;
    }

    size_t dotPos = filename.find_last_of('.');
    if (dotPos != std::string::npos) {
        std::string ext = filename.substr(dotPos + 1);
        // Convert to lowercase for case-insensitive comparison
        for (char& c : ext) {
            c = std::tolower(c);
        }

        // Check against location-configured extensions
        for (const auto& allowedExt : location->cgi_ext) {
            std::string allowedExtLower = allowedExt;
            for (char& c : allowedExtLower) {
                c = std::tolower(c);
            }
            // Remove leading dot from allowed extension for comparison
            if (allowedExtLower.length() > 0 && allowedExtLower[0] == '.') {
                allowedExtLower = allowedExtLower.substr(1);
            }
            if (ext == allowedExtLower) {
                return true;
            }
        }
    }
    return false;
}

} // namespace utils
} // namespace router
