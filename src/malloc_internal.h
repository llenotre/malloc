#ifndef _MALLOC_INTERNAL_H
# define _MALLOC_INTERNAL_H

# include <string.h>

# define _SMALL_BIN_MAX		256
# define _MEDIUM_BIN_MAX	262144

# define CEIL_DIVISION(n0, n1)\
	((n0) % (n1) == 0 ? (n0) / (n1) : (n0) / (n1) + 1)

// TODO 8 bytes magic number for 64 bits
# define _MALLOC_CHUNK_MAGIC	0xe9c9028c

typedef struct _chunk
{
	struct _chunk *prev, *next;

	size_t length;
	char used;
#ifdef _MALLOC_CHUNK_MAGIC
	long magic;
#endif
	char data[0];
} _chunk_t;

typedef struct _block
{
	struct _block *prev, *next;

	size_t pages;
	_chunk_t first_chunk[1];
} _block_t;

void *_alloc_pages(size_t n);
void _free_pages(void *addr, size_t n);

_block_t *_alloc_block(const size_t pages);
void _free_block(_block_t *b);

void *_small_alloc(size_t size);
void *_medium_alloc(size_t size);
void *_large_alloc(size_t size);

#endif
