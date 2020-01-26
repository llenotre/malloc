#include "malloc.h"
#include "malloc_internal.h"

#include <stdlib.h>
#include <unistd.h>

/*
 * Frees the given memory chunk.
 * Does nothing if `ptr` is `NULL`.
 *
 * The function shall make the current process abort
 * if the given `ptr` is invalid.
 */
void free(void *ptr)
{
	_chunk_hdr_t *c;

	if(unlikely(!ptr))
		return;
	c = GET_CHUNK(ptr);
	_chunk_assert(c);
	c->used = 0;
	_bucket_link((_free_chunk_t *) c);
	if(c->next && !c->next->used)
		_merge_chunks(c);
	if(c->prev && !c->prev->used)
	{
		c = c->prev;
		_merge_chunks(c);
	}
	if(c->prev || c->next)
		return;
	_bucket_unlink((_free_chunk_t *) c);
	_free_block(c->block);
}
