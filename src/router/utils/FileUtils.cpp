/**
 * @file FileUtils.cpp
 * @brief File utility functions implementation
 */

#include "FileUtils.hpp"
#include <fstream> // for std::ifstream, std::ios
#include <stdexcept> // for std::runtime_error
#include <filesystem> // for std::filesystem::path, std::filesystem::path::extension
#include <algorithm> // for std::transform

namespace router {
namespace utils {

std::string FileUtils::readFileToString(const std::string& filename) {
  std::ifstream file(filename, std::ios::binary);
  if (!file.is_open()) {
    throw std::runtime_error("Could not open file: " + filename);
  }

  std::string content((std::istreambuf_iterator<char>(file)),
                      std::istreambuf_iterator<char>());
  return content;
}

std::string FileUtils::getContentType(const std::string& filePath) {
  std::string extension = std::filesystem::path(filePath).extension().string();

  // Convert to lowercase
  std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

  if (extension == ".html" || extension == ".htm") return "text/html";
  if (extension == ".css") return "text/css";
  if (extension == ".js") return "application/javascript";
  if (extension == ".json") return "application/json";
  if (extension == ".xml") return "application/xml";
  if (extension == ".txt") return "text/plain";
  if (extension == ".jpg" || extension == ".jpeg") return "image/jpeg";
  if (extension == ".png") return "image/png";
  if (extension == ".gif") return "image/gif";
  if (extension == ".svg") return "image/svg+xml";
  if (extension == ".ico") return "image/x-icon";
  if (extension == ".pdf") return "application/pdf";
  if (extension == ".zip") return "application/zip";
  if (extension == ".mp4") return "video/mp4";
  if (extension == ".mp3") return "audio/mpeg";

  return "application/octet-stream"; // Default for unknown types
}

} // namespace utils
} // namespace router
