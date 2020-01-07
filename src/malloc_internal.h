#ifndef _MALLOC_INTERNAL_H
# define _MALLOC_INTERNAL_H

# include <string.h>

# define _SMALL_BIN_MAX		256
# define _MEDIUM_BIN_MAX	262144

void *alloc_pages(size_t n);
void free_pages(void *addr, size_t n);

void *small_alloc(size_t size);
void *medium_alloc(size_t size);
void *large_alloc(size_t size);

#endif
