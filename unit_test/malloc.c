#include <tester_lib.h>
#include "../src/malloc.h"

#define ALIGN		16
#define ALLOC_COUNT	100

static void *malloc_(const size_t n)
{
	void *p;

	p = malloc(n);
	OIL_ASSERT_ALIGN(p, ALIGN);
	return p;
}

#define malloc(n)	malloc_(n)

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

// TODO rm
#undef ALLOC_COUNT
#define ALLOC_COUNT 2

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
		if(i % 2 == 0)
			free(arr[ALLOC_COUNT - i - 1]);
		else
			free(arr[i]);
		++i;
	}
	OIL_PASS();
}

void oil_prepare_tests(void)
{
	OIL_TEST_REGISTER(null_test);

	OIL_TEST_REGISTER(linear_nofree_test);
	OIL_TEST_REGISTER(linear_free_test);
	OIL_TEST_REGISTER(linear_list_free_test);
	OIL_TEST_REGISTER(linear_large_nofree_test);
	OIL_TEST_REGISTER(linear_large_free_test);

	OIL_TEST_REGISTER(stack_free_test);
	OIL_TEST_REGISTER(intertwined_free_test);
}
