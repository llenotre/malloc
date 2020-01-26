#include "malloc.h"

#include <errno.h>

/*
 * Calls `malloc` with size `nmemb * size`.
 * If the multiplication overflows, `malloc` isn't called and `NULL` is
 * returned.
 */
__attribute__((malloc))
void *calloc(const size_t nmemb, const size_t size)
{
	size_t n;
	void *ptr;

	if(nmemb == 0 || size == 0)
		return NULL;
	if((n = nmemb * size) / size != nmemb)
	{
		errno = ENOMEM;
		return NULL;
	}
	if((ptr = malloc(n)))
		bzero(ptr, size);
	return ptr;
}
