#include "substitute.h"
#include "dis.h"
#include "execmem.h"
#include stringify(TARGET_DIR/jump-patch.h)
#include <psp2kern/kernel/sysmem.h>
#include "../../../taihen_internal.h"

/** The size of each trampoline allocation. We use it for outro and optional
 * intro. Realistically, we do not use an intro.
 */
#define SLAB_SIZE (TD_MAX_REWRITTEN_SIZE + 2 * MAX_JUMP_PATCH_SIZE)
#if (SLAB_SIZE % ARCH_MAX_CODE_ALIGNMENT != 0)
// if not aligned then substitute_hook_functions breaks!
#error SLAB_SIZE Must be aligned to ARCH_MAX_CODE_ALIGNMENT
#endif

/**
 * @file execmem.c
 *
 * @brief      Functions for allocating executable memory and writing to RO
 *             memory.
 *
 *             We only consider two arenas for allocating trampoline executable
 *             memory. One is shared in user space across all processes. The
 *             other is in kernel space for kernel hooks.
 */

/**
 * @brief      Allocate a slab of executable memory.
 *
 *             Two pointers will be returned: the executable ro pointer and a
 *             writable pointer.
 *
 * @param[in]  hint       Unused
 * @param      ptr_p      The writable pointer
 * @param      vma_p      The executable pointer address
 * @param      size_p     The size of the allocation. Always `SLAB_SIZE`.
 * @param      opt        A `tai_substitute_args_t` structure
 * @param[in]  hint  Unused
 *
 * @return     `SUBSTITUTE_OK` or `SUBSTITUTE_ERR_VM` if out of memory
 */
int execmem_alloc_unsealed(UNUSED uintptr_t hint, void **ptr_p, uintptr_t *vma_p, 
                           size_t *size_p, void *opt) {
    return SUBSTITUTE_OK;
}

/**
 * @brief      Flushes icache
 *
 * @param      ptr   Unused
 * @param[in]  vma   Pointer to flush
 * @param      opt   A `tai_substitute_args_t` structure
 *
 * @return     `SUBSTITUTE_OK`
 */
int execmem_seal(UNUSED void *ptr, uintptr_t vma, void *opt) {
    return SUBSTITUTE_OK;
}

/**
 * @brief      Frees executable memory from slab allocator
 *
 * @param      ptr   The writable pointer
 * @param[in]  vma   The executable address. Must match with ptr.
 * @param      opt   A `tai_substitute_args_t` structure
 */
void execmem_free(void *ptr, uintptr_t vma, void *opt) {
}

/**
 * @brief      Write to executable process memory
 *
 * @param      writes        List of writes
 * @param[in]  nwrites       Number of writes
 * @param[in]  callback      Unused
 * @param      callback_ctx  Unused
 *
 * @return     `SUBSTITUTE_OK` or `SUBSTITUTE_ERR_VM` on failure
 */
int execmem_foreign_write_with_pc_patch(struct execmem_foreign_write *writes,
                                        size_t nwrites,
                                        UNUSED execmem_pc_patch_callback callback,
                                        UNUSED void *callback_ctx) {
    return 0;
}
