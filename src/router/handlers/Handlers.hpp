#pragma once

#include <string>

#include "../../request/Request.hpp"
#include "../../response/Response.hpp"

// Function declarations
std::string readFileToString(const std::string& filename);
void getStaticPage(const Request& req, Response& res);
void getStaticFile(const Request& req, Response& res);
