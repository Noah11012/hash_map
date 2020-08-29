typedef void* Any;
typedef struct Map Map;

#define ANY_NULL ((void *) 0)
#define NULLPTR  ((void *) 0)

#include <stddef.h>
static size_t map_int_to_any_value__;
#define INT_TO_ANY(number) (map_int_to_any_value__ = number, &map_int_to_any_value__)

typedef enum {
    MAP_OK,         // Everything is okay
    MAP_OOM,        // Out of memory
    MAP_NOTFOUND,   // Value is not found
} Map_Status_Code;

Map_Status_Code map_init_(Map **map, int value_size);
#define map_init(map, type) map_init_(&map, sizeof(type))
void map_delete(Map *map);

Map_Status_Code map_insert(Map *map, char const *key, Any value);
Map_Status_Code map_get(Map *map, char const *key, Any output_value);

void map_pretty_print(Map *map);
