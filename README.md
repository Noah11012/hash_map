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

    /* This is only needed if you plan to use map_iterate().
       Never call this macro twice with the same type. */
    map_register_new_type(int);

    map_insert(map, "John", INT_TO_ANY(25));
    map_insert(map, "Susan", INT_TO_ANY(28));
    map_insert(map, "Mary", INT_TO_ANY(24));
    map_insert(map, "Jessica", INT_TO_ANY(30));

    int *age_of_john = map_get(map, "John");
    printf("The age of john is %d\n", *age_of_john);

    int array[] = { 1, 2, 3 };
    map_insert_array(map, "An Array", array, 3);

    for (map_iterate(map, it, int)) {
        printf("key = %s\tvalue = %d\n", it->key, *it->value);

        /* Insertion while iterating over the map is not allowed. */
        map_insert(map, "Ian", INT_TO_ANY(26));

        if (it->key[0] == 'J') {
            /* Removing elements is allowed during iteration.
               If you pass it->key to map_remove() or pass in a key
               that so happens to be the same as it->key then the iterator
               will become invalidated. Otherwise the iterator
               should be fine. */
            map_remove(map, it->key);
        }

        /* You can use the is_array flag to distinguish between
           non arrays and arrays */
        if (it->is_array) {
            printf("Printing array\n");
            /* it->count is valid if it->is_array == true. */
            for (int i = 0; i < it->count; i++)
                printf("it->value[%d] = %d\n", i, it->value[i]);
        }
    }

    /* Can be used to debug problems with your map. */
    map_pretty_print(map);

    /* Delete the map and all of its associated memory. */
    map_delete(map);
}
```
