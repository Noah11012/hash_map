#include "map.h"

#include <stdlib.h>
#include <string.h>

typedef struct Bucket {
    char const *key;
    Any value;
    bool in_use;
    struct Bucket *collision_head;

    /* This is only used when this bucket is a "collision node" */
    struct Bucket *next;
} Bucket;

Bucket *bucket_node_new(char const *key) {
    Bucket *bucket = malloc(sizeof *bucket);
    if (!bucket)
        return NULLPTR;

    bucket->key = key;
    bucket->value = NULLPTR;
    bucket->in_use = false;
    bucket->next = NULLPTR;
    
    return bucket;
}

struct Map {
    Bucket *bucket_list;
    int bucket_list_count;
    int bucket_list_capacity;
    int value_size;
    float load_factor;

    /* Only used when iterating */
    int iter_count;
    int iter_position;
    bool in_collision_nodes;
    Bucket *current_collision_node;
};

#define string_equal(string1, string2) (!strcmp(string1, string2))

/* Laughably bad hash function */
static size_t hash_function(char const *key) {
    size_t result = 0;
    for (int i = 0; key[i]; i++) {
        result += key[i];
        /* I have no idea what this does. Just doing random stuff */
        result ^= -(1 - (key[i] - 2));
        result ^= -(1 - (key[i] - 4));
        result ^= -(1 - (key[i] - 8));
        result ^= -(1 - (key[i] - 16));
        result ^= -(1 - (key[i] - 32));
    }

    return result;
}

#define INITIAL_CAPACITY 32

bool map_init_(Map **mapp, int value_size) {
    Map *map = malloc(sizeof *map);
    map->bucket_list = calloc(INITIAL_CAPACITY, sizeof(Bucket));
    if (!map->bucket_list)
        return false;
    map->bucket_list_count = 0;
    map->bucket_list_capacity = INITIAL_CAPACITY;
    map->value_size = value_size;
    map->load_factor = 0.0f;

    *mapp = map;

    return true;
}

void map_delete(Map *map) {
    for (int i = 0; i < map->bucket_list_capacity; i++) {
        Bucket *bucket = map->bucket_list + i;
        if (bucket->value) {
            free(bucket->value);
        }

        Bucket *collision_node_iter = bucket->collision_head;
        while (collision_node_iter) {
            Bucket *next_collision_node = collision_node_iter->next;
            free(collision_node_iter->value);
            free(collision_node_iter);

            collision_node_iter = next_collision_node;
        }
    }

    free(map->bucket_list);
    free(map);
}

void map_insert(Map *map, char const *key, Any value) {
    if (map->load_factor >= 1.0f) {
        Bucket *new_bucket_list = realloc(map->bucket_list, map->bucket_list_capacity);
        if (!new_bucket_list)
            return;
        map->bucket_list_capacity *= 1.5;
        map->bucket_list = new_bucket_list;
        map->load_factor = 0.0f;
    }

    size_t index = hash_function(key) % map->bucket_list_capacity;
    Bucket *bucket = map->bucket_list + index;
    if (!bucket->in_use) {
        bucket->key = key;
        bucket->value = malloc(map->value_size);
        memcpy(bucket->value, value, map->value_size);
        bucket->in_use = true;
        map->bucket_list_count++;
    } else {
        if (string_equal(key, bucket->key)) {
            memcpy(bucket->value, value, map->value_size);
            return;
        }

        if (!bucket->collision_head)
            bucket->collision_head = bucket_node_new(key);
        if (!bucket->collision_head)
            return;

        Bucket *iter = bucket->collision_head;
        while (1) {
            if (!iter->next)
                break;

            iter = iter->next;
        }

        Bucket *bucket_to_use = iter;
        if (bucket_to_use->in_use) {
            bucket_to_use = bucket_node_new(key);
            iter->next = bucket_to_use;
        }

        bucket_to_use->value = malloc(map->value_size);
        memcpy(bucket_to_use, value, map->value_size);
        bucket_to_use->in_use = true;
        map->bucket_list_count++;
        map->load_factor += 0.1f;
    }
}

Any map_get(Map *map, char const *key) {
    size_t index = hash_function(key) % map->bucket_list_capacity;
    Bucket *bucket = map->bucket_list + index;
    if (bucket->in_use) {
        if (string_equal(key, bucket->key))
            return bucket->value;
    }

    Any value = ANY_NULL;
    if (bucket->collision_head) {
        Bucket *iter = bucket->collision_head;
        while (1) {
            if (!iter)
                break;

            if (string_equal(key, iter->key)) {
                value = iter->value;
                break;
            }

            iter = iter->next;
        }
    }

    return value;
}

Map_Iter map_iter_begin(Map *map) {
    map->iter_count = 0;
    map->iter_position = 0;
    map->in_collision_nodes = false;
    return map_iter_next(map);
}

Map_Iter map_iter_next(Map *map) {
    Map_Iter result = { NULLPTR, ANY_NULL };

    if (map->iter_count < map->bucket_list_count) {
        while (map->iter_position < map->bucket_list_capacity) {
            if (map->in_collision_nodes) {
                if (map->current_collision_node->in_use) {
                    result.key = map->current_collision_node->key;
                    result.value = map->current_collision_node->value;
                    map->iter_count++;
                }

                map->current_collision_node = map->current_collision_node->next;
                if (!map->current_collision_node)
                    map->in_collision_nodes = false;
                break;
            }

            Bucket *bucket = map->bucket_list + map->iter_position++;
            if (bucket->in_use) {
                result.key = bucket->key;
                result.value = bucket->value;
                map->iter_count++;
            }

            if (bucket->collision_head) {
                map->in_collision_nodes = true;
                map->current_collision_node = bucket->collision_head;
            }

            if (map_iter_is_valid(result))
                break;
        }
    }

    return result;
}

bool map_iter_is_valid(Map_Iter iter) {
    return iter.key != NULLPTR && iter.value != ANY_NULL;
}

#include <stdio.h>
void map_pretty_print(Map *map) {
    for (int i = 0; i < map->bucket_list_capacity; i++) {
        Bucket *bucket = map->bucket_list + i;
        for (Bucket *iter = bucket->collision_head; iter; iter = iter->next)
            printf("\t{ key: %s, value: %p, in_use: %s }\n",
                   bucket->key,
                   bucket->value,
                   bucket->in_use ? "true" : "false");

        printf("{ key: %s, value: %p, in_use: %s }\n",
               bucket->key,
               bucket->value,
               bucket->in_use ? "true" : "false");
    }
}
