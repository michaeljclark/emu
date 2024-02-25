#include "stddef.h"
#include "stdint.h"

#include "list.h"

/* align to nearest power of two */
#define ALIGN_SIZE(sz, align) (((sz) + ((align)-1)) & ~((align)-1))

/* free list node*/
typedef struct alloc_node
{
	struct list_head node;
	size_t size;
	char block[0];
} alloc_node_t;

/* allocation metadata size */
#define ALLOC_HEADER_SZ __builtin_offsetof(alloc_node_t, block)

/* minimum allocation size of four pointers (32 bytes) */
#define MIN_ALLOC_SZ (ALLOC_HEADER_SZ + sizeof(void*))

/* free list */
static LIST_HEAD(free_list);

static void coalesce_free_list(void)
{
	alloc_node_t *b, *lb = NULL, *t;

	list_for_each_entry_safe(b, t, &free_list, node)
	{
		if (lb)
		{
			/* coalesce adjacent blocks */
			if ((((uintptr_t)&lb->block) + lb->size) == (uintptr_t)b)
			{
				lb->size += sizeof(*b) + b->size;
				list_del(&b->node);
				continue;
			}
		}
		lb = b;
	}
}

static int ispow2(size_t v)
{
	return !(v & (v-1));
}

void *aligned_alloc(size_t align, size_t size)
{
	alloc_node_t* blk = NULL, *alloc_blk = NULL, *new_blk;
	uintptr_t slack = 0;

	if (size == 0 || !align || !ispow2(align)) return NULL;

	/* Align the pointer */
	size = ALIGN_SIZE(size, sizeof(void*));

	/* try to find a big enough block */
	list_for_each_entry(blk, &free_list, node)
	{
		/* calculate slack to align an unaligned block */
		uintptr_t start = (uintptr_t)&blk->block;
		uintptr_t end = ALIGN_SIZE(start, align);
		while (end - start != 0 &&
			   end - start < ALLOC_HEADER_SZ) end += align;
		slack = end - start;

		/* break if the current block is big enough */
		if (blk->size >= size + slack)
		{
			alloc_blk = blk;
			break;
		}
	}

	if (!alloc_blk) return NULL;

	/* split block for alignment if necessary */
	if (slack > 0) {
		uintptr_t start = (uintptr_t)&alloc_blk->block;
		new_blk = (alloc_node_t*)(start + slack - ALLOC_HEADER_SZ);
		new_blk->size = alloc_blk->size - slack;
		alloc_blk->size = slack - ALLOC_HEADER_SZ;
		list_add(&new_blk->node, &alloc_blk->node);
		alloc_blk = new_blk;
	}

	/* split remainder of block if possible */
	if ((alloc_blk->size - size) >= MIN_ALLOC_SZ)
	{
		uintptr_t start = (uintptr_t)&alloc_blk->block;
		new_blk = (alloc_node_t*)(start + size);
		new_blk->size = alloc_blk->size - size - ALLOC_HEADER_SZ;
		alloc_blk->size = size;
		list_add(&new_blk->node, &alloc_blk->node);
	}

	list_del(&alloc_blk->node);

	return &alloc_blk->block;
}

void* malloc(size_t size)
{
	return aligned_alloc(sizeof(void*), size);
}

void free(void* ptr)
{
	alloc_node_t *blk, *free_blk;

	if (ptr)
	{
		blk = container_of(ptr, alloc_node_t, block);

		/* add block to free list in ascending order by pointer */
		list_for_each_entry(free_blk, &free_list, node)
		{
			if (free_blk > blk)
			{
				list_add_tail(&blk->node, &free_blk->node);
				goto blockadded;
			}
		}
		list_add_tail(&blk->node, &free_list);

	blockadded:
		coalesce_free_list();
	}
}

void _malloc_addblock(void* addr, size_t size)
{
	alloc_node_t* blk;

	/* pointer align the block */
	blk = (alloc_node_t*)ALIGN_SIZE((uintptr_t)addr, sizeof(void*));

	/* calculate usable size */
	blk->size = (uintptr_t)addr + size - (uintptr_t)blk - ALLOC_HEADER_SZ;

	/* add the block to the free list */
	list_add(&blk->node, &free_list);
}
