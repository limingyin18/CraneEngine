project(Render)

add_subdirectory(ConstructData)
add_subdirectory(ConstructRender)

# Add source to this project's executable.
file(GLOB SRC "*.hpp" "*.cpp")
add_library(${PROJECT_NAME} ${SRC})

target_link_directories(${PROJECT_NAME} PUBLIC ${Vulkan_INCLUDE_DIRS}/../Lib)

target_link_libraries(${PROJECT_NAME} PUBLIC
                      spdlog
                      Vulkan::Vulkan
                      vma
                      SPIRV-Tools
                      spirv_reflect
                      SPIRV
                      glslang
                      eigen
                      TracyClient
                      TBB::tbb
                      ConstructData
                      ConstructRender
                      )
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")                        
# TODO: Add tests and install targets if needed.
