#include <tester_lib.h>
#include "../src/malloc.h"
#include "../src/malloc_internal.h"

#define ALIGN		16
#define ALLOC_COUNT	100

static void *malloc_(const size_t n)
{
	void *p;

	p = malloc(n);
	OIL_ASSERT_ALIGN(p, ALIGN);
	return p;
}

#ifdef malloc
# define _malloc(n)	malloc_(n)
#else
# define malloc(n)	malloc_(n)
#endif

OIL_TEST(null_test)
{
	OIL_ASSERT(NULL, malloc(0));
}

OIL_TEST(linear_nofree_test)
{
	size_t i = 0;
	void *p;

	while(i < ALLOC_COUNT)
	{
		if(!(p = malloc(i + 1)))
			OIL_FAIL();
		memset(p, 0xff, i + 1);
		++i;
	}
	OIL_PASS();
}

OIL_TEST(linear_free_test)
{
	size_t i = 0;
	void *p;

	while(i < ALLOC_COUNT)
	{
		if(!(p = malloc(i + 1)))
			OIL_FAIL();
		memset(p, 0xff, i + 1);
		free(p);
		++i;
	}
	OIL_PASS();
}

OIL_TEST(linear_list_free_test)
{
	size_t i = 0;
	void *arr[ALLOC_COUNT];

	while(i < ALLOC_COUNT)
	{
		if(!(arr[i] = malloc(i + 1)))
			OIL_FAIL();
		memset(arr[i], 0xff, i + 1);
		++i;
	}
	i = 0;
	while(i < ALLOC_COUNT)
		free(arr[i++]);
	OIL_PASS();
}

// TODO rm
#include <stdio.h>

OIL_TEST(linear_medium_test)
{
	size_t i = 0;
	void *arr[ALLOC_COUNT];

	while(i < ALLOC_COUNT)
	{
		if(!(arr[i] = malloc((i + 1) * 256)))
			OIL_FAIL();
		memset(arr[i], 0xff, (i + 1) * 256);
		++i;
	}
	i = 0;
	while(i < ALLOC_COUNT)
		free(arr[i++]);
	OIL_PASS();
}

OIL_TEST(small_medium_test)
{
	size_t i = 0;
	void *arr[ALLOC_COUNT];

	while(i < ALLOC_COUNT)
	{
		if(!(arr[i] = malloc((i + 1) * 16)))
			OIL_FAIL();
		memset(arr[i], 0xff, (i + 1) * 16);
		++i;
	}
	i = 0;
	while(i < ALLOC_COUNT)
		free(arr[i++]);
	OIL_PASS();
}

OIL_TEST(linear_large_nofree_test)
{
	size_t i = 0;
	void *p;

	while(i < ALLOC_COUNT)
	{
		if(!(p = malloc(512 * 1024)))
			OIL_FAIL();
		memset(p, 0xff, 512 * 1024);
		++i;
	}
	OIL_PASS();
}

OIL_TEST(linear_large_free_test)
{
	size_t i = 0;
	void *arr[ALLOC_COUNT];

	while(i < ALLOC_COUNT)
	{
		if(!(arr[i] = malloc(512 * 1024)))
			OIL_FAIL();
		memset(arr[i], 0xff, 512 * 1024);
		++i;
	}
	i = 0;
	while(i < ALLOC_COUNT)
		free(arr[i++]);
	OIL_PASS();
}

#define ALLOC_SIZE	128
#define STACK_DEPTH	64

static void stack_(const size_t n)
{
	void *p;

	if(!(p = malloc(ALLOC_SIZE)))
		OIL_FAIL();
	memset(p, 0xff, ALLOC_SIZE);
	if(n > 0)
		stack_(n - 1);
	free(p);
}

OIL_TEST(stack_free_test)
{
	stack_(STACK_DEPTH);
	OIL_PASS();
}

OIL_TEST(intertwined_free_test)
{
	size_t i = 0;
	void *arr[ALLOC_COUNT];

	while(i < ALLOC_COUNT)
	{
		if(!(arr[i] = malloc(i + 1)))
			OIL_FAIL();
		memset(arr[i], 0xff, i + 1);
		++i;
	}
	i = 0;
	while(i < ALLOC_COUNT)
	{
		free(arr[i]);
		i += 2;
	}
	i = 1;
	while(i < ALLOC_COUNT)
	{
		free(arr[i]);
		i += 2;
	}
	OIL_PASS();
}

#undef ALLOC_COUNT
#define ALLOC_COUNT 4096

OIL_TEST(small_medium_large_test)
{
	size_t i = 0;
	void *arr[ALLOC_COUNT];

	while(i < ALLOC_COUNT)
	{
		if(!(arr[i] = malloc((i + 1) * 128)))
			OIL_FAIL();
		memset(arr[i], 0xff, (i + 1) * 128);
		++i;
	}
	i = 0;
	while(i < ALLOC_COUNT)
		free(arr[i++]);
	OIL_PASS();
}

void oil_prepare_tests(void)
{
	OIL_TEST_REGISTER(null_test);

	OIL_TEST_REGISTER(linear_nofree_test);
	OIL_TEST_REGISTER(linear_free_test);
	OIL_TEST_REGISTER(linear_list_free_test);

	OIL_TEST_REGISTER(linear_medium_test);
	OIL_TEST_REGISTER(small_medium_test);

	OIL_TEST_REGISTER(linear_large_nofree_test);
	OIL_TEST_REGISTER(linear_large_free_test);

	OIL_TEST_REGISTER(stack_free_test);
	OIL_TEST_REGISTER(intertwined_free_test);

	OIL_TEST_REGISTER(small_medium_large_test);
}
