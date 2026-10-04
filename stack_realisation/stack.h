#ifndef STACK_H
#define STACK_H

#include <stddef.h>

/*3 flags of compilation
-DSTACK_NO_PROTECTION
-DSTACK_CANARY_PROTECTION
-DSTACK_FULL_PROTECTION
*/
#if !defined(STACK_NO_PROTECTION) && !defined(STACK_CANARY_PROTECTION) && !defined(STACK_FULL_PROTECTION)
    #define STACK_FULL_PROTECTION
#endif

#ifdef STACK_FULL_PROTECTION
    #define STACK_USE_CANARIES
    #define STACK_USE_HASH
    #define STACK_USE_POISON
#endif

#ifdef STACK_CANARY_PROTECTION
    #define STACK_USE_CANARIES
#endif


typedef double TypeOfStackElem; // double = TypeOfStackElem
typedef unsigned long long Canary_t; // unsigned long long = Canary_t

typedef enum StackError {
    STACK_OK = 0,
    STACK_NULL,
    STACK_LEFT_CANARY_BROKEN,
    STACK_RIGHT_CANARY_BROKEN,
    STACK_DATA_NULL,
    STACK_LEFT_DATA_CANARY_BROKEN,
    STACK_RIGHT_DATA_CANARY_BROKEN,
    STACK_SIZE_BIGGER_THAN_CAPACITY,
    STACK_MEMORY_ALLOCATION_ERROR,
    STACK_HASH_BROKEN,
    STACK_EMPTY,
    STACK_RESULT_NULL
} StackError;

typedef struct Stack {
#ifdef STACK_USE_CANARIES
    Canary_t left_canary;
#endif
//------------------------------
    TypeOfStackElem *all_data; // pointer to first useful stack element
    size_t size; // current number of elements in stack
    size_t capacity; // max number of elements in stack
#ifdef STACK_USE_HASH
    unsigned long long hash;
#endif
//------------------------------
#ifdef STACK_USE_CANARIES
    Canary_t right_canary;
#endif

} Stack;

void stack_print_error(StackError error);
StackError stack_error(Stack *stack);
int stack_is_ok(Stack *stack);

void stack_activate(Stack *stack, size_t capacity);
void stack_destruct(Stack *stack);
void deactivate_stack(Stack *stack);

int stack_resize(Stack *stack, size_t new_capacity);
void stack_push(Stack *stack, TypeOfStackElem addable_elem);
void stack_append(Stack *stack, TypeOfStackElem elem);
StackError stack_pop(Stack *stack, TypeOfStackElem *result);

void stack_print(Stack *stack);
void stack_dump(Stack *stack);

#endif
