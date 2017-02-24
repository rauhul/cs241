/**
 * MapReduce
 * CS 241 - Fall 2016
 */

#include "common.h"

void usage() {
    fprintf(stderr, "shuffler destination1 destination2 ...\n");
    fprintf(stderr, "where destination1..n are files on the filesystem to which "
                    "the shuffler will write its output\n");
}

int main(int argc, char *argv[]) {

    int N = argc - 1;

    if (argc < 2) {
        usage();
        exit(1);
    }

    int *output_fds = malloc(sizeof(int) * N);

    LOG("N:%d", N);
    for (int i = 0; i < N; i++) {
        LOG("open: %s", argv[i + 1])
        output_fds[i] = open(argv[i + 1], O_WRONLY );
    }

    LOG("read stdin")
    char   *line = NULL;
    size_t  len = 0;
    ssize_t read;

    char *key;
    char *value;

    while ((read = getline(&line, &len, stdin)) != -1) {
        line[read - 1] = '\0';
        key      = line;
        value    = strchr(key, ':');
        value[0] = '\0';
        value   += 2;

        char buf[256];
        snprintf(buf, 256, "%s: %s\n", key, value);
        int len = strlen(buf);
        if (write(output_fds[ hashKey(key) % N ], buf, sizeof(char) * len) == -1) {
            LOG("write failed")
            perror("write failed");
            end(123);
        }
    }

    LOG("SHUFFLER Clean up");
    free(line);
    //close files
    for (int i = 0; i < N; i++) {
        LOG("Close file: %d", i)
        close(output_fds[i]);
    }
    free(output_fds);

    return 0;
}
