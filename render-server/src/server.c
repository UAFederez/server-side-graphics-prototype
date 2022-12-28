#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>

#include "server.h"
#include "base64.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_WRITE_NO_STDIO
#include <stb_image_write.h>

#define THREAD_POOL_SIZE    8
#define PORT                "3434"  // the port which users will connect to
#define MAX_REQUEST_QUEUED  10      // how many pending connections to store in the queue
#define NUM_CHANNELS        4       // render using RGBA

struct client_list {
    int    client_fd;
    struct client_list* next;
};

typedef struct {
    struct client_list* front;
    struct client_list* back;
    uint32_t size;
} ClientQueue;

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

typedef struct {
    pthread_mutex_t mtx;
    pthread_cond_t  cv;
    ClientQueue     clients; 
} ThreadContext;

void write_to_response_stbi(void* context, void* data, int size)
{
    RenderResponse* response = ((RenderResponse*)context);
    append_to_buffer(&response->image_raw_bytes, data, size);
}

float calc_ray_sphere_intersect(Vector3f ray_orig, Vector3f ray_dir, Vector3f sphere_pos)
{
    Vector3f oc = sub_v3f(ray_orig, sphere_pos);
    float a = dot_v3f(ray_dir, ray_dir);
    float b = 2.0 * dot_v3f(oc, ray_dir);
    float c = dot_v3f(oc, oc) - (0.5f * 0.5f);
    float d = b * b - (4 * a * c);

    if(d < 0) {
        return -1.0f;
    }
    return (-b - sqrt(d)) / (2.0 * a);
}

Vector3f trace_ray(Vector3f ray_orig, Vector3f ray_dir, RenderRequest* request)
{
    Vector3f ray_norm = normalize_v3f(&ray_dir);
    Vector3f light = { 1.0f, 1.0f, 1.0f };
    Vector3f dark  = { 0.0f, 0.0f, 0.0f };

    float dist_along_ray = 0.0;
    if((dist_along_ray = calc_ray_sphere_intersect(ray_orig, ray_norm, request->sphere_pos)) > 0) {
        Vector3f point_on_sphere = add_v3f(ray_orig, mulf_v3f(ray_dir, dist_along_ray));
        Vector3f normal = sub_v3f(point_on_sphere, request->sphere_pos);
        Vector3f half = { 0.5, 0.5, 0.5 };
        Vector3f vec_to_light = sub_v3f(request->light_pos, point_on_sphere);

        float lightness = fmax(0.0f, dot_v3f(normal, normalize_v3f(&vec_to_light)));

        return mulf_v3f(add_v3f(normal, half), lightness); 
    }

    return lerp_v3f(&light, &dark, 0.5f + ray_norm.y);
}

void render_image(RenderRequest* request, RenderResponse* response)
{
    const uint32_t num_image_bytes = request->image_width * request->image_height * NUM_CHANNELS;
    response->pixel_buffer = allocate_buffer(num_image_bytes);

    uint8_t* current_pixel = response->pixel_buffer.data;

    Vector3f camera_pos = { 0.0f, 0.0f, 0.0f };
    Vector3f ray_dir    = { 0.0f, 0.0f, 0.0f };
    for(uint32_t y = 0; y < request->image_height; y++) {
        for(uint32_t x = 0; x < request->image_width; x++) {

            float u = (float) x / request->image_width;
            float v = (float) y / request->image_height;

            ray_dir.x = -0.5f + u;
            ray_dir.y =  0.5f - v;
            ray_dir.z =  1.0f;

            Vector3f color = trace_ray(camera_pos, ray_dir, request);

            current_pixel[0] = (uint8_t)( color.x * 255.0f);
            current_pixel[1] = (uint8_t)( color.y * 255.0f);
            current_pixel[2] = (uint8_t)( color.z * 255.0f);
            current_pixel[3] = 0xFF;        
            current_pixel += NUM_CHANNELS;
        }   
    }

    stbi_write_jpg_to_func(&write_to_response_stbi,
                           (void*) response,
                           request->image_width,
                           request->image_height,
                           NUM_CHANNELS,
                           response->pixel_buffer.data,
                           25);

    response->image_base64_encoded.data = (uint8_t*) base64_encode_fast((char*) response->image_raw_bytes.data,
                                                                        response->image_raw_bytes.size);
    response->image_base64_encoded.size = strlen(response->image_base64_encoded.data);
}

void handle_client_connection(int client_socket)
{
    RenderRequest request;
    memset(&request, 0, sizeof(request));

    int num_bytes_received = recv(client_socket, &request, sizeof(request), 0);
    if (num_bytes_received < 0) {
        fprintf(stderr, "Error: %d\n", errno);
        return;
    }

    //printf("Received %d bytes from the client. Expected %lu\n", num_bytes_received, sizeof(request));

    RenderResponse response;
    memset(&response, 0, sizeof(response));

    render_image(&request, &response);

    int num_bytes_sent = send(client_socket, 
                              response.image_base64_encoded.data, 
                              response.image_base64_encoded.size,
                              0);
                              
    if (num_bytes_sent < 0) {
        fprintf(stderr, "Error: %d\n", errno);
    }

    //printf("Replied with %d bytes\n", num_bytes_sent);

    deallocate_buffer(&response.pixel_buffer);
    deallocate_buffer(&response.image_raw_bytes);
    deallocate_buffer(&response.image_base64_encoded);
    close(client_socket);

    return;
}

void* thread_client_handler(void* ctx)
{
    ThreadContext* context = (ThreadContext*) ctx;
    printf("Thread ready to accept connections!\n");
    for(;;) {
        pthread_mutex_lock(&context->mtx);
        struct client_list* next_client = dequeue_client(&context->clients);
        if(next_client == NULL) {
            pthread_cond_wait(&context->cv, &context->mtx);
            next_client = dequeue_client(&context->clients);
        }
        pthread_mutex_unlock(&context->mtx);

        if(next_client != NULL) {
            // We have a new client
            handle_client_connection(next_client->client_fd);
            close(next_client->client_fd);
            free(next_client);
        }
    }
    return NULL;
}

int main()
{
    // Prepare the results linked list to store information about the socket to be used
    // for listening for incoming requests
    struct addrinfo hints;
    struct addrinfo* results;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_UNSPEC;      // either an IPv4 or IPv6 address will do
    hints.ai_socktype = SOCK_STREAM;    // a stream socket to communicate using TCP
    hints.ai_flags    = AI_PASSIVE;     // Automatically fill in address information

    int status;
    printf("Getting address information of the local host...\n");
    if( (status = getaddrinfo(NULL, PORT, &hints, &results)) != 0 ) {
        fprintf(stderr, "getaddrinfo(): %s", gai_strerror(status));
        return 1;
    }

    printf("Requesting for the socket file descriptor...\n");
    int listening_socket = socket(results->ai_family, results->ai_socktype, results->ai_protocol);

    if (listening_socket < 0) {
        fprintf(stderr, "socket(): %d", errno);
        return 1;
    }
    // Allow this server to reuse the address when the program is exited
    int socket_options = 1; 
    if(setsockopt(listening_socket, SOL_SOCKET, SO_REUSEADDR, &socket_options, sizeof(int)) < 0) {
        fprintf(stderr, "setsockopt(): %d", errno);
        return 1;
    }

    // Once we have a socket file descriptor we bind it to a port and an address on our local machine
    printf("Binding the socket...\n");
    if (bind(listening_socket, results->ai_addr, results->ai_addrlen) < 0) {
        fprintf(stderr, "bind(): %d", errno);
        return 1;
    }

    ThreadContext context = {};
    pthread_mutex_init(&context.mtx, NULL);
    pthread_cond_init(&context.cv, NULL);
    memset(&context.clients, 0, sizeof(context.clients));

    pthread_t thread_pool[THREAD_POOL_SIZE];
    for(int i = 0; i < THREAD_POOL_SIZE; i++) {
        pthread_create(&thread_pool[i], NULL, &thread_client_handler, (void*)&context);
    }

    // Mark the socket as passive, e.g. a socket that will be used to listen for incoming connections
    printf("Marking the socket as a passive (listening) socket\n");
    if (listen(listening_socket, MAX_REQUEST_QUEUED) < 0) {
        fprintf(stderr, "listen(): %d", errno);
        return 1;
    }

    // Storing information about the clients that connect to this server
    struct sockaddr_storage client_addr_info;
    socklen_t client_addr_size = sizeof(client_addr_info);

    // We store a string large enough to hold an IPv6 address since it is longer than
    // IPv4 addresses
    char client_address[INET6_ADDRSTRLEN];

    for(;;) {
        int client_socket = accept(listening_socket,
                                   (struct sockaddr*) &client_addr_info,
                                   &client_addr_size);
        
        void* client_sock_addr       = NULL;
        struct sockaddr* client_addr = (struct sockaddr*)&client_addr_info;

        // Need to cast the client socket information to the appropriate type
        // in order to get the proper client address.
        if (client_addr->sa_family == AF_INET) {
            // Client connected with an IPv4 address
            client_sock_addr = &((struct sockaddr_in*)&client_addr_info)->sin_addr;
        } else {
            // Client connected with an IPv6 address
            client_sock_addr = &((struct sockaddr_in6*)&client_addr_info)->sin6_addr;
        }

        inet_ntop(client_addr_info.ss_family,
                  client_sock_addr,
                  client_address,
                  sizeof(client_address));

        printf("Received a connection from host %s\n", client_address);

        pthread_mutex_lock(&context.mtx);
        enqueue_client(&context.clients, client_socket);
        pthread_cond_signal(&context.cv);
        pthread_mutex_unlock(&context.mtx);

    }

    close(listening_socket);
}
