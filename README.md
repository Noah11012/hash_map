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

    map_insert(map, "John", INT_TO_ANY(25));
    map_insert(map, "Susan", INT_TO_ANY(28));
    map_insert(map, "Mary", INT_TO_ANY(24));
    map_insert(map, "Jessica", INT_TO_ANY(30));

    int *age_of_john = map_get(map, "John");
    printf("The age of john is %d\n", *age_of_john);

    int array[] = { 1, 2, 3 };
    map_insert_array(map, "An Array", array, 3);

    for (map_iterate(map, it)) {
        /* Insertion while iterating over the map is not allowed. */
        map_insert(map, "Ian", INT_TO_ANY(26));

        /* You can use the is_array flag to distinguish between
           non arrays and arrays */
        if (it->is_array) {
            printf("Printing array\n");
            /* it->count is valid if it->is_array == true. */
            for (int i = 0; i < it->count; i++)
                printf("it->value[%d] = %d\n", i, ((int *)it->value)[i]);
        } else {
            printf("key = %s\tvalue = %d\n", it->key, *(int *)it->value);
        }

        if (it->key[0] == 'J') {
            /* You can remove elements while iterating.
               Always assume when removing elements inside
               of a map_iterate loop that the iterator
               will become invalidated. */
            map_remove(map, it->key);
        }

    }

    /* Can be used to debug problems with your map. */
    map_pretty_print(map);

    /* Delete the map and all of its associated memory. */
    map_delete(map);
}
```
