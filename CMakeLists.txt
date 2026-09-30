cmake_minimum_required(VERSION 3.25)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(frame_window_counter VERSION 1.0.0 LANGUAGES CXX)

# Source files
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp src/*.hpp)

# Define the mod shared library
add_library(${PROJECT_NAME} SHARED ${SOURCES})

# Target GD version
if (NOT DEFINED GEODE_GD_VERSION)
    set(GEODE_GD_VERSION "2.208" CACHE INTERNAL "Geode GD Version")
endif()

# Check for GEODE_SDK
if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "GEODE_SDK environment variable is not set. Please run 'geode sdk install' or set GEODE_SDK manually.")
endif()

# Include Geode SDK
add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)

# Configure mod and packaging
# DONT_INSTALL ensures it does not try to install to local Geometry Dash folder
setup_geode_mod(${PROJECT_NAME} DONT_INSTALL)
