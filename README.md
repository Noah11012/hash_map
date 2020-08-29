## Hash Map Library

Mainly this library was created for my educational purposes and should not be used in anyone's code base.
Of course you may use the library for your own learning purposes or use this as a template for your own hash map implementation.

Example code:

```c
#include <stdio.h>
#include "map.h"

int main(void) {
    Map *name_to_age;
    map_init(name_to_age, int);

    map_insert(name_to_age, "John", INT_TO_ANY(22));
    map_insert(name_to_age, "Susan", INT_TO_ANY(24));
    map_insert(name_to_age, "Mary", INT_TO_ANY(25));

    // Get the values
    int mary_age;
    map_get(name_to_age, "Mary", &mary_age);

    printf("Mary's age is %d\n", mary_age);

    // If a mapping exists already then this will modify the value
    map_insert(name_to_age, "Mary", INT_TO_ANY(30));
    map_get(name_to_age, "Mary", &mary_age);

    printf("Mary's age is now %d\n", mary_age);

    map_pretty_print(name_to_age); // For debugging purposes

    map_delete(name_to_age);
}
```

Compile:
`clang main.c map.c`
