#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>

#include <iostream>
#include <functional>
#include <vector>

typedef websocketpp::client<websocketpp::config::asio_client> client;

class WebSocketClient
{
    private:
        std::string uri = "ws://friendsshield.site:5678/api/mobile-network/ws";
        std::vector<std::function<void(websocketpp::connection_hdl, client::message_ptr)>> messageHandlers;
        client c;

        void Initialize();
        void OnMessage(websocketpp::connection_hdl, client::message_ptr msg);

    public:
        WebSocketClient(std::string _uri);
        ~WebSocketClient();

        void Run();
        void AddMessageHandler(std::function<void(websocketpp::connection_hdl, client::message_ptr)> handler);
};