cmake_minimum_required(VERSION 3.12)
project(mser-cpp VERSION 1.0.0 LANGUAGES CXX)

# C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Compiler flags
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Library source files
set(MSER_SOURCES
    src/mser.cpp
    src/steady_state_detector.cpp
)

# Create static library
add_library(mser STATIC ${MSER_SOURCES})

# Create shared library
add_library(mser_shared SHARED ${MSER_SOURCES})
set_target_properties(mser_shared PROPERTIES OUTPUT_NAME mser)

# Headers to install
set(MSER_HEADERS
    include/mser/mser.h
    include/mser/steady_state_detector.h
    include/mser/types.h
)

# Examples
option(BUILD_EXAMPLES "Build examples" ON)
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Tests
option(BUILD_TESTS "Build tests" ON)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Installation
install(TARGETS mser mser_shared
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(FILES ${MSER_HEADERS}
    DESTINATION include/mser
)

install(FILES docs/algorithm.md docs/api.md docs/references.md
    DESTINATION share/doc/mser-cpp
)

# Package config
include(CMakePackageConfigHelpers)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/mser-cpp-config-version.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/mser-cpp-config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/mser-cpp-config.cmake"
    INSTALL_DESTINATION lib/cmake/mser-cpp
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/mser-cpp-config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/mser-cpp-config-version.cmake"
    DESTINATION lib/cmake/mser-cpp
)

# Summary
message(STATUS "mser-cpp Configuration Summary:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build examples: ${BUILD_EXAMPLES}")
message(STATUS "  Build tests: ${BUILD_TESTS}")