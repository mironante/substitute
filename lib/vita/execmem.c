#include "substitute.h"
#include "dis.h"
#include "execmem.h"
#include stringify(TARGET_DIR/jump-patch.h)
#include <psp2kern/kernel/sysmem.h>
#include "../../../slab.h"
#include "../../../taihen_internal.h"

/** The size of each trampoline allocation. We use it for outro and optional
 * intro. Realistically, we do not use an intro.
 */
#define PATCH_ITEM_SIZE (TD_MAX_REWRITTEN_SIZE + 2 * MAX_JUMP_PATCH_SIZE)
#if (PATCH_ITEM_SIZE % ARCH_MAX_CODE_ALIGNMENT != 0)
// if not aligned then substitute_hook_functions breaks!
#error PATCH_ITEM_SIZE Must be aligned to ARCH_MAX_CODE_ALIGNMENT
#endif

/** We use the slab allocator for both these things. Choose the larger of the two as size. */
const int g_exe_slab_item_size = PATCH_ITEM_SIZE > sizeof(tai_hook_t) ? PATCH_ITEM_SIZE : sizeof(tai_hook_t);

/**
 * The reason we use the same slab allocator for patches (sized 216 max) and
 * tai_hook_t (size 16) is because in both cases, we need to allocate memory in
 * the user's memory space. One option is to use two different slabs and that
 * would make more sense. However my prediction is that there is not a large
 * number of hooks per process, so the minimum size for the slab (0x1000 bytes)
 * is already too much. Better to waste 200 bytes per allocation than 2000 bytes
 * per process. If usage dictates a need for change, it is easy enough to put
 * them in different slabs.
 */

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
 * @param      size_p     The size of the allocation. Always `SLAB_ITEM_SIZE`.
 * @param      opt        A `tai_substitute_args_t` structure
 * @param[in]  hint  Unused
 *
 * @return     `SUBSTITUTE_OK` or `SUBSTITUTE_ERR_VM` if out of memory
 */
int execmem_alloc_unsealed(UNUSED uintptr_t hint, void **ptr_p, uintptr_t *vma_p, 
                           size_t *size_p, void *opt) {
    struct slab_chain *slab = (struct slab_chain *)opt;

    *ptr_p = slab_alloc(slab, vma_p);
    *size_p = PATCH_ITEM_SIZE;
    if (*ptr_p == NULL) {
        return SUBSTITUTE_ERR_VM;
    } else {
        return SUBSTITUTE_OK;
    }
}

/**
 * @brief      Flushes icache
 *
 * @param      ptr   Unused
 * @param      opt   A `tai_substitute_args_t` structure
 *
 * @return     `SUBSTITUTE_OK`
 */
int execmem_seal(void *ptr, void *opt) {
    uintptr_t vma, ptr_align;
    size_t len_align;
    struct slab_chain *slab = (struct slab_chain *)opt;

    vma = slab_getmirror(slab, ptr);
    vma = vma & ~0x1F;
    ptr_align = (uintptr_t)ptr & ~0x1F;
    len_align = (((uintptr_t)ptr + PATCH_ITEM_SIZE + 0x1F) & ~0x1F) - ptr_align;

    sceKernelCpuDcacheAndL2Flush((void *)ptr_align, len_align);
    sceKernelCpuIcacheAndL2Flush((void *)vma, len_align);

    return SUBSTITUTE_OK;
}

/**
 * @brief      Frees executable memory from slab allocator
 *
 * @param      ptr   The writable pointer
 * @param      opt   A `tai_substitute_args_t` structure
 */
void execmem_free(void *ptr, void *opt) {
    struct slab_chain *slab = (struct slab_chain *)opt;
    slab_free(slab, ptr);
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
