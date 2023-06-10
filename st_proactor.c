#include "st_proactor.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_FDS 100

// Create proactor
void* createProactor(void *args)
{
    proactor_t *proactor = (proactor_t *)args;
    if (proactor == NULL)
    {
        return NULL; // Not enough memory
    }
    FD_ZERO(&proactor->fds);
    proactor->maxfd = -1;
    proactor->count = 0;
    proactor->running = 0;
    proactor->server_listener = 0;
    proactor->current_hot_fd = -1;
    proactor->handle_new_connection = NULL;
    proactor->handle_new_message = NULL;
    // Allocate memory for the handler array and initialize all handlers to NULL.
    // proactor->array_of_handlers = (handler_t *)malloc(sizeof(handler_t));
    
    // if (!proactor->array_of_handlers)
    // {
    //     free(proactor);
    //     return NULL;
    // }

    return proactor;
}

// Start proactor
int runProactor(void *this)
{
    proactor_t *proactor = (proactor_t *)this;
    proactor->running = 1;
    // Create the thread that will run the proactor.
    pthread_create(&proactor->thread, NULL, proactor_run, proactor);
    return 0;
}

// Stop proactor
int cancelProactor(void *this)
{
    proactor_t *proactor = (proactor_t *)this;
    proactor->running = 0;
    pthread_cancel(proactor->thread);
    // free(proactor->array_of_handlers);

    return 0;
}

// Add a file descriptor to proactor
int addFD2Proactor(void *this, int fd, handler_t handler)
{
    proactor_t *proactor = (proactor_t *)this;
    FD_SET(fd, &proactor->fds);
    if (fd > proactor->maxfd)
    {
        proactor->maxfd = fd;
        //  proactor->array_of_handlers = realloc(proactor->array_of_handlers, (proactor->maxfd + 1) * (sizeof(handler_t)));
    }
    // proactor->array_of_handlers[fd] = handler;
    return 0;
}

void *proactor_run(void *this)
{
    proactor_t *proactor = (proactor_t *)this;
    fd_set read_fds; // Temp file descriptor list for select()
    socklen_t addrlen;
    int newfd;                          // newly accept()ed socket descriptor
    struct sockaddr_storage remoteaddr; // client address
    char remoteIP[INET6_ADDRSTRLEN];
    char buf[256]; // buffer for client data
    int nbytes;

    while (proactor->running)
    {
        FD_ZERO(&read_fds);
        // copy the original set to a temp set because select() modifies the set passed to it
        read_fds = proactor->fds;
        // We use select to wait for data on any socket
        if (select(proactor->maxfd + 1, &read_fds, NULL, NULL, NULL) == -1)
        {
            perror("select");
            break;
        }

        // run through the existing connections looking for data to read
        for (int i = 0; i <= proactor->maxfd; i++)
        {
            if (FD_ISSET(i, &read_fds))
            { // we got one!!
                proactor->current_hot_fd = i;
                if (i == proactor->server_listener)
                {
                    // // handle new connections
                    proactor->handle_new_connection(proactor);
                }
                else
                {
                    pthread_t thread;        
                    pthread_create(&thread, NULL, (void *)proactor->handle_new_message, proactor);
                } // END handle data from client
            }
        }
    }

    return NULL;
}

// Remove a file descriptor from proactor
int removeHandler(void *this, int fd)
{
    proactor_t *proactor = (proactor_t *)this;
    if(proactor==NULL||fd<0||fd>proactor->maxfd||!FD_ISSET(fd, &proactor->fds))
    {
        return -1;
    }
    FD_CLR(fd, &proactor->fds);
    proactor->count--;
    // proactor->array_of_handlers[fd] = NULL;
    return 0;
}
