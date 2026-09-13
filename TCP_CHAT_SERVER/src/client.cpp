#include <iostream>
#include <cstring>
#include <string>
#include <thread>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>


// Send all bytes
bool send_all(int socket, const char* data, size_t length)
{
    size_t total_sent = 0;

    while (total_sent < length)
    {
        ssize_t bytes_sent = send(
            socket,
            data + total_sent,
            length - total_sent,
            0
        );

        if (bytes_sent <= 0)
        {
            return false;
        }

        total_sent += bytes_sent;
    }

    return true;
}


// Receive exactly one newline-delimited message
bool receive_message(
    int socket,
    std::string& receive_buffer,
    std::string& message
)
{
    while (true)
    {
        // Look for message boundary
        size_t newline_position =
            receive_buffer.find('\n');

        if (newline_position != std::string::npos)
        {
            message = receive_buffer.substr(
                0,
                newline_position
            );

            // Remove processed message + newline
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

        if (bytes_received <= 0)
        {
            return false;
        }

        receive_buffer.append(
            buffer,
            bytes_received
        );
    }
}


// Receiver thread
void receive_messages(int client_socket)
{
    std::string receive_buffer;

    while (true)
    {
        std::string message;

        if (!receive_message(
            client_socket,
            receive_buffer,
            message
        ))
        {
            std::cout
                << "\nServer disconnected.\n";

            break;
        }

        std::cout
            << "\nServer: "
            << message
            << "\n";

        std::cout
            << "You: "
            << std::flush;
    }
}


int main()
{
    // 1. Create socket
    int client_socket = socket(
        AF_INET,
        SOCK_STREAM,
        0
    );

    if (client_socket < 0)
    {
        std::cerr
            << "Failed to create socket\n";

        return 1;
    }

    std::cout
        << "Client socket created successfully!\n";


    // 2. Configure server address
    sockaddr_in server_address{};

    server_address.sin_family =
        AF_INET;

    server_address.sin_port =
        htons(8080);

    inet_pton(
        AF_INET,
        "10.40.237.100",
        &server_address.sin_addr
    );


    // 3. Connect to server
    int connection_status = connect(
        client_socket,
        reinterpret_cast<sockaddr*>(
            &server_address
        ),
        sizeof(server_address)
    );

    if (connection_status < 0)
    {
        std::cerr
            << "Connection failed\n";

        close(client_socket);

        return 1;
    }

    std::cout
        << "Connected to server successfully!\n";


    // 4. Start receiver thread
    std::thread receiver_thread(
        receive_messages,
        client_socket
    );


    // 5. Send messages
    while (true)
    {
        std::cout << "You: ";

        std::string message;

        std::getline(
            std::cin,
            message
        );

        if (!std::cin)
        {
            break;
        }


        // Add newline framing
        std::string framed_message =
            message + "\n";


        bool success = send_all(
            client_socket,
            framed_message.c_str(),
            framed_message.size()
        );

        if (!success)
        {
            std::cerr
                << "\nSend failed\n";

            break;
        }


        // Quit
        if (message == "/quit")
        {
            shutdown(
                client_socket,
                SHUT_RDWR
            );

            break;
        }
    }


    // 6. Graceful shutdown
    shutdown(
        client_socket,
        SHUT_RDWR
    );

    receiver_thread.join();

    close(client_socket);


    std::cout
        << "Client shut down.\n";

    return 0;
}