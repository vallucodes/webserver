#include "../../response/Response.hpp"
#include "../Router.hpp"
#include "../Request.hpp"
#include <iostream>
#include <cassert>

void test_basic_get_route() {
    std::cout << "Test 1: Basic GET route" << std::endl;
    Router r;

    // Register route
    r.get("/", [](const Request& /*req*/, Response& res) {
        res.setStatus("200 OK");
        res.setBody("Hello 42!");
    });

    Request req;
    req.setMethod("GET");
    req.setPath("/");

    Response res;
    r.handleRequest(req, res);

    assert(res.getStatus() == "200 OK");
    assert(res.getBody() == "Hello 42!");
    std::cout << "Basic GET route test passed" << std::endl;
}

void test_404_not_found() {
    std::cout << "Test 2: 404 Not Found" << std::endl;
    Router r;

    // Register only one route
    r.get("/home", [](const Request& /*req*/, Response& res) {
        res.setBody("Home page");
    });

    // Try to access a different path
    Request req;
    req.setMethod("GET");
    req.setPath("/nonexistent");

    Response res;
    r.handleRequest(req, res);

    assert(res.getStatus() == "404 Not Found");
    assert(res.getBody() == "Route not found");
    std::cout << "404 Not Found test passed" << std::endl;
}

void test_different_paths() {
    std::cout << "Test 3: Multiple paths" << std::endl;
    Router r;

    r.get("/api/users", [](const Request& /*req*/, Response& res) {
        res.setBody("Users API");
    });

    r.get("/api/posts", [](const Request& /*req*/, Response& res) {
        res.setBody("Posts API");
    });

    // Test first route
    Request req1;
    req1.setMethod("GET");
    req1.setPath("/api/users");
    Response res1;
    r.handleRequest(req1, res1);
    assert(res1.getBody() == "Users API");

    // Test second route
    Request req2;
    req2.setMethod("GET");
    req2.setPath("/api/posts");
    Response res2;
    r.handleRequest(req2, res2);
    assert(res2.getBody() == "Posts API");

    std::cout << "Multiple paths test passed" << std::endl;
}

int main() {
    std::cout << "Running Router Tests..." << std::endl;
    std::cout << std::string(40, '=') << std::endl;

    test_basic_get_route();
    test_404_not_found();
    test_different_paths();

    std::cout << std::string(40, '=') << std::endl;
    std::cout << "All router tests passed!" << std::endl;

    return 0;
}
