#include "malloc.h"
#include "malloc_internal.h"

#include <errno.h>

/*
 * Calls `realloc` with size `nmemb * size`.
 * If the multiplication overflows, `realloc` isn't called and `NULL` is
 * returned.
 */
void *reallocarray(void *ptr, const size_t nmemb, const size_t size)
{
	size_t n;

	if(nmemb == 0 || size == 0)
		return NULL;
	if((n = nmemb * size) / size != nmemb)
	{
		errno = ENOMEM;
		return NULL;
	}
	return realloc(ptr, n);
}
