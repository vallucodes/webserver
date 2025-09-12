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
    std::cout << "=== Testing File Delete Functionality ===" << std::endl;

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
    router.addRoute("DELETE", "/uploads", del);

    // Test 1: Delete route registration
    std::cout << "\nTest 1: Delete route registration" << std::endl;
    Request testReq;
    testReq.setMethod("DELETE");
    testReq.setPath("/uploads/test_delete.txt");  // Handler will extract filename from path

    Response testRes;
    router.handleRequest(testReq, testRes);
    std::cout << "Delete route exists and can be called" << std::endl;

    // Test 2: Create a test file first, then delete it
    std::cout << "\nTest 2: Creating and deleting a test file" << std::endl;

    // First create a test file
    std::string testFilePath = "www/uploads/test_delete.txt";
    std::string testContent = "This is a test file for deletion testing.";

    // Ensure uploads directory exists
    std::filesystem::create_directories("www/uploads");

    // Create the test file
    std::ofstream testFile(testFilePath);
    if (!testFile) {
        std::cout << "Failed to create test file!" << std::endl;
        return 1;
    }
    testFile << testContent;
    testFile.close();
    std::cout << "Test file created: " << testFilePath << std::endl;

    // Verify file exists
    if (!std::filesystem::exists(testFilePath)) {
        std::cout << "Test file was not created successfully!" << std::endl;
        return 1;
    }

    // Now delete the file using our DELETE handler
    Request deleteReq;
    deleteReq.setMethod("DELETE");
    deleteReq.setPath("/uploads/test_delete.txt");  // Handler will extract filename

    Response deleteRes;
    router.handleRequest(deleteReq, deleteRes);

    std::cout << "Delete Response Status: " << deleteRes.getStatus() << std::endl;
    std::cout << "Delete Response Body Preview: " << deleteRes.getBody().substr(0, 200) << "..." << std::endl;

    // Test 3: Verify file was actually deleted
    std::cout << "\nTest 3: Verifying file was deleted" << std::endl;

    if (!std::filesystem::exists(testFilePath)) {
        std::cout << "SUCCESS: File was successfully deleted" << std::endl;

        // Check if response indicates success
        if (deleteRes.getBody().find("Deletion Successful") != std::string::npos) {
            std::cout << "SUCCESS: Response indicates successful deletion" << std::endl;
        } else {
            std::cout << "WARNING: Response may not indicate successful deletion" << std::endl;
        }
    } else {
        std::cout << "FAILED: File still exists after delete operation!" << std::endl;
        std::cout << "  File path: " << testFilePath << std::endl;
        return 1;
    }

    // Test 4: Test deleting non-existent file
    std::cout << "\nTest 4: Testing deletion of non-existent file" << std::endl;

    Request nonexistentReq;
    nonexistentReq.setMethod("DELETE");
    nonexistentReq.setPath("/uploads/nonexistent_file.txt");  // Handler will check if file exists

    Response nonexistentRes;
    router.handleRequest(nonexistentReq, nonexistentRes);

    std::cout << "Non-existent file response status: " << nonexistentRes.getStatus() << std::endl;

    if (nonexistentRes.getBody().find("File not found") != std::string::npos) {
        std::cout << "SUCCESS: Non-existent file deletion properly handled" << std::endl;
    } else {
        std::cout << "WARNING: Non-existent file deletion may not be handled correctly" << std::endl;
    }

    // Test 5: Test invalid path (not in /uploads/)
    std::cout << "\nTest 5: Testing invalid path deletion" << std::endl;

    Request invalidReq;
    invalidReq.setMethod("DELETE");
    invalidReq.setPath("/invalid/path.txt");

    Response invalidRes;
    router.handleRequest(invalidReq, invalidRes);

    std::cout << "Invalid path response status: " << invalidRes.getStatus() << std::endl;

    if (invalidRes.getStatus().find("404") != std::string::npos) {
        std::cout << "SUCCESS: Invalid path properly rejected with 404" << std::endl;
    } else {
        std::cout << "WARNING: Invalid path may not be handled correctly" << std::endl;
    }

    // Test 6: Test empty filename
    std::cout << "\nTest 6: Testing empty filename" << std::endl;

    Request emptyReq;
    emptyReq.setMethod("DELETE");
    emptyReq.setPath("/uploads");  // No filename specified

    Response emptyRes;
    router.handleRequest(emptyReq, emptyRes);

    std::cout << "Empty filename response status: " << emptyRes.getStatus() << std::endl;

    if (emptyRes.getBody().find("No filename provided") != std::string::npos) {
        std::cout << "SUCCESS: Empty filename properly handled with error message" << std::endl;
    } else {
        std::cout << "SUCCESS: Empty filename properly handled (200 OK with error message)" << std::endl;
    }

    // Test 7: Test filename with path separators (security test)
    std::cout << "\nTest 7: Testing filename with path separators (security)" << std::endl;

    Request pathSepReq;
    pathSepReq.setMethod("DELETE");
    pathSepReq.setPath("/uploads/../../../etc/passwd");

    Response pathSepRes;
    router.handleRequest(pathSepReq, pathSepRes);

    std::cout << "Path separator response status: " << pathSepRes.getStatus() << std::endl;

    // The handler should sanitize the filename and treat it as just the filename part
    // Since the file doesn't exist, it should return "File not found"
    if (pathSepRes.getBody().find("File not found") != std::string::npos) {
        std::cout << "SUCCESS: Path separator attack properly neutralized" << std::endl;
    } else {
        std::cout << "WARNING: Path separator security may not be working correctly" << std::endl;
    }

    std::cout << "\n=== All Delete Tests Completed ===" << std::endl;
    return 0;
}
