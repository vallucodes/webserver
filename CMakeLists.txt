cmake_minimum_required(VERSION 3.14)
project(WebServerTest)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include Google Test subdirectory
add_subdirectory(libs/gtest)

include(GoogleTest)

# Enable testing
enable_testing()

# Use the general structure
file(GLOB TEST_SOURCES test/unit/*.cpp)
file(GLOB SRC_PARSER   src/parser/*.cpp)
file(GLOB SRC_REQUEST  src/request/*.cpp)
file(GLOB SRC_MESSAGE  src/message/*.cpp)
file(GLOB SRC_RESPONSE src/response/*.cpp)

foreach(test_file ${TEST_SOURCES})
	get_filename_component(test_name ${test_file} NAME_WE)
	add_executable(${test_name}
        ${test_file}
        ${SRC_PARSER}
        ${SRC_REQUEST}
        ${SRC_MESSAGE}
        ${SRC_RESPONSE}
        src/server/HelperFunctions.cpp
    )
	# Add include directories for each test
	target_include_directories(${test_name} PRIVATE
		${CMAKE_SOURCE_DIR}/src/server
		${CMAKE_SOURCE_DIR}/inc
		${CMAKE_SOURCE_DIR}/src/parser
		${CMAKE_SOURCE_DIR}/src/request
		${CMAKE_SOURCE_DIR}/src/response
		${CMAKE_SOURCE_DIR}/src/message
	)

	target_compile_definitions(${test_name} PRIVATE MAX_RESPONSE_SIZE=8)

	target_link_libraries(${test_name} PRIVATE gtest_main)
	gtest_discover_tests(${test_name})
endforeach()

# Debug build settings
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")

# Build and run commands:
# cmake -S . -B build
# cmake --build build
# ctest --test-dir build --verbose
#
# To run specific tests:
# cmake --build build --target request_validation_tests
# ctest -R request_validation_tests --verbose
