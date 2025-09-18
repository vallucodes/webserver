#include  "Parser.hpp"

std::string trim(std::string_view sv) {
    size_t start = sv.find_first_not_of(" ");
    size_t end = sv.find_last_not_of(" ");
    if (start == std::string_view::npos)
        return ("");
    return std::string(sv.substr(start, end - start + 1));
}