/**
 * Machine Problem: Text Editor
 * CS 241 - Fall 2016
 */

#include "document.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_usage_error() { printf("\n  editor <filename>\n\n"); }

void print_line(Document *document, size_t index) {
  char *line = (char *)Document_get_line(document, index);
  if (line) {
    printf("%zu\t%s\n", index, line);
  } else {
    printf("%zu\n", index);
  }
}

void print_document_empty_error() {
  fprintf(stderr, "This file has no lines to display!\n");
}

void invalid_line() { fprintf(stderr, "Invalid Line Number!\n"); }

void invalid_command(const char *command) {
  fprintf(stderr, "Invalid command: \"%s\"\n", command);
}

void print_search_line(int line_number, const char *line, const char *start,
                       const char *search_term) {
  int search_term_len = strlen(search_term);
  printf("%d\t%.*s[%s]%s\n", line_number, (int)(start - line), line,
         search_term, start + search_term_len);
}

char **strsplit(const char *str, const char *delim, size_t *numtokens) {
  char *s = strdup(str);
  size_t tokens_alloc = 1;
  size_t tokens_used = 0;
  char **tokens = calloc(tokens_alloc, sizeof(char *));
  char *token, *rest = s;
  while ((token = strsep(&rest, delim)) != NULL) {
    if (tokens_used == tokens_alloc) {
      tokens_alloc *= 2;
      tokens = realloc(tokens, tokens_alloc * sizeof(char *));
    }
    tokens[tokens_used++] = strdup(token);
  }
  if (tokens_used == 0) {
    free(tokens);
    tokens = NULL;
  } else {
    tokens = realloc(tokens, tokens_used * sizeof(char *));
  }
  *numtokens = tokens_used;
  free(s);
  return tokens;
}
