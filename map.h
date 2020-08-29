typedef void* Any;
typedef struct Map Map;

typedef struct
{
    char const *key;
    Any value;
} Map_Iter;

#define ANY_NULL ((void *) 0)
#define NULLPTR  ((void *) 0)

#include <stddef.h>
#include <stdbool.h>
#define INT_TO_ANY(number) (&(int){ number })

#define map_iter(map, variable_name) Map_Iter variable_name = map_iter_begin(map); map_iter_is_valid(variable_name); variable_name = map_iter_next(map)

Map_Iter map_iter_begin(Map *map);
Map_Iter map_iter_next(Map *map);
bool map_iter_is_valid(Map_Iter iter);

bool map_init_(Map **mapp, int value_size);
#define map_init(map, type) map_init_(&map, sizeof(type))
void map_delete(Map *map);
void map_insert(Map *map, char const *key, Any value);
Any map_get(Map *map, char const *key);
void map_pretty_print(Map *map);
