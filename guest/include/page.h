#pragma once

#include "types.h"

void init_pages(ullong memory_start, ullong memory_end);
ullong pages_inuse();
ullong pages_free();
void* alloc_pages(ullong npages);
void free_pages(void *addr, ullong npages);
