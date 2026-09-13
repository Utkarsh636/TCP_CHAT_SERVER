#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstring>

#include <libwebsockets.h>

#include "chat_server.hpp"

ChatServer chat_server;


struct WebSocketClient {
    std::string username;
    std::vector<std::string> messages;
};


static void queue_message(
    struct lws* wsi,
    const std::string& message
) {
    WebSocketClient* client =
        static_cast<WebSocketClient*>(
            lws_wsi_user(wsi)
        );

    client->messages.push_back(message);

    lws_callback_on_writable(wsi);
}


static void send_to_client(
    struct lws* client_wsi,
    const std::string& message
) {
    std::vector<unsigned char> buffer(
        LWS_PRE + message.size()
    );

    std::memcpy(
        buffer.data() + LWS_PRE,
        message.data(),
        message.size()
    );

    lws_write(
        client_wsi,
        buffer.data() + LWS_PRE,
        message.size(),
        LWS_WRITE_TEXT
    );
}


static int callback_http(
    struct lws* wsi,
    enum lws_callback_reasons reason,
    void* user,
    void* in,
    size_t len
) {
    if (reason == LWS_CALLBACK_HTTP) {

        std::ifstream file("web/index.html");

        if (!file) {
            std::cerr << "Could not open web/index.html\n";
            return -1;
        }

        std::string html(
            (std::istreambuf_iterator<char>(file)),
            std::istreambuf_iterator<char>()
        );

        std::string header =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: " +
            std::to_string(html.size()) +
            "\r\n"
            "Connection: close\r\n"
            "\r\n";

        std::vector<unsigned char> response;

        response.insert(
            response.end(),
            header.begin(),
            header.end()
        );

        response.insert(
            response.end(),
            html.begin(),
            html.end()
        );

        lws_write(
            wsi,
            response.data(),
            response.size(),
            LWS_WRITE_HTTP
        );

        return -1;
    }

    return 0;
}


static int callback_websocket(
    struct lws* wsi,
    enum lws_callback_reasons reason,
    void* user,
    void* in,
    size_t len
) {
    WebSocketClient* client =
        static_cast<WebSocketClient*>(user);


    switch (reason) {

        case LWS_CALLBACK_ESTABLISHED: {

            client->username = "Anonymous";

            chat_server.add_client(
                static_cast<void*>(wsi),
                client->username
            );

            std::cout
                << "Browser connected!\n";

            break;
        }


        case LWS_CALLBACK_RECEIVE: {

            std::string message(
                static_cast<char*>(in),
                len
            );

            if (message.rfind("/username ", 0) == 0) {

                client->username =
                    message.substr(10);

                chat_server.update_username(
                    static_cast<void*>(wsi),
                    client->username
                );

                std::string join_message =
                    client->username + " joined the chat.";

                for (const Client& other : chat_server.get_clients()) {

                    if (other.connection ==
                        static_cast<void*>(wsi)) {
                        continue;
                    }

                    send_to_client(
                        static_cast<struct lws*>(other.connection),
                        join_message
                    );
                }

                std::cout
                    << "Username set to: "
                    << client->username
                    << "\n";

                return 0;
            }

            if (message == "/users") {

                std::string user_list =
                    chat_server.get_user_list();

                send_to_client(
                    wsi,
                    user_list
                );

                return 0;
            }

            if (message == "/quit") {

                std::cout
                    << client->username
                    << " requested to leave.\n";

                return -1;
            }

            std::cout
                << client->username
                << ": "
                << message
                << "\n";


            std::string response =
                client->username + ": " + message;


            std::vector<Client> clients =
                chat_server.get_clients();


            for (const Client& other : clients) {

                if (other.connection ==
                    static_cast<void*>(wsi)) {

                    continue;
                }

                struct lws* other_wsi =
                    static_cast<struct lws*>(
                        other.connection
                    );

                queue_message(
                    other_wsi,
                    response
                );
            }

            break;
        }


        case LWS_CALLBACK_SERVER_WRITEABLE: {

            if (client->messages.empty()) {
                break;
            }

            std::string message =
                client->messages.front();

            client->messages.erase(
                client->messages.begin()
            );

            std::vector<unsigned char> buffer(
                LWS_PRE + message.size()
            );

            std::memcpy(
                buffer.data() + LWS_PRE,
                message.data(),
                message.size()
            );

            lws_write(
                wsi,
                buffer.data() + LWS_PRE,
                message.size(),
                LWS_WRITE_TEXT
            );

            if (!client->messages.empty()) {
                lws_callback_on_writable(wsi);
            }

            break;
        }


        case LWS_CALLBACK_CLOSED: {

            std::string message =
                client->username + " left the chat.";

            for (const Client& other : chat_server.get_clients()) {

                if (other.connection ==
                    static_cast<void*>(wsi)) {
                    continue;
                }

                send_to_client(
                    static_cast<struct lws*>(other.connection),
                    message
                );
            }

            chat_server.remove_client(
                static_cast<void*>(wsi)
            );

            std::cout
                << "Browser disconnected.\n";

            break;
        }


        default:
            break;
    }


    return 0;
}


static const struct lws_protocols protocols[] = {

    {
        "http",
        callback_http,
        0,
        0
    },

    {
        "chat-protocol",
        callback_websocket,
        sizeof(WebSocketClient),
        1024
    },

    LWS_PROTOCOL_LIST_TERM
};


int main() {

    struct lws_context_creation_info info{};

    info.port = 8081;
    info.protocols = protocols;


    struct lws_context* context =
        lws_create_context(&info);


    if (!context) {

        std::cerr
            << "WebSocket server creation failed\n";

        return 1;
    }


    std::cout
        << "WebSocket server running on port 8081...\n";


    while (true) {

        lws_service(
            context,
            1000
        );
    }


    lws_context_destroy(context);

    return 0;
}