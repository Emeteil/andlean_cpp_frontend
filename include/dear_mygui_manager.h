#include <GL/glew.h>
#include <SDL2/SDL.h>

#include <iostream>
#include <chrono>
#include <thread>
#include <cmath>
#include <functional>

#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_sdl2.h"
#include "imgui.h"
#include "implot.h"

class DearMyGuiManager
{
    private:
        std::string windowTitle = "Backend start";
        std::vector<std::function<void()>> widgets;
        std::vector<std::function<void(SDL_Event)>> eventHandlers;

        SDL_Window* window;
        SDL_GLContext gl_context;
        ImGuiIO* io;

        void Initialize();
        void SendToRender();
        void Destroy();
    public:
        DearMyGuiManager();
        ~DearMyGuiManager();
        void AddWidget(std::function<void()> widget);
        void AddEventHandler(std::function<void(SDL_Event)> handler);

        bool MainHandler();

};