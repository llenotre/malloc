#ifndef _MALLOC_H
# define _MALLOC_H

# include <string.h>

# define _SMALL_BIN_MAX		256
# define _MEDIUM_BIN_MAX	262144

void *malloc(size_t size);
void free(void *ptr);
void *calloc(size_t nmemb, size_t size);
void *realloc(void *ptr, size_t size);
void *reallocarray(void *ptr, size_t nmemb, size_t size);

#endif
