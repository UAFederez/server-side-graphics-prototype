#ifndef SSR_CLIENT_QUEUE_H
#define SSR_CLIENT_QUEUE_H

#include <stdint.h>
#include <stdlib.h>

struct client_list {
    int    client_fd;
    struct client_list* next;
};

typedef struct {
    struct client_list* front;
    struct client_list* back;
    uint32_t size;
} ClientQueue;

void enqueue_client(ClientQueue* queue, int new_client);
struct client_list* dequeue_client(ClientQueue* queue);

#endif
