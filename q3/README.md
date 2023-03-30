## Question - 3: Internet Routing

### How to run : 

- ```Run make in the q3 folder. ```
- ```Run ./server on a terminal.```
- ```Enter input for server```
- ```Run ./client on another terminal.```
- ```Run required commands on client. Output will be on ./server.```

### Implementation :

#### Algorithm:
Distance Vector Routing algorithm is used here, which is similar to the Bellman Ford algorithm for shortest distance on a graph. Each node periodically sends its routing table to its neighbours so they can update their routing tables accordingly, eventually optimising the path.

#### Server:

- The server program makes one thread to simulate each node. Another thread is made for each node which sends the routing table of the node to its neighbours periodically.
- The ith node in the server program is binded to the port `8001 + i`. Each node has a welcoming socket bound to this, which listens for connections.
- Whenever two nodes want to communicate, which only happens between neighbouring nodes, they form a new socket.
- The server program handles the incoming messages as follows: each messsage starts with a `msg type` which is
    - 0 for a routing table message.
    - 1 for a `send` message.
    - 2 for a `pt` message.
- The server program handles each client by making seperate threads and passing the socket fd to them as argument. The client is handled in the `handle_connection1 function.

#### Client

- The client has a command line interface which accepts commands from the user.
- There are two accepted commands: `send x <msg> ` and `pt` as given in the assignment. If any other command is given, it throws a `Incrrect command` message.
- With each command, the client opens the fd with port 8001 which is bounded to the 0th node in the server and sends it the encoded message.
- The 0th node then processes the message.

### Follow Up Question:

```How would you handle server failures?```  
If a node fails to send a message forward, we can have a system in which each node sends an acknowledgement message after forwarding to the parent node. This way, if a node fails to send it forward, the parent node doesnt recieve an acknowledgement and sends it to the next node.