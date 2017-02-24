/**
 * Lab: Utilities Unleashed
 * CS 241 - Fall 2016
 */

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <string.h>
#include "format.h"

extern char **environ;

#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#define _LOG 0
#define LOG(format, ...) if (_LOG) { \
    fprintf(stderr, ANSI_COLOR_BLUE format ANSI_COLOR_MAGENTA "\t(%s, %s::%d)\n" ANSI_COLOR_RESET, \
    ## __VA_ARGS__, __FILE__, __func__, __LINE__); }

int flag = 1;

int vaild_env_arg_char(char c) {
	return isalnum(c) || c == '_';
}

void parse_single_env_update(char *_env_arg) {
	LOG("%s", _env_arg)

	char *env_arg = strdup(_env_arg);
	char *iter = env_arg;

	char* arg_start = NULL;
	int arg_len = 1;

	while(*iter) {
		if (*iter == '%') {
			if (arg_start != NULL) {
				break;
			}
			arg_start = iter;
		} else if (arg_start != NULL) {
			if (vaild_env_arg_char(*iter)) {
				arg_len += 1;
			} else {
				break;
			}
		}
		iter += 1;
	}

	if (arg_start != NULL) {

		LOG("%d, %s", arg_len, arg_start)
		char *get_env_tmp = strndup(arg_start + 1, arg_len - 1);
		char *get_env_value = getenv(get_env_tmp);
		free(get_env_tmp);

		if (get_env_value == NULL) {
			get_env_value = "";
		}

		int original_length = strlen(env_arg);
		int additional_length = strlen(get_env_value);

		LOG("%d, %d, %d", original_length, additional_length, arg_len)
		char *new_env_arg = calloc(1, sizeof(char) * (original_length + additional_length - arg_len + 5 ));

		char *iter = env_arg;
		char *iter2 = new_env_arg;
		while (1) {
			if (iter == arg_start) {
				*iter2 = '\0';
				break;
			} else {
				*iter2 = *iter;
				iter += 1;
				iter2 += 1;
			}
		}

		strcat(new_env_arg, get_env_value);
		strcat(new_env_arg, arg_start + arg_len);

		LOG("%s", new_env_arg)
		parse_single_env_update(new_env_arg);

		free(new_env_arg);

	} else {
		LOG("-> update env with %s", env_arg)
		char *equals = strchr(env_arg, '=');
		*equals = '\0';
		if (setenv(env_arg, equals + 1, 1) != 0) {
			print_environment_change_failed();
			flag = 0;
		}
	}


	free(env_arg);
}


void update_env_variables(char *env_list) {
	LOG("%s", env_list)

	char *iter = env_list;
	while (flag) {
		char *comma = strchr(iter, ',');
		if (comma == NULL) {
			parse_single_env_update(iter);
			break;
		}
		*comma = '\0';
		parse_single_env_update(iter);
		iter = comma + 1;
	}
}


int main(int argc, char *argv[]) {

	if (argc == 1) {
		char *s;
		int iter = 0;
		while ((s = environ[iter++]))
			puts(s);
		return 0;
	}

	if (argc == 2) {
		print_env_usage();
		return 0;
	}

	pid_t child = fork();
	if (child == -1) {
		print_fork_failed();
		return 0;
	}

	if (child == 0) {
		update_env_variables(argv[1]);
		if (!flag) {
			exit(1000);
		}

        execvp(argv[2], &argv[2]);
        print_exec_failed();
        exit(1111);
    }


    int status;
    if (waitpid(child, &status, 0) == -1) {
        LOG("Waitpid failed")
    }
    if (WIFEXITED(status)) {
        int exit_code = WEXITSTATUS(status);
        if (exit_code != 0) {
            LOG("exited with status %d\n", exit_code);
        }
    }


	return 0;
}
