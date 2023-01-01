#include "client_queue.h"

void enqueue_client(ClientQueue* queue, int new_client)
{
    struct client_list* new_node = (struct client_list*) malloc(sizeof(struct client_list));
    new_node->client_fd = new_client;
    new_node->next      = NULL;

    if(queue->front == NULL) {
        queue->front = new_node;
        queue->back  = queue->front;
    } else {
        queue->back->next = new_node;
        queue->back = new_node;
    }

    queue->size++;
}

struct client_list* dequeue_client(ClientQueue* queue)
{
    if(queue->front == NULL) {
        return NULL;
    } 

    struct client_list* front = queue->front;
    queue->front  = queue->front->next;

    return front;
}
