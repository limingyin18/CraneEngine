cmake_minimum_required(VERSION 3.20)

project(asset-baker)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

#find_package(nlohmann_json CONFIG REQUIRED)
#find_package(lz4 CONFIG REQUIRED)
#find_package(tinyobjloader CONFIG REQUIRED)
#find_package(glm CONFIG REQUIRED)
#find_package(assimp CONFIG REQUIRED)

add_subdirectory(third_party)

file(GLOB assetlib_src "../assetlib/*.cpp")


# Add source to this project's executable.
add_executable (baker
"asset_main.cpp"
${assetlib_src})
#set_property(TARGET baker PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:extra>")

target_include_directories(baker PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_include_directories(baker PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../assetlib")

target_link_libraries(baker PUBLIC 
                        nlohmann_json
                        lz4
                        tinyobjloader
                        glm 
                        assimp
                        tinygltf
                        )
