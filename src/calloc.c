#include "malloc.h"

void *calloc(const size_t nmemb, const size_t size)
{
	size_t n;

	if(nmemb == 0 || size == 0)
		return NULL;
	if((n = nmemb * size) / size != nmemb)
		return NULL;
	return malloc(n);
}
