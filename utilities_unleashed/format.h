/**
 * Lab: Utilities Unleashed
 * CS 241 - Fall 2016
 */

/**
 * This function gets called whenever
 * a user incorrectly uses your time program.
*/
void print_time_usage();

/**
 * This function gets called whenever
 * a user incorrectly uses your env program.
*/
void print_env_usage();

/**
 * This function gets called whenever
 * a either env or time fails to fork.
*/
void print_fork_failed();

/**
 * This function gets called whenever
 * a either env or time fails to exec.
*/
void print_exec_failed();

/**
 * This function gets called whenever
 * you env fails to change an environment variable.
 * Note: You are still responsible for
 * freeing all resources BEFORE calling this function.
*/
void print_environment_change_failed();

/**
 * Displays the result for your time program.
 * Feed in argv and how long the program took (in seconds).
 * Note: this should not be called on if anything failed
 * in your child process.
*/
void display_results(char **argv, double duration);
