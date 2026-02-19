#include "ui_widget.h"
#include "dear_mygui_manager.h"
#include <imgui.h>
#include <implot.h>

void CreateMobileNetworkWidget(UserData& currentUser, std::mutex& dataMutex, std::vector<SignalData>& signalData)
{
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize, ImGuiCond_Always);

    ImGui::Begin("User Data", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
    {
        std::lock_guard<std::mutex> lock(dataMutex);

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        
        ImVec2 window_size = ImGui::GetWindowSize();
        float split_y = window_size.y / 2;
        
        ImGui::BeginChild("Data Section", ImVec2(0, split_y), true);
        {
            ImGui::SeparatorText("User Information");
            
            ImGui::Columns(2, "User Info", false);
            ImGui::SetColumnWidth(0, 120);

            ImGui::Text("User:");
            ImGui::NextColumn();
            ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "%s", currentUser.user.c_str());
            ImGui::NextColumn();

            ImGui::Columns(1);
            
            ImGui::SeparatorText("Location Data");
            
            if (currentUser.location.latitude != 0.0)
            {
                ImGui::Columns(2, "Location Data", false);
                ImGui::SetColumnWidth(0, 120);

                ImGui::Text("Latitude:");
                ImGui::NextColumn();
                ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "%.6f", currentUser.location.latitude);
                ImGui::NextColumn();

                ImGui::Text("Longitude:");
                ImGui::NextColumn();
                ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "%.6f", currentUser.location.longitude);
                ImGui::NextColumn();

                ImGui::Text("Altitude:");
                ImGui::NextColumn();
                ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.4f, 1.0f), "%.6f", currentUser.location.altitude);
                ImGui::NextColumn();

                ImGui::Text("Time:");
                ImGui::NextColumn();
                ImGui::TextColored(ImVec4(1.0f, 0.4f, 1.0f, 1.0f), "%lld", currentUser.location.time);
                ImGui::NextColumn();

                ImGui::Columns(1);
            }

            ImGui::SeparatorText("Mobile Networks");
            
            for (const auto& network : currentUser.mobileNetworks)
            {
                ImGui::Separator();
                ImGui::TextColored(ImVec4(0.4f, 0.4f, 1.0f, 1.0f), "Network Type: %s", network.networkType.c_str());
                
                ImGui::Columns(2, "Network Data", false);
                ImGui::SetColumnWidth(0, 120);

                ImGui::Text("CellIdentity:");
                ImGui::NextColumn();
                ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.4f, 1.0f), "%s", network.cellIdentity.c_str());
                ImGui::NextColumn();

                ImGui::Text("MCC:");
                ImGui::NextColumn();
                ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.6f, 1.0f), "%s", network.mcc.c_str());
                ImGui::NextColumn();

                ImGui::Text("MNC:");
                ImGui::NextColumn();
                ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.6f, 1.0f), "%s", network.mnc.c_str());
                ImGui::NextColumn();

                ImGui::Text("PCI:");
                ImGui::NextColumn();
                ImGui::TextColored(ImVec4(0.6f, 1.0f, 0.6f, 1.0f), "%d", network.pci);
                ImGui::NextColumn();

                ImGui::Text("TAC:");
                ImGui::NextColumn();
                ImGui::TextColored(ImVec4(0.6f, 1.0f, 0.6f, 1.0f), "%d", network.tac);
                ImGui::NextColumn();

                ImGui::Text("Bands:");
                ImGui::NextColumn();
                ImGui::TextColored(ImVec4(0.8f, 0.8f, 1.0f, 1.0f), "%s", network.bands.c_str());
                ImGui::NextColumn();

                ImGui::Text("SignalStrength:");
                ImGui::NextColumn();
                ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.4f, 1.0f), "%s", network.signalStrength.c_str());
                ImGui::NextColumn();

                ImGui::Text("RSRP:");
                ImGui::NextColumn();
                ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "%d", network.rsrp);
                ImGui::NextColumn();

                ImGui::Text("RSRQ:");
                ImGui::NextColumn();
                ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "%d", network.rsrq);
                ImGui::NextColumn();

                ImGui::Text("RSSI:");
                ImGui::NextColumn();
                ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "%d", network.rssi);
                ImGui::NextColumn();

                ImGui::Text("TimingAdvance:");
                ImGui::NextColumn();
                ImGui::TextColored(ImVec4(0.4f, 1.0f, 1.0f, 1.0f), "%d", network.timingAdvance);
                ImGui::NextColumn();

                ImGui::Text("Time:");
                ImGui::NextColumn();
                ImGui::TextColored(ImVec4(1.0f, 0.4f, 1.0f, 1.0f), "%lld", network.time);
                ImGui::NextColumn();

                ImGui::Columns(1);
            }
        }
        ImGui::EndChild();

        ImGui::BeginChild("Graphs", ImVec2(0, 0), true);
        {
            ImGui::SeparatorText("Graphs");
            
            if (signalData.size() > 0)
            {
                static int selected_cell = 0;
                if (signalData.size() > 1)
                {
                    ImGui::Text("Select Cell:");
                    ImGui::SameLine();

                    std::vector<const char*> cell_names;
                    for (const auto& signal : signalData)
                    {
                        cell_names.push_back(signal.cellIdentity.c_str());
                    }

                    ImGui::Combo("##CellSelect", &selected_cell, cell_names.data(), cell_names.size());
                }
                
                const auto& signal = signalData[selected_cell];
                
                if (signal.timestamps.size() > 0)
                {
                    if (ImPlot::BeginPlot(("Plot##" + signal.cellIdentity).c_str(), ImVec2(-1, -1)))
                    {
                        ImPlot::SetupAxes("Time", "dB", ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
                        ImPlot::SetupAxisLimits(ImAxis_X1, signal.timestamps.front(), signal.timestamps.back());
                        ImPlot::SetupAxisLimits(ImAxis_Y1, -120, -40);

                        if (signal.rsrp_values.size() > 0)
                        {
                            ImPlot::SetNextLineStyle(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), 2);
                            ImPlot::PlotLine("RSRP", signal.timestamps.data(), signal.rsrp_values.data(), signal.timestamps.size());
                        }

                        if (signal.rsrq_values.size() > 0)
                        {
                            ImPlot::SetNextLineStyle(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), 2);
                            ImPlot::PlotLine("RSRQ", signal.timestamps.data(), signal.rsrq_values.data(), signal.timestamps.size());
                        }

                        if (signal.rssi_values.size() > 0)
                        {
                            ImPlot::SetNextLineStyle(ImVec4(0.4f, 0.4f, 1.0f, 1.0f), 2);
                            ImPlot::PlotLine("RSSI", signal.timestamps.data(), signal.rssi_values.data(), signal.timestamps.size());
                        }

                        ImPlot::EndPlot();
                    }
                }
            }
        }
        ImGui::EndChild();
        
        ImGui::PopStyleVar();
    }
    ImGui::End();
}