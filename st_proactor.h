#ifndef ST_PROACTOR_H
#define ST_PROACTOR_H

#include <pthread.h>
#include <stdlib.h>
#include <sys/select.h>

// Connection handler function type.
typedef int (*handler_t)(int fd, void *arg);

// Message handler function type.
typedef int (*message_handler_t)(int fd, void *arg);

// Structure representing a proactor.
typedef struct proactor_t {
    fd_set fds;                     // Set of file descriptors to monitor.
    handler_t *connection_handlers; // Array of connection handler functions, one for each file descriptor.
    message_handler_t *message_handlers; // Array of message handler functions, one for each file descriptor.
    int maxfd;                      // Highest file descriptor number currently in the set.
    int count;                      // Number of file descriptors currently in the set.
    pthread_t thread;               // The thread running the proactor.
    int running;                    // Flag indicating whether the proactor is running.
} proactor_t;

// Function prototypes
void* createProactor(void* args);
int runProactor(void* this);
int cancelProactor(void* this);
int addFD2Proactor(void* this, int fd, handler_t handler, message_handler_t msg_handler);
int removeHandler(void* this, int fd);
void* proactor_run(void* this);

#endif
