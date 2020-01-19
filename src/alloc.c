#include "malloc_internal.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

/*
 * This file handles internal operations for the allocator.
 *
 * Allocations are stored into linked lists of chunks. Every chunk is allocated
 * according to the value into `ALIGNMENT`.
 *
 * Chunks are contained into blocks which represent the memory regions given by
 * the kernel.
 * Blocks are sorted into bins according to the size of the allocations they can
 * handle.
 *
 * - _small_bin: size < _SMALL_BIN_MAX
 * - _medium_bin: size < _MEDIUM_BIN_MAX
 * - _large_bin: size >= _MEDIUM_BIN_MAX
 *
 * Blocks in `_large_bin` contain only one allocation which can be several
 * pages large.
 *
 * _MALLOC_CHUNK_MAGIC is a magic number used in chunk structures to ensure that
 * chunks aren't overwritten. If the value has been changed between two
 * operations of the allocator, the current process shall abort.
 */

/*
 * Bins containing the list of allocated blocks
 */
_block_t *_small_bin = NULL;
_block_t *_medium_bin = NULL;
_block_t *_large_bin = NULL;

/*
 * Buckets containing lists of free chunks.
 * Lists are sorted according to the size of the empty chunk
 *
 * A chunk must be at least `n` bytes large to fit in a bucket, where
 * n=_FIRST_SMALL_BUCKET_SIZE * 2^i . Here, `i` is the index in the array
 */
_free_chunk_t *_small_buckets[_SMALL_BUCKETS_COUNT];

/*
 * Returns the size in bytes of a page of memory on the current system
 */
static inline size_t _get_page_size(void)
{
	static size_t page_size = 0;

	if(__builtin_expect((page_size == 0), 1))
	{
		page_size = sysconf(_SC_PAGE_SIZE);
#ifdef _MALLOC_DEBUG
		dprintf(STDERR_FILENO, "malloc: page size: %zu bytes\n", page_size);
#endif
	}
	if(page_size == 0)
	{
		dprintf(STDERR_FILENO, "abort: _SC_PAGE_SIZE == 0\n");
		abort();
	}
	return page_size;
}

/*
 * Asks the kernel for `n` pages and returns the pointer to the beginning
 * of the allocated region of memory
 */
void *_alloc_pages(const size_t n)
{
	return mmap(NULL, n * _get_page_size(),
		PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
}

/*
 * Frees the given region of `n` memory pages starting at `addr`
 */
void _free_pages(void *addr, const size_t n)
{
	munmap(addr, n * _get_page_size());
}


/*
 * Allocates a `pages` pages long block of memory and creates a chunk on it that
 * covers the whole block
 */
_block_t *_alloc_block(const size_t pages)
{
	_block_t *b;
	_chunk_hdr_t *first_chunk;

	if(pages == 0 || !(b = _alloc_pages(pages)))
		return NULL;
	bzero(b, BLOCK_HDR_SIZE(b));
	b->pages = pages;
	first_chunk = BLOCK_DATA(b);
	first_chunk->length = pages * _get_page_size() - BLOCK_HDR_SIZE(b);
#ifdef _MALLOC_CHUNK_MAGIC
	first_chunk->magic = _MALLOC_CHUNK_MAGIC;
#endif
	return b;
}

/*
 * Unlinks the given block `b` from its bin and frees it
 */
void _free_block(_block_t *b)
{
	if(b->prev)
		b->prev->next = b->next;
	else if(b == _small_bin)
		_small_bin = b->next;
	else if(b == _medium_bin)
		_medium_bin = b->next;
	else if(b == _large_bin)
		_large_bin = b->next;
	if(b->next)
		b->next->prev = b->prev;
	_free_pages(b, b->pages);
}

/*
 * Links the given block to the given bin
 */
static inline void _bin_link(_block_t **bin, _block_t *block)
{
	block->next = *bin;
	*bin = block;
}

/*
 * Returns a small bucket containing chunks large enough to fit an allocation of
 * the given `size`. If `insert` is not zero, the function will return the first
 * bucket that fits even if empty to allow insertion of a new free chunk.
 */
static _free_chunk_t **get_small_bucket(const size_t size, const int insert)
{
	size_t i = 0;

	if(insert)
	{
		while((_FIRST_SMALL_BUCKET_SIZE << (i + 1)) < size
			&& i < _SMALL_BUCKETS_COUNT - 1)
			++i;
	}
	else
		while((!_small_buckets[i]
			|| (_FIRST_SMALL_BUCKET_SIZE << (i + 1)) < size)
				&& i < _SMALL_BUCKETS_COUNT - 1)
			++i;
	return &_small_buckets[i];
}

/*
 * Links the given free chunk to the corresponding bucket
 */
// TODO Handle medium
void _bucket_link(_free_chunk_t *chunk)
{
	_free_chunk_t **bucket;

	if(!(bucket = get_small_bucket(chunk->hdr.length, 1)))
	{
		// TODO Error?
		return;
	}
	chunk->prev_free = NULL;
	chunk->next_free = *bucket;
	*bucket = chunk;
}

/*
 * Unlinks the given free chunk from its bucket
 */
// TODO Handle medium
void _bucket_unlink(_free_chunk_t *chunk)
{
	size_t i = 0;

	if(chunk->prev_free)
		chunk->prev_free->next_free = chunk->next_free;
	if(chunk->next_free)
		chunk->next_free->prev_free = chunk->prev_free;
	chunk->prev_free = NULL;
	chunk->next_free = NULL;
	for(; i < _SMALL_BUCKETS_COUNT; ++i)
		if(chunk == _small_buckets[i])
			_small_buckets[i] = chunk->next_free;
}

/*
 * Allocates the given chunk for size `size`. Chunk might be split to another
 * chunk if large enough. The new free chunk might be inserted in buckets for
 * further allocations.
 */
static void _alloc_chunk(_free_chunk_t *chunk, size_t size)
{
	_free_chunk_t *new_chunk;
	size_t new_len, l;

#ifdef _MALLOC_CHUNK_MAGIC
	if(chunk->hdr.magic != _MALLOC_CHUNK_MAGIC)
	{
		dprintf(STDERR_FILENO, "abort: %s(): corrupted block\n", __func__);
		abort();
	}
#endif
	chunk->hdr.used = 1;
	_bucket_unlink(chunk);
	new_chunk = (_free_chunk_t *) UP_ALIGN(CHUNK_DATA(chunk) + size, ALIGNMENT);
	size = MAX(ALIGNMENT, ((void *) new_chunk - CHUNK_DATA(chunk)));
	new_len = size + CHUNK_HDR_SIZE(new_chunk);
	if(chunk->hdr.length <= new_len + ALIGNMENT)
		return;
	l = chunk->hdr.length;
	chunk->hdr.length = size;
	if((new_chunk->hdr.next = (_chunk_hdr_t *) chunk->hdr.next))
		new_chunk->hdr.next->prev = (_chunk_hdr_t *) new_chunk;
	if((new_chunk->hdr.prev = (_chunk_hdr_t *) chunk))
		new_chunk->hdr.prev->next = (_chunk_hdr_t *) new_chunk;
	new_chunk->hdr.length = l - new_len;
	new_chunk->hdr.used = 0;
#ifdef _MALLOC_CHUNK_MAGIC
	new_chunk->hdr.magic = _MALLOC_CHUNK_MAGIC;
#endif
	_bucket_link(new_chunk);
}

/*
 * Handles a small allocation
 */
void *_small_alloc(const size_t size)
{
	_free_chunk_t **bucket;
	_block_t *b;
	_free_chunk_t *chunk;

	if(!(bucket = get_small_bucket(size, 0)) || !*bucket)
	{
		if(!(b = _alloc_block(_SMALL_BLOCK_PAGES)))
			return NULL;
		_bin_link(&_small_bin, b);
		chunk = BLOCK_DATA(b);
	}
	else
		chunk = *bucket;
	_alloc_chunk(chunk, size);
	return CHUNK_DATA(chunk);
}

/*
 * Handles a medium allocation
 */
void *_medium_alloc(const size_t size)
{
	(void) size;
	// TODO
	return NULL;
}

/*
 * Handles a large allocation
 */
void *_large_alloc(const size_t size)
{
	size_t min_size;
	_block_t *b;
	_chunk_hdr_t *first_chunk;

	min_size = BLOCK_HDR_SIZE(NULL) + CHUNK_HDR_SIZE(BLOCK_HDR_SIZE(NULL))
		+ size;
	if(!(b = _alloc_block(CEIL_DIVISION(min_size, _get_page_size()))))
		return NULL;
	_bin_link(&_large_bin, b);
	first_chunk = BLOCK_DATA(b);
	first_chunk->used = 1;
	return CHUNK_DATA(first_chunk);
}
