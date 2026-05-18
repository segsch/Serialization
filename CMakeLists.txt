cmake_minimum_required(VERSION 3.15)
project(CarCameraCalibrationSerialization LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(JSON_HEADER_URL "https://raw.githubusercontent.com/nlohmann/json/v3.11.2/single_include/nlohmann/json.hpp")
set(JSON_HEADER_PATH "${CMAKE_BINARY_DIR}/external/nlohmann/json.hpp")

file(DOWNLOAD ${JSON_HEADER_URL} ${JSON_HEADER_PATH} SHOW_PROGRESS STATUS download_status)
list(GET download_status 0 download_result)
if(NOT download_result EQUAL 0)
    message(FATAL_ERROR "Could not download nlohmann/json header: ${download_status}")
endif()

include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.15.2
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

include_directories(${CMAKE_BINARY_DIR}/external)
include_directories(src)

add_executable(calibration_app
  src/main.cpp
)

add_executable(serialization_tests
  tests/SerializationTests.cpp
)

target_link_libraries(serialization_tests PRIVATE GTest::gtest GTest::gmock GTest::gtest_main)

enable_testing()
add_test(NAME SerializationTests COMMAND serialization_tests)
