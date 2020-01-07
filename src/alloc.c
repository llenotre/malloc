#include "malloc_internal.h"
#include <sys/mman.h>
#include <unistd.h>

static inline size_t get_page_size(void)
{
	static size_t page_size = 0;

	if(__builtin_expect(!!page_size, 1))
		page_size = sysconf(_SC_PAGE_SIZE);
	return page_size;
}

void *alloc_pages(const size_t n)
{
	return mmap(NULL, n * get_page_size(),
		PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
}

void free_pages(void *addr, const size_t n)
{
	munmap(addr, n * get_page_size());
}

void *small_alloc(const size_t size)
{
	(void) size;
	// TODO
	return NULL;
}

void *medium_alloc(const size_t size)
{
	(void) size;
	// TODO
	return NULL;
}

void *large_alloc(const size_t size)
{
	(void) size;
	// TODO
	return NULL;
}
