#pragma once

#include <string>



// Function declarations
std::string readFileToString(const std::string& filename);

// Commented out until Request/Response classes are properly set up
/*
#include "../request/Request.hpp"
#include "../response/Response.hpp"



void getMainPageHandler(const Request& req, Response& res);
*/
