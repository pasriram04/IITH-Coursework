#include <types.h>
#include <mmap.h>
#include <fork.h>
#include <v2p.h>
#include <page.h>
#define PAGE_SIZE 4096
/*
 * You may define macros and other helper functions here
 * You must not declare and use any static/global variables
 * */

#define PAGE_ALIGN(x) (((x) + 4095) & ~0xFFF)


// Helper functions for cfork

void helper_page_table_creator(struct exec_context *parent, struct exec_context *child, u64 vaddr, int permission) {
    u64 indices[5];
    u64 mask9 = 0x1FF;
    //extracting 5-level page table indices
    indices[0] = (vaddr >> 39) & mask9; //PGD
    indices[1] = (vaddr >> 30) & mask9; //PUD
    indices[2] = (vaddr >> 21) & mask9; //PMD
    indices[3] = (vaddr >> 12) & mask9; //PTE
    indices[4] = vaddr & 0xFFF;         //Offset within page
    u64 *table_parent = (u64 *)osmap(parent->pgd);
    u64 *table_child = (u64 *)osmap(child->pgd);

    for (int level = 0; level < 3; ++level) {
        u64 *entry_parent = table_parent + indices[level];
        u64 *entry_child = table_child + indices[level];
        if (!(*entry_child & 1)) {
            if (!(*entry_parent & 1)){
                return; //Parent doesn't have this mapping
            }
            u64 new_pfn = os_pfn_alloc(OS_PT_REG);
            *entry_child = ((new_pfn << 12) & ~0xFFF) | 1;
            if (permission & PROT_WRITE)
                *entry_child |= 0x8;
            *entry_child |= 0x10; //User-accessible
        }
        else if (permission & PROT_WRITE) {
                *entry_child |= 0x8;
        }
        //going to next level
        table_parent = (u64 *)osmap((*entry_parent) >> 12);
        table_child = (u64 *)osmap((*entry_child) >> 12);
    }

    //we ar enow at PTE level
    u64 *pte_parent = table_parent + indices[3];
    u64 *pte_child = table_child + indices[3];
    if (*pte_parent & 1) {
        *pte_parent &= ~(1UL << 3);     // Clear COW bit (bit 3)
        *pte_child = *pte_parent;
        get_pfn(*pte_parent >> 12);
    }
    else {
        *pte_child = 0x0;
    }
}

void helper_initialize_child_context(struct exec_context *child, struct exec_context *parent) {
    child->ppid = parent->pid;
    child->ticks_to_alarm = parent->ticks_to_alarm;
    child->alarm_config_time = parent->alarm_config_time;
    child->ticks_to_sleep = parent->ticks_to_sleep;
    child->pending_signal_bitmap = parent->pending_signal_bitmap;
    child->regs = parent->regs;
    child->state = parent->state;
    child->used_mem = parent->used_mem;
    child->type = parent->type;
    memcpy(child->name, parent->name, CNAME_MAX);
}

void helper_clone_vm_areas(struct exec_context *parent, struct exec_context *child){
    struct vm_area *parent_vma = parent->vm_area;
    struct vm_area **child_vma_ptr = &(child->vm_area);
    while (parent_vma) {
        *child_vma_ptr = os_alloc(sizeof(struct vm_area));
        (*child_vma_ptr)->vm_start = parent_vma->vm_start;
        (*child_vma_ptr)->vm_end = parent_vma->vm_end;
        (*child_vma_ptr)->access_flags = parent_vma->access_flags;
        (*child_vma_ptr)->vm_next = NULL;
        child_vma_ptr = &((*child_vma_ptr)->vm_next);
        parent_vma = parent_vma->vm_next;
    }
}

void helper_copy_other_stuff(struct exec_context *parent, struct exec_context *child) {
    for (int i = 0; i < MAX_MM_SEGS; i++) {
        child->mms[i] = parent->mms[i];
    }
    for (int i = 0; i < MAX_OPEN_FILES; i++) {
        child->files[i] = parent->files[i];
    }
    for (int i = 0; i < MAX_SIGNALS; i++) {
        child->sighandlers[i] = parent->sighandlers[i];
    }
    child->pgd = os_pfn_alloc(OS_PT_REG);
}

void helper_map_child_memory(struct exec_context *parent, struct exec_context *child) {
    struct vm_area *vma = child->vm_area;
    while (vma) {
        for (u64 addr = vma->vm_start; addr < vma->vm_end; addr += PAGE_SIZE) {
            helper_page_table_creator(parent, child, addr, vma->access_flags);
        }
        vma = vma->vm_next;
    }
    //map memory segments
    for (int i = 0; i<MAX_MM_SEGS; i++) {
        u64 start = child->mms[i].start;
        u64 end = (i == MM_SEG_STACK)?(child->mms[i].end):(child->mms[i].next_free);
        for (; start < end; start += PAGE_SIZE) {
            helper_page_table_creator(parent, child, start, child->mms[i].access_flags);
        }
    }
}

void flush_tlbs(void)
{
    u64 cr3;
    //inline assmebly
    asm volatile (
        "mov %%cr3, %0"
        : "=r" (cr3)
    );
    asm volatile (
        "mov %0, %%cr3"
        :
        : "r" (cr3)
        : "memory"
    );
}


int helper_free_pfns(u64 start, u64 end)
{
    u64 addr = start;
    for (addr = start; addr < end; addr += PAGE_SIZE)
    {
        struct exec_context *current = get_current_ctx();

        u64 *pgd_virtual = (u64 *)(osmap(current->pgd));
        u64 number = addr;
        u64 pgd_index = (number >> 39) & 0x1FF;
        u64 pud_index = (number >> 30) & 0x1FF;
        u64 pmd_index = (number >> 21) & 0x1FF;
        u64 pte_index = (number >> 12) & 0x1FF;

        u64 *pgd = (u64 *)osmap(current->pgd);

        if (!((pgd[pgd_index]) & 1))
        {
            return 0;
        }
        u64 *pud = (u64 *)osmap(pgd[pgd_index] >> 12);
        if (!(pud[pud_index] & 0x1))
        {
            return 0;
        }
        u64 *pmd = (u64 *)osmap(pud[pud_index] >> 12);
        if (!(pmd[pmd_index] & 0x1))
        {
            return 0;
        }
        u64 *pte = (u64 *)osmap(pmd[pmd_index] >> 12);
        if (!(pte[pte_index] & 0x1))
        {
            return 0;
        }

        u64 victim_pfm = (pte[pte_index]) >> 12;
        put_pfn(victim_pfm);
        if (get_pfn_refcount(victim_pfm) == 0)
        {
            os_pfn_free(USER_REG, victim_pfm);
        }
        pte[pte_index] = 0x0;
    }
    flush_tlbs();
    return 0;
}

int update_page_perm(u64 addr, int perm)
{
    struct exec_context *curr = get_current_ctx();
    u64 *pgd_virtual = (u64 *)(osmap(curr->pgd));
    u64 number = addr;
    u64 pgd_index = (number >> 39) & 0x1FF;
    u64 pud_index = (number >> 30) & 0x1FF;
    u64 pmd_index = (number >> 21) & 0x1FF;
    u64 pte_index = (number >> 12) & 0x1FF;
    u64 *pgd = (u64 *)osmap(curr->pgd);

    if (pgd[pgd_index] & 1)
    {
        if ((perm & PROT_WRITE) != 0)
        {
            pgd[pgd_index] = pgd[pgd_index] | 0x8;
        }
    }
    else
    {
        return 0;
    }
    u64 *pud = (u64 *)osmap(pgd[pgd_index] >> 12);
    if ((pud[pud_index] & 0x1))
    {
        if ((perm & PROT_WRITE) != 0)
        {
            pud[pud_index] = pud[pud_index] | 0x8;
        }
    }
    else
    {
        return 0; 
    }
    u64 *pmd = (u64 *)osmap(pud[pud_index] >> 12);
    if ((pmd[pmd_index] & 0x1))
    {
        if ((perm & PROT_WRITE) != 0)
        {
            pmd[pmd_index] = pmd[pmd_index] | 8;
        }
    }
    else
    {
        return 0; 
    }

    u64 *pte = (u64 *)osmap(pmd[pmd_index] >> 12);
    if ((pte[pte_index] & 0x1))
    {
        if ((perm & PROT_WRITE) != 0)
        {
            u64 pfn_shared = pte[pte_index];
            u32 pfn_shared_phys = (pfn_shared >> 12);
            if (get_pfn_refcount(pfn_shared_phys) == 1)
            {
                pte[pte_index] = pte[pte_index] | 0x8;
            }
        }
        else
        {
            u64 mask = ~(1ll << 3);
            pte[pte_index] = pte[pte_index] & mask;
        }
    }
    else{
        return 0; 
    }
    flush_tlbs();
    return 0;
}

int helper_update_perm(u64 start, u64 end, int permission)
{
    struct exec_context *curr = get_current_ctx();
    struct vm_area *cur = NULL;
    u64 addr = start;
    for (addr = start; addr < end; addr += PAGE_SIZE)
    {
        cur = curr->vm_area->vm_next;
        while (cur != NULL)
        {
            if (cur->vm_start <= addr && cur->vm_end >= end)
            {
                update_page_perm(addr, permission);
            }
            cur = cur->vm_next;
        }
    }
    flush_tlbs();
    return 0;
}
/**
 * mprotect System call Implementation.
 */
long vm_area_mprotect(struct exec_context *current, u64 addr, int length, int prot)
{
    if (!current || length <= 0)
        return -EINVAL;
    if (!(prot == PROT_READ || prot == (PROT_READ | PROT_WRITE)))
        return -EINVAL;
    if (addr < MMAP_AREA_START || addr >= MMAP_AREA_END || (addr & 0xFFF))
        return -EINVAL;

    u64 aligned_start = addr;
    u64 aligned_end = PAGE_ALIGN(addr + length);

    struct vm_area *dummy = current->vm_area;
    struct vm_area *prev = dummy;
    struct vm_area *curr = dummy->vm_next;

    while (curr)
    {
        //skip if no overlap
        if (curr->vm_end <= aligned_start || curr->vm_start >= aligned_end)
        {
            prev = curr;
            curr = curr->vm_next;
            continue;
        }

        // changing the fully inside split into up to 3 parts
        if (curr->vm_start < aligned_start && curr->vm_end > aligned_end)
        {
            struct vm_area *mid = os_alloc(sizeof(struct vm_area));
            struct vm_area *after = os_alloc(sizeof(struct vm_area));
            if (!mid || !after)
                return -EINVAL;

            mid->vm_start = aligned_start;
            mid->vm_end = aligned_end;
            mid->access_flags = prot;

            after->vm_start = aligned_end;
            after->vm_end = curr->vm_end;
            after->access_flags = curr->access_flags;

            mid->vm_next = after;
            after->vm_next = curr->vm_next;

            curr->vm_end = aligned_start;

            curr->vm_next = mid;

            stats->num_vm_area += 2;
            break;
            // return 0;
        }
        //Overlaps from left only- shrink left, insert modified right
        else if (curr->vm_start < aligned_start && curr->vm_end > aligned_start && curr->vm_end <= aligned_end)
        {
            struct vm_area *new_vma = os_alloc(sizeof(struct vm_area));
            if (!new_vma)
                return -EINVAL;

            new_vma->vm_start = aligned_start;
            new_vma->vm_end = curr->vm_end;
            new_vma->access_flags = prot;
            new_vma->vm_next = curr->vm_next;

            curr->vm_end = aligned_start;
            curr->vm_next = new_vma;

            stats->num_vm_area++;
            break;
        }

        //Ooverlaps from right only- shrink right, insert modified left
        else if (curr->vm_start >= aligned_start && curr->vm_start < aligned_end && curr->vm_end > aligned_end)
        {
            struct vm_area *new_vma = os_alloc(sizeof(struct vm_area));
            if (!new_vma)
                return -EINVAL;

            new_vma->vm_start = curr->vm_start;
            new_vma->vm_end = aligned_end;
            new_vma->access_flags = prot;

            curr->vm_start = aligned_end;

            new_vma->vm_next = curr;
            prev->vm_next = new_vma;

            stats->num_vm_area++;
            break;
        }
        // Fully inside, just update protection
        else if (curr->vm_start >= aligned_start && curr->vm_end <= aligned_end)
        {
            curr->access_flags = prot;
            prev = curr;
            curr = curr->vm_next;
            continue;
        }
        prev = curr;
        curr = curr->vm_next;
    }
    helper_update_perm(aligned_start, aligned_end, prot);
    return 0;
}

/**
 * mmap system call implementation.
 */
long vm_area_map(struct exec_context *current, u64 addr, int length, int prot, int flags)
{
    if (length < 0 || (addr % PAGE_SIZE != 0) || (prot != PROT_READ && prot != (PROT_WRITE | PROT_READ)) || (flags != 0 && flags != MAP_FIXED))
    {
        return -EINVAL;
    }
    // nedd to change the if condition
    long ans = -EINVAL;
    if (!(current->vm_area))
    {
        // adding the dummy
        struct vm_area *first = os_alloc(sizeof(struct vm_area));
        first->vm_start = MMAP_AREA_START;
        first->vm_end = MMAP_AREA_START + (1 << 12);
        first->access_flags = 0x0;
        first->vm_next = NULL;
        current->vm_area = first;
        stats->num_vm_area = 1;
    }
    if (length <= 0 || length > (2 << 20))
        return -EINVAL;
    if (!(flags == 0 || flags == MAP_FIXED))
        return -EINVAL;

    u64 aligned_len = PAGE_ALIGN(length);
    u64 start_addr = 0;
    u64 end_addr = 0;

    // if addr is not page aligned
    if (addr && (addr & 0xFFF))
        return -EINVAL;

    struct vm_area *dummy = current->vm_area; // Always points to dummy node
    struct vm_area *prev = dummy;
    struct vm_area *curr = dummy->vm_next;

    // Determine start and end address based on addr and flags
    if (addr)
    {
        start_addr = addr;
        end_addr = start_addr + aligned_len;

        if (start_addr < MMAP_AREA_START || end_addr > MMAP_AREA_END)
            return -EINVAL;

        int flag_overlap = 0;
        while (curr)
        {
            if (!(addr + aligned_len <= curr->vm_start || addr >= curr->vm_end))
            {
                if (flags == MAP_FIXED)
                    return -EINVAL; 
                else
                {
                    flag_overlap = 1;
                    break;
                }
            }
            curr = curr->vm_next;
        }
        if (flag_overlap)
            ans = vm_area_map(current, 0, aligned_len, prot, 0);
        else
        {
            prev = dummy;
            curr = dummy->vm_next;
            while (curr && curr->vm_start < start_addr)
            {
                prev = curr;
                curr = curr->vm_next;
            }
        }
    }
    else
    {
        u64 candidate = MMAP_AREA_START + PAGE_SIZE;
        while (curr)
        {
            if ((curr->vm_start - candidate) >= aligned_len)
            {
                start_addr = candidate;
                end_addr = start_addr + aligned_len;
                break;
            }
            candidate = curr->vm_end;
            prev = curr;
            curr = curr->vm_next;
        }

        if (!start_addr && (MMAP_AREA_END - candidate >= aligned_len))
        {
            start_addr = candidate;
            end_addr = start_addr + aligned_len;
        }

        if (!start_addr)
            return -EINVAL;
    }

    // Merging conditions
    int merge_prev = (prev && prev != dummy && prev->vm_end == start_addr && prev->access_flags == prot);
    int merge_next = (curr && curr->vm_start == end_addr && curr->access_flags == prot);

    if (merge_prev && merge_next)
    {
        prev->vm_end = curr->vm_end;
        prev->vm_next = curr->vm_next;
        os_free(curr, sizeof(struct vm_area));
        stats->num_vm_area--;
        return start_addr;
    }
    else if (merge_prev)
    {
        prev->vm_end = end_addr;
        return start_addr;
    }
    else if (merge_next)
    {
        curr->vm_start = start_addr;
        return start_addr;
    }
    else
    {
        struct vm_area *new_vma = os_alloc(sizeof(struct vm_area));
        if (!new_vma)
            return -EINVAL;

        new_vma->vm_start = start_addr;
        new_vma->vm_end = end_addr;
        new_vma->access_flags = prot;
        new_vma->vm_next = curr;
        prev->vm_next = new_vma;

        stats->num_vm_area++;
        return start_addr;
    }
    return ans;
}

/**
 * munmap system call implemenations
 */

long vm_area_unmap(struct exec_context *current, u64 addr, int length)
{
    if (!current || length <= 0)
        return -EINVAL;
    if (addr < MMAP_AREA_START || addr >= MMAP_AREA_END || (addr & 0xFFF))
        return -EINVAL;

    u64 aligned_start = addr;
    u64 aligned_end = PAGE_ALIGN(addr + length); // Round up end

    struct vm_area *dummy = current->vm_area;
    struct vm_area *prev = dummy;
    struct vm_area *curr = dummy->vm_next;

    while (curr)
    {
        //No overlap
        if (curr->vm_end <= aligned_start || curr->vm_start >= aligned_end)
        {
            prev = curr;
            curr = curr->vm_next;
            continue;
        }

        //Case 1 - entire VMA is inside unmap range remove node
        if (curr->vm_start >= aligned_start && curr->vm_end <= aligned_end)
        {
            prev->vm_next = curr->vm_next;
            os_free(curr, sizeof(struct vm_area));
            stats->num_vm_area--;
            curr = prev->vm_next;
            continue;
        }

        //Case 2 - VMA partially overlaps on the left, shrink right
        if (curr->vm_start < aligned_start && curr->vm_end > aligned_start && curr->vm_end <= aligned_end)
        {
            curr->vm_end = aligned_start;
            prev = curr;
            curr = curr->vm_next;
            continue;
        }

        //Case 3: VMA partially overlaps on the right, shrink left
        if (curr->vm_start >= aligned_start && curr->vm_start < aligned_end && curr->vm_end > aligned_end)
        {
            curr->vm_start = aligned_end;
            prev = curr;
            curr = curr->vm_next;
            continue;
        }

        //Case 4: VMA splits (middle chunk unmapped), split into two VMAs
        if (curr->vm_start < aligned_start && curr->vm_end > aligned_end)
        {
            struct vm_area *new_vma = os_alloc(sizeof(struct vm_area));
            if (!new_vma)
                return -EINVAL;

            new_vma->vm_start = aligned_end;
            new_vma->vm_end = curr->vm_end;
            new_vma->access_flags = curr->access_flags;
            new_vma->vm_next = curr->vm_next;

            curr->vm_end = aligned_start;
            curr->vm_next = new_vma;
            stats->num_vm_area++;
            break;
        }
    }
    helper_free_pfns(aligned_start, aligned_end); // NEED TO IMPLEMENT IT
    return 0; 
}

/**
 * Function will invoked whenever there is page fault for an address in the vm area region
 * created using mmap
 */

long vm_area_pagefault(struct exec_context *current, u64 addr, int error_code)
{
    if (!current)
        return -1;

    struct vm_area *vma = current->vm_area->vm_next;
    u64 page_addr = addr;

    while (vma && !(vma->vm_start <= page_addr && page_addr < vma->vm_end))
    {
        vma = vma->vm_next;
    }

    if (!vma || ((vma->access_flags == PROT_READ) && ((error_code >> 1) & 1)))
    {
        return -EINVAL;
    }

    if (error_code == 0x7)
    {
        return handle_cow_fault(current, page_addr, vma->access_flags);
    }

    u64 number = addr;
    u64 pgd_index = (number >> 39) & 0x1FF;
    u64 pud_index = (number >> 30) & 0x1FF;
    u64 pmd_index = (number >> 21) & 0x1FF;
    u64 pte_index = (number >> 12) & 0x1FF;

    u64 *pgd = (u64 *)osmap(current->pgd);

    if (!(pgd[pgd_index] & 0x1))
    { // present bit =0
        u64 pud_pfn = os_pfn_alloc(OS_PT_REG);
        if (!pud_pfn)
            return -1;
        pgd[pgd_index] = (pud_pfn << 12) | ((vma->access_flags & PROT_WRITE) ? 0x9 : 0x1) | 0x10;
    }
    else if (vma->access_flags & PROT_WRITE)
    {
        pgd[pgd_index] |= 0x8;
    }

    u64 *pud = (u64 *)osmap(pgd[pgd_index] >> 12);
    if (!(pud[pud_index] & 0x1))
    {
        u64 pmd_pfn = os_pfn_alloc(OS_PT_REG);
        if (!pmd_pfn)
            return -1;
        pud[pud_index] = (pmd_pfn << 12) | ((vma->access_flags & PROT_WRITE) ? 0x9 : 0x1) | 0x10;
    }
    else if (vma->access_flags & PROT_WRITE)
    {
        pud[pud_index] |= 0x8;
    }

    u64 *pmd = (u64 *)osmap(pud[pud_index] >> 12);
    if (!(pmd[pmd_index] & 0x1))
    {
        u64 pte_pfn = os_pfn_alloc(OS_PT_REG);
        if (!pte_pfn)
            return -1;
        pmd[pmd_index] = (pte_pfn << 12) | ((vma->access_flags & PROT_WRITE) ? 0x9 : 0x1) | 0x10;
    }
    else if (vma->access_flags & PROT_WRITE)
    {
        pmd[pmd_index] |= 0x8;
    }

    u64 *pte = (u64 *)osmap(pmd[pmd_index] >> 12);
    if (!(pte[pte_index] & 0x1))
    {
        u64 data_pfn = os_pfn_alloc(USER_REG);
        if (!data_pfn)
            return -1;
        pte[pte_index] = (data_pfn << 12) | ((vma->access_flags & PROT_WRITE) ? 0x9 : 0x1) | 0x10;
    }
    else if (vma->access_flags & PROT_WRITE)
    {
        pte[pte_index] |= 0x8;
    }

    return 1;
}

/**
 * cfork system call implemenations
 * The parent returns the pid of child process. The return path of
 * the child process is handled separately through the calls at the
 * end of this function (e.g., setup_child_context etc.)
 */

long do_cfork() {
    u32 child_pid;
    struct exec_context *parent = get_current_ctx();
    struct exec_context *child = get_new_ctx();

    helper_initialize_child_context(child, parent);
    helper_clone_vm_areas(parent, child);
    helper_copy_other_stuff(parent, child);
    helper_map_child_memory(parent, child);
    flush_tlbs();

    copy_os_pts(parent->pgd, child->pgd);
    do_file_fork(child);
    setup_child_context(child);
    
    return child->pid;
}


/* Cow fault handling, for the entire user address space
 * For address belonging to memory segments (i.e., stack, data)
 * it is called when there is a CoW violation in these areas.
 *
 * For vm areas, your fault handler 'vm_area_pagefault'
 * should invoke this function
 * */

long handle_cow_fault(struct exec_context *current, u64 vaddr, int access_flags) {
    const u64 MASK_9 = 0x1FF;
    const u64 MASK_12 = 0xFFF;
    u64 indices[4];
    u64 temp = vaddr >> 12;
    for (int i = 3; i >= 0; i--){
        indices[i] = temp & MASK_9;
        temp >>= 9;
    }
    u64 *level = (u64 *)osmap(current->pgd);
    //Walking from PGD to PUD to PMD
    for (int depth = 0; depth < 3; ++depth) {
        u64 *entry = level + indices[depth];
        if (!(*entry & 1)){
            u64 pfn = os_pfn_alloc(OS_PT_REG);
            *entry = ((pfn << 12) & ~0xFFF) | 1;
            if (access_flags & PROT_WRITE) {
                *entry |= 0x8;
            }
            *entry |= 0x10;
        }
        else if (access_flags & PROT_WRITE){
            *entry |= 0x8;
        }
        level = (u64 *)osmap((*entry) >> 12);
    }
    // Now, PTE level
    u64 *pte = level + indices[3];
    if (*pte & 1){
        u64 phys_pfn = (*pte) >> 12;
        if (get_pfn_refcount(phys_pfn) > 1){
            put_pfn(phys_pfn);
            u64 new_pfn = os_pfn_alloc(USER_REG);
            *pte = ((new_pfn << 12) & ~0xFFF) | 1;
            if (access_flags & PROT_WRITE){
                *pte |= 0x8;
            }
            *pte |= 0x10;
            memcpy(osmap(new_pfn), osmap(phys_pfn), PAGE_SIZE);
        }
        else if (access_flags & PROT_WRITE) {
            *pte |= 0x8;
        }
    }
    else {
        u64 new_pfn = os_pfn_alloc(USER_REG);
        *pte = ((new_pfn << 12) & ~0xFFF) | 1;
        if (access_flags & PROT_WRITE) {
            *pte |= 0x8;
        }
        *pte |= 0x10;
    }
    flush_tlbs();
    return 1;
}