#include <stdlib.h>

#include "multiset.h"

multiset_error char_multiset_create(char_multiset_t *set)
{
	set->memsize = 16;
	set->size = 0;

	set->set = (char *)malloc(sizeof(char) * set->memsize);
	if (NULL == set->set)
		return MULTISET_MEMORY_ERROR;

	return MULTISET_OK;
}

int char_multiset_has(char_multiset_t *set, const char c)
{
	for (int i = 0; i < set->size; i++) {
		if (set->set[i] == c)
			return 1;
	}

	return 0;
}

multiset_error char_multiset_put(char_multiset_t *set, const char c)
{
	if (char_multiset_has(set, c))
		return MULTISET_DUPLICATE_ERROR;

	if (set->size == set->memsize) {
		set->memsize *= 2;
		set->set =
		    (char *)realloc(set->set, sizeof(char) * set->memsize);
		if (NULL == set->set)
			return MULTISET_MEMORY_ERROR;
	}

	set->set[set->size++] = c;

	return MULTISET_OK;
}

int char_multiset_get_count(char_multiset_t *set, const char c)
{
	int count = 0;
	for (int i = 0; i < set->size; i++) {
		if (set->set[i] == c)
			count++;
	}

	return count;
}

void char_multiset_destroy(char_multiset_t *set)
{
	free(set->set);
	set->set = NULL;
	set->size = 0;
	set->memsize = 0;
}
