#include <iostream>
#include <mutex>
#include <string>
#include <vector>

#include "dear_mygui_manager.h"
#include "websocket_client.h"

#include "json.hpp"
using json = nlohmann::json;

int main()
{
    WebSocketClient* webSocketClient = new WebSocketClient("ws://94.159.111.243:5678/api/mobile-network/ws");
    DearMyGuiManager dearMyGuiManager;

    struct MobileNetworkData {
        std::string networkType;
        std::string cellIdentity;
        std::string mcc;
        std::string mnc;
        int pci;
        int tac;
        std::string bands;
        std::string signalStrength;
        int rsrp;
        int rsrq;
        int rssi;
        int timingAdvance;
        long long time;
    };

    struct LocationData {
        std::string user;
        double latitude;
        double longitude;
        double altitude;
        long long time;
    };

    struct UserData {
        std::string user;
        std::vector<MobileNetworkData> mobileNetworks;
        LocationData location;
    };

    UserData currentUser;
    std::mutex dataMutex;

    webSocketClient->AddMessageHandler([&currentUser, &dataMutex](websocketpp::connection_hdl hdl, client::message_ptr msg) {
        std::string payload = msg->get_payload();
        
        try
        {
            json j = json::parse(payload);
            
            for (auto& [userKey, userData] : j.items())
            {
                std::lock_guard<std::mutex> lock(dataMutex);
                
                UserData user;
                user.user = userKey;
                
                if (userData.contains("location_data"))
                {
                    auto& loc = userData["location_data"];
                    user.location.user = userKey;
                    user.location.latitude = loc["Latitude"];
                    user.location.longitude = loc["Longitude"];
                    user.location.altitude = loc["Altitude"];
                    user.location.time = loc["Time"];
                }
                
                if (userData.contains("mobile_network_data_list") && userData["mobile_network_data_list"].contains("MobileNetworks"))
                {
                    for (auto& network : userData["mobile_network_data_list"]["MobileNetworks"])
                    {
                        MobileNetworkData netData;
                        netData.networkType = network["NetworkType"];
                        netData.cellIdentity = network["CellIdentity"];
                        netData.mcc = network.value("MCC", "");
                        netData.mnc = network.value("MNC", "");
                        netData.pci = network["PCI"];
                        netData.tac = network["TAC"];
                        netData.bands = network["Bands"];
                        netData.signalStrength = network["SignalStrength"];
                        netData.rsrp = network["RSRP"];
                        netData.rsrq = network["RSRQ"];
                        netData.rssi = network["RSSI"];
                        netData.timingAdvance = network["TimingAdvance"];
                        netData.time = network["Time"];
                        user.mobileNetworks.push_back(netData);
                    }
                }
                
                currentUser = user;
                
            }
        }
        catch (const json::exception& e) {}
    });

    dearMyGuiManager.AddWidget([&currentUser, &dataMutex]() {
        ImGui::SetNextWindowSize(ImVec2(600, 500), ImGuiCond_FirstUseEver);

        ImGui::Begin("User Data", nullptr, ImGuiWindowFlags_NoCollapse);
        {
            std::lock_guard<std::mutex> lock(dataMutex);

            ImGui::BeginChild("User Data", ImVec2(0, 0), true);
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
        }
        ImGui::End();
    });

    std::thread ws_thread([webSocketClient] {
        webSocketClient->Run();
    });

    while (dearMyGuiManager.MainHandler());
    
    webSocketClient->~WebSocketClient();
    ws_thread.join();
    return 0;
}