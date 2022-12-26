#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <cerrno>

#define PORT                "3434"  // the port which users will connect to
#define MAX_REQUEST_QUEUED  10      // how many pending connections to store in the queue

int main()
{
    // Prepare the results linked list to store information about the socket to be used
    // for listening for incoming requests
    struct addrinfo hints;
    struct addrinfo* results;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags    = AI_PASSIVE;

    int status;
    printf("Getting address information of the local host...\n");
    if( (status = getaddrinfo(NULL, PORT, &hints, &results)) != 0 ) {
        fprintf(stderr, "getaddrinfo(): %s", gai_strerror(status));
        return 1;
    }

    // Initialize the socket with the information resolved from the previous results
    // Ideally, given the parameters set in hints, the socket has
    //      either an IPv4 or IPv6 address on one of the network interfaceS (AF_UNSPEC)
    //      a stream socket which communicates over TCP (SOCK_STREAM)
    //      and that the IP address information is filled in (AI_PASSIVE)
    printf("Requesting for the socket file descriptor...\n");
    int listening_socket = socket(results->ai_family, results->ai_socktype, results->ai_protocol);

    if (listening_socket < 0) {
        fprintf(stderr, "socket(): %d", errno);
        return 1;
    }

    // Once we have a socket file descriptor we bind it to a port and an address on our local machine
    printf("Binding the socket...\n");
    if (bind(listening_socket, results->ai_addr, results->ai_addrlen) < 0) {
        fprintf(stderr, "bind(): %d", errno);
        return 1;
    }

    // Mark the socket as passive, e.g. a socket that will be used to listen for incoming connections
    printf("Marking the socket as a passive (listening) socket\n");
    if (listen(listening_socket, MAX_REQUEST_QUEUED) < 0) {
        fprintf(stderr, "listen(): %d", errno);
        return 1;
    }

    // These are used to store information about clients that connect to this server
    struct sockaddr_storage client_addr;
    socklen_t addr_size = sizeof(client_addr);

    // For now, a fixed size buffer is used to store incoming data
    const uint32_t BUFFER_SIZE = 1024;
    char buffer[BUFFER_SIZE];
    
    // TODO: 
    // This is a *very* simple implementation of the processing loop for now, this means that
    // each request is synchronously handled without multiplexing.
    while(true) {
        printf("Ready to accept connections...\n");

        int client_socket = accept(listening_socket,
                                   (struct sockaddr*) &client_addr,
                                   &addr_size);

        // Ready to receive the incoming data
        int num_bytes_in = recv(client_socket, buffer, BUFFER_SIZE, 0);
        printf("Received %d bytes from the client", num_bytes_in);
        printf("Client request data: %s\n", buffer);

        // Send back a response, in the future this will be image data for the rendered frame
        // but for testing purposes right now this is a simple HTTP response
        const char* message = 
            "HTTP/1.1\r\n"
            "Content-Type: text/html\r\n\r\n"
            "<html><h1>Hello world!</h1></html>";

        send(client_socket, message, strlen(message), 0);
        close(client_socket);
    }

}
