/**
 * Machine Problem: Password Cracker
 * CS 241 - Fall 2016
 */

#include <assert.h>
#include <crypt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define USERNAME_LEN 8
#define PASSWORD_LEN 8

void printHelp() {
  printf("\n"
         "  create_examples [-soln] <count> <min-iter> <max-iter>\n"
         "\n"
         "    Print <count> sample inputs for cracker1 or cracker2.\n"
         "    Each one will take between <min-iter> and <max_iter> iterations\n"
         "    to solve (chosen randomly between those two limits.\n"
         "    <count>, <min-iter>, and <max-iter> must be positive, and\n"
         "    <max-iter> must be >= <min-iter>.\n"
         "\n"
         "    -soln: include the solution and iteration count in the output\n"
         "\n");
  exit(1);
}

void randomizeString(char *buf, int len) {
  int i;
  for (i = 0; i < len; i++)
    buf[i] = 'a' + (rand() % 26);
  buf[len] = 0;
}

// create a password that will take iter_count iterations to crack
void createPassword(char *password, char *pw_prefix, int iter_count) {
  assert(iter_count > 0);

  // first randomize the whole thing
  randomizeString(password, PASSWORD_LEN);
  strcpy(pw_prefix, password);

  // now convert iter_count into a base 26 number

  int digit_pos = PASSWORD_LEN - 1;
  iter_count--; // subtract 1 since iter_count is being used as an offset
  while (iter_count > 0) {
    int digit = iter_count % 26;
    password[digit_pos] = 'a' + digit;
    pw_prefix[digit_pos] = '.';
    iter_count /= 26;
    digit_pos--;
  }

  // *prefix_len = PASSWORD_LEN - digit_pos;
}

int main(int argc, char **argv) {
  int count, min_iter, max_iter, include_solution = 0, argno = 1;

  if (argc < 2)
    printHelp();

  if (!strcmp(argv[argno], "-soln")) {
    include_solution = 1;
    argno++;
  }

  if (argc - argno != 3)
    printHelp();

  if (!sscanf(argv[argno++], "%d", &count) || count <= 0)
    printHelp();
  if (!sscanf(argv[argno++], "%d", &min_iter) || min_iter <= 0)
    printHelp();
  if (!sscanf(argv[argno++], "%d", &max_iter) || max_iter <= 0)
    printHelp();
  if (max_iter < min_iter)
    printHelp();

  // # of examples it limited because the auto-generated username
  // only has room for 7 digits.
  if (count > 10000000) {
    printf("This only supports up to 10000000 examples.\n");
    return 1;
  }

  // randomize the random seed so you don't get the same output for
  // the same inputs
  srand(time(NULL));

  char username[USERNAME_LEN + 1], password[PASSWORD_LEN + 1],
      password_prefix[PASSWORD_LEN + 1];

  int i;
  for (i = 0; i < count; i++) {
    // randomizeString(username, USERNAME_LEN);
    sprintf(username, "u%07d", i);

    // FYI, 26^8 = 208,827,064,576
    // so 8 letters are more than enough to encode the largest possible
    // value for a signed int (2,147,483,647)

    int iter_count = min_iter + rand() % (max_iter - min_iter + 1);
    createPassword(password, password_prefix, iter_count);

    char *hash = crypt(password, "xx");

    if (include_solution) {
      printf("%s %s %s %s %d\n", username, hash, password_prefix, password,
             iter_count);
    } else {
      printf("%s %s %s\n", username, hash, password_prefix);
    }
  }

  return 0;
}
