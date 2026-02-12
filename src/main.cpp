#include <iostream>
#include <mutex>
#include <string>
#include <vector>

#include "dear_mygui_manager.h"
#include "websocket_client.h"

int main()
{
    WebSocketClient* webSocketClient = new WebSocketClient("ws://friendsshield.site:5678/api/mobile-network/ws");
    DearMyGuiManager dearMyGuiManager;

    struct LocationData {
        std::string user;
        double latitude;
        double longitude;
        double altitude;
        long long time;
    };

    LocationData lastLocation = {};
    std::mutex locationMutex;

    webSocketClient->AddMessageHandler([&lastLocation, &locationMutex](websocketpp::connection_hdl hdl, client::message_ptr msg) {
        std::string payload = msg->get_payload();
        size_t userStart = payload.find("\"Emeteil\":");

        if (userStart == std::string::npos) 
            return;

        size_t locationStart = payload.find("\"location_data\":", userStart);
        if (locationStart == std::string::npos)
            return;
        
        size_t latStart = payload.find("\"Latitude\":", locationStart);
        size_t lonStart = payload.find("\"Longitude\":", locationStart);
        size_t altStart = payload.find("\"Altitude\":", locationStart);
        size_t timeStart = payload.find("\"Time\":", locationStart);

        if (latStart == std::string::npos || lonStart == std::string::npos || altStart == std::string::npos || timeStart == std::string::npos)
            return;
        
        std::lock_guard<std::mutex> lock(locationMutex);
        lastLocation.user = "Emeteil";

        size_t latComma = payload.find(',', latStart);
        size_t lonComma = payload.find(',', lonStart);
        size_t altComma = payload.find(',', altStart);
        size_t timeEnd = payload.find('}', timeStart);

        if (latComma == std::string::npos || lonComma == std::string::npos || altComma == std::string::npos || timeEnd == std::string::npos)
            return;

        lastLocation.latitude = std::stod(payload.substr(latStart + 11, latComma - latStart - 11));
        lastLocation.longitude = std::stod(payload.substr(lonStart + 12, lonComma - lonStart - 12));
        lastLocation.altitude = std::stod(payload.substr(altStart + 11, altComma - altStart - 10));
        lastLocation.time = std::stoll(payload.substr(timeStart + 7, timeEnd - timeStart - 7));
    });

    dearMyGuiManager.AddWidget([&lastLocation, &locationMutex]() {
        ImGui::SetNextWindowSize(ImVec2(300, 200), ImGuiCond_FirstUseEver);

        ImGui::Begin("Location Data", nullptr, ImGuiWindowFlags_NoCollapse);
        {
            std::lock_guard<std::mutex> lock(locationMutex);

            ImGui::BeginChild("Location Data", ImVec2(0, 0), true);
            {
                ImGui::SeparatorText("Location Data");
                ImGui::Columns(2, "Location Data", false);
                ImGui::SetColumnWidth(0, 120);

                ImGui::Text("User:");
                ImGui::NextColumn();
                ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "%s", lastLocation.user.c_str());
                ImGui::NextColumn();

                ImGui::Text("Latitude:");
                ImGui::NextColumn();
                ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "%.6f", lastLocation.latitude);
                ImGui::NextColumn();

                ImGui::Text("Longitude:");
                ImGui::NextColumn();
                ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "%.6f", lastLocation.longitude);
                ImGui::NextColumn();

                ImGui::Text("Altitude:");
                ImGui::NextColumn();
                ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.4f, 1.0f), "%.6f", lastLocation.altitude);
                ImGui::NextColumn();

                ImGui::Text("Time:");
                ImGui::NextColumn();
                ImGui::TextColored(ImVec4(1.0f, 0.4f, 1.0f, 1.0f), "%lld", lastLocation.time);
                ImGui::NextColumn();

                ImGui::Columns(1);
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