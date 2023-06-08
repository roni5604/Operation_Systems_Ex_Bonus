#ifndef ST_REACTOR_H
#define ST_REACTOR_H

#include <pthread.h>
#include <stdlib.h>
#include <sys/select.h>

// Handler function type.
typedef void (*handler_t)(int fd, void *arg);

// Structure representing a reactor.
typedef struct reactor_t {
    fd_set fds;            // Set of file descriptors to monitor.
    handler_t *handlers;   // Array of handler functions, one for each file descriptor.
    void **args;           // Array of argument pointers to pass to each handler function.
    int maxfd;             // Highest file descriptor number currently in the set.
    int count;             // Number of file descriptors currently in the set.
    pthread_t thread;      // The thread running the reactor.
    int running;           // Flag indicating whether the reactor is running.
} reactor_t;

// Function prototypes
void* createReactor();
void stopReactor(void* this);
void startReactor(void* this);
void addFd(void* this, int fd, handler_t handler);
void waitFor(void* this);
void* reactor_run(void* arg);
void removeFd(reactor_t* reactor, int fd);


#endif