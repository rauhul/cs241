/**
 * Machine Problem: Text Editor
 * CS 241 - Fall 2016
 */

#include "document.h"
#include "editor.h"
#include "format.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *get_filename(int argc, char *argv[]) {
    return argc == 2 ? argv[1] : NULL;
}

void handle_display_command(Document *document, const char *command) {
    size_t size = Document_size(document);

    if (!size) {
        print_document_empty_error();
        return;
    }

    char *number = strchr(command, ' ');

    if (number) {
        number += 1;
        size_t line_num = atoi(number);
        
        if (line_num > size) {
            invalid_line();
            return;
        }

        size_t startline = line_num > 5         ? line_num - 5 : 1;
        size_t endline   = line_num + 5 <= size ? line_num + 5 : size;

        for (size_t iter = startline; iter <= endline; iter++) {
            print_line(document, iter);
        }
    } else {
        for (size_t iter = 1; iter <= size; iter++) {
            print_line(document, iter);
        }
    }
}

void document_write(Document *document, size_t line_num, char *string_start, int set) {
    if (set) {
        Document_set_line(document, line_num, string_start);
    } else {
        Document_insert_line(document, line_num, string_start);
    }
}

void handle_write_command(Document *document, const char *command) {
    char *mutable_command = malloc((strlen(command)+1)*sizeof(char));
    strcpy(mutable_command, command);
    char *num_start = mutable_command + 2;
    
    char *num_end   = strchr(num_start, ' ');
    *num_end        = '\0';

    size_t line_num = atoi(num_start);

    char *iter         = num_end + 1;
    char *string_start = iter;

    int set = 1; //1 -> set, 0 -> insert
    if (line_num > Document_size(document))
        set = 0;

    while (1) {
        if(*iter == '\0') {
            document_write(document, line_num, string_start, set);
            set = 0;
            break;
        } else if (*iter == '$') {
            *iter = '\0';
            document_write(document, line_num, string_start, set);
            set = 0;
            line_num++;
            iter++;
            string_start = iter;
        } else {
            iter++;
        }
    }

    free(mutable_command);
}

void document_append(Document *document, size_t line_num, char *string_start) {
    if (line_num > Document_size(document)) {
        Document_insert_line(document, line_num, string_start);
    } else {
        const char *document_line = Document_get_line(document, line_num);
        
        if (document_line) {
            char *new_document_line = malloc((strlen(document_line) + strlen(string_start) + 1) * sizeof(char));
            strcpy(new_document_line, document_line);
            strcat(new_document_line, string_start);
            Document_set_line(document, line_num, new_document_line);
            free(new_document_line);
        } else {
            Document_insert_line(document, line_num, string_start);
        }
    }
}

void handle_append_command(Document *document, const char *command) {
    char *mutable_command = malloc((strlen(command)+1)*sizeof(char));
    strcpy(mutable_command, command);
    char *num_start = mutable_command + 2;
    
    char *num_end   = strchr(num_start, ' ');
    *num_end        = '\0';

    size_t line_num = atoi(num_start);

    char *iter         = num_end + 1;
    char *string_start = iter;

    while (1) {
        if(*iter == '\0') {
            document_append(document, line_num, string_start);
            break;
        } else if (*iter == '$') {
            *iter = '\0';
            document_append(document, line_num, string_start);
            line_num++;
            iter++;
            string_start = iter;
        } else {
            iter++;
        }
    }

    free(mutable_command);
}

void handle_delete_command(Document *document, const char *command) {
    size_t size = Document_size(document);

    char *number = strchr(command, ' ');

    if (number) {
        number += 1;
        size_t line_num = atoi(number);
        
        if (line_num > size) {
            invalid_line();
            return;
        }

        Document_delete_line(document, line_num);
    } else {
        invalid_line();
        return;
    }
}

void handle_search_command(Document *document, const char *command) {
    size_t size = Document_size(document);

    const char *search_term = command + 1;
    if (*search_term) {
        const char *line;
        char *substring;
        for (size_t iter = 1; iter <= size; iter++) {
            line = Document_get_line(document, iter);
            substring = strstr(line, search_term);
            if (substring)
                print_search_line(iter, line, substring, search_term);
        }
    }
}

void handle_save_command(Document *document, const char *filename) {
    Document_write_to_file(document, filename);
}