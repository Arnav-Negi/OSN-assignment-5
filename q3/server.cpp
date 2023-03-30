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
#include <time.h>

#include <bits/stdc++.h>

using namespace std;
#define PORT_ARG 8001
#define MAX_N 64
#define BUFSIZE 4096

#define DVR_MSG 0
#define SEND_MSG 1
#define PT_MSG 2

vector<vector<pair<int, int>>> adj;
typedef struct
{
    int index;
    pthread_t node_thread, dvr_thread;
    int num_connections;
    vector<array<int, 3>> connections;
    pthread_mutex_t conn_mutex;
} node_info;

vector<node_info> nodes;

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

int get_socket_fd(struct sockaddr_in *ptr, int node_idx)
{
    struct sockaddr_in server_obj = *ptr;

    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0)
    {
        perror("Error in socket creation for CLIENT");
        exit(-1);
    }

    int port_num = PORT_ARG + node_idx;

    memset(&server_obj, 0, sizeof(server_obj)); // Zero out structure
    server_obj.sin_family = AF_INET;
    server_obj.sin_port = htons(port_num); // convert to big-endian order

    if (connect(socket_fd, (struct sockaddr *)&server_obj, sizeof(server_obj)) < 0)
    {
        perror("Problem in connecting to the server");
        close(socket_fd);
        exit(-1);
    }

    return socket_fd;
}

void *SendDVR(void *arg)
{
    int node_idx = *(int *)arg;

    while (1)
    {
        sleep(5);
        string sendstring;
        char tempbuf[BUFSIZE];
        bzero(tempbuf, BUFSIZE);
        sprintf(tempbuf, "%d %d %ld ", DVR_MSG, node_idx, nodes[node_idx].connections.size());
        sendstring += string(tempbuf);
        for (auto arr : nodes[node_idx].connections)
        {
            bzero(tempbuf, BUFSIZE);
            sprintf(tempbuf, "%d %d ", arr[0], arr[2]);
            sendstring += string(tempbuf);
        }

        for (auto send_to : adj[node_idx])
        {
            struct sockaddr_in target_node_obj;
            int socket_fd = get_socket_fd(&target_node_obj, send_to.first);

            send_string_on_socket(socket_fd, sendstring);
            close(socket_fd);
        }
    }

    pthread_exit(NULL);
}

void *handle_connection(void *arg)
{
    int client_socket_fd = (*(pair<int, int> *)arg).first;
    int node_idx = (*(pair<int, int> *)arg).second;
    free(arg);
    int clientnode_delay;
    int bytes_recieved;
    int msg_type;
    string msg;
    stringstream stream;

    if (client_socket_fd < 0)
    {
        perror("ERROR while accept() system call occurred in SERVER");
        exit(-1);
    }
    tie(msg, bytes_recieved) = read_string_from_socket(client_socket_fd, BUFSIZE);
    stream << msg;
    stream >> msg_type;
    if (msg_type == DVR_MSG)
    {
        int client_node, len_table, found;
        stream >> client_node >> len_table;

        for (auto arr : nodes[node_idx].connections)
        {
            if (client_node == arr[0])
            {
                clientnode_delay = arr[2];
                break;
            }
        }
        vector<pair<int, int>> routing_table(len_table, {0, 0});
        for (int i = 0; i < len_table; i++)
        {
            stream >> routing_table[i].first >> routing_table[i].second;
        }

        pthread_mutex_lock(&nodes[node_idx].conn_mutex);
        for (auto pa : routing_table)
        {
            found = 0;
            for (auto &conn : nodes[node_idx].connections)
            {
                if (conn[0] == pa.first)
                {
                    if (conn[2] > pa.second + clientnode_delay)
                    {
                        conn[2] = pa.second + clientnode_delay;
                        conn[1] = client_node;
                    }
                    found = 1;
                    break;
                }
            }
            // node not found, add.
            if (!found)
            {
                nodes[node_idx].connections.push_back({pa.first, client_node, pa.second});
            }
        }

        pthread_mutex_unlock(&nodes[node_idx].conn_mutex);
    }
    else if (msg_type == SEND_MSG)
    {
        int destination_node, source_node, found;
        string send_msg;
        stream >> destination_node >> source_node >> send_msg;
        found = 0;
        if (destination_node == node_idx)
        {
            found = 1;
            cout << "Data recieved at node: " << node_idx << "; \n";
            cout << "Source: " << source_node << "; \n";
            cout << "Destination: " << destination_node << "; \n";
            cout << "Message: \"" << send_msg << "\";\n";
            cout << endl;
        }
        else
        {
            for (auto arr : nodes[node_idx].connections)
            {
                if (arr[0] == destination_node)
                {
                    // send thru arr[0].
                    cout << "Data recieved at node: " << node_idx << "; \n";
                    cout << "Source: " << source_node << "; \n";
                    cout << "Destination: " << destination_node << "; \n";
                    cout << "Forwarded_Destination: " << arr[1] << "; \n";
                    cout << "Message: \"" << send_msg << "\";\n";
                    cout << endl;
                    // send to arr[1]
                    char sendbuf[BUFSIZE];
                    bzero(sendbuf, BUFSIZE);
                    sprintf(sendbuf, "%d %d %d %s", SEND_MSG, destination_node, node_idx, send_msg.c_str());
                    struct sockaddr_in next_node_obj;
                    int socket_fd = get_socket_fd(&next_node_obj, arr[1]);
                    send_string_on_socket(socket_fd, string(sendbuf));
                    close(socket_fd);
                    found = 1;
                    break;
                }
            }
            if (!found)
            {
                cout << "Destination node not found. Message could not be sent.\n";
            }
        }
    }
    else if (msg_type == PT_MSG)
    {
        cout << "dest\tforw\tdelay\n";
        for (auto to : nodes[node_idx].connections)
        {
            if (to[0] != node_idx)
                cout << to[0] << "\t" << to[1] << "\t" << to[2] << endl;
        }
        cout << endl;
    }
    else
    {
        cout << "Incorrect packet recieved.\n";
        close(client_socket_fd);
        exit(-1);
    }
    close(client_socket_fd);
    pthread_exit(NULL);
}

void *sim_node(void *arg)
{
    int node_idx = *(int *)arg;

    int wel_socket_fd, port_no, client_socket_fd;

    struct sockaddr_in node_addr_obj, client_addr_obj;

    wel_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (wel_socket_fd < 0)
    {
        perror("ERROR: Couldn't create welcome socket for node ");
        printf("%d.\n", node_idx);
        exit(-1);
    }

    bzero((char *)&node_addr_obj, sizeof(node_addr_obj));
    port_no = PORT_ARG + node_idx;
    node_addr_obj.sin_family = AF_INET;
    node_addr_obj.sin_addr.s_addr = INADDR_ANY;
    node_addr_obj.sin_port = htons(port_no);

    if (bind(wel_socket_fd, (struct sockaddr *)&node_addr_obj, sizeof(node_addr_obj)) < 0)
    {
        perror("Error on bind on welcome socket: ");
        close(wel_socket_fd);
        exit(-1);
    }

    listen(wel_socket_fd, MAX_N);
    socklen_t clilen = sizeof(client_addr_obj);

    while (1)
    {
        client_socket_fd = accept(wel_socket_fd, (struct sockaddr *)&client_addr_obj, &clilen);
        if (client_socket_fd < 0)
        {
            perror("ERROR while accept() system call occurred in SERVER");
            exit(-1);
        }

        pthread_t newthread;
        pair<int, int> *p = (pair<int, int> *)malloc(sizeof(pair<int, int>));
        p->first = client_socket_fd;
        p->second = node_idx;
        pthread_create(&newthread, NULL, handle_connection, (void *)p);
    }
    close(wel_socket_fd);
    pthread_exit(NULL);
}

int main()
{
    int m, n, u, v, w;
    cin >> n >> m;
    int array_of_args[n];
    adj.resize(n);
    for (int i = 0; i < m; i++)
    {
        cin >> u >> v >> w;
        adj[v].push_back({u, w});
        adj[u].push_back({v, w});
    }

    nodes.resize(n);

    for (int i = 0; i < n; i++)
    {
        nodes[i].num_connections = adj[i].size();
        nodes[i].connections.resize(adj[i].size());
        nodes[i].index = i;
        for (int j = 0; j < adj[i].size(); j++)
        {
            nodes[i].connections[j][0] = nodes[i].connections[j][1] = adj[i][j].first;
            nodes[i].connections[j][2] = adj[i][j].second;
        }
        pthread_mutex_init(&nodes[i].conn_mutex, NULL);
    }

    for (int i = 0; i < n; i++)
    {
        array_of_args[i] = i;
        pthread_create(&(nodes[i].node_thread), NULL, sim_node, (void *)&array_of_args[i]);
        pthread_create(&(nodes[i].dvr_thread), NULL, SendDVR, (void *)&array_of_args[i]);
    }

    for (int i = 0; i < n; i++)
    {
        pthread_join(nodes[i].node_thread, NULL);
        pthread_join(nodes[i].dvr_thread, NULL);
        pthread_mutex_destroy(&nodes[i].conn_mutex);
    }

    exit(0);
}