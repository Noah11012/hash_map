## Hash Map Library

Example code:

```c
#include <stdio.h>
#include "map.h"

int main(void) {
    Map *name_to_age;
    map_init(name_to_age, int);

    map_insert("John", 22);
    map_insert("Susan", 24);
    map_insert("Mary", 25);

    // Get the values
    int mary_age;
    map_get(map, "Mary", &mary_age);

    printf("Mary's age is %d\n", mary_age);

    // If a mapping exists already then this will modify the value
    map_insert(map, "Mary", 30);
    map_get(map, "Mary", mary_age);

    printf("Mary's age is now %d\n", mary_age);

    map_pretty_print(map); // For debugging purposes

    map_delete(map);
}
```

Compile:
`clang main.c map.c`
