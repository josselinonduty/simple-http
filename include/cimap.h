#ifndef CIMAP_H
#define CIMAP_H

#include <stdbool.h>
#include <stddef.h>

typedef struct cimap_entry_t {
    char* key;
    char* value;
    struct cimap_entry_t* next;
} cimap_entry_t;

typedef struct cimap_t {
    cimap_entry_t** buckets;
    size_t bucket_count;
    size_t count;
    bool case_sensitivity;
} cimap_t;

cimap_t* cimap_create(size_t initial_size, bool case_sensitive);
int cimap_free(cimap_t* map);
int cimap_set(cimap_t* map, const char* key, const char* value);
const char* cimap_get(const cimap_t* map, const char* key);
int cimap_remove(cimap_t* map, const char* key);
int cimap_contains(const cimap_t* map, const char* key);
size_t cimap_size(const cimap_t* map);
int cimap_clear(cimap_t* map);
int cimap_resize(cimap_t* map, size_t new_size);

typedef struct cimap_iterator_t {
    const cimap_t* map;
    size_t bucket_index;
    cimap_entry_t* current;
} cimap_iterator_t;

cimap_iterator_t *cimap_iterator(const cimap_t* map);
int cimap_next(cimap_iterator_t* iterator, const char** key, const char** value);
void cimap_iterator_free(cimap_iterator_t* iterator);

#endif