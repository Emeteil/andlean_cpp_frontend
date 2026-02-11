#include "dear_mygui_manager.h"

void DearMyGuiManager::Initialize()
{
    // 1) Инициализация SDL
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
    window = SDL_CreateWindow(
        windowTitle.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        1024, 768, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    gl_context = SDL_GL_CreateContext(window);

    // 2) Инициализация контекста Dear Imgui
    ImGui::CreateContext();
    ImPlot::CreateContext();

    // Ввод\вывод
    io = &ImGui::GetIO();
    io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Включить Keyboard Controls
    io->ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Включить Gamepad Controls
    io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Включить Docking

    // 2.1) Привязка Imgui к SDL2 и OpenGl backend'ам
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init("#version 330");
    SetupStyle();
}

void DearMyGuiManager::SetupStyle()
{
    auto& style = ImGui::GetStyle();
    auto* colors = style.Colors;

    style.WindowRounding = 8.0f;
    style.ChildRounding = 8.0f;
    style.FrameRounding = 6.0f;
    style.PopupRounding = 8.0f;
    style.ScrollbarRounding = 12.0f;
    style.GrabRounding = 8.0f;
    style.TabRounding = 8.0f;
    style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
    style.WindowPadding = ImVec2(15, 15);
    style.ItemSpacing = ImVec2(10, 10);
    style.FramePadding = ImVec2(8, 6);

    colors[ImGuiCol_Text] = ImVec4(0.95f, 0.96f, 0.98f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.10f, 0.14f, 0.95f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.12f, 0.12f, 0.16f, 1.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.12f, 0.12f, 0.16f, 0.98f);
    colors[ImGuiCol_Border] = ImVec4(0.20f, 0.20f, 0.28f, 0.50f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.16f, 0.16f, 0.22f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.24f, 0.24f, 0.32f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.20f, 0.20f, 0.28f, 1.00f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.08f, 0.08f, 0.12f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.10f, 0.10f, 0.14f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.20f, 0.26f, 0.36f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.26f, 0.34f, 0.46f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.30f, 0.40f, 0.54f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.20f, 0.26f, 0.36f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.34f, 0.46f, 1.00f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.30f, 0.40f, 0.54f, 1.00f);
    colors[ImGuiCol_Separator] = ImVec4(0.20f, 0.20f, 0.28f, 1.00f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.35f, 0.61f, 1.00f, 1.00f);
}

void DearMyGuiManager::SendToRender()
{
    // 3.3) Отправляем на рендер;
    ImGui::Render();
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    SDL_GL_SwapWindow(window);
}

void DearMyGuiManager::Destroy()
{
    // 4) Закрываем приложение безопасно.
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();
    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

DearMyGuiManager::DearMyGuiManager()
{
    Initialize();
}

DearMyGuiManager::~DearMyGuiManager()
{
    Destroy();
}

bool DearMyGuiManager::MainHandler()
{
    // 3.0) Обработка event'ов (inputs, window resize, mouse moving, etc.);
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        std::cout << "Processing some event: "<< event.type << std::endl;
        ImGui_ImplSDL2_ProcessEvent(&event);
        if (event.type == SDL_QUIT)
            return false;
    }

    // 3.1) Начинаем создавать новый фрейм;
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
    ImGui::DockSpaceOverViewport(0, nullptr, ImGuiDockNodeFlags_None);

    for (auto widget : widgets)
    {
        widget();
    }

    SendToRender();

    return true;
}

void DearMyGuiManager::AddWidget(std::function<void()> widget)
{
    widgets.push_back(widget);
}

void DearMyGuiManager::AddEventHandler(std::function<void(SDL_Event)> handler)
{
    eventHandlers.push_back(handler);
}