project(OCEAN LANGUAGES CXX)

file(GLOB src "*.cpp" "*.hpp")
add_executable(${PROJECT_NAME} ${src})
target_link_libraries(${PROJECT_NAME} PUBLIC
    imgui
    SDL2::SDL2 
    SDL2::SDL2main 
    SDL2_IMGUI_BASE
    OceanFFT
    )

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${assetsDir} 
    $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${shadersDir}
    $<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders)