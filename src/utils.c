#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

size_t fgetline(char **lineptr, size_t *memsize, FILE *stream)
{
	if (NULL != *lineptr) {
		free(*lineptr);
		*lineptr = NULL;
	}

	*memsize = 16;
	*lineptr = malloc(*memsize);
	if (NULL == *lineptr)
		return -1;

	size_t i = 0;
	int c;

	while ((c = fgetc(stream)) != EOF) {
		if (i >= *memsize - 1) {
			*memsize *= 2;
			char *newptr = realloc(*lineptr, *memsize);
			if (NULL == newptr) {
				return -1;
			}
			*lineptr = newptr;
		}

		(*lineptr)[i++] = c;

		if ('\n' == c) {
			break;
		}
	}

	if (0 == i && EOF == c) {
		return 0;
	}

	(*lineptr)[i] = '\0';
	return i;
}

char *strdup(const char *s)
{
	size_t size = strlen(s) + 1;
	char *p = malloc(size);
	if (p) {
		memcpy(p, s, size);
	}
	return p;
}

int str_compare(const char *s1, const char *s2, bool case_sensitive)
{
	if (case_sensitive) {
		return strcmp(s1, s2);
	}
	while (*s1 && *s2) {
		char c1 = tolower(*s1);
		char c2 = tolower(*s2);
		if (c1 != c2) {
			return c1 - c2;
		}
		s1++;
		s2++;
	}
	return *s1 - *s2;
}
