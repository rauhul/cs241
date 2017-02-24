/**
 * Chatroom Lab
 * CS 241 - Fall 2016
 */
#include "camelCaser.h"
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <ctype.h>


char **camel_caser(const char *input_s) { 
    if (!input_s)
        return NULL;

    //
    char c;
    int input_iter      = 0;
    int count_sentences = 0;

    while ((c = input_s[input_iter++])) {
        if (ispunct(c)) {
            count_sentences++;
        }    
    }

    char **output_s = malloc((count_sentences + 1) * sizeof(char*));
    output_s[count_sentences] = NULL;

    //
    int sentence_iter    = 0;
    int count_characters = 0;
    input_iter           = 0;
    
    while ((c = input_s[input_iter++])) {
        if (ispunct(c)) {
            output_s[sentence_iter] = malloc((count_characters + 1) * sizeof(char));
            output_s[sentence_iter][count_characters] = '\0';

            sentence_iter++;
            count_characters = 0;
        } else if (isspace(c)) {

        } else {
            count_characters++;
        }
    }

    //
    input_iter       = 0;

    char input_char;
    bool capitalize  = false;
    bool first_char  = true;
    sentence_iter    = 0;
    count_characters = 0;

    while ((c = input_s[input_iter++])) {
        if (!output_s[sentence_iter])
            break;
        if (ispunct(c)) {
            capitalize = false;
            first_char = true;
            sentence_iter++;
            count_characters = 0;
        } else if (isspace(c)) {
            capitalize = true;
        } else {
            if (isalpha(c)) {
                input_char = capitalize && !first_char ? toupper(c) : tolower(c);
            } else {
                input_char = c;
            }
            capitalize = false;
            first_char = false;
            output_s[sentence_iter][count_characters] = input_char;
            count_characters++;
        }
    }

    return output_s;
}
