#include "map.h"

#include <stdlib.h>
#include <string.h>

#define string_equal(string1, string2) (!strncmp(string1, string2, strlen(string1)))

typedef struct Chain_Node Chain_Node;
struct Chain_Node {
    char const *key;
    Any value;
    Chain_Node *next;
};

typedef struct Map_Node Map_Node;
struct Map_Node {
    char const *key;
    Any value;
    int in_use;
    Map_Node *next;
    Chain_Node *chain_head;
};

struct Map {
    Map_Node *head;
    Map_Node *tail;
    int value_size;
    int node_count;
    int used_node_count;
};

static Map_Node *get_node(Map_Node *head, size_t index) {
    Map_Node *iter = head;
    for (size_t i = 0; i < index; i++) {
        iter = iter->next;
    }

    return iter;
}

static Map_Node *map_node_new(void) {
    Map_Node *node = malloc(sizeof *node);
    if (!node)
        return NULLPTR;

    node->key = NULLPTR;
    node->value = ANY_NULL;
    node->in_use = 0;
    node->next = NULLPTR;
    node->chain_head = NULLPTR;

    return node;
}

static Chain_Node *chain_node_new(char const *key, Any value) {
    Chain_Node *node = malloc(sizeof *node);
    if (!node)
        return NULLPTR;

    node->key = key;
    node->value = value;
    node->next = NULLPTR;

    return node;
}

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

static void free_all_map_nodes(Map_Node *head) {
    Map_Node *iter = head;
    while (1) {
        if (!iter)
            break;

        Map_Node *next_node = iter->next;
        free(iter->value);
        free(iter);
        iter = next_node;
    }
}

#define INITIAL_NODE_COUNT 32

Map_Status_Code map_init_(Map **mapp, int value_size) {
    *mapp = malloc(sizeof **mapp);
    Map *map = *mapp;
    map->head = map_node_new();
    if (!map->head)
        return MAP_OOM;

    Map_Status_Code status = MAP_OK;
    Map_Node *iter = map->head;
    Map_Node *prev_iter = NULLPTR;
    for (int i = 1; i < INITIAL_NODE_COUNT; i++) {
        prev_iter = iter;
        iter = iter->next;
        iter = map_node_new();
        prev_iter->next = iter;
        if (!iter) {
            free_all_map_nodes(map->head);
            status = MAP_OOM;
            break;
        }
    }

    map->tail = iter;
    map->tail->next = NULLPTR;
    map->value_size = value_size;
    map->node_count = INITIAL_NODE_COUNT;
    map->used_node_count = 0;

    return status;
}

void map_delete(Map *map) {
    free_all_map_nodes(map->head);
    free(map);
}

Map_Status_Code map_insert(Map *map, char const *key, Any value) {
    size_t index = hash_function(key) % map->node_count;
    Map_Node *node = get_node(map->head, index);
    Map_Status_Code status = MAP_OK;
    if (!node->in_use) {
        node->key = key;
        node->value = malloc(map->value_size);
        memcpy(node->value, value, map->value_size);
        node->in_use = 1;
        map->used_node_count++;
    } else {
        if (string_equal(key, node->key)) {
            memcpy(node->value, value, map->value_size);
        } else {
            if (!node->chain_head) {
                node->chain_head = malloc(sizeof *node->chain_head);
                if (!node->chain_head)
                    status = MAP_OOM;
            }

            Chain_Node *iter = node->chain_head;
            Chain_Node *prev_iter = NULLPTR;
            while (1)  {
                if (!iter)
                    break;

                prev_iter = iter;
                iter = iter->next;
            }

            iter = prev_iter;
            iter->next = chain_node_new(key, value);
            if (!iter->next)
                status = MAP_OOM;
        }
    }

    return status;
}

Map_Status_Code map_get(Map *map, char const *key, Any output_value) {
    Map_Status_Code status = MAP_OK;
    size_t index = hash_function(key) % map->node_count;
    Map_Node *node = get_node(map->head, index);
    if (node->in_use) {
        if (string_equal(key, node->key)) {
            memcpy(output_value, node->value, map->value_size);
        } else {
            status = MAP_NOTFOUND;
            for (Chain_Node *iter = node->chain_head; iter; iter = iter->next) {
                if (string_equal(key, iter->key)) {
                    status = MAP_OK;
                    memcpy(output_value, node->value, map->value_size);
                    break;
                }
            }
        }
    }

    return status;
}

#include <stdio.h>
void map_pretty_print(Map *map) {
    Map_Node *iter = map->head;
    while (1) {
        if (!iter)
            break;

        printf("{ key: %s, value: %p, in_use: %s, next: %p }\n",
               iter->key,
               iter->value,
               (iter->in_use ? "true" : "false"),
               iter->next);

        iter = iter->next;
    }
}
