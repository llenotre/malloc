#include "malloc.h"
#include "malloc_internal.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#ifdef _MALLOC_DEBUG
extern _block_t *_small_bin;
extern _block_t *_medium_bin;
extern _block_t *_large_bin;

extern _free_chunk_t *_small_buckets[_SMALL_BUCKETS_COUNT];
extern _free_chunk_t *_medium_buckets[_MEDIUM_BUCKETS_COUNT];

/*
 * Prints global informations.
 */
void _debug_print_malloc_info(void)
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
# ifndef _MALLOC_DEBUG_SHOW_FREE
			if(c->used)
# endif
			{
# ifdef _MALLOC_DEBUG_SHOW_FREE
				dprintf(STDERR_FILENO, "%p - %p: %zu bytes (%s)\n",
					((_used_chunk_t *) c)->data,
						((_used_chunk_t *) c)->data + c->length, c->length,
							(c->used ? "used" : "free"));
# else
				dprintf(STDERR_FILENO, "%p - %p: %zu bytes\n",
					((_used_chunk_t *) c)->data,
						((_used_chunk_t *) c)->data + c->length, c->length);
# endif
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

// TODO Check for loops
/*
 * Checks if the given block is in inconsistent state.
 */
static void _check_block(const char *bin, _block_t *b)
{
	_chunk_hdr_t *c;
	void *end;

	c = BLOCK_DATA(b);
	while(c)
	{
# ifdef _MALLOC_CHUNK_MAGIC
		if(c->magic != _MALLOC_CHUNK_MAGIC)
		{
			dprintf(STDERR_FILENO, "%s bin contains corrupted element %p\n",
				bin, c);
			abort();
		}
# endif
		if(c->prev == c || c->next == c || (c->prev && c->prev == c->next))
		{
			dprintf(STDERR_FILENO, "%s bin contains small loop at %p\n",
				bin, c);
			abort();
		}
		if((c->prev && c->prev->next != c) || (c->next && c->next->prev != c))
		{
			dprintf(STDERR_FILENO, "%s bin has broken linked list at %p\n",
				bin, c);
			abort();
		}
		if((c->prev && c->prev >= c) || (c->next && c->next <= c))
		{
			dprintf(STDERR_FILENO, "%s bin has non-linear linked list at %p\n",
				bin, c);
			abort();
		}
		if(!c->used
			&& ((c->prev && !c->prev->used) || (c->next && !c->next->used)))
		{
			dprintf(STDERR_FILENO, "%s bin has non-linear linked list at %p\n",
				bin, c);
			abort();
		}
		end = CHUNK_DATA(c) + c->length;
		if((!c->next && !IS_ALIGNED(end, _get_page_size()))
			|| (c->next && c->next != end))
		{
			dprintf(STDERR_FILENO, "%s bin has %s at %p (%ju)\n",
				bin, ((void *) c->next < end ? "gap" : "overlap"), c,
					(intptr_t) end - (intptr_t) c->next);
			abort();
		}
		c = c->next;
	}
}

// TODO Check chunks size
/*
 * Checks if the given bucket chunk is in inconsistent state.
 */
static void _check_bucket(const char *bucket_type, const size_t bucket_id,
	_free_chunk_t *c)
{
	size_t n = 0;

	while(c)
	{
# ifdef _MALLOC_CHUNK_MAGIC
		if(c->hdr.magic != _MALLOC_CHUNK_MAGIC)
		{
			dprintf(STDERR_FILENO, "%s bucket %zu contains corrupted element at\
 %zu (%p)\n", bucket_type, bucket_id, n, c);
			abort();
		}
# endif
		if(c->hdr.used)
		{
			dprintf(STDERR_FILENO, "%s bucket %zu contains used element at\
 %zu (%p)\n", bucket_type, bucket_id, n, c);
			exit(1);
		}
		if((n == 0 && c->prev_free)
			|| (c->next_free && c->next_free->prev_free != c))
		{
			dprintf(STDERR_FILENO, "%s bucket %zu has broken linked list at\
 %zu (%p)\n", bucket_type, bucket_id, n, c);
			exit(1);
		}
		// TODO Check size of elements according to bucket
		++n;
		c = c->next_free;
	}
}

/*
 * Checks if any bin, bucket or chunk is in inconsistent state.
 */
void _debug_global_check(void)
{
	_block_t *b;
	size_t i;

	b = _small_bin;
	while(b)
	{
		_check_block("SMALL", b);
		b = b->next;
	}
	b = _medium_bin;
	while(b)
	{
		_check_block("MEDIUM", b);
		b = b->next;
	}
	b = _large_bin;
	while(b)
	{
		_check_block("LARGE", b);
		b = b->next;
	}
	for(i = 0; i < _SMALL_BUCKETS_COUNT; ++i)
		_check_bucket("SMALL", i, _small_buckets[i]);
	for(i = 0; i < _MEDIUM_BUCKETS_COUNT; ++i)
		_check_bucket("MEDIUM", i, _small_buckets[i]);
}
#endif
