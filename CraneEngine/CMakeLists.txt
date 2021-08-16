# CMakeList.txt : CMake project for CraneVision, include source and define
# project specific logic here.
#
project(CraneVision)

# Add source to this project's executable.
file(GLOB SRC "*.hpp" "*.cpp")
add_library(${PROJECT_NAME} ${SRC})

target_link_libraries(${PROJECT_NAME} PUBLIC
                      spdlog
                      Vulkan::Vulkan
                      vma)
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")                        
# TODO: Add tests and install targets if needed.
