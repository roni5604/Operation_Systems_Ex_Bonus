#ifndef ST_PROACTOR_H
#define ST_PROACTOR_H

#include <pthread.h>
#include <stdlib.h>
#include <sys/select.h>

// Connection handler function type.
typedef int (*handler_t)(void *arg);

// Structure representing a proactor.
typedef struct proactor_t {
    fd_set fds;                     // Set of file descriptors to monitor.
    // handler_t* array_of_handlers;
    int maxfd;                      // Highest file descriptor number currently in the set.
    int count;                      // Number of file descriptors currently in the set.
    pthread_t thread;               // The thread running the proactor.
    int running;                    // Flag indicating whether the proactor is running.
    int current_hot_fd;
    int server_listener;
    handler_t handle_new_connection;
    handler_t handle_new_message;
} proactor_t;

// Function prototypes
void* createProactor(void* args);
int runProactor(void* this);
int cancelProactor(void* this);
int addFD2Proactor(void* this, int fd, handler_t handler);
int removeHandler(void* this, int fd);
void* proactor_run(void* this);

#endif
