#include <stdbool.h>
#include <stddef.h>

typedef void* Any;
typedef struct Map Map;

typedef struct {
    char const *key;
    Any value;
    bool is_array;
    int count;
} Map_Iter;

#define map_register_new_type(type) \
    typedef struct { \
        char const *key; \
        type *value; \
        bool is_array; \
        int count; \
    } Map_Iter_##type \

#define ANY_NULL ((void *) 0)
#define NULLPTR  ((void *) 0)

#define INT_TO_ANY(number) (&(int){ number })

#define map_iterate(map, variable_name, type) \
    Map_Iter_##type *variable_name = (Map_Iter_##type *)map_iter_begin(map); \
    map_iter_is_valid((Map_Iter *)variable_name); \
    variable_name = (Map_Iter_##type *)map_iter_next(map) \

Map_Iter *map_iter_begin(Map *map);
Map_Iter *map_iter_next(Map *map);
bool map_iter_is_valid(Map_Iter *iter);

bool map_init_(Map **mapp, int value_size);
#define map_init(map, type) map_init_(&map, sizeof(type))
void map_delete(Map *map);
void map_insert(Map *map, char const *key, Any value);
void map_insert_array(Map *map, char const *key, Any value, int count);
Any map_get(Map *map, char const *key);
void map_remove(Map *map, char const *key);
void map_pretty_print(Map *map);
