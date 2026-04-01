#include "ui_components.h"
#include <imgui.h>
#include <implot.h>
#include <vector>
#include <string>

void DrawMapWindow(bool& open, std::vector<SignalData>& signals, std::string& selectedIdentity, std::mutex& mtx)
{
    if (!open) return;

    if (ImGui::Begin("Map", &open))
    {
        std::lock_guard<std::mutex> lock(mtx);

        // if (signals.empty())
        // {
        //     ImGui::Text("Oops");
        //     ImGui::End();
        //     return;
        // }

        ImPlot::BeginPlot("##ImOsmMapPlot");

        // Параметры для отображения картинки
        ImVec2 _uv0{0, 1};        // Top-left of the texture
        ImVec2 _uv1{1, 0};        // Bottom-right of the texture
        ImVec4 _tint{1, 1, 1, 1}; // Цвет, накладываемый поверх нашего изображения
        ImVec2 bmin{0, 0};
        ImVec2 bmax{256, 256};

        ImPlot::EndPlot();
    }
    ImGui::End();
}