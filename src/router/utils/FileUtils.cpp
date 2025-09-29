/**
 * @file FileUtils.cpp
 * @brief File utility functions implementation
 */

#include "FileUtils.hpp"
#include "../HttpConstants.hpp"
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

  if (extension == file_extensions::HTML || extension == file_extensions::HTM) return content_types::HTML;
  if (extension == file_extensions::CSS) return content_types::CSS;
  if (extension == file_extensions::JS) return content_types::JAVASCRIPT;
  if (extension == ".json") return "application/json";
  if (extension == ".xml") return "application/xml";
  if (extension == ".txt") return content_types::PLAIN_TEXT;
  if (extension == file_extensions::JPG || extension == file_extensions::JPEG) return content_types::JPG;
  if (extension == file_extensions::PNG) return content_types::PNG;
  if (extension == file_extensions::GIF) return content_types::GIF;
  if (extension == ".svg") return "image/svg+xml";
  if (extension == file_extensions::ICO) return content_types::ICO;
  if (extension == ".pdf") return "application/pdf";
  if (extension == ".zip") return "application/zip";
  if (extension == ".mp4") return "video/mp4";
  if (extension == ".mp3") return "audio/mpeg";

  return "application/octet-stream"; // Default for unknown types
}

} // namespace utils
} // namespace router
