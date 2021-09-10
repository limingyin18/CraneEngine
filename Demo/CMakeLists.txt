project(Demo)

# sdl2
find_package(SDL2 CONFIG REQUIRED)

# imgui
set(IMGUI_DIR "../third_party/imgui")
file(GLOB srcImgui "${IMGUI_DIR}/*.cpp")
add_library(imgui
    "${srcImgui}"
    "${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_sdl.cpp")
target_include_directories(imgui PUBLIC "${IMGUI_DIR}")
target_link_libraries(imgui Vulkan::Vulkan SDL2::SDL2)

# sdl imgui base
add_subdirectory(SDL2_BASE)
add_subdirectory(SDL2_IMGUI_BASE)
add_subdirectory(BASIC)
add_subdirectory(CLOTH)
add_subdirectory(MINI_GAME)
