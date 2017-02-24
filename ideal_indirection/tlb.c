/**
 * Ideal Indirection Lab
 * CS 241 - Fall 2016
 */
#include "tlb.h"
TLB *TLB_create() { return calloc(1, sizeof(TLB)); }

void *TLB_get_physical_address(TLB **head, void *key) {
  // If the first node has the value we want, then we can just return it.
  if ((*head)->key == key) {
    return (*head)->value;
  }
  // Using the 'slow pointer and fast pointer' trick for singly linked lists:

  // There are a lot of interesting interview questions that arise from this
  // trick:
  //  * Given a singly linked list and head pointer determine if the list has a
  //  cycle
  //  * Given a singly linked list and a head pointer return to me the 'kth'
  //  element from the end.
  //  * Given a singly linked list and a head pointer return to me the middle
  //  element.

  TLB *slow = *head;
  TLB *fast = (*head)->next;
  // Finds the correct node and promote it to the head.
  while (fast) {
    if (fast->key == key) {
      // We found the correct node!
      // Now we need to move it to remove it from the list and promote as the
      // new head of the linked list.
      slow->next = fast->next;
      fast->next = *head;
      *head = fast;
      return fast->value;
    }
    fast = fast->next;
    slow = slow->next;
  }
  return NULL;
}

void TLB_add_physical_address(TLB **head, void *key, void *value) {
  size_t num_nodes = 2;
  TLB *slow = *head;
  TLB *fast = (*head)->next;
  // Figure out how many nodes are in the linked list.
  while (fast && fast->next) {
    num_nodes++;
    fast = fast->next;
    slow = slow->next;
  }
  if (num_nodes >= MAX_NODES) {
    // We need to evict the least recently used item.
    // At this point fast is pointing to the tail.
    free(fast);
    slow->next = NULL;
  }
  // Appends new node to the head of the linked list.
  TLB *new_head = TLB_create();
  new_head->key = key;
  new_head->value = value;
  new_head->next = *head;
  *head = new_head;
}

void TLB_flush(TLB **tlb) {
  TLB_delete(*tlb);
  (*tlb) = TLB_create();
}

void TLB_delete(TLB *tlb) {
  if (tlb) {
    TLB_delete(tlb->next);
    free(tlb);
  }
}
