#pragma once

#include <string> // for std::string
#include <vector> // for std::vector

std::string executeCgiScript(const std::string& scriptPath, const std::vector<std::string>& env, const std::string& input);
