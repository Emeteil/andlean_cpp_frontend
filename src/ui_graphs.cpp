#include "ui_components.h"
#include <imgui.h>
#include <implot.h>
#include <vector>
#include <string>
#include <functional>
#include <algorithm>

void DrawGraphWindow(bool& open, std::vector<SignalData>& signals, std::string& selectedIdentity, std::mutex& mtx)
{
    if (!open) return;

    static const ImVec4 colorPool[] = {
        ImVec4(0.95f, 0.36f, 0.41f, 1.0f),
        ImVec4(0.24f, 0.82f, 0.50f, 1.0f),
        ImVec4(0.28f, 0.56f, 0.98f, 1.0f),
        ImVec4(0.98f, 0.70f, 0.23f, 1.0f),
        ImVec4(0.60f, 0.35f, 0.85f, 1.0f),
        ImVec4(0.15f, 0.75f, 0.75f, 1.0f),
        ImVec4(1.00f, 0.40f, 0.70f, 1.0f),
        ImVec4(0.50f, 0.80f, 1.00f, 1.0f)
    };
    const int poolSize = sizeof(colorPool) / sizeof(ImVec4);

    if (ImGui::Begin("Signal Graphs", &open))
    {
        std::lock_guard<std::mutex> lock(mtx);

        if (signals.empty())
        {
            ImGui::Text("Oops");
            ImGui::End();
            return;
        }

        if (selectedIdentity.empty())
            selectedIdentity = signals[0].cellIdentity;

        ImGui::SetNextItemWidth(200);
        if (ImGui::BeginCombo("Highlight Cell", selectedIdentity.c_str()))
        {
            for (const auto& s : signals)
            {
                bool is_selected = (selectedIdentity == s.cellIdentity);
                if (ImGui::Selectable(s.cellIdentity.c_str(), is_selected))
                    selectedIdentity = s.cellIdentity;
                
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        float plotHeight = ImGui::GetContentRegionAvail().y / 3.05f;

        auto DrawPlot = [&](const char* label, const std::function<const std::vector<double>&(const SignalData&)>& getter)
        {
            if (ImPlot::BeginPlot(label, ImVec2(-1, plotHeight)))
            {
                ImPlot::SetupAxes("Time", "Value", ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
                
                for (const auto& sig : signals)
                {
                    const auto& values = getter(sig);
                    if (values.empty()) continue;

                    ImVec4 color = colorPool[std::hash<std::string>{}(sig.cellIdentity) % poolSize];

                    if (sig.cellIdentity == selectedIdentity)
                    {
                        ImPlot::SetNextLineStyle(color, 2.5f);
                        ImPlot::PlotLine(sig.cellIdentity.c_str(), sig.timestamps.data(), values.data(), (int)sig.timestamps.size());
                    }
                    else
                    {
                        ImVec4 darkedColor = color;
                        darkedColor.w = 0.42f;
                        ImPlot::SetNextLineStyle(darkedColor, 1.0f);
                        ImPlot::PlotLine(sig.cellIdentity.c_str(), sig.timestamps.data(), values.data(), (int)sig.timestamps.size());
                    }
                }
                ImPlot::EndPlot();
            }
        };

        DrawPlot("RSRP (dBm)", [](const SignalData& s) -> const std::vector<double>& { return s.rsrp_values; });
        DrawPlot("RSRQ (dB)", [](const SignalData& s) -> const std::vector<double>& { return s.rsrq_values; });
        DrawPlot("RSSI (dBm)", [](const SignalData& s) -> const std::vector<double>& { return s.rssi_values; });
    }
    ImGui::End();
}