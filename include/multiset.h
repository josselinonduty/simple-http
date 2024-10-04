#ifndef MULTISET_H
#define MULTISET_H

/**
 * Multiset data structure
 * 
 * This is a simple multiset data structure that stores characters.
 * It is not meant to be efficient/optimized, but rather to be simple.
 * It should be used on small sets of data only.
 */

typedef enum multiset_error {
    MULTISET_OK = 0,
    MULTISET_MEMORY_ERROR = -1,
    MULTISET_DUPLICATE_ERROR = -2,
} multiset_error;

typedef struct char_multiset_t {
    int memsize;
    char *set;
    int size;
} char_multiset_t;

multiset_error char_multiset_create(char_multiset_t *set);
int char_multiset_has(char_multiset_t *set, const char c);
multiset_error char_multiset_put(char_multiset_t *set, const char c);
int char_multiset_get_count(char_multiset_t *set, const char c);
void char_multiset_destroy(char_multiset_t *set);

#endif