/**
 * Ideal Indirection Lab
 * CS 241 - Fall 2016
 */
#ifndef __MMU_H__
#define __MMU_H__
#include "page_table.h"
#include "tlb.h"

// 32768 is the largest process id on most linux systems
#define MAX_PROCESS_ID 32768
// 51bit for a virtual address
#define VIRTUAL_ADDRESS_LENGTH 51
// 12bits for a page_number
#define PAGE_NUMBER_LENGTH 12
// 15bits for an offset
#define OFFSET_LENGTH 15

typedef struct {
  PageTable *base_pts[MAX_PROCESS_ID];
  TLB *tlb;
  size_t num_page_faults;
  size_t num_tlb_misses;
  size_t curr_pid;
} MMU;

/**
  Notes on virtual address:

  For this assignment we are going to have a 51 bit virtual address.
  Note that on modern computers you have 64 bits of address space to play with,
  but we are only going to use the lower 51 bits.

  Here is a diagram of how we are going to use the 64 bits:

  |-------------|------------|------------|------------|---------------|
    UNUSED (13)    VPN1 (12)    VPN2 (12)    VPN3 (12)    Offset (15)

  And for this assignment you will deal with 3 levels of page tables where each
  page table has 2^12 entries, which is why each Virtual Page Number (vpn) is 12
  bits long.
  This means that if PageTable *base_pt is a pointer to the base page table,
  then PageTable_get_entry(base_pt, VPN1) will get you a pointer to the second
  level page table.
  This also means that every entry in the third level page table points to an
  actual frame in memory.
  Each frame is 2^15 bytes large. This is because a frame is the same size as
  the page table struct (refer to kernel.c), which is 4096 void pointers and a
  void pointer is 8 bytes.
  This means we need 15 bits to be able to address every byte in a frame, which
  is why the offset is 15 bits.
  A corollary is that if PageTable *pt3 is a third level page table, then (char
  *) PageTable_get_entry(pt3, VPN3) + offset will get you a physical address.
*/

/**
 * Allocate and return a new MMU structure.
 */
MMU *MMU_create();

/**
  Gets the physical address from a virtual address for a pid if it has already
  been assigned.
  Whenever possible you should see if the physical address is stored in the TLB
  before going through multiple page tables.
  This is because page tables are orders of magnitude slower than the cache of a
  TLB.
  If the address has not already been assigned, then you should ask the kernel
  for a frame in memory and map the virtual address to that frame for that
  process id (see ask_kernel_for_frame()).

  Note: Your TLB needs to be updated whenever this method is called.
  Note: Your page tables should be sparse, which means you should assign entries
  as your user requests them.
  Note: All the first level page tables have already been created.
 */
void *MMU_get_physical_address(MMU *mmu, void *address, size_t pid);

/**
  You need to call this function every time the MMU cache misses with the TLB
  and needs to check the page tables.
*/
void MMU_tlb_miss(MMU *mmu, void *address, size_t pid);

/**
  Raises a page fault whenever the MMU gets an addresses from a page table that
  does not map to physical memory.
  Note: 'virutal_address' is the address that the CPU wanted the MMU to
  translate and not an address of a page table entry.
*/
void MMU_raise_page_fault(MMU *mmu, void *virtual_address, size_t pid);

/**
  Adds a process with 'pid' to the MMU by creating it's first level pagetable.
*/
void MMU_add_process(MMU *mmu, size_t pid);

/**
  Free all the physical memory used by a certain process given by 'pid', so that
  other process can use that space.
*/
void MMU_free_process_tables(MMU *mmu, size_t pid);

/**
  Frees all the memory used by the MMU
*/
void MMU_delete(MMU *mmu);

#endif /* __MMU_H__ */
