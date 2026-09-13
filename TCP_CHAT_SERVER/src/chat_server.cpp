#include "chat_server.hpp"

#include <iostream>
#include <algorithm>

void ChatServer::add_client(
    void* connection,
    const std::string& username
) {
    std::lock_guard<std::mutex> lock(clients_mutex);

    clients.push_back({
        connection,
        username
    });

    std::cout
        << username
        << " joined the chat.\n";
}


void ChatServer::remove_client(void* connection) {
    std::lock_guard<std::mutex> lock(clients_mutex);

    clients.erase(
        std::remove_if(
            clients.begin(),
            clients.end(),
            [connection](const Client& client) {
                return client.connection == connection;
            }
        ),
        clients.end()
    );
}

void ChatServer::update_username(
    void* connection,
    const std::string& username
) {
    std::lock_guard<std::mutex> lock(clients_mutex);

    for (Client& client : clients) {

        if (client.connection == connection) {

            client.username = username;
            return;
        }
    }
}


std::vector<Client> ChatServer::get_clients() {

    std::lock_guard<std::mutex> lock(clients_mutex);

    return clients;
}


std::string ChatServer::get_user_list() {

    std::lock_guard<std::mutex> lock(clients_mutex);

    std::string result = "Connected users: ";

    for (size_t i = 0; i < clients.size(); ++i) {

        result += clients[i].username;

        if (i + 1 < clients.size()) {
            result += ", ";
        }
    }

    return result;
}