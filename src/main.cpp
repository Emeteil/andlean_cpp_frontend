#include <iostream>
#include <mutex>
#include <string>
#include <vector>
#include <thread>

#include "dear_mygui_manager.h"
#include "network/websocket_client.h"
#include "core/data_structures.h"
#include "core/parsing.h"
#include "ui/ui_widget.h"
#include "core/mercator_projection.h"

int main()
{
    WebSocketClient* webSocketClient = new WebSocketClient("ws://94.159.111.243:5678/api/mobile-network/ws");
    DearMyGuiManager dearMyGuiManager;

    UserData currentUser;
    std::mutex dataMutex;
    std::vector<SignalData> signalData;

    webSocketClient->AddMessageHandler([&currentUser, &dataMutex, &signalData](websocketpp::connection_hdl hdl, client::message_ptr msg) {
        std::string payload = msg->get_payload();
        ParseMobileNetworkData(payload, currentUser, dataMutex, signalData);
    });

    dearMyGuiManager.AddWidget([&currentUser, &dataMutex, &signalData]() {
        CreateMobileNetworkWidget(currentUser, dataMutex, signalData);
    });

    std::thread ws_thread([webSocketClient] {
        webSocketClient->Run();
    });

    while (dearMyGuiManager.MainHandler());
    
    webSocketClient->~WebSocketClient();
    ws_thread.join();
    return 0;
}