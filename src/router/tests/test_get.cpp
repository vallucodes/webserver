#include "../Router.hpp"
#include "../handlers/Handlers.hpp"
#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>
#include <cassert>
#include <unistd.h>

// Bring HTTP constants into scope
using namespace http;

int main() {
    std::cout << "=== Testing GET Request Handler Functionality ===" << std::endl;

    // Change to project root directory so file paths work correctly
    if (chdir("../../..") != 0) {
        std::cerr << "ERROR: Could not change to project root directory" << std::endl;
        return 1;
    }

    Router router;

    // Setup routes (same as main server)
    router.addRoute("GET", "/", get);
    router.addRoute("GET", "/index.html", get);
    router.addRoute("GET", "/upload.html", get);
    router.addRoute("GET", "/upload_error.html", get);
    router.addRoute("GET", "/upload_success.html", get);

    // Test 1: GET route registration
    std::cout << "\nTest 1: GET route registration" << std::endl;
    Request testReq;
    testReq.setMethod("GET");
    testReq.setPath("/");

    Response testRes;
    router.handleRequest(testReq, testRes);
    std::cout << "GET route exists and can be called" << std::endl;

    // Test 2: Test root path (/) serves index.html
    std::cout << "\nTest 2: Testing root path serving index.html" << std::endl;

    Request rootReq;
    rootReq.setMethod("GET");
    rootReq.setPath("/");

    Response rootRes;
    router.handleRequest(rootReq, rootRes);

    std::cout << "Root path response status: " << rootRes.getStatus() << std::endl;

    if (rootRes.getStatus().find("200") != std::string::npos) {
        std::cout << "Root path properly serves content (200 OK)" << std::endl;

        // Check if response contains HTML content (basic validation)
        if (rootRes.getBody().find("<html") != std::string::npos ||
            rootRes.getBody().find("<!DOCTYPE html") != std::string::npos) {
            std::cout << "Response contains HTML content" << std::endl;
        } else {
            std::cout << "WARNING: Response may not contain expected HTML content" << std::endl;
        }
    } else {
        std::cout << "Root path did not return 200 OK" << std::endl;
        return 1;
    }

    // Test 3: Test /index.html path
    std::cout << "\nTest 3: Testing /index.html path" << std::endl;

    Request indexReq;
    indexReq.setMethod("GET");
    indexReq.setPath("/index.html");

    Response indexRes;
    router.handleRequest(indexReq, indexRes);

    std::cout << "Index path response status: " << indexRes.getStatus() << std::endl;

    if (indexRes.getStatus().find("200") != std::string::npos) {
        std::cout << "Index.html path properly serves content (200 OK)" << std::endl;

        // Both root and index.html should return the same content
        if (rootRes.getBody() == indexRes.getBody()) {
            std::cout << "Root and index.html return identical content" << std::endl;
        } else {
            std::cout << "WARNING: Root and index.html return different content" << std::endl;
        }
    } else {
        std::cout << "Index.html path did not return 200 OK" << std::endl;
        return 1;
    }

    // Test 4: Test serving other HTML files
    std::cout << "\nTest 4: Testing other HTML files" << std::endl;

    Request uploadReq;
    uploadReq.setMethod("GET");
    uploadReq.setPath("/upload.html");

    Response uploadRes;
    router.handleRequest(uploadReq, uploadRes);

    std::cout << "Upload.html response status: " << uploadRes.getStatus() << std::endl;

    if (uploadRes.getStatus().find("200") != std::string::npos) {
        std::cout << "Upload.html properly serves content (200 OK)" << std::endl;

        // Check content type header
        std::string contentType = uploadRes.getHeader("Content-Type");
        if (contentType.find("text/html") != std::string::npos) {
            std::cout << "Correct Content-Type header: " << contentType << std::endl;
        } else {
            std::cout << "WARNING: Incorrect or missing Content-Type header: " << contentType << std::endl;
        }
    } else {
        std::cout << "Upload.html did not return 200 OK" << std::endl;
        return 1;
    }

    // Test 5: Test non-existent file (should return 404)
    std::cout << "\nTest 5: Testing non-existent file" << std::endl;

    Request nonexistentReq;
    nonexistentReq.setMethod("GET");
    nonexistentReq.setPath("/nonexistent.html");

    Response nonexistentRes;
    router.handleRequest(nonexistentReq, nonexistentRes);

    std::cout << "Non-existent file response status: " << nonexistentRes.getStatus() << std::endl;

    if (nonexistentRes.getStatus().find("404") != std::string::npos) {
        std::cout << "Non-existent file properly returns 404 Not Found" << std::endl;
    } else {
        std::cout << "WARNING: Non-existent file did not return 404" << std::endl;
    }

    // Test 6: Test directory traversal protection
    std::cout << "\nTest 6: Testing directory traversal protection" << std::endl;

    Request traversalReq;
    traversalReq.setMethod("GET");
    traversalReq.setPath("/../../../etc/passwd");

    Response traversalRes;
    router.handleRequest(traversalReq, traversalRes);

    std::cout << "Directory traversal response status: " << traversalRes.getStatus() << std::endl;

    // Should return 404 since the file doesn't exist, not allow traversal
    if (traversalRes.getStatus().find("404") != std::string::npos) {
        std::cout << "Directory traversal properly blocked (404 Not Found)" << std::endl;
    } else {
        std::cout << "WARNING: Directory traversal may not be properly blocked" << std::endl;
    }

    // Test 7: Test empty path
    std::cout << "\nTest 7: Testing empty path" << std::endl;

    Request emptyReq;
    emptyReq.setMethod("GET");
    emptyReq.setPath("");

    Response emptyRes;
    router.handleRequest(emptyReq, emptyRes);

    std::cout << "Empty path response status: " << emptyRes.getStatus() << std::endl;

    if (emptyRes.getStatus().find("404") != std::string::npos) {
        std::cout << "Empty path properly handled (404 Not Found)" << std::endl;
    } else {
        std::cout << "WARNING: Empty path handling may not be correct" << std::endl;
    }

    std::cout << "\n=== All GET Tests Completed ===" << std::endl;
    return 0;
}
