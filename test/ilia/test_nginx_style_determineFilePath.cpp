#include <iostream>
#include <string>
#include <vector>
#include <cassert>

// Mock the necessary structures and constants
struct Location {
    std::string location;
    std::vector<std::string> allowed_methods;
    std::string index;
    bool autoindex;
    std::string cgi_path;
    std::vector<std::string> cgi_ext;
    std::string upload_path;
    std::string return_url;
};

namespace page {
    const std::string WWW = "www";
    const std::string INDEX_HTML = "www/index.html";
}

// Nginx-style determineFilePath function
std::string determineFilePath(const std::string& requestPath, const Location* location) {
  // Handle root path - always serve index.html
  if (requestPath == "/") {
    return page::INDEX_HTML;
  }

  // If no location context, serve directly from www root
  if (!location) {
    return page::WWW + requestPath;
  }

  // Get the location prefix (e.g., "/api", "/static/", "/uploads")
  const std::string& locationPrefix = location->location;

  // Check if request path starts with location prefix
  if (requestPath.length() < locationPrefix.length() ||
      requestPath.substr(0, locationPrefix.length()) != locationPrefix) {
    // Path doesn't match location prefix, serve as-is from www root
    return page::WWW + requestPath;
  }

  // Extract the path after the location prefix
  std::string pathAfterPrefix = requestPath.substr(locationPrefix.length());

  // Handle empty path after prefix (e.g., "/api" -> "")
  if (pathAfterPrefix.empty()) {
    pathAfterPrefix = "/";
  }
  // Ensure path starts with "/" (e.g., "users" -> "/users")
  else if (pathAfterPrefix[0] != '/') {
    pathAfterPrefix = "/" + pathAfterPrefix;
  }

  return page::WWW + pathAfterPrefix;
}

void testNginxStyleDetermineFilePath() {
    std::cout << "Testing nginx-style determineFilePath() function...\n";
    std::cout << "=" << std::string(60, '=') << "\n";

    // Test 1: Root path without location
    std::string result1 = determineFilePath("/", nullptr);
    std::cout << "Test 1:  '/' -> '" << result1 << "' (expected: 'www/index.html')\n";
    assert(result1 == "www/index.html");

    // Test 2: Regular path without location
    std::string result2 = determineFilePath("/style.css", nullptr);
    std::cout << "Test 2:  '/style.css' -> '" << result2 << "' (expected: 'www/style.css')\n";
    assert(result2 == "www/style.css");

    // Test 3: Path that doesn't match location prefix
    Location loc1;
    loc1.location = "/api";
    std::string result3 = determineFilePath("/style.css", &loc1);
    std::cout << "Test 3:  '/style.css' with location '/api' -> '" << result3 << "' (expected: 'www/style.css')\n";
    assert(result3 == "www/style.css");

    // Test 4: Path that matches location prefix exactly
    Location loc2;
    loc2.location = "/api";
    std::string result4 = determineFilePath("/api", &loc2);
    std::cout << "Test 4:  '/api' with location '/api' -> '" << result4 << "' (expected: 'www/')\n";
    assert(result4 == "www/");

    // Test 5: Path that matches location prefix with subpath
    Location loc3;
    loc3.location = "/api";
    std::string result5 = determineFilePath("/api/users", &loc3);
    std::cout << "Test 5:  '/api/users' with location '/api' -> '" << result5 << "' (expected: 'www/users')\n";
    assert(result5 == "www/users");

    // Test 6: Location with trailing slash
    Location loc4;
    loc4.location = "/static/";
    std::string result6 = determineFilePath("/static/images/logo.png", &loc4);
    std::cout << "Test 6:  '/static/images/logo.png' with location '/static/' -> '" << result6 << "' (expected: 'www/images/logo.png')\n";
    assert(result6 == "www/images/logo.png");

    // Test 7: Location with trailing slash, exact match
    Location loc5;
    loc5.location = "/uploads/";
    std::string result7 = determineFilePath("/uploads/", &loc5);
    std::cout << "Test 7:  '/uploads/' with location '/uploads/' -> '" << result7 << "' (expected: 'www/')\n";
    assert(result7 == "www/");

    // Test 8: Short path that doesn't match longer location
    Location loc6;
    loc6.location = "/very/long/path";
    std::string result8 = determineFilePath("/very", &loc6);
    std::cout << "Test 8:  '/very' with location '/very/long/path' -> '" << result8 << "' (expected: 'www/very')\n";
    assert(result8 == "www/very");

    std::cout << "=" << std::string(60, '=') << "\n";
    std::cout << "✅ All tests passed! Nginx-style determineFilePath() is working correctly.\n";
}

int main() {
    testNginxStyleDetermineFilePath();
    return 0;
}
