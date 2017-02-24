/**
 * Ideal Indirection Lab
 * CS 241 - Fall 2016
 */
#ifndef __PageTable_H__
#define __PageTable_H__

#include <stdlib.h>
#define PAGETABLE_SIZE 4096
typedef struct {
  // These entries could point to a frame in physical memory or they could point
  // to another page table.
  // Who knows?
  void *entries[PAGETABLE_SIZE];
  // 4096 = 2^12
} PageTable;

/**
  Allocate and return a new Page Table.
*/
PageTable *PageTable_create();

/**
  Gets the 'index'th entry from 'pt'.

  Note: You must use this method for getting entries of a page table and under
  no circumstance are you to break the page table abstraction.
  A NULL pointer means that the 'index'-th entry in 'pt' has not been assigned
  yet to somewhere in physical memory.
*/
void *PageTable_get_entry(PageTable *pt, size_t index);

/**
  Sets the 'index'th entry from 'pt'.

  Note: You must use this method for setting entries of a page table and under
  no circumstances are you to break the page table abstraction.
  A NULL pointer means that the 'index'-th page in 'pt' has not been assigned
  yet to somewhere in physical memory.
*/
void PageTable_set_entry(PageTable *pt, size_t index, void *entry);

void Pagetable_delete_tree(PageTable *base);

#endif /* __PageTable_H__ */
