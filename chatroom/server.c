/**
 * Chatroom Lab
 * CS 241 - Fall 2016
 */
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "utils.h"

#define MAX_CLIENTS 8

void *process_client(void *p);
void write_to_clients(const char *message, size_t size);

static volatile int sessionEnd;
static volatile int serverSocket;

static volatile int clientsCount;
static volatile int clients[MAX_CLIENTS];

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

struct addrinfo hints, *infoptr; // So no need to use memset global variables

void close_server(int signal) {
    if (signal == SIGINT) {
        shutdown(serverSocket, SHUT_RDWR);
        close(serverSocket);
        serverSocket = -1;
        for (int i = 0; i < MAX_CLIENTS; i++)
            if (clients[i] != -1)
                shutdown(clients[i], SHUT_RDWR);
        sessionEnd = 1;
        if (infoptr) {
            freeaddrinfo(infoptr);
        }
    }
}

void exit_failure() {
    close_server(SIGINT);

    if (infoptr) {
        freeaddrinfo(infoptr);
    }
    exit(1);
}

void run_server(char *port) {

    int retval;
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        perror("socket():");
        exit_failure();
    }

    hints.ai_family     = AF_INET;
    hints.ai_socktype   = SOCK_STREAM;
    hints.ai_flags      = AI_PASSIVE;

    retval = getaddrinfo(NULL, port, &hints, &infoptr);
    if (retval) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(retval));
        exit_failure();
    }

    int optval = 1;
    retval = setsockopt(serverSocket, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
    if (retval == -1) {
        perror("setsockopt():");
        exit_failure();
    }

    if (bind(serverSocket, infoptr->ai_addr, infoptr->ai_addrlen)) {
        perror("bind():");
        exit_failure();
    }

    if (listen(serverSocket, 10)) {
        perror("listen():");
        exit_failure();
    }

    struct sockaddr_in *infoptr_addr = (struct sockaddr_in *) infoptr->ai_addr;
    printf("Listening on file descriptor %d, port %d\n", serverSocket, ntohs(infoptr_addr->sin_port));
    printf("Waiting for connection...\n");

    for (int i = 0; i < MAX_CLIENTS; i++)
        clients[i] = -1;

    while (sessionEnd == 0) {

        if (clientsCount < MAX_CLIENTS) {

            // Can now start accepting and processing client connections
            struct sockaddr clientAddr;
            socklen_t clientAddrlen = sizeof(struct sockaddr);
            memset(&clientAddr, 0, sizeof(struct sockaddr));

            int client_fd = accept(serverSocket, (struct sockaddr *) &clientAddrlen, &clientAddrlen);
            if (sessionEnd != 0) {
                fprintf(stderr, "accept():: Invalid argument\n");
                break;
            }
            if (client_fd == -1) {
                perror("accept():");
                exit_failure();
            }

            // Looking for a lowest available clientId
            // Writing client file descriptor to clients array
            intptr_t clientId = -1;
            pthread_mutex_lock(&mutex);
            for (int i = 0; i < MAX_CLIENTS; i++)
                if (clients[i] == -1) {
                    clients[i] = client_fd;

                    // Printing out IP address of newly joined clients
                    char clientIp[INET_ADDRSTRLEN];
                    if (inet_ntop(AF_INET, &clientAddr, clientIp, clientAddrlen) != 0) {
                        printf("Client %d joined on %s\n", i, clientIp);
                    }
                    clientId = i;
                    break;
                }
            clientsCount++;
            printf("Currently servering %d clients\n", clientsCount);
            pthread_mutex_unlock(&mutex);

            // Launching a new thread to serve the client
            pthread_t thread;
            int retval = pthread_create(&thread, NULL, process_client, (void *)clientId);
            if (retval != 0) {
                perror("pthread_create():");
                exit(1);
            }
            printf("Waiting for connection...\n");
        }
    }
    if (serverSocket != -1) {
        close_server(SIGINT);
    }
}

// Broadcasting message to every active client
// DO NOT MODIFY THE FUNCTION BELOW
void write_to_clients(const char *message, size_t size) {
    pthread_mutex_lock(&mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] != -1) {
            ssize_t retval = write_message_size(size, clients[i]);
            if (retval > 0)
                retval = write_all_to_socket(clients[i], message, size);
            if (retval == -1)
                perror("write():");
            printf("sent %zu bytes\n", retval);
        }
    }
    pthread_mutex_unlock(&mutex);
}

// Read messages from client and sends it out to everyone
// DO NOT MODIFY THE FUNCTION BELOW
void *process_client(void *p) {
    pthread_detach(pthread_self());
    intptr_t clientId = (intptr_t)p;
    ssize_t retval = 1;
    char *buffer = NULL;

    while (retval > 0 && sessionEnd == 0) {
        retval = get_message_size(clients[clientId]);
        if (retval > 0) {
            buffer = calloc(1, retval);
            retval = read_all_from_socket(clients[clientId], buffer, retval);
        }
        if (retval > 0)
            write_to_clients(buffer, retval);

        free(buffer);
        buffer = NULL;
    }

    printf("User %d left\n", (int)clientId);
    close(clients[clientId]);

    pthread_mutex_lock(&mutex);
    clients[clientId] = -1;
    clientsCount--;
    pthread_mutex_unlock(&mutex);

    return NULL;
}

int main(int argc, char **argv) {

    if (argc != 2) {
        fprintf(stderr, "./server <port>\n");
        return -1;
    }

    signal(SIGINT, close_server);
    run_server(argv[1]);

    return 0;
}