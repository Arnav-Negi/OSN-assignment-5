#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <pthread.h>

#include <bits/stdc++.h>

#define SERVER_PORT 8001

using namespace std;

pair<string, int> read_string_from_socket(const int &fd, int bytes)
{
    std::string output;
    output.resize(bytes);

    int bytes_received = read(fd, &output[0], bytes - 1);
    if (bytes_received <= 0)
    {
        cerr << "Failed to read data from socket. \n";
    }

    output[bytes_received] = 0;
    output.resize(bytes_received);
    return {output, bytes_received};
}

int send_string_on_socket(int fd, const string &s)
{
    int bytes_sent = write(fd, s.c_str(), s.length());
    if (bytes_sent < 0)
    {
        cerr << "Failed to SEND DATA via socket.\n";
    }

    return bytes_sent;
}

int get_socket_fd(struct sockaddr_in *ptr)
{
    struct sockaddr_in server_obj = *ptr;

    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0)
    {
        perror("Error in socket creation for CLIENT");
        exit(-1);
    }

    int port_num = SERVER_PORT;

    memset(&server_obj, 0, sizeof(server_obj)); // Zero out structure
    server_obj.sin_family = AF_INET;
    server_obj.sin_port = htons(port_num); // convert to big-endian order

    if (connect(socket_fd, (struct sockaddr *)&server_obj, sizeof(server_obj)) < 0)
    {
        perror("Problem in connecting to the server");
        exit(-1);
    }

    return socket_fd;
}

int main()
{
    

    string cmd, to_send;
    int dest;
    while (1)
    {
        cout << "> ";
        cin >> cmd;
        if (cmd.empty()) continue;
        struct sockaddr_in server_obj;
        int socket_fd = get_socket_fd(&server_obj);
        if (cmd.compare("send") == 0)
        {   
            cin >> dest >> cmd;
            to_send.resize(0);
            to_send += string("1 ") + to_string(dest) + string(" 0 ") + cmd;
            send_string_on_socket(socket_fd, to_send);
        }
        else if (cmd.compare("pt") == 0)
        {
            send_string_on_socket(socket_fd, string("2"));
        }
        else
        {
            cout << "Incorrect command.\n";
        }
        close(socket_fd);
    }
}