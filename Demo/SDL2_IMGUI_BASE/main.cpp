#include <iostream>
#include <exception>
#include <memory>

#include <SDL2/SDL.h>

#include "SDL2_IMGUI_BASE.hpp"

using namespace std;

static const int WIDTH = 1280;
static const int HEIGHT = 720;

static const string TITLE = "SDL2 Testing";

int main(int argc, char **argv)
{
    SDL_Init(SDL_INIT_VIDEO);
    SDL_WindowFlags window_flags = SDL_WindowFlags(SDL_WINDOW_VULKAN | SDL_WINDOW_BORDERLESS);
    shared_ptr<SDL_Window> window{SDL_CreateWindow(
                                      TITLE.c_str(),           //window title
                                      SDL_WINDOWPOS_UNDEFINED, //window position x (don't care)
                                      SDL_WINDOWPOS_UNDEFINED, //window position y (don't care)
                                      WIDTH,                   //window width in pixels
                                      HEIGHT,                  //window height in pixels
                                      window_flags),
                                  SDL_DestroyWindow};

    try
    {
        SDL2_IMGUI_BASE sdl2_imgui_base(window);
        sdl2_imgui_base.init();
        SDL_Event event;
        do
        {
            if (SDL_PollEvent(&event) != 0)
            {
                ImGui_ImplSDL2_ProcessEvent(&event);
            }
            sdl2_imgui_base.update();
        } while (event.type != SDL_QUIT);
    }
    catch (const exception &e)
    {
        LOGE(e.what());
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}