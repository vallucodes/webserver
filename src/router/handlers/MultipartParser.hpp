#pragma once

#include <string>
#include <vector>

namespace router::handlers {

/**
 * @brief Structure to hold parsed multipart data
 */
struct MultipartData {
  std::string filename;
  std::string content;
  bool isValid = false;
};

/**
 * @brief Multipart form data parser
 */
class MultipartParser {
public:
  /**
   * @brief Parse multipart form data from request body
   * @param body The request body containing multipart data
   * @param boundary The boundary string for multipart data
   * @return Parsed multipart data
   */
  static MultipartData parseMultipartData(const std::string& body, const std::string& boundary);

private:
  /**
   * @brief Extract filename from multipart part
   * @param filePart The multipart part containing file data
   * @return Extracted filename or empty string if not found
   */
  static std::string extractFilename(const std::string& filePart);

  /**
   * @brief Extract file content from multipart part
   * @param filePart The multipart part containing file data
   * @return Extracted file content or empty string if not found
   */
  static std::string extractFileContent(const std::string& filePart);

  /**
   * @brief Clean file content by removing trailing CRLF
   * @param content The file content to clean
   * @return Cleaned file content
   */
  static std::string cleanFileContent(const std::string& content);
};

} // namespace router::handlers
