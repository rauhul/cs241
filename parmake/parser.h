/**
 * Parallel Make
 * CS 241 - Fall 2016
 */

#pragma once

#include "rule.h"

/**
 * Parses the file with path given by makeFileName using a syntax very similar
 * to the syntax of GNU Make.
 *
 * You are not required to handle any Makefile syntax which this parser does not
 * recognize. This includes Makefile macros.
 *
 * The parser will only call your provided callback function on targets
 * for a subset of the targets in the file it is parsing. This
 * subset is the subset of targets and commands which are needed to correctly
 * compile all targets passed to this function (see the parameter list).
 *
 * For example, if a Makefile contained 3 targets:
 *
 * a: b
 *
 * b:
 *
 * c:
 *
 *
 * Calling parser_parse_makefile with a target list containing only "a" would
 * result in calls to your callback functions for the target "a" and for the
 * target "b" (since a depends on b).
 *
 * Calling parser_parse_makefile with a target list containing only "c" would
 * only result in a call your callback with the target "c" and a calls to the
 * commands callback with the commands for "c".
 *
 * Calling parser_parse_makefile with a target list containing "a", "b", and "c"
 * would result in calls to your callbacks for all the targets and commands in
 * the file.
 *
 * If targets is empty (eg. Targets == NULL), the first target in the file will
 * be used. (This is consistent with GNU make)
 *
 * For you callback, it will return a rule struct for every rule needed, please
 * refer
 * the docs for more detail on how the rule struct works! Small note:
 * dependencies
 * that are not rules, will also be passed back as a rule_t.
 *
 * @param makeFileName Path to valid makefile.
 * @param run_targets Null-terminated list of targets as listed on command-line.
 * @param parsed_new_target Function pointer to callback function for new
 * target.
 *
 * @return Void.
 */
void parser_parse_makefile(const char *makeFileName, char **targets,
                           void (*parsed_new_target)(rule_t *target));
