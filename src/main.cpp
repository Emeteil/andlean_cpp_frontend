#include <iostream>
#include <mutex>

#include "dear_mygui_manager.h"
#include "websocket_client.h"

int main()
{
    WebSocketClient* webSocketClient = new WebSocketClient("ws://friendsshield.site:5678/api/mobile-network/ws");
    DearMyGuiManager dearMyGuiManager;
    std::string lastMessage = "";
    std::mutex messageMutex;

    webSocketClient->AddMessageHandler([&lastMessage, &messageMutex](websocketpp::connection_hdl hdl, client::message_ptr msg) {
        std::lock_guard<std::mutex> lock(messageMutex);
        lastMessage = msg->get_payload();
    });

    dearMyGuiManager.AddWidget([&lastMessage, &messageMutex]() {
        static int counter = 0;
        
        ImGui::Begin("Hello, world!"); 
            ImGui::Text("This is some useful text.");    
            if (ImGui::Button("Button"))                         
                counter++;
            ImGui::Text("counter = %d", counter);
            
            std::string message;
            {
                std::lock_guard<std::mutex> lock(messageMutex);
                message = lastMessage;
            }
            if (!message.empty())
                ImGui::Text("Last message: %s", message.c_str());
            
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