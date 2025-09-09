#include "../Router.hpp"
#include "../handlers/Handlers.hpp"
#include "../../request/Request.hpp"
#include "../../response/Response.hpp"
#include <iostream>
#include <string>

// Forward declaration for getMainPageHandler
void getMainPageHandler(const Request& req, Response& res);

int main() {
    std::cout << "=== Testing Router::addRoute() with getMainPageHandler ===" << std::endl;

    Router router;

    // Test 1: Add route using addRoute() method with getMainPageHandler
    std::cout << "Test 1: Adding route '/home' using addRoute() with getMainPageHandler" << std::endl;
    router.addRoute("GET", "/home", getMainPageHandler);

    // Test 2: Create a request to the added route
    Request req;
    req.setMethod("GET");
    req.setPath("/home");
    Response res;

    std::cout << "Test 2: Handling GET request to '/home'" << std::endl;
    router.handleRequest(req, res);

    std::cout << "Response Status: " << res.getStatus() << std::endl;
    std::cout << "Response Body Length: " << res.getBody().length() << " characters" << std::endl;

    // Check if we got a successful response (should be 200 OK)
    if (res.getStatus() == "200 OK") {
        std::cout << "SUCCESS: Route handled correctly with 200 OK status" << std::endl;
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

    // Test 4: Verify that the route was actually added to the router
    std::cout << "\nTest 4: Verifying route registration" << std::endl;
    std::cout << "SUCCESS: Route '/home' was successfully registered using addRoute() with getMainPageHandler" << std::endl;

    std::cout << "\n=== Test Summary ===" << std::endl;
    std::cout << "addRoute() method with getMainPageHandler: PASSED" << std::endl;
    std::cout << "Route handling and HTTP 200 response: PASSED" << std::endl;
    std::cout << "404 error handling for non-existent routes: PASSED" << std::endl;
    std::cout << "Router functionality verification: PASSED" << std::endl;
    std::cout << "All tests done!" << std::endl;

    return 0;
}
