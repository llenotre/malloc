#include "malloc.h"
#include "malloc_internal.h"

#include <stdio.h>
#include <unistd.h>

// TODO rm
#define _MALLOC_DEBUG_SHOW_FREE

#ifdef _MALLOC_DEBUG
extern _block_t *_small_bin;
extern _block_t *_medium_bin;
extern _block_t *_large_bin;

/*
 * Prints global informations.
 */
void _print_malloc_info(void)
{
	size_t i;

	dprintf(STDERR_FILENO, "Small buckets (max: %zu):\n", _SMALL_BIN_MAX);
	for(i = 0; i < _SMALL_BUCKETS_COUNT; ++i)
		dprintf(STDERR_FILENO, " %zu", (size_t) _FIRST_SMALL_BUCKET_SIZE << i);
	dprintf(STDERR_FILENO, "\nMedium buckets (max: %zu):\n", _MEDIUM_BIN_MAX);
	for(i = 0; i < _MEDIUM_BUCKETS_COUNT; ++i)
		dprintf(STDERR_FILENO, " %zu", (size_t) _FIRST_MEDIUM_BUCKET_SIZE << i);
	dprintf(STDERR_FILENO, "\nSmall block pages: %zu\n", _SMALL_BLOCK_PAGES);
	dprintf(STDERR_FILENO, "Medium block pages: %zu\n", _MEDIUM_BLOCK_PAGES);
}

/*
 * Prints chunks lists for the specified block.
 */
static size_t debug_print(const char *str, _block_t *b)
{
	_chunk_hdr_t *c;
	size_t total = 0;

	while(b)
	{
		dprintf(STDERR_FILENO, "%s: %p\n", str, b);
		c = BLOCK_DATA(b);
		while(c)
		{
#ifndef _MALLOC_DEBUG_SHOW_FREE
			if(c->used)
#endif
			{
#ifdef _MALLOC_DEBUG_SHOW_FREE
				dprintf(STDERR_FILENO, "%p - %p: %zu bytes (%s)\n",
					((_used_chunk_t *) c)->data,
						((_used_chunk_t *) c)->data + c->length, c->length,
							(c->used ? "used" : "free"));
#else
				dprintf(STDERR_FILENO, "%p - %p: %zu bytes\n",
					((_used_chunk_t *) c)->data,
						((_used_chunk_t *) c)->data + c->length, c->length);
#endif
				total += c->length;
			}
			c = c->next;
		}
		b = b->next;
	}
	return total;
}

/*
 * Prints blocks with memory along with their type and the list of chunks inside
 */
void _debug_show_alloc(void)
{
	size_t total = 0;

	total += debug_print("SMALL", _small_bin);
	total += debug_print("MEDIUM", _medium_bin);
	total += debug_print("LARGE", _large_bin);
	dprintf(STDERR_FILENO, "Total: %zu bytes\n", total);
}
#endif
