project(CraneEngine)

add_subdirectory(Physics)
add_subdirectory(Render)

# Add source to this project's executable.
file(GLOB SRC "*.hpp" "*.cpp")
add_library(${PROJECT_NAME} ${SRC})

target_link_libraries(${PROJECT_NAME} PUBLIC
                      Render
                      Physics
                      assetlib
                      glslang
                      SPIRV
                      VkFFT)
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")                        
# TODO: Add tests and install targets if needed.

add_subdirectory(Module/OceanFFT)