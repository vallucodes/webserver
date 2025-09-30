#include "MultipartParser.hpp"

namespace router::handlers {

MultipartData MultipartParser::parseMultipartData(const std::string& body, const std::string& boundary) {
  MultipartData result;

  // Find file boundaries
  const size_t fileStart = body.find(boundary);
  if (fileStart == std::string::npos) {
    return result; // isValid remains false
  }

  const size_t fileEnd = body.find(boundary, fileStart + boundary.length());
  const std::string filePart = body.substr(fileStart, fileEnd - fileStart);

  // Extract filename
  result.filename = extractFilename(filePart);
  if (result.filename.empty()) {
    return result; // isValid remains false
  }

  // Extract file content
  result.content = extractFileContent(filePart);
  if (result.content.empty()) {
    return result; // isValid remains false
  }

  // Clean the content
  result.content = cleanFileContent(result.content);
  result.isValid = true;

  return result;
}

std::string MultipartParser::extractFilename(const std::string& filePart) {
  const size_t filenamePos = filePart.find("filename=\"");
  if (filenamePos == std::string::npos) {
    return "";
  }

  // filename=\" -> 10
  const size_t filenameEnd = filePart.find("\"", filenamePos + 10);
  if (filenameEnd == std::string::npos) {
    return "";
  }

  return filePart.substr(filenamePos + 10, filenameEnd - filenamePos - 10);
}

std::string MultipartParser::extractFileContent(const std::string& filePart) {
  const size_t contentStart = filePart.find("\r\n\r\n");
  if (contentStart == std::string::npos) {
    return "";
  }

  return filePart.substr(contentStart + 4);
}

std::string MultipartParser::cleanFileContent(const std::string& content) {
  std::string cleaned = content;

  // Remove trailing \r\n if present
  if (cleaned.length() >= 2 && cleaned.substr(cleaned.length() - 2) == "\r\n") {
    cleaned = cleaned.substr(0, cleaned.length() - 2);
  }

  return cleaned;
}

} // namespace router::handlers
