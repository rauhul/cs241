/**
 * Parallel Make
 * CS 241 - Fall 2016
 */

#pragma once

#include "vector.h"

/**
 * Rule Data Structure
 */
typedef struct {
  char *target;         // Name of target
  Vector *dependencies; // Vector of pointers to rules
  Vector *commands;     // Vector of pointers to char arrays
  int state;            // State of the rule. Defaults to 0
} rule_t;

/**
 * Initializes Rule data structure.
 */
void rule_init(rule_t *rule);

/**
 * Destroys Rule data structure. DOES NOT FREE THE ORIGINAL
 */
void rule_destroy(rule_t *rule);

/**
 * Soft copies a rule into another rule. Doesn't deep copy vectors
 */
void rule_soft_copy(rule_t *rule, rule_t *cpy_rule);
