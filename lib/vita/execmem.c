#include <psp2kern/kernel/sysmem.h>
#include "execmem.h"
#include "substitute.h"

int execmem_alloc_unsealed(uintptr_t hint, void **page_p, size_t *size_p) {
    return 0;
}

int execmem_seal(void *page) {
    return SUBSTITUTE_OK;
}

void execmem_free(void *page) {
}

int execmem_foreign_write_with_pc_patch(struct execmem_foreign_write *writes,
                                        size_t nwrites,
                                        execmem_pc_patch_callback callback,
                                        void *callback_ctx) {
    return 0;
}
