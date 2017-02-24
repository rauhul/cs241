/**
 * Parallel Make
 * CS 241 - Fall 2016
 */

/* files */
#include "common_vector.h"
#include "parmake.h"
#include "parser.h"

/* libraries */
#include <assert.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

/* macros */

/* constants */

/* rule->state
    -1 failed
     0 default
     1 ready to run
     2 running
     3 success
     4 DAG checkmark
*/

/* globals */
int thread_count    = 0;
int num_targets     = 0;
char **targets      = NULL;
char *makefile_name = NULL;
Vector *rules       = NULL;
Vector *all_rules   = NULL;

pthread_mutex_t m_rules;
pthread_cond_t c_wait;

/* helper functions */
void end(char *message, int error) {

    /* cleanup targets */
    for (int i = 0; i < num_targets; i++)
        free(targets[i]);

    if (targets)
        free(targets);

    /* cleanup makefile_name */
    if (makefile_name)
        free(makefile_name);

    /* print error message */
    if (message)
        puts(message);

    /* exit error number */
    exit(error);
}

void print_rules() {
    printf("RULES:\n");
    for (size_t i = 0; i < Vector_size(rules); i++) {
        rule_t *rule = (rule_t *) Vector_get(rules, i);
        printf("%s: %d\n", rule->target, rule->state);
    }
    printf("\n");
}

/* main */
void validate_rule(rule_t *rule) {
    if (rule->state != 0)
        return;

    int all_dependencies_passed = 1;
    int any_dependency_failed = 0;
    for (size_t i = 0; i < Vector_size(rule->dependencies); i++) {
        rule_t *dependency = (rule_t *) Vector_get(rule->dependencies, i);
        if (dependency->state == -1) {
            any_dependency_failed = 1;
            break;
        } else if (dependency->state != 3) {
            all_dependencies_passed = 0;
            break;
        }
    }

    if (any_dependency_failed) {
        rule->state = -1;
    } else if (all_dependencies_passed) {
        rule->state = 1;
    } else {
        rule->state = 0;
    }
}

void mark_unsatisfiable_rules(rule_t *rule) {
    rule->state = 4;

    for (size_t i = 0; i < Vector_size(rule->dependencies); i++) {
        rule_t *dependency = (rule_t *) Vector_get(rule->dependencies, i);
        if (dependency->state == 4) {
            rule->state = -1;
            return;
        }
        mark_unsatisfiable_rules(dependency);
        if (dependency->state == -1) {
            rule->state = -1;
            return;
        }
    }

    rule->state = 0;
}

int run_rule_commands(rule_t *rule) {
    for (size_t i = 0; i < Vector_size(rule->commands); i++) {
        int retval = system((char *) Vector_get(rule->commands, i));
        if (retval != 0) {
            rule->state = -1;
            return -1;
        }
    }
    return 0;
}

void execute_rule(rule_t *rule) {
    if (rule->state != 1)
        assert(0);

    rule->state = 2;

    if (access(rule->target, F_OK) == -1) {
        for (size_t i = 0; i < Vector_size(rule->commands); i++) {
            int retval = system((char *) Vector_get(rule->commands, i));
            if (retval != 0) {
                rule->state = -1;
                return;
            }
        }
    } else {
        struct stat attr;
        stat(rule->target, &attr);
        time_t last_modifed = attr.st_mtime;

        int run_commands = 0;
        for (size_t i = 0; i < Vector_size(rule->dependencies); i++) {
            char *dependency = ((rule_t *) Vector_get(rule->dependencies, i))->target;
            if (access(dependency, F_OK) == -1) {
                run_commands = 1;
                break;
            } else {
                stat(dependency, &attr);
                if (attr.st_mtime > last_modifed) {
                    run_commands = 1;
                    break;
                }
            }
        }

        if (run_commands) {
            for (size_t i = 0; i < Vector_size(rule->commands); i++) {
                int retval = system((char *) Vector_get(rule->commands, i));
                if (retval != 0) {
                    rule->state = -1;
                    return;
                }
            }
        }
    }

    rule->state = 3;
}

void parser_callback(rule_t *rule) {
    mark_unsatisfiable_rules(rule);
    Vector_append(all_rules, rule);
    if (rule->state != -1) {
        Vector_append(rules, rule);
    }
}

void pthread_exit_if_needed() {
    size_t rules_remaining = Vector_size(rules);
    if (rules_remaining == 0) {
        pthread_cond_broadcast(&c_wait);
        pthread_mutex_unlock(&m_rules);
        pthread_exit(0);
    }
}

void thread() {
    while (1) {
        pthread_mutex_lock(&m_rules);

        /* find a rule to execute*/
        rule_t *run_rule = NULL;
        while(1) {
            pthread_exit_if_needed();

            size_t index = 0;
            for (size_t i = 0; i < Vector_size(rules); i++) {
                rule_t *rule = Vector_get(rules, index);
                validate_rule(rule);

                int state = rule->state;
                if (state == -1) {
                    Vector_delete(rules, index);
                } else if (state == 0) {
                    index++;
                } else if (state == 1) {
                    run_rule = rule;
                    Vector_delete(rules, index);
                    break;
                } else if (state == 2) {
                    Vector_delete(rules, index);
                } else if (state == 3) {
                    Vector_delete(rules, index);
                } else {
                    assert(0);
                }
            }

            if (run_rule == NULL) {
                pthread_exit_if_needed();
                pthread_cond_wait(&c_wait, &m_rules);
            } else {
                break;
            }
        }

        if (run_rule == NULL) assert(0);

        pthread_mutex_unlock(&m_rules);
        execute_rule(run_rule);
        pthread_cond_broadcast(&c_wait);
    }
}

int parmake(int argc, char **argv) {

    /* parse flags */
    int c;
    while ((c = getopt(argc, argv, "f:j:")) != -1) {
        switch (c) {
        case 'f':
            if (!makefile_name && optarg) {
                makefile_name = strdup(optarg);
            }
            break;
        case 'j':
            thread_count = (int) atoi(optarg);
            break;
        default:
            break;
        }
    }

    /* parse targets */
    num_targets = argc - optind;
    if (num_targets > 0) {
        targets = malloc(sizeof(char *) * num_targets);
    }

    for (int i = 0; i < num_targets; i++) {
        targets[i] = strdup(argv[i + optind]);
    }

    /* validate makefile_name */
    FILE *makefile;
    if (makefile_name) {
        makefile = fopen(makefile_name, "r");
        if (makefile == NULL) {
            end("Fatal Error - 1\n > Makefile specified by -f could not be opened", 1);
        }
        fclose(makefile);
    } else {
        makefile_name = strdup("./makefile");
        makefile = fopen(makefile_name, "r");
        if (makefile == NULL) {
            free(makefile_name);
            makefile_name = strdup("./Makefile");
            makefile = fopen(makefile_name, "r");
            if (makefile == NULL) {
                end("Fatal Error - 2\n > No Makefile could be opened", 2);
            }
        }
        fclose(makefile);
    }

    /* validate thread_count */
    if (thread_count < 1) {
        thread_count = 1;
    }

    // makefile_name is valid, thread_count is >= 1, targets are enumerated
    rules = Vector_create(copy_pointer, destroy_pointer);
    all_rules = Vector_create(copy_pointer, destroy_pointer);

    // /* */
    parser_parse_makefile(makefile_name, targets, parser_callback);

    // /* */
    pthread_mutex_init(&m_rules, NULL);
    pthread_cond_init(&c_wait, NULL);
    pthread_t *thread_pool = malloc(sizeof(pthread_t) * thread_count);


    /* */
    for (int i = 0; i < thread_count; i++) {
        pthread_create(thread_pool + i, NULL, (void *) &thread, NULL);
    }

    /* */
    for (int i = 0; i < thread_count; i++) {
        pthread_join(thread_pool[i], NULL);
    }

    // /* clean up */
    free(thread_pool);
    pthread_cond_destroy(&c_wait);
    pthread_mutex_destroy(&m_rules);
    for (size_t i = 0; i < Vector_size(all_rules); i++) {
        rule_t *rule = (rule_t *) Vector_get(all_rules, i);
        rule_destroy(rule);
        free(rule);
    }
    Vector_destroy(all_rules);
    Vector_destroy(rules);
    end(NULL, 0);
    return 0;
}