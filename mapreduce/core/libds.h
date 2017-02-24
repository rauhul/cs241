/**
 * MapReduce
 * CS 241 - Fall 2016
 */

#pragma once

typedef struct _datastore_t { void *root; } datastore_t;

/**
 * Initializes the data store.
 *
 * @param ds
 *    An uninitialized data store.
 */
void datastore_init(datastore_t *ds);

/**
 * Adds the key-value pair (key, value) to the data store, if and only if the
 * key does not already exist in the data store.
 *
 * The data store will make an internal copy key and value, if necessary, so
 * the user of the data store may free these strings if necessary.
 *
 * @param ds
 *   An initialized data store.
 * @param key
 *   The key to be added to the data store.
 * @param value
 *   The value to associated with the new key.
 *
 * @retval 0
 *   The key already exists in the data store.
 * @retval non-zero
 *   Success
 */
int datastore_put(datastore_t *ds, const char *key, const char *value);

/**
 * Retrieves the current value, for a specific key.
 *
 * @param ds
 *   An initialized data store.
 * @param key
 *   The specific key to retrieve the value.
 *
 * @return
 *   If the data store contains the key, a new string containing the value
 *   will be returned.  It is the responsibility of the user of the data
 *   store to free the value returned. If the data store does not contain
 *   the key, NULL will be returned
 */
const char *datastore_get(datastore_t *ds, const char *key);

/**
 * Updates the specific key in the data store if and only if the
 * key exists in the data store.
 *
 * The data store will make an internal copy key and value, if necessary, so
 * the user of the data store may free the strings passed in as arugments.
 *
 * @param ds
 *   An initialized data store.
 * @param key
 *   The specific key to update in the data store.
 * @param value
 *   The updated value to the key in the data store.
 *
 * @retval 0
 *    The key was not found in the data store.
 * @retval non-zero
 *    Success
 */
int datastore_update(datastore_t *ds, const char *key, const char *value);

/**
 * Deletes a specific key from the data store.
 *
 * @param ds
 *   An initialized data store.
 * @param key
 *   The specific key to update in the data store.
 *
 * @retval 0
 *    The key was not found in the data store.
 * @retval non-zero
 *    The key was deleted from the data store.
 */
int datastore_delete(datastore_t *ds, const char *key);

/**
 * Iterates over the datastore, calling the callback function on each key and
 * value in the datastore.
 *
 * @param ds
 *   An initialized datastore
 * @param f
 *   Function to call on each key, value pair in the datastore
 */
typedef void (*datastore_iterfun)(const char *, const char *, void *);
void datastore_iterate(datastore_t *ds, datastore_iterfun f, void *arg);

/**
 * Destroys the data store, freeing any memory that is in use by the
 * data store.
 *
 * @param ds
 *   An initialized data store.
 */
void datastore_destroy(datastore_t *ds);
