# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.20)

project (Crane)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Vulkan REQUIRED)
find_package(SDL2 REQUIRED)

# Include sub-projects.
add_subdirectory(third_party)
add_subdirectory(assetlib)
add_subdirectory (CraneEngine)

add_subdirectory(shaders)

set(assetsDir ${CMAKE_CURRENT_SOURCE_DIR}/assets_export)
set(shadersDir ${CMAKE_CURRENT_SOURCE_DIR}/shaders/spv)
add_subdirectory (Demo)
