/**
 * Machine Problem: Wearables
 * CS 241 - Fall 2016
 */

#include <assert.h>
#include <fcntl.h>
#include <netdb.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "utils.h"
#include "vector.h"
#include "wearable.h"

#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#define _LOG 0
#define LOG(format, ...) if (_LOG) { \
    fprintf(stderr, ANSI_COLOR_BLUE format ANSI_COLOR_MAGENTA "\t(%s, %s::%d)\n" ANSI_COLOR_RESET, \
    ## __VA_ARGS__, __FILE__, __func__, __LINE__); }

#define BLOCK_SIZE 64

typedef struct {
    pthread_t thread;
    int fd;
    long timestamp;
    int completed;
} thread_data;

// The wearable server socket, which all wearables connect to.
int wearable_server_fd;
int server_active;
pthread_mutex_t vector_mutex;
pthread_cond_t  user_request_thread_cond;

int user_request_socketfd;
int flag;

Vector *data_vector;
Vector *wearables_vector;

void *fake_constructor(void *elem) { return elem; }
void fake_destructor(void *elem) { }

void signal_received(int sig) {
    LOG("sig: %d", sig);

    if (sig == SIGINT) {
        server_active = 0;
        shutdown(wearable_server_fd, SHUT_RDWR);

        if (flag) {
            shutdown(user_request_socketfd, SHUT_RDWR);
        }

        pthread_cond_broadcast(&user_request_thread_cond);

    } else {
        assert(0);
    }
}


int wait_to_send_data(long timestamp) {
    for (size_t i = 0; i < Vector_size(wearables_vector); i++) {
        thread_data *td = (thread_data *) Vector_get(wearables_vector, i);
        if (!td->completed) {
            if (timestamp > td->timestamp) {
                return 1;
            }
        }
    }
    return 0;
}

void *wearable_processor_thread(void *args) {
    pthread_mutex_lock(&vector_mutex);
    thread_data *td = (thread_data *)args;
    int socketfd = td->fd;
    pthread_mutex_unlock(&vector_mutex);
    LOG("wearable started")

    char buff[BLOCK_SIZE];
    memset(buff, 0, BLOCK_SIZE);
    while (read(socketfd, buff, BLOCK_SIZE) > 0) {
        SampleData *data;
        long timestamp;
        extract_key(buff, &timestamp, &data);
        memset(buff, 0, BLOCK_SIZE);

        timestamp_entry *entry = malloc(sizeof(timestamp_entry));
        entry->time = (uint64_t) timestamp;
        entry->data = (void *) data;

        pthread_mutex_lock(&vector_mutex);
        Vector_append(data_vector, entry);
        LOG("%lu: %s %d", timestamp, data->type_, data->data_)
        td->timestamp = (uint64_t) timestamp;

        pthread_cond_broadcast(&user_request_thread_cond);
        pthread_mutex_unlock(&vector_mutex);
    }
    pthread_mutex_lock(&vector_mutex);
    td->completed = 1;
    pthread_cond_broadcast(&user_request_thread_cond);
    pthread_mutex_unlock(&vector_mutex);

    LOG("wearable exited")
    close(socketfd);
    return NULL;
}

bool type_1_predicate(timestamp_entry *entry) {
    SampleData *data = (SampleData *) entry->data;
    return strcmp(data->type_, TYPE1) == 0;
}

bool type_2_predicate(timestamp_entry *entry) {
    SampleData *data = (SampleData *) entry->data;
    return strcmp(data->type_, TYPE2) == 0;
}

bool type_3_predicate(timestamp_entry *entry) {
    SampleData *data = (SampleData *) entry->data;
    return strcmp(data->type_, TYPE3) == 0;
}

void *user_request_thread(void *args) {
    user_request_socketfd = *((int *)args);
    flag = 1;

    char buff[BLOCK_SIZE];
    memset(buff, 0, BLOCK_SIZE);
    long timestamp1;
    long timestamp2;

    ssize_t num_type1_entries;
    ssize_t num_type2_entries;
    ssize_t num_type3_entries;
    timestamp_entry *type1_entries;
    timestamp_entry *type2_entries;
    timestamp_entry *type3_entries;

    while (read(user_request_socketfd, buff, BLOCK_SIZE) > 0) {
        if (sscanf(buff, "%lu:%lu", &timestamp1, &timestamp2) != 2) {
            LOG("incorrect buffer format")
            break;
        }
        LOG("RECV REQUEST: %lu,%lu", timestamp1, timestamp2)

        memset(buff, 0, BLOCK_SIZE);

        pthread_mutex_lock(&vector_mutex);
        while (wait_to_send_data(timestamp2) && server_active) {
            LOG("sleep");
            pthread_cond_wait(&user_request_thread_cond, &vector_mutex);
        }

        if (!server_active) {
            pthread_mutex_unlock(&vector_mutex);
            break;
        }

        // Write out statistics for data between those timestamp ranges
        num_type1_entries = gather_timestamps(data_vector, timestamp1, timestamp2, type_1_predicate, &type1_entries);
        num_type2_entries = gather_timestamps(data_vector, timestamp1, timestamp2, type_2_predicate, &type2_entries);
        num_type3_entries = gather_timestamps(data_vector, timestamp1, timestamp2, type_3_predicate, &type3_entries);
        pthread_mutex_unlock(&vector_mutex);

        write_results(user_request_socketfd, TYPE1, type1_entries, num_type1_entries);
        write_results(user_request_socketfd, TYPE2, type2_entries, num_type2_entries);
        write_results(user_request_socketfd, TYPE3, type3_entries, num_type3_entries);
        write(user_request_socketfd, "\r\n", 2);

        free(type1_entries);
        free(type2_entries);
        free(type3_entries);
    }

    flag = 1;

    LOG("start clean up")

    close(user_request_socketfd);
    return NULL;
}


// given a string with the port value, set up a passive socket file
// descriptor and return it
int open_server_socket(const char *port) {

    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1) {
        perror("socket():");
        exit(EXIT_FAILURE);
    }

    struct addrinfo hints, *result;
    memset(&hints, 0, sizeof(struct addrinfo));

    hints.ai_family     = AF_INET;
    hints.ai_socktype   = SOCK_STREAM;
    hints.ai_flags      = AI_PASSIVE;

    int retval = getaddrinfo(NULL, port, &hints, &result);
    if (retval) {
        fprintf(stderr, "getaddrinfo()::%s\n", gai_strerror(retval));
        exit(EXIT_FAILURE);
    }

    int optval = 1;
    retval = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
    if (retval == -1) {
        perror("setsockopt():");
        exit(EXIT_FAILURE);
    }

    if (bind(socket_fd, result->ai_addr, result->ai_addrlen)) {
        perror("bind():");
        exit(EXIT_FAILURE);
    }

    if (listen(socket_fd, 10)) {
        perror("listen():");
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(result);

    return socket_fd;
}

int wearable_server(const char *wearable_port, const char *request_port) {
    LOG("wearable_port: %s, request_port: %s", wearable_port, request_port);

    // setup signal handler for SIGINT
    signal(SIGINT, signal_received);

    server_active = 1;

    data_vector      = Vector_create(fake_constructor, fake_destructor);
    wearables_vector = Vector_create(fake_constructor, fake_destructor);

    pthread_mutex_init(&vector_mutex, NULL);
    pthread_cond_init(&user_request_thread_cond, NULL);

    int request_server_fd = open_server_socket(request_port);
    wearable_server_fd    = open_server_socket(wearable_port);

    pthread_t request_thread;
    int request_socket = accept(request_server_fd, NULL, NULL);
    pthread_create(&request_thread, NULL, user_request_thread, &request_socket);
    close(request_server_fd);

    // accept continous requests on the wearable port
    while(server_active) {
        thread_data *td = malloc(sizeof(thread_data));
        td->fd = accept(wearable_server_fd, NULL, NULL);
        if (td->fd < 0) {
            free(td);
            break;
        }
        td->completed = 0;
        pthread_mutex_lock(&vector_mutex);
        Vector_append(wearables_vector, td);
        pthread_create(&(td->thread), NULL, wearable_processor_thread, (void*)td);
        pthread_mutex_unlock(&vector_mutex);
    }
    close(wearable_server_fd);


    // Cleanup anything we've allocated.
    pthread_join(request_thread, NULL);

    // join all the wearable threads and clean up the alloced thread_data
    pthread_mutex_lock(&vector_mutex);
    while (Vector_size(wearables_vector)) {
        thread_data *td = (thread_data *) Vector_get(wearables_vector, 0);
        pthread_mutex_unlock(&vector_mutex);

        pthread_join(td->thread, NULL);
        free(td);

        pthread_mutex_lock(&vector_mutex);
        Vector_delete(wearables_vector, 0);
    }
    pthread_mutex_unlock(&vector_mutex);

    // clean up the allocated wearable data points
    while (Vector_size(data_vector)) {
        timestamp_entry *entry = (timestamp_entry *) Vector_get(data_vector, 0);
        SampleData *data = (SampleData *) entry->data;
        free(data->type_);
        free(data);
        free(entry);
        Vector_delete(data_vector, 0);
    }

    // clean up cond variable and mutexes
    pthread_cond_destroy(&user_request_thread_cond);
    pthread_mutex_destroy(&vector_mutex);

    // clean up vector data structures
    Vector_destroy(wearables_vector);
    Vector_destroy(data_vector);

    return 0;
}
