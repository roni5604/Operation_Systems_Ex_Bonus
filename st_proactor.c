#include "st_proactor.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

// Create proactor
void* createProactor(void* args) {
    proactor_t* proactor = (proactor_t*) malloc(sizeof(proactor_t));
    if (proactor == NULL) {
        return NULL; // Not enough memory
    }
    FD_ZERO(&proactor->fds);
    proactor->maxfd = -1;
    proactor->count = 0;
    proactor->connection_handlers = NULL;
    proactor->message_handlers = NULL;
    proactor->running = 0;
    return proactor;
}

// Start proactor
int runProactor(void* this) {
    proactor_t* proactor = (proactor_t*) this;
    proactor->running = 1;
    // Create the thread that will run the proactor.
    // pthread_create(&proactor->thread, NULL, proactor_run, proactor);
    pthread_create(&proactor->thread, NULL, proactor_run, proactor);
    return 0;
}

// Stop proactor
int cancelProactor(void* this) {
    proactor_t* proactor = (proactor_t*) this;
    proactor->running = 0;
    return 0;
}

// Add a file descriptor to proactor
int addFD2Proactor(void* this, int fd, handler_t handler, message_handler_t msg_handler) {
    proactor_t* proactor = (proactor_t*) this;
    FD_SET(fd, &proactor->fds);
    if (fd > proactor->maxfd) {
        proactor->maxfd = fd;
    }
    proactor->count++;
    // Store the handlers somewhere, the details of this are up to you.
    return 0;
}
void* proactor_run(void* this) {
    proactor_t* proactor = (proactor_t*) this;
    fd_set read_fds; // Temp file descriptor list for select()
    struct timeval timeout;

    while (proactor->running) {
        // copy the original set to a temp set because select() modifies the set passed to it
        read_fds = proactor->fds;
        timeout.tv_sec = 2; // Set a timeout of 2 seconds. Adjust this value as needed.
        timeout.tv_usec = 0;

        // We use select to wait for data on any socket
        if (select(proactor->maxfd + 1, &read_fds, NULL, NULL, &timeout) == -1) {
            perror("select");
            continue;
        }

        // run through the existing connections looking for data to read
        for (int i = 0; i <= proactor->maxfd; i++) {
            if (FD_ISSET(i, &read_fds)) { // we got one!!
                // call the appropriate handler for this file descriptor
                // for example:
                handler_t handler = proactor->connection_handlers[i];
                if (handler != NULL) {
                    handler(i, proactor);
                }
            }
        }
    }

    return NULL;
}


// Remove a file descriptor from proactor
int removeHandler(void* this, int fd) {
    proactor_t* proactor = (proactor_t*) this;
    FD_CLR(fd, &proactor->fds);
    proactor->count--;
    // Remove the handlers somewhere, the details of this are up to you.
    return 0;
}
