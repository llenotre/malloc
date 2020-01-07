#include "malloc_internal.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void free(void *ptr)
{
	_chunk_t *c;
	_block_t *b;

	if(!ptr)
		return;
	c = ptr - sizeof(_chunk_t);
#ifdef _MALLOC_CHUNK_MAGIC
	if(c->magic != _MALLOC_CHUNK_MAGIC)
	{
		dprintf(STDERR_FILENO, "free(): corrupted chunk\n");
		abort();
	}
#endif
	if(!c->used)
	{
		dprintf(STDERR_FILENO, "free(): invalid free\n");
		abort();
	}
	if(!c->prev && !c->next)
	{
		b = (void *) ((void *) c - (void *) ((_block_t *) 0)->first_chunk);
		_free_block(b);
		return;
	}
	c->used = 0;
	if(c->next && !c->next->used)
	{
		if((c->next->prev = c->prev))
			c->next->prev->next = c->next;
		c->next->length += sizeof(_chunk_t) + c->length;
	}
	if(c->prev && !c->prev->used)
	{
		if((c->prev->next = c->next))
			c->prev->next->prev = c->prev;
		c->prev->length += sizeof(_chunk_t) + c->length;
	}
}
