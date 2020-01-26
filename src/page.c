#include "malloc_internal.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

// TODO Do not use printf/dprintf for errors printing

/*
 * This file handles pages allocation and freeing.
 *
 * The process asks the kernel for pages using `mmap`
 * and frees them using `munmap`.
 *
 * The size of a memory page is retrieved using `sysconf(_SC_PAGE_SIZE)`.
 */

/*
 * Returns the size in bytes of a page of memory on the current system.
 */
size_t _get_page_size(void)
{
	static size_t page_size = 0;

	if(likely(page_size == 0))
	{
		page_size = sysconf(_SC_PAGE_SIZE);
#ifdef _MALLOC_DEBUG
		dprintf(STDERR_FILENO, "malloc: page size: %zu bytes\n", page_size);
		_debug_print_malloc_info();
#endif
	}
	if(unlikely(page_size == 0))
	{
		dprintf(STDERR_FILENO, "abort: _SC_PAGE_SIZE == 0\n");
		abort();
	}
	return page_size;
}

/*
 * Asks the kernel for `n` pages and returns the pointer to the beginning
 * of the allocated region of memory.
 */
__attribute__((malloc))
void *_alloc_pages(const size_t n)
{
	return mmap(NULL, n * _get_page_size(),
		PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
}

/*
 * Frees the given region of `n` memory pages starting at `addr`.
 */
void _free_pages(void *addr, const size_t n)
{
	munmap(addr, n * _get_page_size());
}
