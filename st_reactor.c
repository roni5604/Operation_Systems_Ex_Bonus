#include "st_reactor.h"

#define MAX_FDS 1024 // Arbitrary maximum number of file descriptors

void* createReactor() {
    reactor_t* reactor = malloc(sizeof(reactor_t));
    if (reactor == NULL) {
        return NULL; // Allocation failed
    }

    FD_ZERO(&reactor->fds);
    reactor->handlers = calloc(MAX_FDS, sizeof(handler_t));
    reactor->args = calloc(MAX_FDS, sizeof(void *));
    reactor->maxfd = -1;
    reactor->count = 0;
    reactor->running = 0;

    return reactor;
}

void stopReactor(void* this) {
    reactor_t* reactor = (reactor_t *) this;
    if (reactor == NULL || !reactor->running) {
        return;
    }

    reactor->running = 0;
    pthread_join(reactor->thread, NULL); // Wait for the reactor thread to exit
}

void startReactor(void* this) {
    reactor_t* reactor = (reactor_t *) this;
    if (reactor == NULL || reactor->running) {
        return;
    }

    reactor->running = 1;
    pthread_create(&reactor->thread, NULL, reactor_run, reactor);
}

void addFd(void* this, int fd, handler_t handler) {
    reactor_t *reactor = (reactor_t *)this;
    if (reactor == NULL || fd < 0 || fd >= MAX_FDS || handler == NULL) {
        return;
    }

    FD_SET(fd, &reactor->fds);
    reactor->handlers[fd] = handler;
    reactor->count++;
    if (fd > reactor->maxfd) {
        reactor->maxfd = fd;
    }
}

void waitFor(void* this) {
    reactor_t *reactor = (reactor_t *)this;
    if (reactor == NULL || !reactor->running) {
        return;
    }

    pthread_join(reactor->thread, NULL); // Wait for the reactor thread to exit
}

void* reactor_run(void* arg) {
    reactor_t* reactor = arg;
    fd_set read_fds;

    while (reactor->running) {
        read_fds = reactor->fds;
        if (select(reactor->maxfd + 1, &read_fds, NULL, NULL, NULL) == -1) {
            // Handle select() error
            break;
        }

        for (int i = 0; i <= reactor->maxfd; i++) {
            if (FD_ISSET(i, &read_fds)) {
                // Invoke the handler function for this fd
                reactor->handlers[i](i, reactor->args[i]);
            }
        }
    }

    return NULL;
}

void removeFd(reactor_t* reactor, int fd) {
    if (reactor == NULL || fd < 0 || fd >= MAX_FDS || !FD_ISSET(fd, &reactor->fds)) {
        return;
    }

    FD_CLR(fd, &reactor->fds);
    reactor->handlers[fd] = NULL;
    reactor->args[fd] = NULL;
    reactor->count--;
    if (fd == reactor->maxfd) {
        // Find the new maxfd
        for (int i = fd - 1; i >= 0; i--) {
            if (FD_ISSET(i, &reactor->fds)) {
                reactor->maxfd = i;
                break;
            }
        }
    }
}
