#pragma once

#include <string>

#include "../../request/Request.hpp"
#include "../../response/Response.hpp"
#include "../../router/Router.hpp"

// Function declarations
std::string readFileToString(const std::string& filename);
void get(const Request& req, Response& res);
void post(const Request& req, Response& res);
void del(const Request& req, Response& res);
