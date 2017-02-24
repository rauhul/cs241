/**
 * Machine Problem: Vector
 * CS 241 - Fall 2016
 */

#include "document.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <string.h>

struct _document {
    Vector *vector;
};

// This is the constructor function for string element.
// Use this as copy_constructor callback in vector.
void *my_copy_ctor(void *elem) {
    char *str = elem;
    assert(str);
    return strdup(str);
}

// This is the destructor function for string element.
// Use this as destructor callback in vector.
void my_destructor(void *elem) { free(elem); }

Document *Document_create() {
    Document *document = malloc(sizeof(Document));
    if (!document)
        return NULL;

    document->vector = Vector_create(my_copy_ctor, my_destructor);
    if (!document->vector) {
        free(document);
        return NULL;
    }

    return document;
}

void Document_write_to_file(Document *document, const char *filename) {
    assert(document);
    assert(filename);

    FILE *fd = fopen(filename, "w+");

    char *line;
    for (size_t iter = 0; iter < Vector_size(document->vector); iter++) {
        if ( (line = Vector_get(document->vector, iter)) )
            fprintf(fd, "%s\n", line);
        else
            fprintf(fd, "\n");
    }

    fclose(fd);
}

Document *Document_create_from_file(const char *filename) {
    assert(filename);

    Document *document = Document_create();

    FILE *fd = fopen(filename, "r");
    if (!fd)
        return document;

    char *buffer        = NULL;
    size_t  buffer_size = 0;
    ssize_t size;

    size_t line_number = 1;
    while ((size = getline(&buffer, &buffer_size, fd)) != -1) {
        if (size && buffer[size-1] == '\n') {
            buffer[size-1] = '\0';
            size--;
        }

        if (size)
            Document_insert_line(document, line_number, buffer);

        line_number++;
    }
    free(buffer);
    fclose(fd);

    return document;
}

void Document_destroy(Document *document) {
    assert(document);
    Vector_destroy(document->vector);
    free(document);
}

size_t Document_size(Document *document) {
    assert(document);
    return Vector_size(document->vector);
}

void Document_set_line(Document *document, size_t line_number,
                                             const char *str) {
    assert(document);
    assert(str);
    assert(line_number > 0);
    size_t index = line_number - 1;
    Vector_set(document->vector, index, (void *)str);
}

const char *Document_get_line(Document *document, size_t line_number) {
    assert(document);
    assert(line_number > 0);
    size_t index = line_number - 1;
    return (const char *)Vector_get(document->vector, index);
}

void Document_insert_line(Document *document, size_t line_number,
                                                    const char *str) {
    assert(document);
    assert(str);
    assert(line_number > 0);
    size_t index = line_number - 1;
    Vector_insert(document->vector, index, (void *)str);
}

void Document_delete_line(Document *document, size_t line_number) {
    assert(document);
    assert(line_number > 0);
    size_t index = line_number - 1;
    Vector_delete(document->vector, index);
}
