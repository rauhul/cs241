/**
 * MapReduce
 * CS 241 - Fall 2016
 */

#include <search.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libds.h"

typedef struct _datastore_entry_t {
  const char *key, *value;
} datastore_entry_t;

// global function pointer, to the action we are currently executing
// see datastore_iterate for an explanation
static datastore_iterfun curr_action;

// global arg pointer, to the arg we will pass to datastore_iterfun
// see datastore_iterate and my_action for an explanation
static void *curr_iter_arg;

/** Private. */
static int compare(const void *a, const void *b) {
  return strcmp(((const datastore_entry_t *)a)->key,
                ((const datastore_entry_t *)b)->key);
}

/** Private. */
static datastore_entry_t *malloc_entry_t(const char *key, const char *value) {
  datastore_entry_t *entry =
      (datastore_entry_t *)malloc(sizeof(datastore_entry_t));
  entry->key = key;
  entry->value = value;

  return entry;
}

/** Private. */
static datastore_entry_t *dictionary_tfind(datastore_t *ds, const char *key) {
  datastore_entry_t tentry = {key, NULL};
  void *tresult = tfind((void *)&tentry, &ds->root, compare);

  if (tresult == NULL)
    return NULL;
  else
    return *((datastore_entry_t **)tresult);
}

/** Private. */
static void dictionary_tdelete(datastore_t *ds, const char *key) {
  datastore_entry_t tentry = {key, NULL};
  tdelete((void *)&tentry, &ds->root, compare);
}

/** Private. */
static void destroy_with_element_free(void *ptr) {
  datastore_entry_t *entry = (datastore_entry_t *)ptr;

  free((void *)entry->key);
  free((void *)entry->value);
  free(entry);
}

/** private */
static void my_action(const void *node, VISIT v,
                      int depth __attribute__((unused))) {
  datastore_entry_t **node_prt = (datastore_entry_t **)node;
  datastore_entry_t *n = *node_prt;

  switch (v) {
  case preorder:
    break;
  case postorder:
    curr_action(n->key, n->value, curr_iter_arg);
    break;
  case endorder:
    break;
  case leaf:
    curr_action(n->key, n->value, curr_iter_arg);
    break;
  }
}

void datastore_init(datastore_t *ds) { ds->root = NULL; }

int datastore_put(datastore_t *ds, const char *key, const char *value) {
  if (dictionary_tfind(ds, key) == NULL) {
    datastore_entry_t *entry = malloc_entry_t(strdup(key), strdup(value));
    tsearch((void *)entry, &ds->root, compare);

    return 1;
  }
  return 0;
}

const char *datastore_get(datastore_t *ds, const char *key) {
  datastore_entry_t *entry = dictionary_tfind(ds, key);

  if (entry == NULL) {
    return NULL;
  } else {
    const char *value = strdup(entry->value);
    return value;
  }
}

int datastore_update(datastore_t *ds, const char *key, const char *value) {
  datastore_entry_t *entry = dictionary_tfind(ds, key);

  if (entry == NULL) {
    // key does not exist
    return 0;
  } else {
    free((void *)entry->value);
    entry->value = strdup(value);
    return 1;
  }
}

int datastore_delete(datastore_t *ds, const char *key) {
  datastore_entry_t *entry = dictionary_tfind(ds, key);

  if (entry == NULL) {
    return 0;
  } else {
    dictionary_tdelete(ds, key);

    free((void *)entry->key);
    free((void *)entry->value);
    free(entry);

    return 1;
  }
}

void datastore_iterate(datastore_t *ds, datastore_iterfun f, void *arg) {
  // because twalk doesn't give us any way to pass a user defined arg into the
  // action function, we set a global "curr_action" and call that from our
  // action function
  curr_action = f;
  curr_iter_arg = arg;
  twalk(ds->root, my_action);
  curr_action = NULL;
  curr_iter_arg = NULL;
}

void datastore_destroy(datastore_t *ds) {
  tdestroy(ds->root, destroy_with_element_free);
  ds->root = NULL;
}
