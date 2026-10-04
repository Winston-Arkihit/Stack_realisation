#include "stack.h"

#include <stdio.h>
#include <stdlib.h>

#define STACK_ASSERT(condition)                                      \
    do {                                                            \
        if (!(condition)) {                                         \
            fprintf(stderr, "STACK ASSERT ERROR: %s\n%s:%d\n",      \
                    #condition, __FILE__, __LINE__);                \
            abort();                                                \
        }                                                           \
    } while (0)

//####CONST_BLOCK#######################################################
const Canary_t LEFT_CANARY_VALUE  = 0xDEAD80E3; // <3
const Canary_t RIGHT_CANARY_VALUE = 0xBADC0FFE;

#ifdef STACK_USE_POISON
const TypeOfStackElem POISON_VALUE = -666.0;
#endif

#ifdef STACK_USE_HASH
const unsigned long long HASH_START_VALUE = 14695981039346656037ULL;
const unsigned long long HASH_MULTIPLIER = 1099511628211ULL;
#endif

//####FUNCTIONS_BLOCK#################################################
#ifdef STACK_USE_CANARIES
static Canary_t *left_data_canary(Stack *stack) {
    STACK_ASSERT(stack != NULL);
    STACK_ASSERT(stack->all_data != NULL);

    return ((Canary_t *) stack->all_data) - 1;
}

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$

static Canary_t *right_data_canary(Stack *stack) {
    STACK_ASSERT(stack != NULL);
    STACK_ASSERT(stack->all_data != NULL);

    return (Canary_t *) (stack->all_data + stack->capacity);
}

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$

static void *stack_memory_begin(Stack *stack) {
    return (void *) left_data_canary(stack);
}

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$

static size_t stack_memory_size(size_t capacity) {
    return sizeof(Canary_t) + capacity * sizeof(TypeOfStackElem) + sizeof(Canary_t);
}
#endif

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$

#ifdef STACK_USE_HASH
static unsigned long long hash_bytes(unsigned long long hash, const void *data, size_t size) {
    const unsigned char *bytes = (const unsigned char *) data;

    for (size_t i = 0; i < size; i++) {
        hash ^= bytes[i];
        hash *= HASH_MULTIPLIER;
    }

    return hash;
}

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$

static unsigned long long stack_count_hash(Stack *stack) {
    STACK_ASSERT(stack != NULL);
    STACK_ASSERT(stack->all_data != NULL);

    unsigned long long hash = HASH_START_VALUE;

    #ifdef STACK_USE_CANARIES
    hash = hash_bytes(hash, &stack->left_canary, sizeof(stack->left_canary));
    #endif

    hash = hash_bytes(hash, &stack->all_data, sizeof(stack->all_data));
    hash = hash_bytes(hash, &stack->size, sizeof(stack->size));
    hash = hash_bytes(hash, &stack->capacity, sizeof(stack->capacity));

    #ifdef STACK_USE_CANARIES
    hash = hash_bytes(hash, &stack->right_canary, sizeof(stack->right_canary));
    hash = hash_bytes(hash, left_data_canary(stack), sizeof(Canary_t));
    #endif

    hash = hash_bytes(hash, stack->all_data, stack->capacity * sizeof(TypeOfStackElem));

    #ifdef STACK_USE_CANARIES
    hash = hash_bytes(hash, right_data_canary(stack), sizeof(Canary_t));
    #endif

    return hash;
}

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$

static void stack_update_hash(Stack *stack) {
    STACK_ASSERT(stack != NULL);
    STACK_ASSERT(stack->all_data != NULL);

    stack->hash = stack_count_hash(stack);
}
#endif

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$

static void stack_report_error(Stack *stack, StackError error) {
    stack_print_error(error);

    if (error != STACK_OK) {
        stack_dump(stack);
    }
}

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$

void stack_print_error(StackError error) {
    switch (error) {
        case STACK_OK:
            break;

        case STACK_NULL:
            printf("Stack pointer is NULL\n");
            break;

        case STACK_LEFT_CANARY_BROKEN:
            printf("Left canary is broken\n");
            break;

        case STACK_RIGHT_CANARY_BROKEN:
            printf("Right canary is broken\n");
            break;

        case STACK_DATA_NULL:
            printf("Stack is not activated\n");
            break;

        case STACK_LEFT_DATA_CANARY_BROKEN:
            printf("Left data canary is broken\n");
            break;

        case STACK_RIGHT_DATA_CANARY_BROKEN:
            printf("Right data canary is broken\n");
            break;

        case STACK_SIZE_BIGGER_THAN_CAPACITY:
            printf("Stack size is bigger than capacity\n");
            break;

        case STACK_MEMORY_ALLOCATION_ERROR:
            printf("Memory allocation error\n");
            break;

        case STACK_HASH_BROKEN:
            printf("Stack hash is broken\n");
            break;

        case STACK_EMPTY:
            printf("Stack is empty\n");
            break;

        case STACK_RESULT_NULL:
            printf("Result pointer is NULL\n");
            break;
    }
}

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$

#ifdef STACK_USE_CANARIES
static StackError stack_canaries_error(Stack *stack) {
    if (stack == NULL) {
        return STACK_NULL;
    }

    if (stack->left_canary != LEFT_CANARY_VALUE) {
        return STACK_LEFT_CANARY_BROKEN;
    }

    if (stack->right_canary != RIGHT_CANARY_VALUE) {
        return STACK_RIGHT_CANARY_BROKEN;
    }

    return STACK_OK;
}

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$

static StackError stack_data_canaries_error(Stack *stack) {
    if (stack == NULL) {
        return STACK_NULL;
    }

    if (stack->all_data == NULL) {
        return STACK_DATA_NULL;
    }

    if (*left_data_canary(stack) != LEFT_CANARY_VALUE) {
        return STACK_LEFT_DATA_CANARY_BROKEN;
    }

    if (*right_data_canary(stack) != RIGHT_CANARY_VALUE) {
        return STACK_RIGHT_DATA_CANARY_BROKEN;
    }

    return STACK_OK;
}
#endif

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$

StackError stack_error(Stack *stack) {
    if (stack == NULL) {
        return STACK_NULL;
    }

    #ifdef STACK_USE_CANARIES
    StackError error = stack_canaries_error(stack);
    if (error != STACK_OK) {
        return error;
    }
    #endif

    if (stack->all_data == NULL) {
        return STACK_DATA_NULL;
    }

    #ifdef STACK_USE_CANARIES
    error = stack_data_canaries_error(stack);
    if (error != STACK_OK) {
        return error;
    }
    #endif

    if (stack->size > stack->capacity) {
        return STACK_SIZE_BIGGER_THAN_CAPACITY;
    }

    #ifdef STACK_USE_HASH
    if (stack->hash != stack_count_hash(stack)) {
        return STACK_HASH_BROKEN;
    }
    #endif

    return STACK_OK;
}

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$

int stack_is_ok(Stack *stack) {
    StackError error = stack_error(stack);
    stack_report_error(stack, error);
    return error == STACK_OK;
}

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$

void stack_activate(Stack *stack, size_t capacity) {
    if (stack == NULL) {
        stack_report_error(stack, STACK_NULL);
        return;
    }

    #ifdef STACK_USE_CANARIES
    stack->left_canary = LEFT_CANARY_VALUE;
    stack->right_canary = RIGHT_CANARY_VALUE;
    #endif

    #ifdef STACK_USE_CANARIES
    void *memory = calloc(1, stack_memory_size(capacity));
    #else
    void *memory = calloc(capacity, sizeof(TypeOfStackElem));
    #endif

    if (memory == NULL) {
        stack_report_error(stack, STACK_MEMORY_ALLOCATION_ERROR);
        return;
    }

    #ifdef STACK_USE_CANARIES
    Canary_t *left_canary = (Canary_t *) memory;
    *left_canary = LEFT_CANARY_VALUE;
    stack->all_data = (TypeOfStackElem *) (left_canary + 1);
    #else
    stack->all_data = (TypeOfStackElem *) memory;
    #endif

    stack->size = 0;
    stack->capacity = capacity;

    #ifdef STACK_USE_CANARIES
    *right_data_canary(stack) = RIGHT_CANARY_VALUE;
    #endif

    #ifdef STACK_USE_POISON
    for (size_t i = 0; i < stack->capacity; i++) {
        stack->all_data[i] = POISON_VALUE;
    }
    #endif

    #ifdef STACK_USE_HASH
    stack_update_hash(stack);
    #endif
}

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$

int stack_resize(Stack *stack, size_t new_capacity) {
    if (!stack_is_ok(stack)) {
        return 0;
    }

    if (new_capacity < stack->size) {
        new_capacity = stack->size;
    }

    #ifdef STACK_USE_POISON
    size_t old_capacity = stack->capacity;
    #endif

    #ifdef STACK_USE_CANARIES
    void *memory = realloc(stack_memory_begin(stack), stack_memory_size(new_capacity));
    #else
    void *memory = realloc(stack->all_data, new_capacity * sizeof(TypeOfStackElem));
    #endif

    if (memory == NULL) {
        stack_report_error(stack, STACK_MEMORY_ALLOCATION_ERROR);
        return 0;
    }

    #ifdef STACK_USE_CANARIES
    Canary_t *left_canary = (Canary_t *) memory;
    *left_canary = LEFT_CANARY_VALUE;
    stack->all_data = (TypeOfStackElem *) (left_canary + 1);
    #else
    stack->all_data = (TypeOfStackElem *) memory;
    #endif

    stack->capacity = new_capacity;

    #ifdef STACK_USE_POISON
    for (size_t i = old_capacity; i < stack->capacity; i++) {
        stack->all_data[i] = POISON_VALUE;
    }
    #endif

    #ifdef STACK_USE_CANARIES
    *right_data_canary(stack) = RIGHT_CANARY_VALUE;
    #endif

    #ifdef STACK_USE_HASH
    stack_update_hash(stack);
    #endif

    return 1;
}

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$

void stack_push(Stack *stack, TypeOfStackElem addable_elem) {
    if (!stack_is_ok(stack)) {
        return;
    }

    if (stack->size == stack->capacity) {
        size_t new_capacity = stack->capacity * 2;

        if (new_capacity == 0) {
            new_capacity = 1;
        }

        if (!stack_resize(stack, new_capacity)) {
            return;
        }
    }

    stack->all_data[stack->size] = addable_elem;
    stack->size += 1;

    #ifdef STACK_USE_HASH
    stack_update_hash(stack);
    #endif
}

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$

void stack_append(Stack *stack, TypeOfStackElem elem) {
    stack_push(stack, elem);
}

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$

StackError stack_pop(Stack *stack, TypeOfStackElem *result) {
    StackError error = stack_error(stack);
    if (error != STACK_OK) {
        stack_report_error(stack, error);
        return error;
    }

    if (result == NULL) {
        stack_report_error(stack, STACK_RESULT_NULL);
        return STACK_RESULT_NULL;
    }

    if (stack->size == 0) {
        stack_report_error(stack, STACK_EMPTY);
        return STACK_EMPTY;
    }

    stack->size--;
    *result = stack->all_data[stack->size];

    #ifdef STACK_USE_POISON
    stack->all_data[stack->size] = POISON_VALUE;
    #endif

    #ifdef STACK_USE_HASH
    stack_update_hash(stack);
    #endif

    return STACK_OK;
}

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$

void deactivate_stack(Stack *stack) {
    if (stack == NULL) {
        stack_report_error(stack, STACK_NULL);
        return;
    }

    if (stack->all_data != NULL) {
        #ifdef STACK_USE_CANARIES
        free(stack_memory_begin(stack));
        #else
        free(stack->all_data);
        #endif
    }

    stack->all_data = NULL;
    stack->size = 0;
    stack->capacity = 0;

    #ifdef STACK_USE_HASH
    stack->hash = 0;
    #endif
}

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$

void stack_destruct(Stack *stack) {
    deactivate_stack(stack);
}

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$

void stack_print(Stack *stack) {
    if (!stack_is_ok(stack)) {
        return;
    }

    printf("**************************\n");
    printf("Stack size = %zu, capacity = %zu\n", stack->size, stack->capacity);

    for (size_t i = 0; i < stack->size; i++) {
        printf("[%zu] = %lf\n", i, stack->all_data[i]);
    }

    printf("**************************\n");
}

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$

void stack_dump(Stack *stack) {
    printf("========== STACK DUMP ==========\n");

    #ifdef STACK_NO_PROTECTION
    printf("protection mode    = no protection\n");
    #elif defined(STACK_CANARY_PROTECTION)
    printf("protection mode    = canaries\n");
    #elif defined(STACK_FULL_PROTECTION)
    printf("protection mode    = full protection\n");
    #endif

    if (stack == NULL) {
        printf("Stack pointer      = NULL\n");
        printf("================================\n");
        return;
    }

    printf("Stack pointer      = %p\n", (void *) stack);

    #ifdef STACK_USE_CANARIES
    printf("left_canary        = 0x%llX\n", stack->left_canary);
    printf("right_canary       = 0x%llX\n", stack->right_canary);
    #endif

    printf("all_data           = %p\n", (void *) stack->all_data);
    printf("size               = %zu\n", stack->size);
    printf("capacity           = %zu\n", stack->capacity);

    #ifdef STACK_USE_HASH
    printf("hash               = %llu\n", stack->hash);
    #endif

    if (stack->all_data != NULL) {
        #ifdef STACK_USE_CANARIES
        printf("left_data_canary   = 0x%llX\n", *left_data_canary(stack));
        printf("right_data_canary  = 0x%llX\n", *right_data_canary(stack));
        #endif

        for (size_t i = 0; i < stack->capacity; i++) {
            if (i < stack->size) {
                printf("* [%zu] = %lf\n", i, stack->all_data[i]);
            }
            else {
                #ifdef STACK_USE_POISON
                printf("  [%zu] = %lf (poison)\n", i, stack->all_data[i]);
                #else
                printf("  [%zu] = %lf\n", i, stack->all_data[i]);
                #endif
            }
        }
    }

    StackError error = stack_error(stack);
    printf("stack condition    = ");
    stack_print_error(error);

    if (error == STACK_OK) {
        printf("OK\n");
    }

    printf("================================\n");
}
