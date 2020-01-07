#include "malloc_internal.h"

#include <sys/mman.h>
#include <unistd.h>

static _block_t *small_bin = NULL;
static _block_t *medium_bin = NULL;
static _block_t *large_bin = NULL;

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

static _block_t *_alloc_block(const size_t pages)
{
	_block_t *b;

	if(pages == 0 || !(b = _alloc_pages(pages)))
		return NULL;
	bzero(b, sizeof(_block_t));
	b->pages = pages;
	b->first_chunk->length = pages * _get_page_size() - sizeof(_block_t);
#ifdef _MALLOC_CHUNK_MAGIC
	b->first_chunk->magic = _MALLOC_CHUNK_MAGIC;
#endif
	return b;
}

void *_small_alloc(const size_t size)
{
	(void) size;
	(void) small_bin;
	// TODO
	return NULL;
}

void *_medium_alloc(const size_t size)
{
	(void) size;
	(void) medium_bin;
	// TODO
	return NULL;
}

void *_large_alloc(const size_t size)
{
	_block_t *b;

	if(!(b = _alloc_block(CEIL_DIVISION(size, _get_page_size()))))
		return NULL;
	b->next = large_bin;
	large_bin = b;
	b->first_chunk->used = 1;
	return b->first_chunk->data;
}
