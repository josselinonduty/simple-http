#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>

#include "cimap.h"
#include "utils.h"

#define CIMAP_LOAD_FACTOR 0.75

static size_t hash(const char *str, size_t bucket_count, bool case_sensitivity)
{
	size_t hash = 5381;
	int c;
	while ((c = *str++)) {
		hash =
		    ((hash << 5) + hash) + (case_sensitivity ? c : tolower(c));
	}
	return hash % bucket_count;
}

cimap_t *cimap_create(size_t initial_size, bool case_sensitivity)
{
	cimap_t *map = malloc(sizeof(cimap_t));
	if (!map)
		return NULL;

	map->bucket_count = initial_size;
	map->count = 0;
	map->case_sensitivity = case_sensitivity;
	map->buckets = calloc(initial_size, sizeof(cimap_entry_t *));
	if (!map->buckets) {
		free(map);
		return NULL;
	}
	return map;
}

int cimap_free(cimap_t *map)
{
	if (!map)
		return -1;

	for (size_t i = 0; i < map->bucket_count; i++) {
		cimap_entry_t *entry = map->buckets[i];
		while (entry) {
			cimap_entry_t *next = entry->next;
			free(entry->key);
			free(entry->value);
			free(entry);
			entry = next;
		}
	}
	free(map->buckets);
	free(map);
	return 0;
}

int cimap_set(cimap_t *map, const char *key, const char *value)
{
	if (!map || !key || !value)
		return -1;

	size_t index = hash(key, map->bucket_count, map->case_sensitivity);
	cimap_entry_t *entry = map->buckets[index];

	while (entry) {
		if (str_compare(entry->key, key, map->case_sensitivity) == 0) {
			char *new_value = strdup(value);
			if (!new_value)
				return -1;
			free(entry->value);
			entry->value = new_value;
			return 0;
		}
		entry = entry->next;
	}

	cimap_entry_t *new_entry = malloc(sizeof(cimap_entry_t));
	if (!new_entry)
		return -1;

	new_entry->key = strdup(key);
	new_entry->value = strdup(value);
	if (!new_entry->key || !new_entry->value) {
		free(new_entry->key);
		free(new_entry->value);
		free(new_entry);
		return -1;
	}

	new_entry->next = map->buckets[index];
	map->buckets[index] = new_entry;
	map->count++;

	if ((float)map->count / map->bucket_count > CIMAP_LOAD_FACTOR) {
		return cimap_resize(map, map->bucket_count * 2);
	}

	return 0;
}

const char *cimap_get(const cimap_t *map, const char *key)
{
	if (!map || !key)
		return NULL;

	size_t index = hash(key, map->bucket_count, map->case_sensitivity);
	cimap_entry_t *entry = map->buckets[index];

	while (entry) {
		if (str_compare(entry->key, key, map->case_sensitivity) == 0) {
			return entry->value;
		}
		entry = entry->next;
	}

	return NULL;
}

int cimap_remove(cimap_t *map, const char *key)
{
	if (!map || !key)
		return -1;

	size_t index = hash(key, map->bucket_count, map->case_sensitivity);
	cimap_entry_t *entry = map->buckets[index];
	cimap_entry_t *prev = NULL;

	while (entry) {
		if (str_compare(entry->key, key, map->case_sensitivity) == 0) {
			if (prev) {
				prev->next = entry->next;
			} else {
				map->buckets[index] = entry->next;
			}
			free(entry->key);
			free(entry->value);
			free(entry);
			map->count--;
			return 0;
		}
		prev = entry;
		entry = entry->next;
	}

	return 1;		// Key not found
}

int cimap_contains(const cimap_t *map, const char *key)
{
	return cimap_get(map, key) != NULL ? 0 : 1;
}

size_t cimap_size(const cimap_t *map)
{
	return map ? map->count : 0;
}

int cimap_clear(cimap_t *map)
{
	if (!map)
		return -1;

	for (size_t i = 0; i < map->bucket_count; i++) {
		cimap_entry_t *entry = map->buckets[i];
		while (entry) {
			cimap_entry_t *next = entry->next;
			free(entry->key);
			free(entry->value);
			free(entry);
			entry = next;
		}
		map->buckets[i] = NULL;
	}
	map->count = 0;
	return 0;
}

int cimap_resize(cimap_t *map, size_t new_size)
{
	if (!map || new_size < map->count)
		return -1;

	cimap_entry_t **new_buckets = calloc(new_size, sizeof(cimap_entry_t *));
	if (!new_buckets)
		return -1;

	for (size_t i = 0; i < map->bucket_count; i++) {
		cimap_entry_t *entry = map->buckets[i];
		while (entry) {
			cimap_entry_t *next = entry->next;
			size_t new_index =
			    hash(entry->key, new_size, map->case_sensitivity);
			entry->next = new_buckets[new_index];
			new_buckets[new_index] = entry;
			entry = next;
		}
	}

	free(map->buckets);
	map->buckets = new_buckets;
	map->bucket_count = new_size;
	return 0;
}

cimap_iterator_t *cimap_iterator(const cimap_t *map)
{
	if (!map)
		return NULL;

	cimap_iterator_t *iterator = malloc(sizeof(cimap_iterator_t));
	if (!iterator)
		return NULL;

	iterator->map = map;
	iterator->bucket_index = 0;
	iterator->current = NULL;

	return iterator;
}

int cimap_next(cimap_iterator_t *iterator, const char **key, const char **value)
{
	if (!iterator || !key || !value)
		return -1;

	while (iterator->bucket_index < iterator->map->bucket_count) {
		if (iterator->current) {
			*key = iterator->current->key;
			*value = iterator->current->value;
			iterator->current = iterator->current->next;
			return 0;
		}

		iterator->bucket_index++;
		if (iterator->bucket_index < iterator->map->bucket_count) {
			iterator->current =
			    iterator->map->buckets[iterator->bucket_index];
		}
	}

	return 1;		// No more elements
}

void cimap_iterator_free(cimap_iterator_t *iterator)
{
	free(iterator);
}
