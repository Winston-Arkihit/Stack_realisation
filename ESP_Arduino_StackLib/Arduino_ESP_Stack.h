#ifndef ARDUINO_ESP_STACK_H
#define ARDUINO_ESP_STACK_H

#include <Arduino.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

typedef double StackElem_t;
typedef unsigned long long Canary_t;

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
    STACK_EMPTY,
    STACK_RESULT_NULL
} StackError;

typedef struct Stack {
    Canary_t left_canary;
//------------------------------
    StackElem_t *all_data;
    size_t size;
    size_t capacity;
//------------------------------
    Canary_t right_canary;
} Stack;

static const Canary_t LEFT_CANARY_VALUE  = 0xDEAD80E3;
static const Canary_t RIGHT_CANARY_VALUE = 0xBADC0FFE;
static const StackElem_t POISON_VALUE = -666.0;

static inline Canary_t *left_data_canary(Stack *stack) {
    return ((Canary_t *) stack->all_data) - 1;
}

static inline Canary_t *right_data_canary(Stack *stack) {
    return (Canary_t *) (stack->all_data + stack->capacity);
}

static inline size_t stack_memory_size(size_t capacity) {
    return sizeof(Canary_t) + capacity * sizeof(StackElem_t) + sizeof(Canary_t);
}

static inline StackError stack_error(Stack *stack) {
    if (stack == NULL) {
        return STACK_NULL;
    }

    if (stack->left_canary != LEFT_CANARY_VALUE) {
        return STACK_LEFT_CANARY_BROKEN;
    }

    if (stack->right_canary != RIGHT_CANARY_VALUE) {
        return STACK_RIGHT_CANARY_BROKEN;
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

    if (stack->size > stack->capacity) {
        return STACK_SIZE_BIGGER_THAN_CAPACITY;
    }

    return STACK_OK;
}

static inline const char *stack_error_text(StackError error) {
    switch (error) {
        case STACK_OK:
            return "STACK_OK";
        case STACK_NULL:
            return "STACK_NULL";
        case STACK_LEFT_CANARY_BROKEN:
            return "STACK_LEFT_CANARY_BROKEN";
        case STACK_RIGHT_CANARY_BROKEN:
            return "STACK_RIGHT_CANARY_BROKEN";
        case STACK_DATA_NULL:
            return "STACK_DATA_NULL";
        case STACK_LEFT_DATA_CANARY_BROKEN:
            return "STACK_LEFT_DATA_CANARY_BROKEN";
        case STACK_RIGHT_DATA_CANARY_BROKEN:
            return "STACK_RIGHT_DATA_CANARY_BROKEN";
        case STACK_SIZE_BIGGER_THAN_CAPACITY:
            return "STACK_SIZE_BIGGER_THAN_CAPACITY";
        case STACK_MEMORY_ALLOCATION_ERROR:
            return "STACK_MEMORY_ALLOCATION_ERROR";
        case STACK_EMPTY:
            return "STACK_EMPTY";
        case STACK_RESULT_NULL:
            return "STACK_RESULT_NULL";
        default:
            return "UNKNOWN_STACK_ERROR";
    }
}

static inline bool stack_is_ok(Stack *stack) {
    return stack_error(stack) == STACK_OK;
}

static inline StackError stack_activate(Stack *stack, size_t capacity) {
    if (stack == NULL) {
        return STACK_NULL;
    }

    stack->left_canary = LEFT_CANARY_VALUE;
    stack->right_canary = RIGHT_CANARY_VALUE;

    void *memory = calloc(1, stack_memory_size(capacity));
    if (memory == NULL) {
        stack->all_data = NULL;
        stack->size = 0;
        stack->capacity = 0;
        return STACK_MEMORY_ALLOCATION_ERROR;
    }

    Canary_t *left_canary = (Canary_t *) memory;
    *left_canary = LEFT_CANARY_VALUE;

    stack->all_data = (StackElem_t *) (left_canary + 1);
    stack->size = 0;
    stack->capacity = capacity;

    *right_data_canary(stack) = RIGHT_CANARY_VALUE;

    for (size_t i = 0; i < stack->capacity; i++) {
        stack->all_data[i] = POISON_VALUE;
    }

    return STACK_OK;
}

static inline StackError stack_resize(Stack *stack, size_t new_capacity) {
    StackError error = stack_error(stack);
    if (error != STACK_OK) {
        return error;
    }

    if (new_capacity < stack->size) {
        new_capacity = stack->size;
    }

    size_t old_capacity = stack->capacity;

    void *memory = realloc(left_data_canary(stack), stack_memory_size(new_capacity));
    if (memory == NULL) {
        return STACK_MEMORY_ALLOCATION_ERROR;
    }

    Canary_t *left_canary = (Canary_t *) memory;
    *left_canary = LEFT_CANARY_VALUE;

    stack->all_data = (StackElem_t *) (left_canary + 1);
    stack->capacity = new_capacity;

    *right_data_canary(stack) = RIGHT_CANARY_VALUE;

    for (size_t i = old_capacity; i < stack->capacity; i++) {
        stack->all_data[i] = POISON_VALUE;
    }

    return STACK_OK;
}

static inline StackError stack_push(Stack *stack, StackElem_t value) {
    StackError error = stack_error(stack);
    if (error != STACK_OK) {
        return error;
    }

    if (stack->size == stack->capacity) {
        size_t new_capacity = stack->capacity * 2;
        if (new_capacity == 0) {
            new_capacity = 1;
        }

        error = stack_resize(stack, new_capacity);
        if (error != STACK_OK) {
            return error;
        }
    }

    stack->all_data[stack->size] = value;
    stack->size++;

    return STACK_OK;
}

static inline StackError stack_pop(Stack *stack, StackElem_t *result) {
    StackError error = stack_error(stack);
    if (error != STACK_OK) {
        return error;
    }

    if (result == NULL) {
        return STACK_RESULT_NULL;
    }

    if (stack->size == 0) {
        return STACK_EMPTY;
    }

    stack->size--;
    *result = stack->all_data[stack->size];
    stack->all_data[stack->size] = POISON_VALUE;

    return STACK_OK;
}

static inline StackError stack_destruct(Stack *stack) {
    if (stack == NULL) {
        return STACK_NULL;
    }

    if (stack->all_data != NULL) {
        free(left_data_canary(stack));
    }

    stack->all_data = NULL;
    stack->size = 0;
    stack->capacity = 0;

    return STACK_OK;
}

static inline void stack_dump(Stack *stack, Print &out) {
    out.println("========== STACK DUMP ==========");

    if (stack == NULL) {
        out.println("Stack pointer = NULL");
        out.println("================================");
        return;
    }

    out.print("Stack pointer = ");
    out.println((uintptr_t) stack, HEX);
    out.print("all_data      = ");
    out.println((uintptr_t) stack->all_data, HEX);
    out.print("size          = ");
    out.println(stack->size);
    out.print("capacity      = ");
    out.println(stack->capacity);
    out.print("left_canary   = 0x");
    out.println((unsigned long) stack->left_canary, HEX);
    out.print("right_canary  = 0x");
    out.println((unsigned long) stack->right_canary, HEX);

    if (stack->all_data != NULL) {
        out.print("data left     = 0x");
        out.println((unsigned long) *left_data_canary(stack), HEX);
        out.print("data right    = 0x");
        out.println((unsigned long) *right_data_canary(stack), HEX);

        for (size_t i = 0; i < stack->capacity; i++) {
            if (i < stack->size) {
                out.print("* [");
            }
            else {
                out.print("  [");
            }

            out.print(i);
            out.print("] = ");
            out.println(stack->all_data[i]);
        }
    }

    StackError error = stack_error(stack);
    out.print("condition     = ");
    out.println(stack_error_text(error));
    out.println("================================");
}

#endif
