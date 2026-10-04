# C Stack Library

Small dynamic stack library for C.

## Files

- `stack.h` - include this file in your project.
- `stack.c` - compile this file together with your project.
- `Stack_Header_Only.h` - one-file version: include it and use stack functions without `stack.c`.
- `example_header_only.c` - small example for `Stack_Header_Only.h`.

## Header-Only Usage

If you want the simplest version, copy only:

```text
Stack_Header_Only.h
```

Then include it:

```c
#include "Stack_Header_Only.h"
```

Build example:

```bat
g++ -DSTACK_FULL_PROTECTION -x c "main.c" -o "program.exe"
```

You do not need to compile `stack.c` in this mode.

## Protection Modes

Choose one compile flag:

```bat
-DSTACK_NO_PROTECTION
-DSTACK_CANARY_PROTECTION
-DSTACK_FULL_PROTECTION
```

If no flag is selected, `STACK_FULL_PROTECTION` is used by default.

## Build Example

```bat
g++ -DSTACK_FULL_PROTECTION -I "C_Stack_Library" -x c "main.c" -x c "C_Stack_Library\stack.c" -o "program.exe"
```

For pure GCC:

```bat
gcc -DSTACK_FULL_PROTECTION -I "C_Stack_Library" "main.c" "C_Stack_Library\stack.c" -o "program.exe"
```

## Usage Example

```c
#include "stack.h"

#include <stdio.h>

int main(void) {
    Stack stack = {};
    stack_activate(&stack, 4);

    stack_push(&stack, 10);
    stack_push(&stack, 20);

    TypeOfStackElem value = 0;
    if (stack_pop(&stack, &value) == STACK_OK) {
        printf("%lf\n", value);
    }

    stack_destruct(&stack);
    return 0;
}
```

