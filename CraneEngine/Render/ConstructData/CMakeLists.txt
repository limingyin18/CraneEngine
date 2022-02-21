project(ConstructData)

file(GLOB SRC "*.hpp" "*.cpp")
add_library(${PROJECT_NAME} ${SRC})

target_link_libraries(${PROJECT_NAME} PUBLIC
                      spdlog
                      Vulkan::Vulkan
                      vma
                      eigen
                      )
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")