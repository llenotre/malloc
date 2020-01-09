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

void *_alloc_pages(const size_t n)
{
	return mmap(NULL, n * _get_page_size(),
		PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
}

void _free_pages(void *addr, const size_t n)
{
	munmap(addr, n * _get_page_size());
}

_block_t *_alloc_block(const size_t size)
{
	size_t pages;
	_block_t *b;

	pages = CEIL_DIVISION(sizeof(_block_t) + size, _get_page_size());
	if(pages == 0 || !(b = _alloc_pages(pages)))
		return NULL;
	bzero(b, sizeof(_block_t));
	b->pages = pages;
	b->first_chunk->length = size;
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

static _free_chunk_t **get_small_bucket(const size_t size)
{
	size_t i = 0;
	_free_chunk_t **chunk;

	while(i < _SMALL_BUCKETS_COUNT
		&& (((size_t) 1 << i) < size || !_small_buckets[i]))
		++i;
	if(i < _SMALL_BUCKETS_COUNT)
		chunk = &_small_buckets[i];
	else
		chunk = &_small_remaining;
	return (*chunk ? chunk : NULL);
}

// TODO Handle medium
void _bucket_link(_free_chunk_t *chunk)
{
	_free_chunk_t **bucket;

	if(!(bucket = get_small_bucket(chunk->hdr.length)))
	{
		// TODO Error?
		return;
	}
	chunk->next_free = *bucket;
	*bucket = chunk;
}

// TODO Handle medium
void _bucket_unlink(_free_chunk_t *chunk)
{
	_free_chunk_t **bucket;

	if(!(bucket = get_small_bucket(chunk->hdr.length)))
		return;
	if(chunk == *bucket)
		*bucket = chunk->next_free;
}

static void _split_chunk(_free_chunk_t *chunk, const size_t size)
{
	size_t l;
	_free_chunk_t *new_chunk;

	l = chunk->hdr.length;
	chunk->hdr.length = size;
	chunk->hdr.used = 1;
	if(chunk->hdr.length <= size + sizeof(_chunk_hdr_t))
		return;
#ifdef _MALLOC_CHUNK_MAGIC
	if(chunk->hdr.magic != _MALLOC_CHUNK_MAGIC)
	{
		dprintf(STDERR_FILENO, "abort: %s(): corrupted block\n", __func__);
		abort();
	}
#endif
	new_chunk = (_free_chunk_t *) (((_used_chunk_t *) chunk)->data + size);
	if((new_chunk->hdr.prev = (_chunk_hdr_t *) chunk))
		new_chunk->hdr.prev->next = (_chunk_hdr_t *) new_chunk;
	if((new_chunk->hdr.next = (_chunk_hdr_t *) chunk->hdr.next))
		new_chunk->hdr.next->prev = (_chunk_hdr_t *) new_chunk;
	new_chunk->hdr.length = l - (size + sizeof(_chunk_hdr_t));
	new_chunk->hdr.used = 0;
#ifdef _MALLOC_CHUNK_MAGIC
	new_chunk->hdr.magic = _MALLOC_CHUNK_MAGIC;
#endif
	_bucket_link(new_chunk);
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
		_bucket_unlink(chunk);
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

	if(!(b = _alloc_block(size)))
		return NULL;
	_bin_link(&_large_bin, b);
	b->first_chunk->used = 1;
	return ((_used_chunk_t *) b->first_chunk)->data;
}
