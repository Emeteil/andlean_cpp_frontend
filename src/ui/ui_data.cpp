#include "ui/ui_components.h"
#include <imgui.h>

void DrawDataWindow(bool& open, UserData& user, std::mutex& mtx)
{
    if (!open) return;

    if (ImGui::Begin("Data Information", &open))
    {
        std::lock_guard<std::mutex> lock(mtx);
        
        if (ImGui::CollapsingHeader("User Info", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Columns(2, "UserInfoCols", false);
            ImGui::SetColumnWidth(0, 100);

            ImGui::Text("User:");
            ImGui::NextColumn();
            ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "%s", user.user.c_str());
            ImGui::NextColumn();

            ImGui::Text("Latitude:");
            ImGui::NextColumn();
            ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "%.6f", user.location.latitude);
            ImGui::NextColumn();

            ImGui::Text("Longitude:");
            ImGui::NextColumn();
            ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "%.6f", user.location.longitude);
            ImGui::NextColumn();

            ImGui::Columns(1);
        }

        if (ImGui::CollapsingHeader("Mobile Networks", ImGuiTreeNodeFlags_DefaultOpen))
        {
            for (const auto& net : user.mobileNetworks)
            {
                if (ImGui::TreeNodeEx(net.cellIdentity.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
                {
                    ImGui::Columns(2, "NetCols", false);
                    ImGui::SetColumnWidth(0, 100);

                    ImGui::Text("Type:");
                    ImGui::NextColumn();
                    ImGui::TextColored(ImVec4(0.4f, 0.4f, 1.0f, 1.0f), "%s", net.networkType.c_str());
                    ImGui::NextColumn();

                    ImGui::Text("MCC:");
                    ImGui::NextColumn();
                    ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.6f, 1.0f), "%s", net.mcc.c_str());
                    ImGui::NextColumn();

                    ImGui::Text("MNC:");
                    ImGui::NextColumn();
                    ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.6f, 1.0f), "%s", net.mnc.c_str());
                    ImGui::NextColumn();

                    ImGui::Text("PCI:");
                    ImGui::NextColumn();
                    ImGui::TextColored(ImVec4(0.6f, 1.0f, 0.6f, 1.0f), "%d", net.pci);
                    ImGui::NextColumn();

                    ImGui::Text("TAC:");
                    ImGui::NextColumn();
                    ImGui::TextColored(ImVec4(0.6f, 1.0f, 0.6f, 1.0f), "%d", net.tac);
                    ImGui::NextColumn();

                    ImGui::Text("RSRP:");
                    ImGui::NextColumn();
                    ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "%d dBm", net.rsrp);
                    ImGui::NextColumn();

                    ImGui::Text("RSRQ:");
                    ImGui::NextColumn();
                    ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "%d dBm", net.rsrq);
                    ImGui::NextColumn();

                    ImGui::Text("RSSI:");
                    ImGui::NextColumn();
                    ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "%d dBm", net.rssi);
                    ImGui::NextColumn();

                    ImGui::Columns(1);
                    ImGui::TreePop();
                }
            }
        }
    }
    ImGui::End();
}