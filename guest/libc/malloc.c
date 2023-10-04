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
	char* block;
} alloc_node_t;

/* allocation metadata size */
#define ALLOC_HEADER_SZ __builtin_offsetof(alloc_node_t, block)

/* minimum allocation size of 32 bytes */
#define MIN_ALLOC_SZ ALLOC_HEADER_SZ + 32

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

void* malloc(size_t size)
{
	void* ptr = NULL;
	alloc_node_t* blk = NULL;

	if (size > 0)
	{
		/* Align the pointer */
		size = ALIGN_SIZE(size, sizeof(void*));

		/* try to find a big enough block */
		list_for_each_entry(blk, &free_list, node)
		{
			if (blk->size >= size)
			{
				ptr = &blk->block;
				break;
			}
		}

		if (ptr)
		{
			/* split block if possible */
			if ((blk->size - size) >= MIN_ALLOC_SZ)
			{
				alloc_node_t* new_blk;
				new_blk = (alloc_node_t*)((uintptr_t)(&blk->block) + size);
				new_blk->size = blk->size - size - ALLOC_HEADER_SZ;
				blk->size = size;
				list_add(&new_blk->node, &blk->node);
			}

			list_del(&blk->node);
		}

	}

	return ptr;
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
