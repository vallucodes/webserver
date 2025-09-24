#pragma once

#include <iostream>
#include <sstream>
#include <unordered_set>
#include "../request/Request.hpp"
#include "../response/Response.hpp"

class Parser{
    private:
    public:
        static Request parseRequest(const std::string& httpString, bool& kick_me, bool bad_request);
        // static std::string serializeResponse(const Response& response);
};

std::string trim(std::string_view sv);