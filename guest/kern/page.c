#include "stdio.h"
#include "page.h"
#include "bitmap.h"
#include "x86_mmu.h"

typedef struct
{
	ullong addr;
	ullong npages;
	ullong fpages;
	ullong offset;
} page_pool;

page_pool pages;

inline ullong bitmap_pages() { return 1 + pages.npages / (PAGE_SIZE << 3); }
inline ullong bitmap_size() { return bitmap_pages() * (PAGE_SIZE << 3); }

void init_pages(ullong start, ullong end)
{
    pages.addr = start;
    pages.npages = (end - start) >> PAGE_SHIFT;
    pages.fpages = pages.npages - bitmap_pages();
    bitmap_set((ullong*)pages.addr, bitmap_size(), 0, bitmap_pages(), 1);
    printf("# page pool   %llx - %llx [%lld pages]\n",
           start, end, pages.npages);
}

ullong pages_inuse()
{
	return pages.npages - pages.fpages;
}

ullong pages_free()
{
	return pages.fpages;
}

void* alloc_pages(ullong npages)
{
	ullong o;

	if (pages.fpages < npages) return NULL;
	o = bitmap_scan((ullong*)pages.addr, bitmap_size(), pages.offset, npages);
	if (o == (ullong)-1) return NULL;
	pages.offset = o + npages;
	pages.fpages = pages.fpages - npages;
    bitmap_set((ullong*)pages.addr, bitmap_size(), o, npages, 1);
    return (uchar*)pages.addr + (o << PAGE_SHIFT);
}

void free_pages(void *addr, ullong npages)
{
	ullong o = ((ullong)addr - pages.addr) >> PAGE_SHIFT;
    bitmap_set((ullong*)pages.addr, bitmap_size(), o, npages, 0);
	pages.fpages = pages.fpages + npages;
}
