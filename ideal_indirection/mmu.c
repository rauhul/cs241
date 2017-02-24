/**
 * Ideal Indirection Lab
 * CS 241 - Fall 2016
 */
#include "kernel.h"
#include "mmu.h"
#include <assert.h>
#include <stdio.h>
#include <stdint.h>


#define SANITIZE_MASK 0x0007ffffffffffff
#define TLB_MASK      0x0007ffffffff8000
#define VPN1_MASK     0x0007ff8000000000
#define VPN2_MASK     0x0000007ff8000000
#define VPN3_MASK     0x0000000007ff8000
#define Offset_MASK   0x0000000000007fff

#define VPN1_SHIFT   39
#define VPN2_SHIFT   27
#define VPN3_SHIFT   15
#define Offset_SHIFT  0

#define SANITIZE(addr) (  (uintptr_t) addr & (uintptr_t) SANITIZE_MASK)
#define TLB(addr)      (  (uintptr_t) addr & (uintptr_t) TLB_MASK)
#define VPN1(addr)     (( (uintptr_t) addr & (uintptr_t) VPN1_MASK)   >> VPN1_SHIFT)
#define VPN2(addr)     (( (uintptr_t) addr & (uintptr_t) VPN2_MASK)   >> VPN2_SHIFT)
#define VPN3(addr)     (( (uintptr_t) addr & (uintptr_t) VPN3_MASK)   >> VPN3_SHIFT)
#define Offset(addr)   (( (uintptr_t) addr & (uintptr_t) Offset_MASK) >> Offset_SHIFT)

MMU *MMU_create() {
	MMU *mmu = calloc(1, sizeof(MMU));
	mmu->tlb = TLB_create();
	mmu->curr_pid = 0;
	return mmu;
}

void *MMU_get_physical_address(MMU *mmu, void *virtual_address, size_t pid) {
	assert(pid < MAX_PROCESS_ID);

    void *tlb_addr  = (void *) TLB(virtual_address);
    virtual_address = (void *) SANITIZE(virtual_address);

    if (mmu->curr_pid == pid) {
        void *frame_address = TLB_get_physical_address(&(mmu->tlb), tlb_addr);
        if (frame_address) {
            size_t offset = Offset(virtual_address);
            return frame_address + offset;
        } else {
            MMU_tlb_miss(mmu, virtual_address, pid);
        }
    } else {
        MMU_tlb_miss(mmu, virtual_address, pid);
        mmu->curr_pid = pid;
        TLB_flush(&(mmu->tlb));
    }

	size_t vpn1   = VPN1(virtual_address);
	size_t vpn2   = VPN2(virtual_address);
	size_t vpn3   = VPN3(virtual_address);
    size_t offset = Offset(virtual_address);

	PageTable *virtualPage1 = mmu->base_pts[pid];
    if (!virtualPage1) {
        MMU_add_process(mmu, pid);
        virtualPage1 = mmu->base_pts[pid];
    }

    PageTable *virtualPage2 = PageTable_get_entry(virtualPage1, vpn1);
    if (!virtualPage2) {
        MMU_raise_page_fault(mmu, virtual_address, pid);
        virtualPage2 = PageTable_create();
        PageTable_set_entry(virtualPage1, vpn1, virtualPage2);
    }

    PageTable *virtualPage3 = PageTable_get_entry(virtualPage2, vpn2);
    if (!virtualPage3) {
        MMU_raise_page_fault(mmu, virtual_address, pid);
        virtualPage3 = PageTable_create();
        assert(virtualPage3);
        PageTable_set_entry(virtualPage2, vpn2, virtualPage3);
    }

    void *frame_address = PageTable_get_entry(virtualPage3, vpn3);
    if (!frame_address) {
        MMU_raise_page_fault(mmu, virtual_address, pid);
        frame_address = ask_kernel_for_frame();
        PageTable_set_entry(virtualPage3, vpn3, frame_address);
    }

    TLB_add_physical_address(&(mmu->tlb), tlb_addr, frame_address);

	return frame_address + offset;
}

void MMU_tlb_miss(MMU *mmu, void *address, size_t pid) {
	assert(pid < MAX_PROCESS_ID);
	mmu->num_tlb_misses++;
	printf("Process [%lu] tried to access [%p] and it couldn't find it in the "
				 "TLB so the MMU has to check the PAGE TABLES\n",
				 pid, address);
}

void MMU_raise_page_fault(MMU *mmu, void *address, size_t pid) {
	assert(pid < MAX_PROCESS_ID);
	mmu->num_page_faults++;
	printf(
			"Process [%lu] tried to access [%p] and the MMU got an invalid entry\n",
			pid, address);
}

void MMU_add_process(MMU *mmu, size_t pid) {
	assert(pid < MAX_PROCESS_ID);
	mmu->base_pts[pid] = PageTable_create();
}

void MMU_free_process_tables(MMU *mmu, size_t pid) {
	assert(pid < MAX_PROCESS_ID);
	PageTable *base_pt = mmu->base_pts[pid];
	Pagetable_delete_tree(base_pt);
}

void MMU_delete(MMU *mmu) {
	for (size_t i = 0; i < MAX_PROCESS_ID; i++) {
		MMU_free_process_tables(mmu, i);
	}
	TLB_delete(mmu->tlb);
	free(mmu);
}
