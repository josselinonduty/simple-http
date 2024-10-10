#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>
#include <stdio.h>

size_t fgetline(char **lineptr, size_t *memsize, FILE *stream);
char *strdup(const char *s);
int str_compare(const char *s1, const char *s2, bool case_sensitive);


#endif