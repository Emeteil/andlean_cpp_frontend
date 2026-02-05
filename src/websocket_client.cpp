#include "websocket_client.h"

void WebSocketClient::Initialize()
{
    // Set logging to be pretty verbose (everything except message payloads)
    c.set_access_channels(websocketpp::log::alevel::all);
    c.clear_access_channels(websocketpp::log::alevel::frame_payload);
    c.set_error_channels(websocketpp::log::elevel::all);

    // Initialize ASIO
    c.init_asio();

    // Register our message handler
    c.set_message_handler([this](websocketpp::connection_hdl hdl, client::message_ptr msg) {
        this->OnMessage(hdl, msg);
    });
}
void WebSocketClient::OnMessage(websocketpp::connection_hdl hdl, client::message_ptr msg)
{
    for (auto handler : messageHandlers)
    {
        handler(hdl, msg);
    }
}

WebSocketClient::WebSocketClient(std::string _uri)
{
    Initialize();
}

WebSocketClient::~WebSocketClient()
{
    c.stop();
}

void WebSocketClient::Run()
{
    websocketpp::lib::error_code ec;
    client::connection_ptr con = c.get_connection(uri, ec);
    if (ec)
    {
        std::cout << "could not create connection because: " << ec.message() << std::endl;
        return;
    }

    // Note that connect here only requests a connection. No network messages are
    // exchanged until the event loop starts running in the next line.
    c.connect(con);

    // Start the ASIO io_service run loop
    // this will cause a single connection to be made to the server. c.run()
    // will exit when this connection is closed.
    c.run();
}

void WebSocketClient::AddMessageHandler(std::function<void(websocketpp::connection_hdl, client::message_ptr)> handler)
{
    messageHandlers.push_back(handler);
}