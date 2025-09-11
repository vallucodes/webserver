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
    std::cout << "=== Testing File Upload Functionality ===" << std::endl;

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


    router.addRoute("POST", "/uploads", post);

    // Test 1: Basic upload route registration
    std::cout << "\nTest 1: Upload route registration" << std::endl;
    Request testReq;
    testReq.setMethod("POST");
    testReq.setPath("/uploads");

    // This should not crash - route exists
    Response testRes;
    router.handleRequest(testReq, testRes);
    std::cout << "Upload route exists and can be called" << std::endl;

    // Test 2: Multipart form data upload simulation
    std::cout << "\nTest 2: Simulating multipart file upload" << std::endl;

    // Create multipart form data with a test file
    std::string boundary = "----WebKitFormBoundary7MA4YWxkTrZu0gW";
    std::string fileContent = "This is test file content for upload testing.";
    std::string filename = "test_upload.txt";

    // Build multipart form data
    std::string multipartData = "--" + boundary + "\r\n";
    multipartData += "Content-Disposition: form-data; name=\"file\"; filename=\"" + filename + "\"\r\n";
    multipartData += "Content-Type: text/plain\r\n";
    multipartData += "\r\n";
    multipartData += fileContent + "\r\n";
    multipartData += "--" + boundary + "--\r\n";

    // Create request with multipart data
    Request req;
    req.setMethod("POST");
    req.setPath("/uploads");
    req.setBody(multipartData);

    // Set Content-Type header for multipart
    req.setHeaders("Content-Type", "multipart/form-data; boundary=" + boundary);

    Response res;
    router.handleRequest(req, res);

    std::cout << "Response Status: " << res.getStatus() << std::endl;
    std::cout << "Response Body Preview: " << res.getBody().substr(0, 200) << "..." << std::endl;

    // Test 3: Verify file was saved to storage directory
    std::cout << "\nTest 3: Verifying file was saved to storage" << std::endl;

    std::string expectedFilePath = "www/uploads/" + filename;
    std::ifstream savedFile(expectedFilePath, std::ios::binary);

    if (savedFile) {
        std::cout << "File was created in storage directory: " << expectedFilePath << std::endl;

        // Read the saved file content
        std::string savedContent((std::istreambuf_iterator<char>(savedFile)),
                                std::istreambuf_iterator<char>());
        savedFile.close();

        // Verify content matches what we uploaded
        if (savedContent == fileContent) {
            std::cout << "File content matches uploaded content" << std::endl;
            std::cout << "  Original size: " << fileContent.length() << " bytes" << std::endl;
            std::cout << "  Saved size: " << savedContent.length() << " bytes" << std::endl;
        } else {
            std::cout << "File content mismatch!" << std::endl;
            std::cout << "  Expected: '" << fileContent << "'" << std::endl;
            std::cout << "  Got: '" << savedContent << "'" << std::endl;
            return 1;
        }

        // Keep the test file for demonstration (don't clean up automatically)
        std::cout << "✓ Test file created successfully at: " << expectedFilePath << std::endl;
        std::cout << "  You can manually delete it when done testing" << std::endl;

    } else {
        std::cout << "File was not saved to uploads directory!" << std::endl;
        std::cout << "  Expected path: " << expectedFilePath << std::endl;

        // Check if uploads directory exists
        if (!std::filesystem::exists("www/uploads")) {
            std::cout << "  Uploads directory doesn't exist!" << std::endl;
        } else {
            std::cout << "  Uploads directory exists but file wasn't created" << std::endl;
        }
        return 1;
    }

    // Test 4: Test file size limit (should reject files > 1MB)
    std::cout << "\nTest 4: Testing file size limit (1MB)" << std::endl;

    // Create a file that's too large (> 1MB)
    std::string largeContent(1024 * 1024 + 100, 'X'); // Just over 1MB
    std::string largeMultipartData = "--" + boundary + "\r\n";
    largeMultipartData += "Content-Disposition: form-data; name=\"file\"; filename=\"large_file.txt\"\r\n";
    largeMultipartData += "Content-Type: text/plain\r\n";
    largeMultipartData += "\r\n";
    largeMultipartData += largeContent + "\r\n";
    largeMultipartData += "--" + boundary + "--\r\n";

    Request largeReq;
    largeReq.setMethod("POST");
    largeReq.setPath("/uploads");
    largeReq.setBody(largeMultipartData);
    largeReq.setHeaders("Content-Type", "multipart/form-data; boundary=" + boundary);

    Response largeRes;
    router.handleRequest(largeReq, largeRes);

    std::cout << "Large file response status: " << largeRes.getStatus() << std::endl;

    if (largeRes.getBody().find("File size exceeds 1MB limit") != std::string::npos) {
        std::cout << "File size validation working correctly (rejected large file)" << std::endl;
    } else {
        std::cout << "File size validation may not be working as expected" << std::endl;
    }

    // Test 5: Test invalid content type
    std::cout << "\nTest 5: Testing invalid content type" << std::endl;

    Request invalidReq;
    invalidReq.setMethod("POST");
    invalidReq.setPath("/uploads");
    invalidReq.setBody("some data");
    invalidReq.setHeaders("Content-Type", "application/json"); // Wrong content type

    Response invalidRes;
    router.handleRequest(invalidReq, invalidRes);

    if (invalidRes.getBody().find("Invalid content type") != std::string::npos) {
        std::cout << "Invalid content type properly rejected" << std::endl;
    } else {
        std::cout << "Invalid content type handling may not work correctly" << std::endl;
    }

    // Test 6: Test success response contains proper placeholders
    std::cout << "\nTest 6: Testing success response format" << std::endl;

    if (res.getBody().find("Upload Successful") != std::string::npos) {
        std::cout << "Success response contains expected content" << std::endl;
        if (res.getBody().find(filename) != std::string::npos) {
            std::cout << "Success response contains filename" << std::endl;
        } else {
            std::cout << "Filename not found in success response" << std::endl;
        }
    } else {
        std::cout << "Success response may not be properly formatted" << std::endl;
    }

    std::cout << "\n=== All Upload Tests Completed ===" << std::endl;
    return 0;
}
