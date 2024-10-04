#include <stdio.h>
#include <stdlib.h>

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
