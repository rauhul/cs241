/**
 * Ideal Indirection Lab
 * CS 241 - Fall 2016
 */
#include "page_table.h"
#include <unistd.h>
PageTable *PageTable_create() { return calloc(1, sizeof(PageTable)); }

void *PageTable_get_entry(PageTable *pt, size_t index) {
  usleep(250000);
  return pt->entries[index];
}

void PageTable_set_entry(PageTable *pt, size_t index, void *entry) {
  usleep(250000);
  pt->entries[index] = entry;
}

void Pagetable_delete_tree(PageTable *base) {
  if (base) {
    for (size_t i = 0; i < PAGETABLE_SIZE; i++) {
      PageTable *pt1 = base->entries[i];
      if (pt1) {
        for (size_t j = 0; j < PAGETABLE_SIZE; j++) {
          PageTable *pt2 = pt1->entries[j];
          if (pt2) {
            for (size_t k = 0; k < PAGETABLE_SIZE; k++) {
              PageTable *pt3 = pt2->entries[k];
              if (pt3) {
                free(pt3);
              }
            }
            free(pt2);
          }
        }
        free(pt1);
      }
    }
    free(base);
  }
}
