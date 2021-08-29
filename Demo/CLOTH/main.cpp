#include <iostream>
#include <exception>
#include <memory>

#include <SDL2/SDL.h>
#include <imgui.h>

#include "CLOTH.hpp"

using namespace std;

static const int WIDTH = 1280;
static const int HEIGHT = 720;

static const string TITLE = "SDL2 Testing";

void processInputEvent(SDL_Event &event, Crane::Input &input);

int main(int argc, char **argv)
{
    SDL_Init(SDL_INIT_VIDEO);
    SDL_WindowFlags window_flags{SDL_WINDOW_VULKAN};
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
        CLOTH clothApp(window);
        clothApp.init();
        SDL_Event event;
        do
        {
            if (SDL_PollEvent(&event) != 0)
            {
                processInputEvent(event, clothApp.input);

                ImGui_ImplSDL2_ProcessEvent(&event);
            }
            clothApp.update();
        } while (event.type != SDL_QUIT);
    }
    catch (const exception &e)
    {
        LOGE(e.what());
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

void processInputEvent(SDL_Event &event, Crane::Input &input)
{
    switch (event.type)
    {
    case SDL_MOUSEMOTION:
    {
        input.setMousePosition(event.motion.y, event.motion.x);
        break;
    }
    case SDL_MOUSEWHEEL:
    {
        input.setMouseScroll(event.wheel.y, event.wheel.x);
        break;
    }
    case SDL_MOUSEBUTTONDOWN:
    {
        switch (event.button.button)
        {
        case SDL_BUTTON_LEFT:
            input.buttonDown("mouseButtonLeft");
            break;
        case SDL_BUTTON_RIGHT:
            input.buttonDown("mouseButtonRight");
            break;
        case SDL_BUTTON_MIDDLE:
            input.buttonDown("mouseButtonMiddle");
            break;
        default:
            break;
        }
        break;
    }
    case SDL_MOUSEBUTTONUP:
    {
        switch (event.button.button)
        {
        case SDL_BUTTON_LEFT:
            input.buttonUp("mouseButtonLeft");
            break;
        case SDL_BUTTON_RIGHT:
            input.buttonUp("mouseButtonRight");
            break;
        case SDL_BUTTON_MIDDLE:
            input.buttonUp("mouseButtonMiddle");
            break;
        default:
            break;
        }
        break;
    }
    case SDL_KEYDOWN:
    {
        switch (event.key.keysym.sym)
        {
        case SDLK_LCTRL:
            input.buttonDown("lctrl");
            break;
        case SDLK_f:
            input.buttonDown("f");
            break;
        case SDLK_a:
            input.buttonDown("a");
            break;
        case SDLK_d:
            input.buttonDown("d");
            break;
        case SDLK_s:
            input.buttonDown("s");
            break;
        case SDLK_w:
            input.buttonDown("w");
            break;
        case SDLK_q:
            input.buttonDown("q");
            break;
        case SDLK_e:
            input.buttonDown("e");
            break;

        default:
            break;
        }
        break;
    }
    case SDL_KEYUP:
    {
        switch (event.key.keysym.sym)
        {
        case SDLK_LCTRL:
            input.buttonUp("lctrl");
            break;
        case SDLK_f:
            input.buttonUp("f");
            break;
        case SDLK_a:
            input.buttonUp("a");
            break;
        case SDLK_w:
            input.buttonUp("w");
            break;
        case SDLK_s:
            input.buttonUp("s");
            break;
        case SDLK_d:
            input.buttonUp("d");
            break;
        case SDLK_q:
            input.buttonUp("q");
            break;
        case SDLK_e:
            input.buttonUp("e");
            break;

        default:
            break;
        }
        break;
    }
    default:
        break;
    }
}