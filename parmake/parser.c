/**
 * Parallel Make
 * CS 241 - Fall 2016
 */

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common_vector.h"
#include "parser.h"
#include "rule.h"
#include "vector.h"

/** Boolean datatype*/
typedef enum { False = 0, True = 1 } boolean;

/**
 * Creates dynamically sized array of tokens from string.
 * Token is a space ;)
 */
static char **parse_line(char *line) {
  assert(line != NULL);
  char **tokens = malloc((strlen(line) + 1) * sizeof(char *));
  int tokIdx = 0;
  tokens[tokIdx] = strtok(line, " ");
  while (tokens[tokIdx] != NULL) {
    tokIdx++;
    tokens[tokIdx] = strtok(NULL, " ");
  }
  return tokens;
}

/*
 * Finds the index of a string in a vector of strings by a target name.
 * - Sets up foundIdx with the found index if true is returned.
 *   Returns false if it couldn't find it.
 */
static boolean findTargetsIndexByTargetName(Vector *rules, const char *target,
                                            size_t *foundIdx) {
  assert(rules != NULL);
  assert(target != NULL);
  assert(foundIdx != NULL);
  size_t valIdx;
  for (valIdx = 0; valIdx < Vector_size(rules); valIdx++) {
    char *curValRule = Vector_get(rules, valIdx);
    if (!strcmp(curValRule, target)) {
      (*foundIdx) = valIdx;
      return True;
    }
  }
  return False;
}

/*
 * Find the index of a rule in a vector of rules by comparing its target name
 * - Sets up foundIdx with the found index if true is returned.
 *   Returns false if it couldn't find it.
 */
static boolean findRulesIndexByTargetName(Vector *rules, const char *target,
                                          size_t *foundIdx) {
  assert(rules != NULL);
  assert(target != NULL);
  assert(foundIdx != NULL);
  size_t valIdx;
  for (valIdx = 0; valIdx < Vector_size(rules); valIdx++) {
    rule_t *curValRule = Vector_get(rules, valIdx);
    if (!strcmp(curValRule->target, target)) {
      (*foundIdx) = valIdx;
      return True;
    }
  }
  return False;
}

/*
 * Recursively find all the targets needed given a start target
 * - List is treated as a set
 * - Adds its self to the list
 * - Recurses on all dependencies
 */
static void recursivelyAddDependenciesToVector(rule_t *target,
                                               Vector *dependencies) {
  // If the target is not in the list, add it
  size_t foundIdx;
  if (!findTargetsIndexByTargetName(dependencies, target->target, &foundIdx))
    Vector_insert(dependencies, 0, target->target);

  // Recurse on dependencies if not apart of list already
  size_t depIdx;
  for (depIdx = 0; depIdx < Vector_size(target->dependencies); depIdx++) {
    rule_t *depRule = Vector_get(target->dependencies, depIdx);
    if (!findTargetsIndexByTargetName(dependencies, depRule->target, &foundIdx))
      recursivelyAddDependenciesToVector(depRule, dependencies);
  }
}

/**
 * Removes all rules which are not descendants of simulated DAG rooted at
 * targets. Sets up rules with new list, frees data not used.
 */
static void filterOnTargets(Vector **rules, char **targets) {
  Vector *neededTargets = Vector_create(copy_string, destroy_string);
  Vector *validRules = Vector_create(copy_pointer, destroy_pointer);

  size_t tarIdx;
  for (tarIdx = 0; targets[tarIdx] != NULL; tarIdx++) {
    Vector_insert(neededTargets, 0, targets[tarIdx]);
  }

  // Recursively find all dependencies needed for given target
  size_t idx;
  for (idx = 0; idx < Vector_size(*rules); idx++) {
    rule_t *curRule = Vector_get(*rules, idx);
    size_t foundIdx;
    if (findTargetsIndexByTargetName(neededTargets, curRule->target, &foundIdx))
      recursivelyAddDependenciesToVector(curRule, neededTargets);
  }

  // initialize validRules with targets
  for (idx = 0; idx < Vector_size(*rules); idx++) {
    rule_t *curRule = Vector_get(*rules, idx);

    // Check if target is in the target list
    for (tarIdx = 0; tarIdx < Vector_size(neededTargets); tarIdx++) {
      if (strcmp(curRule->target, Vector_get(neededTargets, tarIdx)) == 0) {
        // If its not in the list already add it, should never be tho?
        size_t foundIdx;
        if (!findRulesIndexByTargetName(validRules, curRule->target,
                                        &foundIdx)) {
          Vector_insert(validRules, Vector_size(validRules), curRule);
          Vector_delete((*rules), idx);
          idx--;
        }
      }
    }
  }

  // Free non-used targets
  for (idx = 0; idx < Vector_size(*rules); idx++) {
    rule_t *curRule = Vector_get(*rules, idx);
    rule_destroy(curRule);
    free(curRule);
  }

  // cleanup old rules
  Vector_destroy(*rules);
  Vector_destroy(neededTargets);

  // assign new queue
  *rules = validRules;
}

/**
 * Calls appropriate callback functions for a given rule.
 */
static void notifyRules(Vector *rules, void (*notify_target)(rule_t *)) {
  size_t i;
  for (i = 0; i < Vector_size(rules); i++) {
    rule_t *rule = Vector_get(rules, i);
    notify_target(rule);
  }
}

/**
 * Parsers a makefile for certain run_targets, then calls a callback function
 * per target found.
 */
void parser_parse_makefile(const char *makeFileName, char **run_targets,
                           void (*parsed_new_target)(rule_t *target)) {
  boolean run_targetsMalloced = False;

  // Open the file
  FILE *f = fopen(makeFileName, "r");
  assert(f != NULL);

  // This will contain all the parsed rules
  Vector *rules = Vector_create(copy_pointer, destroy_pointer);
  // This will contain the current rules we are parsing commands and
  // dependencies for
  Vector *curRules = Vector_create(copy_pointer, destroy_pointer);

  char *lineBuf = NULL;
  size_t bytes;
  ssize_t len;
  // Read lines until end of file
  while ((len = getline(&lineBuf, &bytes, f)) != -1) {

    // Found non-zero new line
    // Lets set up the null bit
    if (len && lineBuf[len - 1] == '\n') {
      lineBuf[--len] = '\0';
      // Found windows line ending
      if (len && lineBuf[len - 1] == '\r')
        lineBuf[--len] = '\0';
    }

    // Found start of rule line?
    if (isalnum(lineBuf[0])) {
      // Find first colon to split on
      char *depLine = strstr(lineBuf, ":");

      // Did we actually find it?
      assert(depLine != NULL);

      // Remove the colon, and setup target name and dependency string
      depLine[0] = '\0';
      depLine = depLine + 1;

      // We found a new rule
      // So lets complete the previous rules
      // As in push to rule vector
      size_t ruleIdx;
      for (ruleIdx = 0; ruleIdx < Vector_size(curRules); ruleIdx++) {
        Vector_insert(rules, Vector_size(rules), Vector_get(curRules, ruleIdx));
      }
      // Lets clean up
      // However we aren't really cleanup (cause data is now in rule vector)
      // Lets just set the size and make it think it cleaned up
      Vector_destroy(curRules);

      // Recreate vector
      // BECAUSE Our implementation of vector frees itself :/
      curRules = Vector_create(copy_pointer, destroy_pointer);

      // Find all the targets before the colon by space token
      char **targets = parse_line(lineBuf);
      size_t tarIdx;
      for (tarIdx = 0; targets[tarIdx] != NULL; tarIdx++) {
        // Create new rule
        rule_t *newRule = malloc(sizeof(rule_t));
        rule_init(newRule);
        char *target = strdup(targets[tarIdx]);
        newRule->target = target;

        size_t foundIdx = 0;
        // Fudge we found a duplicate target :/
        if (findRulesIndexByTargetName(rules, targets[tarIdx], &foundIdx)) {
          // There is cases where we find dependency rules that were established
          // as rules before hand were created
          // So this would just override the date, which is fine, just seems
          // inefficient

          // Delete that oldy and replace it
          rule_t *dupRule = Vector_get(rules, foundIdx);
          Vector_delete(rules, foundIdx);
          // Gotta free old attributes
          rule_destroy(dupRule);
          // Soft copy, Just copies vector pointers
          rule_soft_copy(dupRule, newRule);
          // We copied over all the info so lets free dis
          free(newRule);
          // Set up so we can add to current rules
          newRule = dupRule;
        }

        // Guaranteed to have a rule
        Vector_insert(curRules, Vector_size(curRules), newRule);
      }

      // If no run targets were specified, malloc dis to be the first run target
      if (run_targets == NULL || run_targets[0] == NULL) {
        run_targetsMalloced = True; // Mark so we can free it later
        run_targets = calloc(sizeof(char *), tarIdx + 1);
        size_t rtIdx;
        for (rtIdx = 0; rtIdx < tarIdx; rtIdx++)
          run_targets[rtIdx] = strdup(targets[rtIdx]);
      }
      free(targets);

      // Now same thing for dependencies
      char **dependencies = parse_line(depLine);
      int depIdx;
      for (depIdx = 0; dependencies[depIdx] != NULL; depIdx++) {
        rule_t *foundRule;
        size_t foundIdx = 0;
        // Does this dependency exist as a rule already?
        if (findRulesIndexByTargetName(rules, dependencies[depIdx],
                                       &foundIdx)) {
          foundRule = Vector_get(rules, foundIdx);
        } else if (findRulesIndexByTargetName(curRules, dependencies[depIdx],
                                              &foundIdx)) {
          foundRule = Vector_get(curRules, foundIdx);
        } else {
          // Create new rule
          foundRule = malloc(sizeof(rule_t));
          rule_init(foundRule);
          char *target = strdup(dependencies[depIdx]);
          foundRule->target = target;
          // Insert into the original rules list as well, now an empty rule
          Vector_insert(rules, Vector_size(rules), foundRule);
        }
        size_t ruleIdx;
        for (ruleIdx = 0; ruleIdx < Vector_size(curRules); ruleIdx++) {
          rule_t *curRule = Vector_get(curRules, ruleIdx);
          // Insert the pointer!!!! Adjacency list
          Vector_insert(curRule->dependencies,
                        Vector_size(curRule->dependencies), foundRule);
        }
      }
      free(dependencies);
    }
    // Found start of command line
    else if (lineBuf[0] == '\t') {
      // Do we have a current rule?
      assert(Vector_size(curRules) != 0);

      // Push Command into every rule that we are parsing
      size_t idx;
      for (idx = 0; idx < Vector_size(curRules); idx++) {
        rule_t *curRule = Vector_get(curRules, idx);
        Vector_append(curRule->commands, lineBuf + 1);
      }

    }
    // Invalid line, just ignore
    else {
      continue;
    }
  }

  // Add the rest of the curRules to the rules list
  size_t ruleIdx;
  for (ruleIdx = 0; ruleIdx < Vector_size(curRules); ruleIdx++) {
    Vector_append(rules, Vector_get(curRules, ruleIdx));
  }
  Vector_destroy(curRules);

  // Filter out on our run_targets
  filterOnTargets(&rules, run_targets);

  // send callbacks
  notifyRules(rules, (void *)parsed_new_target);

  if (run_targetsMalloced) {
    int tarIdx;
    for (tarIdx = 0; run_targets[tarIdx] != NULL; tarIdx++)
      free(run_targets[tarIdx]);
    free(run_targets);
  }

  free(lineBuf);
  Vector_destroy(
      rules); // Just tell the vector its empty, all data was passed to callback
  fclose(f);
}
