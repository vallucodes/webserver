#include "Router.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include <iostream>

int main() {
    Router router;

    // Register test routes manually
    router.get("/hello", [](const Request& /*req*/, Response& res) {
        res.setStatus("200 OK");
        res.setBody("<html><body><h1>Hello, World!</h1></body></html>");
    });

    router.post("/hello", [](const Request& /*req*/, Response& res) {
        res.setStatus("200 OK");
        res.setBody("<html><body><h1>Hello, POST!</h1></body></html>");
    });

    router.del("/hello", [](const Request& /*req*/, Response& res) {
        res.setStatus("200 OK");
        res.setBody("<html><body><h1>Hello, DELETE!</h1></body></html>");
    });

    // Test 1: GET /hello (should succeed if configured)
    Request req1;
    req1.setMethod("GET");
    req1.setPath("/hello");
    Response res1;
    router.handleRequest(req1, res1);
    std::cout << "Test 1 Status: " << res1.getStatus() << std::endl;
    std::cout << "Test 1 Body: " << res1.getBody() << std::endl << std::endl;

    // Test 2: POST /hello (should succeed if configured)
    Request req2;
    req2.setMethod("POST");
    req2.setPath("/hello");
    Response res2;
    router.handleRequest(req2, res2);
    std::cout << "Test 2 Status: " << res2.getStatus() << std::endl;
    std::cout << "Test 2 Body: " << res2.getBody() << std::endl << std::endl;

    // Test 3: DELETE /hello (should succeed if configured)
    Request req3;
    req3.setMethod("DELETE");
    req3.setPath("/hello");
    Response res3;
    router.handleRequest(req3, res3);
    std::cout << "Test 3 Status: " << res3.getStatus() << std::endl;
    std::cout << "Test 3 Body: " << res3.getBody() << std::endl << std::endl;

    // Test 4: GET /nonexistent (should return 404)
    Request req4;
    req4.setMethod("GET");
    req4.setPath("/nonexistent");
    Response res4;
    router.handleRequest(req4, res4);
    std::cout << "Test 4 Status: " << res4.getStatus() << std::endl;
    std::cout << "Test 4 Body: " << res4.getBody() << std::endl << std::endl;

    // Test 5: PUT /hello (should return 405 - no PUT handler configured)
    Request req5;
    req5.setMethod("PUT");
    req5.setPath("/hello");
    Response res5;
    router.handleRequest(req5, res5);
    std::cout << "Test 5 Status: " << res5.getStatus() << std::endl;
    std::cout << "Test 5 Body: " << res5.getBody() << std::endl << std::endl;

    return 0;
}
