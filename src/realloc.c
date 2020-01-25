#include "malloc.h"
#include "malloc_internal.h"

/*
 * If `ptr` is `NULL` and `size` is not zero, the function is equivalent to
 * `malloc`. If `size` is zero and `ptr` is not NULL, the function is equivalent
 * to free.
 *
 * Else, the function tries to increase the given chunk of memory to the given
 * new size. If the size of the chunk cannot be increased, a new chunk will be
 * allocated, the data from the old chunk will be copied to the new one and
 * the old one will be freed.
 */
void *realloc(void *ptr, const size_t size)
{
	_chunk_hdr_t *c;
	void *p;

	if(!ptr)
		return malloc(size);
	if(size == 0)
	{
		free(ptr);
		return NULL;
	}
	c = GET_CHUNK(ptr);
	_chunk_assert(c);
	if(size <= c->length)
		return ptr; // TODO Shrink chunk?
	if(c->next && !c->next->used)
	{
		// TODO Check if next chunk is large enough
		// TODO Shrink/eat next chunk (and unlink it)
		return ptr;
	}
	if(!(p = malloc(size)))
		return NULL;
	memcpy(p, ptr, MIN(c->length, size));
	free(ptr);
	return p;
}
