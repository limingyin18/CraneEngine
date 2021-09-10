cmake_minimum_required(VERSION 3.20)

set(CMAKE_GENERATOR_PLATFORM x64)

project(asset-baker)

find_package(nlohmann_json CONFIG REQUIRED)
find_package(lz4 CONFIG REQUIRED)
find_package(tinyobjloader CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)

set(CMAKE_CXX_STANDARD 17)

file(GLOB assetlib_src "../assetlib/*.cpp")


# Add source to this project's executable.
add_executable (baker
"asset_main.cpp"
${assetlib_src})
#set_property(TARGET baker PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:extra>")

target_include_directories(baker PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_include_directories(baker PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../assetlib")

find_path(TINYGLTF_INCLUDE_DIRS "tiny_gltf.h")
target_include_directories(baker PRIVATE ${TINYGLTF_INCLUDE_DIRS})
target_link_libraries(baker PUBLIC 
                        nlohmann_json::nlohmann_json
                        lz4::lz4
                        tinyobjloader::tinyobjloader
                        glm::glm 
                        assimp::assimp
                        )
