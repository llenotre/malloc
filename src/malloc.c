#include "malloc.h"
#include "malloc_internal.h"

/*
 * Allocates a chunk of memory of size `size` and returns a pointer to the
 * beginning. Pointer is suitably aligned to fit any built-in type.
 *
 * Memory chunk is cleared before being returned.
 * If a size of zero is given, `NULL` is returned.
 */
__attribute__((malloc))
void *malloc(const size_t size)
{
	void *ptr;

	if(size == 0)
		return NULL;
	if(size < _SMALL_BIN_MAX)
		ptr = _small_alloc(size);
	else if(size < _MEDIUM_BIN_MAX)
		ptr = _medium_alloc(size);
	else
		ptr = _large_alloc(size);
	if(ptr)
		bzero(ptr, size);
	return ptr;
}
