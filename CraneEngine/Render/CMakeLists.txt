# CMakeList.txt : CMake project for CraneVision, include source and define
# project specific logic here.
#
project(Render)

# Add source to this project's executable.
file(GLOB SRC "*.hpp" "*.cpp")
add_library(${PROJECT_NAME} ${SRC})

message(${SRC})

target_link_directories(${PROJECT_NAME} PUBLIC ${Vulkan_INCLUDE_DIRS}/../Lib)

target_link_libraries(${PROJECT_NAME} PUBLIC
                      spdlog
                      vulkan
                      vma
                      #SPIRV-Tools
                      spirv_reflect
                      SPIRV
                      glslang
                      eigen
                      TracyClient
                      #tbb
                      )
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")                        
# TODO: Add tests and install targets if needed.
