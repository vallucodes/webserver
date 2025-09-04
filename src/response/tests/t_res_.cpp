#include "../Response.hpp"
#include <assert.h>

int main() {
    Response res;

    // test res.setStatus() and res.getStatus()
    res.setStatus("200 OK");
    std::cout << "Test: res.setStatus() and res.getStatus()"<< std::endl;
    assert(res.getStatus() == "200 OK");
    std::cout << res.getStatus() << std::endl;

    // test res.setBody() and res.getBody()
    res.setBody("Hello World!");
    std::cout << "Test: res.setBody() and res.getBody()"<< std::endl;
    assert(res.getBody() == "Hello World!");
    std::cout << res.getBody() << std::endl;

    std::cout << "All tests passed" << std::endl;

    return 0;
}
