/**
 * Parallel Make
 * CS 241 - Fall 2016
 */

#include <assert.h>

#include "common_vector.h"
#include "rule.h"

void rule_init(rule_t *rule) {
  assert(rule != NULL);
  rule->target = NULL;
  rule->dependencies = Vector_create(copy_pointer, destroy_pointer);
  rule->commands = Vector_create(copy_string, destroy_string);
  rule->state = 0;
}

void rule_destroy(rule_t *rule) {
  assert(rule != NULL);
  free(rule->target); // Can free null :)
  Vector_destroy(rule->dependencies);
  Vector_destroy(rule->commands);
}

void rule_soft_copy(rule_t *rule, rule_t *cpy_rule) {
  assert(rule != NULL);
  rule->target = cpy_rule->target;
  rule->dependencies = cpy_rule->dependencies;
  rule->commands = cpy_rule->commands;
  rule->state = cpy_rule->state;
}
