cmake_minimum_required(VERSION 3.14)
project(WebServerTest)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include Google Test subdirectory
add_subdirectory(libs/gtest)

# Enable testing
enable_testing()

# Add test executable(s)
# add_executable(<test name> <test/testFile.cpp> <file to be tested location>)
# add_executable(testRequestComplete test/requestComplete.cpp src/server/HelperFunctions.cpp)
# add_executable(testParseRequest test/parseRequestTest.cpp src/parser/Parser.cpp src/request/Request.cpp src/parser/ParserUtils.cpp src/message/AMessage.cpp)
add_executable(testDecodeChunkedBody test/decodeChunkedBody.cpp src/server/HelperFunctions.cpp)


# Headers locations
target_include_directories(testDecodeChunkedBody PRIVATE
    ${CMAKE_SOURCE_DIR}/src/server
    ${CMAKE_SOURCE_DIR}/inc
    ${CMAKE_SOURCE_DIR}/parser
    ${CMAKE_SOURCE_DIR}/request
    ${CMAKE_SOURCE_DIR}/response
    ${CMAKE_SOURCE_DIR}/message
)

# Link Google Test libraries to your tests
# target_link_libraries(testRequestComplete PRIVATE gtest_main)
# target_link_libraries(testParseRequest PRIVATE gtest_main)
target_link_libraries(testDecodeChunkedBody PRIVATE gtest_main)

include(GoogleTest)
# gtest_discover_tests(testRequestComplete)
# gtest_discover_tests(testParseRequest)
gtest_discover_tests(testDecodeChunkedBody)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")

# cmake -S . -B build
# cmake --build build
# ctest --test-dir build --verbose

# cmake --build build --target testParseRequest
# ctest -R testParseRequest --verbose


