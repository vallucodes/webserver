#pragma once

#include <string> // for std::string
#include <vector> // for std::vector
#include <map> // for std::map

struct CgiResult {
  std::map<std::string, std::string> headers;
  std::string body;
  std::string status; // HTTP status line (e.g., "HTTP/1.1 200 OK")
  bool success;

  CgiResult() : success(false) {}
};

std::string executeCgiScript(const std::string& scriptPath, const std::vector<std::string>& env, const std::string& input);
CgiResult executeAndParseCgiScript(const std::string& scriptPath, const std::vector<std::string>& env, const std::string& input);
