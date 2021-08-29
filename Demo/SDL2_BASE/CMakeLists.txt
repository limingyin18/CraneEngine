project(SDL2_BASE LANGUAGES CXX)

add_executable(${PROJECT_NAME} "SDL2_BASE.cpp" "main.cpp")
target_link_libraries(${PROJECT_NAME} PUBLIC
    SDL2::SDL2 
    SDL2::SDL2main 
    CraneEngine 
    )
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${assetsDir} 
    $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets)

add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${shadersDir} 
    $<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders)
