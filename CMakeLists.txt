﻿# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.18)

project (Crane)

#set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
SET(CMAKE_BUILD_TYPE "Debug")

#SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb")
#SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")
set(ENV{VULKAN_SDK} "C:\\VulkanSDK\\1.3.216.0")
find_package(Vulkan REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
 find_package(TBB CONFIG REQUIRED)
#find_package(glslang REQUIRED)

# Include sub-projects.
add_subdirectory(third_party)
add_subdirectory(assetlib)
add_subdirectory (CraneEngine)

add_subdirectory(shaders)

set(assetsDir ${CMAKE_CURRENT_SOURCE_DIR}/assets_export)
set(shadersDir ${CMAKE_CURRENT_SOURCE_DIR}/shaders/spv)
add_subdirectory (Demo)
