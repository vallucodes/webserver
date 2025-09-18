#include <iostream>
// int main() {
//     std::cout << __cplusplus << std::endl;
//     return 0;
// }
// test_string_view.cpp
#include <string_view>
#include <iostream>
int main() {
    std::string_view sv = "Hello, C++17!";
    std::cout << sv << std::endl;
    return 0;
}
