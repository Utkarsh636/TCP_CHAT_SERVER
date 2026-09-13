#include <iostream>
#include <cstring>
#include <string>
#include <thread>
#include <vector>
#include <mutex>
#include <algorithm>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>


struct Client {
    int socket;
    std::string username;
};


std::vector<Client> clients;
std::mutex clients_mutex;


// Send all bytes
bool send_all(int socket, const char* data, size_t length) {

    size_t total_sent = 0;

    while (total_sent < length) {

        ssize_t bytes_sent = send(
            socket,
            data + total_sent,
            length - total_sent,
            0
        );

        if (bytes_sent <= 0) {
            return false;
        }

        total_sent += bytes_sent;
    }

    return true;
}


// Receive one newline-terminated message
bool receive_message(
    int socket,
    std::string& receive_buffer,
    std::string& message
) {

    while (true) {

        // Look for message delimiter
        size_t newline_position =
            receive_buffer.find('\n');

        if (newline_position != std::string::npos) {

            message = receive_buffer.substr(
                0,
                newline_position
            );

            // Remove message + '\n'
            receive_buffer.erase(
                0,
                newline_position + 1
            );

            return true;
        }


        // Need more data
        char buffer[1024];

        ssize_t bytes_received = recv(
            socket,
            buffer,
            sizeof(buffer),
            0
        );

        if (bytes_received <= 0) {
            return false;
        }

        receive_buffer.append(
            buffer,
            bytes_received
        );
    }
}


// Broadcast message to everyone except sender
void broadcast_message(
    const std::string& message,
    int sender_socket
) {

    std::string framed_message = message + "\n";

    std::vector<int> recipient_sockets;

    {
        std::lock_guard<std::mutex> lock(clients_mutex);

        for (const Client& client : clients) {
            if (client.socket != sender_socket) {
                recipient_sockets.push_back(client.socket);
            }
        }
    }

    for (int socket : recipient_sockets) {
        send_all(
            socket,
            framed_message.c_str(),
            framed_message.size()
        );
    }
}


// Send connected users
void send_user_list(int client_socket)
{
    std::lock_guard<std::mutex> lock(clients_mutex);

    std::string user_list = "Connected users: ";

    for (size_t i = 0; i < clients.size(); ++i)
    {
        user_list += clients[i].username;

        if (i + 1 < clients.size())
        {
            user_list += ", ";
        }
    }

    user_list += "\n";

    send_all(
        client_socket,
        user_list.c_str(),
        user_list.size()
    );
}


// Handle one client
void handle_client(int client_socket) {

    std::cout << "Handling client...\n";


    // Buffer for TCP stream
    std::string receive_buffer;


    // -------------------------
    // Receive username
    // -------------------------

    const char* prompt =
        "Enter your username:\n";

    send_all(
        client_socket,
        prompt,
        strlen(prompt)
    );


    std::string username;

    if (!receive_message(
        client_socket,
        receive_buffer,
        username
    )) {

        close(client_socket);
        return;
    }


    if (username.empty()) {
        username = "Anonymous";
    }


    // -------------------------
    // Register client
    // -------------------------

    {
        std::lock_guard<std::mutex> lock(
            clients_mutex
        );

        clients.push_back({
            client_socket,
            username
        });
    }


    std::cout
        << username
        << " joined the chat.\n";


    std::string join_message =
        username + " joined the chat.";

    broadcast_message(
        join_message,
        client_socket
    );


    // -------------------------
    // Main chat loop
    // -------------------------

    while (true) {

        std::string message;

        if (!receive_message(
            client_socket,
            receive_buffer,
            message
        )) {

            std::cout
                << username
                << " disconnected.\n";

            break;
        }


        std::cout
            << username
            << ": "
            << message
            << '\n';


        // /users
        if (message == "/users") {

            send_user_list(
                client_socket
            );

            continue;
        }


        // /quit
        if (message == "/quit") {
            break;
        }


        // Normal message
        std::string formatted_message =
            username + ": " + message;

        broadcast_message(
            formatted_message,
            client_socket
        );
    }


    // -------------------------
    // Remove client
    // -------------------------

    std::string leave_message =
        username + " left the chat.";

    broadcast_message(
        leave_message,
        client_socket
    );


    {
        std::lock_guard<std::mutex> lock(
            clients_mutex
        );

        clients.erase(
            std::remove_if(
                clients.begin(),
                clients.end(),
                [client_socket](
                    const Client& client
                ) {
                    return client.socket ==
                           client_socket;
                }
            ),
            clients.end()
        );
    }


    shutdown(
        client_socket,
        SHUT_RDWR
    );

    close(client_socket);


    std::cout
        << username
        << " handler finished.\n";
}


int main() {

    // -------------------------
    // Create server socket
    // -------------------------

    int server_socket = socket(
        AF_INET,
        SOCK_STREAM,
        0
    );

    if (server_socket < 0) {

        std::cerr
            << "Socket creation failed\n";

        return 1;
    }

    std::cout
        << "Server socket created!\n";

    int reuse = 1;

    if (setsockopt(
            server_socket,
            SOL_SOCKET,
            SO_REUSEADDR,
            &reuse,
            sizeof(reuse)
        ) < 0) {

        std::cerr << "setsockopt failed\n";
        close(server_socket);
        return 1;
    }


    // -------------------------
    // Configure address
    // -------------------------

    sockaddr_in server_address{};

    server_address.sin_family =
        AF_INET;

    server_address.sin_addr.s_addr =
        INADDR_ANY;

    server_address.sin_port =
        htons(8080);


    // -------------------------
    // Bind
    // -------------------------

    if (bind(
        server_socket,
        reinterpret_cast<sockaddr*>(
            &server_address
        ),
        sizeof(server_address)
    ) < 0) {

        std::cerr
            << "Bind failed\n";

        close(server_socket);

        return 1;
    }

    std::cout
        << "Socket bound to port 8080!\n";


    // -------------------------
    // Listen
    // -------------------------

    if (listen(
        server_socket,
        10
    ) < 0) {

        std::cerr
            << "Listen failed\n";

        close(server_socket);

        return 1;
    }

    std::cout
        << "Server is listening...\n";


    // -------------------------
    // Accept clients forever
    // -------------------------

    while (true) {

        sockaddr_in client_address{};

        socklen_t client_address_length =
            sizeof(client_address);


        int client_socket = accept(
            server_socket,
            reinterpret_cast<sockaddr*>(
                &client_address
            ),
            &client_address_length
        );


        if (client_socket < 0) {

            std::cerr
                << "Accept failed\n";

            continue;
        }


        char client_ip[INET_ADDRSTRLEN];

        inet_ntop(
            AF_INET,
            &client_address.sin_addr,
            client_ip,
            sizeof(client_ip)
        );

        std::cout
            << "Client connected from "
            << client_ip
            << "\n";


        // One thread per client
        std::thread client_thread(
            handle_client,
            client_socket
        );


        client_thread.detach();
    }


    close(server_socket);

    return 0;
}