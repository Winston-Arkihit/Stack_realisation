#include "Stack_Header_Only.h"

int main(void) {
    Stack stack = {};
    stack_activate(&stack, 2);

    stack_push(&stack, 10);
    stack_push(&stack, 20);
    stack_push(&stack, 30);

    TypeOfStackElem value = 0;
    if (stack_pop(&stack, &value) == STACK_OK) {
        printf("%lf\n", value);
    }

    stack_destruct(&stack);

    return 0;
}
