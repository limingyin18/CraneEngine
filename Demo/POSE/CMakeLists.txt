project(POSE LANGUAGES CXX)

find_package(PythonInterp REQUIRED)

find_package(Torch REQUIRED
            NO_MODULE
            PATHS /home/ylm/Library/libtorch
            NO_DEFAULT_PATH)
find_package(OpenCV)

include_directories( ${OpenCV_INCLUDE_DIRS} )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")

file(GLOB src "*.cpp" "*.hpp")
add_executable(${PROJECT_NAME} ${src})
target_link_libraries(${PROJECT_NAME} PUBLIC
    glslang
    imgui
    SDL2::SDL2 
    SDL2::SDL2main 
    SDL2_IMGUI_BASE
    ${TORCH_LIBRARIES}
    ${OpenCV_LIBS}
    )
target_compile_features(${PROJECT_NAME} PUBLIC cxx_range_for)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${assetsDir} 
    $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${shadersDir}
    $<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders)