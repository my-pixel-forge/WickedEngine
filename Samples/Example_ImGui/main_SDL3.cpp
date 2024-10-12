// WickedEngineTests.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "sdl3.h"
#include "ImGui/imgui_impl_sdl3.h"
#include <SDL3/SDL_events.h>
#include <stdexcept>

int sdl_loop(Example_ImGui &tests)
{
    bool quit = false;
    while (!quit)
    {
        tests.Run();
        SDL_Event event;
        while(SDL_PollEvent(&event)){
            switch(event.type){
                case SDL_EVENT_QUIT:
                    quit = true;
                    break;
                case SDL_EVENT_WINDOW_CLOSE_REQUESTED: // exit tests
                    quit = true;
                    break;
                case SDL_EVENT_WINDOW_RESIZED:
                    // Tells the engine to reload window configuration (size and dpi)
                    tests.SetWindow(tests.window);
                    break;
                case SDL_EVENT_WINDOW_FOCUS_LOST: //TODO
                    tests.is_window_active = false;
                    break;
                case SDL_EVENT_WINDOW_FOCUS_GAINED:
                    tests.is_window_active = true;
                    if (wi::shadercompiler::GetRegisteredShaderCount() > 0)
                    {
                        std::thread([] {
                            wi::backlog::post("[Shader check] Started checking " + std::to_string(wi::shadercompiler::GetRegisteredShaderCount()) + " registered shaders for changes...");
                            if (wi::shadercompiler::CheckRegisteredShadersOutdated())
                            {
                                wi::backlog::post("[Shader check] Changes detected, initiating reload...");
                                wi::eventhandler::Subscribe_Once(wi::eventhandler::EVENT_THREAD_SAFE_POINT, [](uint64_t userdata) {
                                    wi::renderer::ReloadShaders();
                                    });
                            }
                            else
                            {
                                wi::backlog::post("[Shader check] All up to date");
                            }
                            }).detach();
                    }
                    break;
                default:
                    break;
            }
            wi::input::sdlinput::ProcessEvent(event);
            ImGui_ImplSDL3_ProcessEvent(&event);
        }
    }

    return 0;
}

int main(int argc, char *argv[])
{
    Example_ImGui exampleImGui;
    // TODO: Place code here.

    wi::arguments::Parse(argc, argv);

    sdl3::sdlsystem_ptr_t system = sdl3::make_sdlsystem(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_GAMEPAD);
    if (!system) {
        throw sdl3::SDLError("Error creating SDL2 system");
    }

    sdl3::window_ptr_t window = sdl3::make_window(
            "Wicked Engine Tests",
            1280, 800,
            SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
    if (!window) {
        throw sdl3::SDLError("Error creating window");
    }

    exampleImGui.SetWindow(window.get());

    int ret = sdl_loop(exampleImGui);

    SDL_Quit();
    return ret;
}
