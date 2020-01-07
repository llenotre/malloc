#include "malloc.h"
#include "malloc_internal.h"

#ifdef _MALLOC_DEBUG
extern _block_t *_small_bin;
extern _block_t *_medium_bin;
extern _block_t *_large_bin;

static size_t debug_print(const char *str, _block_t *b)
{
	_chunk_t *c;
	size_t total = 0;

	while(b)
	{
		printf("%s: %p\n", str, b);
		c = b->first_chunk;
		while(c)
		{
			printf("%p - %p: %zu bytes\n",
				c->data, c->data + c->length, c->length);
			total += c->length;
			c = c->next;
		}
		b = b->next;
	}
	return total;
}

void _debug_show_alloc(void)
{
	size_t total = 0;

	total += debug_print("SMALL", _small_bin);
	total += debug_print("MEDIUM", _medium_bin);
	total += debug_print("LARGE", _large_bin);
	printf("Total: %zu bytes\n", total);
}
#endif