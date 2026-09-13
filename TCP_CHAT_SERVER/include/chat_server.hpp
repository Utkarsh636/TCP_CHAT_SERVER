#ifndef CHAT_SERVER_HPP
#define CHAT_SERVER_HPP

#include <string>
#include <vector>
#include <mutex>

struct Client {
    void* connection;
    std::string username;
};

class ChatServer {

private:

    std::vector<Client> clients;
    std::mutex clients_mutex;

public:

    void add_client(
        void* connection,
        const std::string& username
    );

    void update_username(
        void* connection,
        const std::string& username
    );

    void remove_client(void* connection);

    std::vector<Client> get_clients();

    std::string get_user_list();
};

#endif