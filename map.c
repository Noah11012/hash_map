#include "map.h"

#include <stdlib.h>
#include <string.h>

typedef struct Bucket {
    char const *key;
    Any value;
    bool in_use;
    struct Bucket *collision_head;

    /* These are only used when this bucket is a "collision node" */
    struct Bucket *next;
    struct Bucket *last_collision_node;
} Bucket;

Bucket *bucket_node_new(void) {
    Bucket *bucket = malloc(sizeof *bucket);
    if (!bucket)
        return NULLPTR;

    bucket->key = NULLPTR;
    bucket->value = NULLPTR;
    bucket->in_use = false;
    bucket->collision_head = NULLPTR;
    bucket->next = NULLPTR;
    
    return bucket;
}

struct Map {
    Bucket *bucket_list;
    int bucket_list_count;
    int bucket_list_capacity;
    int value_size;
    float load_factor;
    bool locked;

    /* Only used when iterating */
    Map_Iter iter;
    int iter_position;
    Bucket *current_collision_node; /* If not null then we are currently iterating over the collision nodes */
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
    map->locked = false;

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
    if (map->locked)
        return;
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

        Bucket *next_collision_node = bucket_node_new();
        next_collision_node->key = key;
        next_collision_node->value = malloc(map->value_size);
        memcpy(next_collision_node->value, value, map->value_size);
        next_collision_node->in_use = true;
        if (bucket->collision_head)
            bucket->last_collision_node->next = next_collision_node;
        else
            bucket->collision_head = next_collision_node;
        bucket->last_collision_node = next_collision_node;
        map->load_factor += 0.1f;
        map->bucket_list_count++;
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

void map_remove(Map *map, char const *key) {
    size_t index = hash_function(key) % map->bucket_list_capacity;
    Bucket *bucket = map->bucket_list + index;
    Bucket *bucket_to_remove = NULLPTR;
    bool is_collision_node = false;
    if (bucket->in_use) {
        if (string_equal(key, bucket->key))
            bucket_to_remove = bucket;

        if (!bucket_to_remove) {
            for (Bucket *iter = bucket->collision_head; iter; iter = iter->next) {
                if (iter->in_use) {
                    if (string_equal(key, iter->key)) {
                        bucket_to_remove = iter;
                        is_collision_node = true;
                        break;
                    }
                }
            }
        }

        bucket_to_remove->key = NULLPTR;
        free(bucket_to_remove->value);
        bucket_to_remove->value = NULLPTR;
        bucket_to_remove->in_use = false;
    }
}

Map_Iter *map_iter_begin(Map *map) {
    map->iter_position = 0;
    map->locked = true;
    map->iter.key = NULLPTR;
    map->iter.value = ANY_NULL;
    map->current_collision_node = NULL;
    return map_iter_next(map);
}

Map_Iter *map_iter_next(Map *map) {
    Bucket *bucket_to_return = NULLPTR;
    map->iter.key = NULL;
    map->iter.value = NULL;
    while (map->iter_position < map->bucket_list_capacity) {
        Bucket *bucket = map->bucket_list + map->iter_position;

        if (map->current_collision_node) {
            bucket_to_return = map->current_collision_node;
            map->current_collision_node = map->current_collision_node->next;

            if (!map->current_collision_node) {
                map->current_collision_node = NULLPTR;
                map->iter_position++;
            }
        } else {
            if (bucket->in_use)
                bucket_to_return = bucket;

            if (bucket->collision_head) {
                map->current_collision_node = bucket->collision_head;
            }

            if (!map->current_collision_node)
                map->iter_position++;
        }

        if (bucket_to_return) {
            map->iter.key = bucket_to_return->key;
            map->iter.value = bucket_to_return->value;
            break;
        }
    }

    if (!map_iter_is_valid(map->iter))
        map->locked = false;

    return &map->iter;
}

bool map_iter_is_valid(Map_Iter iter) {
    return iter.key != NULLPTR && iter.value != ANY_NULL;
}

#include <stdio.h>
void map_pretty_print(Map *map) {
    for (int i = 0; i < map->bucket_list_capacity; i++) {
        Bucket *bucket = map->bucket_list + i;
        printf("{ key: %s, value: %p, in_use: %s }\n",
               bucket->key,
               bucket->value,
               bucket->in_use ? "true" : "false");

        for (Bucket *iter = bucket->collision_head; iter; iter = iter->next) {
            printf("\t⤷ { key: %s, value: %p, in_use: %s }\n",
                   iter->key,
                   iter->value,
                   iter->in_use ? "true" : "false");
        }
    }
}
