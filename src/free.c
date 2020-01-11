#include "malloc_internal.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void free(void *ptr)
{
	_chunk_hdr_t *c;
	_block_t *b;

	if(!ptr)
		return;
	c = GET_CHUNK(ptr);
#ifdef _MALLOC_CHUNK_MAGIC
	if(c->magic != _MALLOC_CHUNK_MAGIC)
	{
		dprintf(STDERR_FILENO, "abort: %s(): corrupted chunk\n", __func__);
		abort();
	}
#endif
	if(!c->used)
	{
		dprintf(STDERR_FILENO, "abort: %s(): invalid free\n", __func__);
		abort();
	}
	c->used = 0;
	if(c->next && !c->next->used)
	{
		if((c->next->prev = c->prev))
			c->next->prev->next = c->next;
		c->length += sizeof(_chunk_hdr_t) + c->next->length;
	}
	if(c->prev && !c->prev->used)
	{
		if((c->prev->next = c->next))
			c->prev->next->prev = c->prev;
		c->prev->length += sizeof(_chunk_hdr_t) + c->length;
	}
	_bucket_link((_free_chunk_t *) c);
	if(c->next && c->next->used)
		return;
	while(c->prev)
	{
		if(c->used)
			return;
		c = c->prev;
	}
	_bucket_unlink((_free_chunk_t *) c);
	b = (void *) c - OFFSET_OF(_block_t, first_chunk);
	_free_block(b);
}
