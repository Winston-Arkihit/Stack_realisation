#include "stack.h"

#include <stdio.h>

int main(void) {
    Stack stack = {};

    stack_activate(&stack, 2);

    stack_append(&stack, 10);
    stack_push(&stack, 20);
    stack_push(&stack, 30);

    stack_print(&stack);
    stack_dump(&stack);

    TypeOfStackElem value = 0;

    if (stack_pop(&stack, &value) == STACK_OK) {
        printf("%lf\n", value); // 30
    }

    if (stack_pop(&stack, &value) == STACK_OK) {
        printf("%lf\n", value); // 20
    }

    if (stack_pop(&stack, &value) == STACK_OK) {
        printf("%lf\n", value); // 10
    }

    #ifdef STACK_USE_CANARIES
    stack.left_canary = 123;
    stack_push(&stack, 40);
    #endif

    stack_destruct(&stack);

    return 0;
}
