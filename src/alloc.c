#include "malloc_internal.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

_block_t *_small_bin = NULL;
_block_t *_medium_bin = NULL;
_block_t *_large_bin = NULL;

_free_chunk_t *_small_buckets[_SMALL_BUCKETS_COUNT];
_free_chunk_t *_small_remaining;

static inline size_t _get_page_size(void)
{
	static size_t page_size = 0;

	if(__builtin_expect(!!page_size, 1))
		page_size = sysconf(_SC_PAGE_SIZE);
	return page_size;
}

void *_alloc_pages(const size_t n)
{
	return mmap(NULL, n * _get_page_size(),
		PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
}

void _free_pages(void *addr, const size_t n)
{
	munmap(addr, n * _get_page_size());
}

_block_t *_alloc_block(const size_t pages)
{
	_block_t *b;

	if(pages == 0 || !(b = _alloc_pages(pages)))
		return NULL;
	bzero(b, sizeof(_block_t));
	b->pages = pages;
	b->first_chunk->length = pages * _get_page_size()
		- OFFSET_OF(_block_t, first_chunk);
#ifdef _MALLOC_CHUNK_MAGIC
	b->first_chunk->magic = _MALLOC_CHUNK_MAGIC;
#endif
	return b;
}

void _free_block(_block_t *b)
{
	if(b->prev)
		b->prev->next = b->next;
	else
	{
		if(b == _small_bin)
			_small_bin = _small_bin->next;
		else if(b == _medium_bin)
			_medium_bin = _medium_bin->next;
		else if(b == _large_bin)
			_large_bin = _large_bin->next;
	}
	if(b->next)
		b->next->prev = b->prev;
	_free_pages(b, b->pages);
}

static inline void _bin_link(_block_t **bin, _block_t *block)
{
	block->next = *bin;
	*bin = block;
}

void _bucket_link(_free_chunk_t *chunk)
{
	// TODO
	(void) chunk;
}

void _bucket_unlink(_free_chunk_t *chunk)
{
	// TODO
	(void) chunk;
}

static _free_chunk_t **get_small_bucket(const size_t size)
{
	size_t i = 0;
	_free_chunk_t **chunk;

	while((1 << i) < size || !_small_buckets[i])
		++i;
	if(i < _SMALL_BUCKETS_COUNT)
		chunk = &_small_buckets[i];
	else
		chunk = &_small_remaining;
	return (*chunk ? chunk : NULL);
}

static void _split_chunk(_free_chunk_t *chunk, const size_t size)
{
	_free_chunk_t *new_chunk;

	if(chunk->hdr.length < size + sizeof(_used_chunk_t) + sizeof(_free_chunk_t))
		return;
#ifdef _MALLOC_CHUNK_MAGIC
	if(chunk->hdr.magic != _MALLOC_CHUNK_MAGIC)
	{
		dprintf(STDERR_FILENO, "_split_chunk(): corrupted block\n");
		abort();
	}
#endif
	new_chunk = (_free_chunk_t *) (((_used_chunk_t *) chunk)->data + size);
	if((new_chunk->hdr.prev = (_chunk_hdr_t *) chunk))
		new_chunk->hdr.prev->next = (_chunk_hdr_t *) new_chunk;
	if((new_chunk->hdr.next = (_chunk_hdr_t *) chunk->hdr.next))
		new_chunk->hdr.next->prev = (_chunk_hdr_t *) new_chunk;
	new_chunk->hdr.length = chunk->hdr.length
		- (size + sizeof(_used_chunk_t) + sizeof(_free_chunk_t));
	new_chunk->hdr.used = 0;
#ifdef _MALLOC_CHUNK_MAGIC
	new_chunk->hdr.magic = _MALLOC_CHUNK_MAGIC;
#endif
	// TODO Add `new_chunk` to bucket
	chunk->hdr.length = size;
	chunk->hdr.used = 1;
}

void *_small_alloc(const size_t size)
{
	_free_chunk_t **bucket;
	_block_t *b;
	_free_chunk_t *chunk;

	if(!(bucket = get_small_bucket(size)))
	{
		if(!(b = _alloc_block(_SMALL_BUCKETS_COUNT)))
			return NULL;
		_bin_link(&_small_bin, b);
		chunk = (void *) b->first_chunk;
	}
	else
	{
		chunk = *bucket;
		*bucket = chunk->next_free;
	}
	_split_chunk(chunk, size);
	return ((_used_chunk_t *) chunk)->data;
}

void *_medium_alloc(const size_t size)
{
	(void) size;
	// TODO
	return NULL;
}

void *_large_alloc(const size_t size)
{
	_block_t *b;

	if(!(b = _alloc_block(CEIL_DIVISION(size, _get_page_size()))))
		return NULL;
	_bin_link(&_large_bin, b);
	b->first_chunk->used = 1;
	return ((_used_chunk_t *) b->first_chunk)->data;
}
