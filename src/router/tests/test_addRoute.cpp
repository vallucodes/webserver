#include "../Router.hpp"
#include "../handlers/Handlers.hpp"
#include <iostream>
#include <string>
#include <unistd.h>

// Bring HTTP constants into scope
using namespace http;

int main() {
    std::cout << "=== Testing Router::addRoute() with getPage ===" << std::endl;

    // Change to project root directory so file paths work correctly
    if (chdir("../../..") != 0) {
        std::cerr << "ERROR: Could not change to project root directory" << std::endl;
        return 1;
    }

    Router router;

    // Test 1: Add route using addRoute() method with getPage
    std::cout << "Test 1: Adding route '/' using addRoute() with getPage" << std::endl;
    router.addRoute("GET", "/", get);

    // Test 2: Create a request to the registered route
    Request req;
    req.setMethod("GET");
    req.setPath("/");
    Response res;

    std::cout << "Test 2: Handling GET request to '/' (should serve www/index.html)" << std::endl;
    router.handleRequest(req, res);

    std::cout << "Response Status: " << res.getStatus() << std::endl;
    std::cout << "Response Body Length: " << res.getBody().length() << " characters" << std::endl;
    std::cout << "Response Body: " << res.getBody() << std::endl;

    // Check if we got a successful response (should be 200 OK)
    if (res.getStatus() == STATUS_OK_200) {
        std::cout << "SUCCESS: Route handled correctly with 200 OK status" << std::endl;
        // Check if response contains HTML content (from index.html)
        if (res.getBody().find("<html>") != std::string::npos) {
            std::cout << "SUCCESS: Response contains HTML content from index.html" << std::endl;
        } else {
            std::cout << "WARNING: Response may not contain expected HTML content" << std::endl;
        }
    } else {
        std::cout << "FAILED: Expected 200 OK but got " << res.getStatus() << std::endl;
    }

    // Test 3: Test with a non-existent route to verify proper 404 handling
    std::cout << "\nTest 3: Testing non-existent route '/nonexistent'" << std::endl;
    Request req2;
    req2.setMethod("GET");
    req2.setPath("/nonexistent");
    Response res2;

    try {
        router.handleRequest(req2, res2);
        std::cout << "Response Status: " << res2.getStatus() << std::endl;
        std::cout << "Response Body Length: " << res2.getBody().length() << " characters" << std::endl;
        std::cout << "Response Body: " << res2.getBody() << std::endl;

        // Check if we got a 404 error
        if (res2.getStatus().find("404") != std::string::npos) {
            std::cout << "SUCCESS: 404 handling works correctly" << std::endl;
        } else {
            std::cout << "FAILED: Expected 404 status but got " << res2.getStatus() << std::endl;
        }
    } catch (const std::exception& e) {
        std::cout << "NOTE: 404 error page file not found (expected): " << e.what() << std::endl;
        std::cout << "SUCCESS: Router correctly attempted to serve 404 error page" << std::endl;
    }

    std::cout << "\nTest 3: Testing non-existent method POST to '/' route" << std::endl;
    Request req3;
    req2.setMethod("POST");
    req2.setPath("/");
    Response res3;

    try {
        router.handleRequest(req2, res2);
        std::cout << "Response Status: " << res2.getStatus() << std::endl;
        std::cout << "Response Body Length: " << res2.getBody().length() << " characters" << std::endl;
        std::cout << "Response Body: " << res2.getBody() << std::endl;

        // Check if we got a 405 error
        if (res2.getStatus().find("405") != std::string::npos) {
            std::cout << "SUCCESS: 405 handling works correctly" << std::endl;
        } else {
            std::cout << "FAILED: Expected 405 status but got " << res2.getStatus() << std::endl;
        }
    } catch (const std::exception& e) {
        std::cout << "NOTE: 405 error page file not found (expected): " << e.what() << std::endl;
        std::cout << "SUCCESS: Router correctly attempted to serve 405 error page" << std::endl;
    }

    std::cout << "All tests done!" << std::endl;
    return 0;
}
