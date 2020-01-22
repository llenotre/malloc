#include "malloc.h"
#include "malloc_internal.h"

#include <stdio.h>

// TODO rm
#define _MALLOC_DEBUG_SHOW_FREE

#ifdef _MALLOC_DEBUG
extern _block_t *_small_bin;
extern _block_t *_medium_bin;
extern _block_t *_large_bin;

static size_t debug_print(const char *str, _block_t *b)
{
	_chunk_hdr_t *c;
	size_t total = 0;

	while(b)
	{
		printf("%s: %p\n", str, b);
		c = BLOCK_DATA(b);
		while(c)
		{
#ifndef _MALLOC_DEBUG_SHOW_FREE
			if(c->used)
#endif
			{
#ifdef _MALLOC_DEBUG_SHOW_FREE
				printf("%p - %p: %zu bytes (%s)\n", ((_used_chunk_t *) c)->data,
					((_used_chunk_t *) c)->data + c->length, c->length,
						(c->used ? "used" : "free"));
#else
				printf("%p - %p: %zu bytes\n", ((_used_chunk_t *) c)->data,
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
	printf("Total: %zu bytes\n", total);
}
#endif
