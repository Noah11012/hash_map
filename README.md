## Hash Map Library

Mainly this library was created for my educational purposes and should not be used in anyone's code base.
Of course you may use the library for your own learning purposes or use this as a template for your own hash map implementation.

Example code:

```c
#include <stdio.h>

#include "map.h"

int main(void) {
    Map *map;
    if (!map_init(map, int)) {
        printf("Failed to create map\n");
        return 1;
    }

    map_register_new_type(type);

    map_insert(map, "John", INT_TO_ANY(25));
    map_insert(map, "Susan", INT_TO_ANY(28));
    map_insert(map, "Mary", INT_TO_ANY(24));
    map_insert(map, "Jessica", INT_TO_ANY(30));

    for(map_iterate(map, it, int)) {
        printf("key = %s\tvalue = %d\n", it->key, *it->value);
    }

    map_pretty_print(map);

    map_delete(map);
}
```
