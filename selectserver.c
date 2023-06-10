#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>

#include "st_proactor.h"

#define PORT "9034" // port we're listening on

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

int handle_new_connection(void *arg);
int handle_client_message(void *arg);

int main(void)
{
    int listener;                       // listening socket descriptor
    struct sockaddr_storage remoteaddr; // client address
    socklen_t addrlen;
    int newfd;                          // newly accept()ed socket descriptor
    char buf[256]; // buffer for client data
    int nbytes;

    char remoteIP[INET6_ADDRSTRLEN];

    int yes = 1; // for setsockopt() SO_REUSEADDR, below
    int rv , i , j;
    

    struct addrinfo hints, *ai, *p;

    // Instantiate the proactor object.
    proactor_t *proactor = (proactor_t *) malloc(sizeof(proactor_t));
    createProactor(proactor);

    // get us a socket and bind it
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0)
    {
        fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
        exit(1);
    }

    for (p = ai; p != NULL; p = p->ai_next)
    {
        listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listener < 0)
        {
            continue;
        }

        // lose the pesky "address already in use" error message
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        // Set the socket to non-blocking mode
        int flags = fcntl(listener, F_GETFL, 0);
        if (flags == -1)
        {
            perror("fcntl");
            exit(EXIT_FAILURE);
        }

        if (fcntl(listener, F_SETFL, flags | O_NONBLOCK) == -1)
        {
            perror("fcntl");
            exit(EXIT_FAILURE);
        }

        if (bind(listener, p->ai_addr, p->ai_addrlen) < 0)
        {
            close(listener);
            continue;
        }

        break;
    }

    // if we got here, it means we didn't get bound
    if (p == NULL)
    {
        fprintf(stderr, "selectserver: failed to bind\n");
        exit(2);
    }

    freeaddrinfo(ai); // all done with this

    // listen
    if (listen(listener, 10) == -1)
    {
        perror("listen");
        exit(3);
    }

    printf("selectserver listening on port: %s\n", PORT);


    // add the listener to the master set
    addFD2Proactor(proactor, listener, handle_new_connection);

    // keep track of the biggest file descriptor
    proactor->maxfd = listener; // so far, it's this one
    proactor->server_listener = listener;
    // run the proactor

    proactor->handle_new_connection = handle_new_connection;
    proactor->handle_new_message = handle_client_message;
    
    runProactor(proactor);

    sleep(1);

    pthread_join((pthread_t)&proactor->thread, NULL);

    // Close the listener socket when finished
    close(listener);
    // Free the proactor object
    cancelProactor(proactor);
    free(proactor);

    return 0;
}


// This function handles new connections
int handle_new_connection(void *arg)
{
    // Convert arg to proactor pointer
    proactor_t *proactor = (proactor_t *)arg;

    struct sockaddr_storage remoteaddr; // client address
    socklen_t addrlen = sizeof remoteaddr;
    char remoteIP[INET6_ADDRSTRLEN];
    int newfd = accept(proactor->current_hot_fd, (struct sockaddr *)&remoteaddr, &addrlen);
    if (newfd == -1)
    {
        perror("accept");
        return -1;
    }
    else
    {
        proactor->count++;
        // add the new connection to the proactor
        addFD2Proactor(proactor, newfd, proactor->handle_new_message);
        printf("selectserver: new connection from %s on socket %d\n",
               inet_ntop(remoteaddr.ss_family,
                         get_in_addr((struct sockaddr *)&remoteaddr), remoteIP, INET6_ADDRSTRLEN), newfd);
        printf("\n Number of users online is %d\n", proactor->count); // not include the listener
    }
    return 0;
}

// This function handles data from a client
int handle_client_message(void *arg)
{
    // Convert arg to proactor pointer
    proactor_t *proactor = (proactor_t *)arg;
    if(proactor==NULL)
    {
        return -1;
    }

    char buf[256]; // buffer for client data
    int nbytes;
    
    // printf("current Hot fd = %d\n", proactor->current_hot_fd);
    nbytes = recv(proactor->current_hot_fd, buf, sizeof buf, 0);
    if (nbytes <= 0)
    {
        // got error or connection closed by client
        if (nbytes == 0)
        {
            // connection closed
            printf("selectserver: socket %d hung up\n", proactor->current_hot_fd);
        }
        close(proactor->current_hot_fd); // bye!
        removeHandler(proactor, proactor->current_hot_fd);
        printf("\n Number of users online is %d\n", proactor->count);
        return -1;
    }
    else
    {
        // we got some data from a client
        for (int j = 0; j <= proactor->maxfd; j++)
        {
            // send to everyone!
            if (FD_ISSET(j, &proactor->fds))
            {
                // except the listener and ourselves
                if (j != proactor->current_hot_fd && j != proactor->server_listener)
                {
                    if (send(j, buf, nbytes, 0) == -1)
                    {
                        perror("send");
                    }
                }
            }
        }
    }
    return 0;
}
