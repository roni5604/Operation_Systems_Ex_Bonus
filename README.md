# Operation_Systems_Ex4

How to use the project:

First step is to run all the project using Makefile. To use the Makefile you should enter the command: make

it will generate a react_server and a client files which you could run using ./react_server and ./client <ip> and with the instructions below.

## Multiple Chat
By using the reactor design pattern our server handles multiple concurrent client connections, and sends data received from one client to all other clients.
listening on port 9034 
 so there will be at list 3 sides communication. 
 The usage like this:
- The server side:
  
  export LD_LIBRARY_PATH=.:$LD_LIBRARY_PATH
  
  ./react_server
- The client A side: ./client IP
- The client B side: ./client IP

the communication done using IPv4 TCP protocol.
“multiple chat” means a tool that can read input from keyboard, and in the same time listen to the socket of the other side.


> Example command:
>
> Server: 
> 
> export LD_LIBRARY_PATH=.:$LD_LIBRARY_PATH
> 
> ./react_server
>
> Client A: ./client 127.0.0.1
>
> Client B: ./client 127.0.0.1
>
> and now each side can send messages throw the keyboard and press enter to sent to other clients.

To quit (exit) from the chat program, press CTRL+C

