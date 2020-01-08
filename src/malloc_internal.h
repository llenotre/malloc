#ifndef _MALLOC_INTERNAL_H
# define _MALLOC_INTERNAL_H

# include <string.h>

# define _FIRST_SMALL_BUCKET_SIZE	8
# define _SMALL_BUCKETS_COUNT		6

# define _SMALL_BIN_MAX\
	(_FIRST_SMALL_BUCKET_SIZE << (_SMALL_BUCKETS_COUNT - 1))
# define _MEDIUM_BIN_MAX	262144

# define _SMALL_BLOCK_PAGES		2
# define _MEDIUM_BLOCK_PAGES	4

# define _MALLOC_CHUNK_MAGIC\
	(0x5ea310c36f405b33 & (sizeof(long) == 8 ? ~0 : 0xffffffff))

# define CEIL_DIVISION(n0, n1)\
	((n0) % (n1) == 0 ? (n0) / (n1) : (n0) / (n1) + 1)
# define OFFSET_OF(type, field)	((uintptr_t) &((type *) 0)->field)

# define GET_CHUNK(ptr)	((ptr) - OFFSET_OF(_used_chunk_t, data))

typedef struct _chunk_hdr
{
	struct _chunk_hdr *prev, *next;

	size_t length;
	char used;
# ifdef _MALLOC_CHUNK_MAGIC
	long magic;
# endif
} _chunk_hdr_t;

typedef struct _used_chunk
{
	_chunk_hdr_t hdr;
	char data[0];
} _used_chunk_t;

typedef struct _free_chunk
{
	_chunk_hdr_t hdr;
	struct _free_chunk *next_free;
} _free_chunk_t;

typedef struct _block
{
	struct _block *prev, *next;

	size_t pages;
	_chunk_hdr_t first_chunk[1];
} _block_t;

void *_alloc_pages(size_t n);
void _free_pages(void *addr, size_t n);

_block_t *_alloc_block(const size_t pages);
void _free_block(_block_t *b);

void _bucket_link(_free_chunk_t *chunk);
void _bucket_unlink(_free_chunk_t *chunk);

void *_small_alloc(size_t size);
void *_medium_alloc(size_t size);
void *_large_alloc(size_t size);

#endif
