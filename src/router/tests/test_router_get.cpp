#include "Router.hpp"
#include <iostream>

int main() {
    Router router;

    // Register handlers
    router.get("/hello", [](const Request& req, Response& res) {
        (void)req;
        res.setStatus("200 OK");
        res.setBody("Hello from GET!");
    });

    router.post("/hello", [](const Request& /*req*/, Response& res) {
        res.setStatus("200 OK");
        res.setBody("Hello from POST!");
    });

    router.del("/hello", [](const Request& /*req*/, Response& res) {
        res.setStatus("200 OK");
        res.setBody("Hello from DELETE!");
    });

    // Test 1: GET /hello (should succeed)
    Request req1;
    req1.setMethod("GET");
    req1.setPath("/hello");
    Response res1;
    router.handleRequest(req1, res1);
    std::cout << "Test 1 Status: " << res1.getStatus() << std::endl;
    std::cout << "Test 1 Body: " << res1.getBody() << std::endl << std::endl;

    // Test 2: POST /hello (should succeed)
    Request req2;
    req2.setMethod("POST");
    req2.setPath("/hello");
    Response res2;
    router.handleRequest(req2, res2);
    std::cout << "Test 2 Status: " << res2.getStatus() << std::endl;
    std::cout << "Test 2 Body: " << res2.getBody() << std::endl << std::endl;

    // Test 3: GET /nonexistent (should return 404)
    Request req3;
    req3.setMethod("GET");
    req3.setPath("/nonexistent");
    Response res3;
    router.handleRequest(req3, res3);
    std::cout << "Test 3 Status: " << res3.getStatus() << std::endl;
    std::cout << "Test 3 Body: " << res3.getBody() << std::endl << std::endl;

    // Test 4: PUT /hello (should return 405 - no PUT handler registered)
    Request req4;
    req4.setMethod("PUT");
    req4.setPath("/hello");
    Response res4;
    router.handleRequest(req4, res4);
    std::cout << "Test 4 Status: " << res4.getStatus() << std::endl;
    std::cout << "Test 4 Body: " << res4.getBody() << std::endl << std::endl;

    return 0;
}
