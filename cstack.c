#include "cstack.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define STACK_CAPACITY 16

struct node
{
    struct node* prev;
    unsigned int size;
    char data[];
};

struct stack
{
    int reserved;
    struct node* top;
    unsigned int count;
};

static struct stack stackTable[STACK_CAPACITY] = {0};

static struct stack* find_stack(hstack_t hstack)
{
    if (hstack < 0 || hstack >= STACK_CAPACITY || !stackTable[hstack].reserved)
    {
        return NULL;
    }

    return &stackTable[hstack];
}

hstack_t stack_new(void)
{
    for (int i = 0; i < STACK_CAPACITY; ++i)
    {
        if (!stackTable[i].reserved)
        {
            stackTable[i].top = NULL;
            stackTable[i].count = 0;
            stackTable[i].reserved = 1;

            return i;
        }
    }

    return -1;
}

void stack_free(const hstack_t hstack)
{
    struct stack* stack = find_stack(hstack);

    if (stack == NULL)
    {
        return;
    }

    struct node* node = stack->top;

    while (node != NULL)
    {
        struct node* prev = node->prev;

        free(node);
        node = prev;
    }

    stack->top = NULL;
    stack->count = 0;
    stack->reserved = 0;
}

void stack_free_all(void)
{
    for (int i = 0; i < STACK_CAPACITY; ++i)
    {
        stack_free(i);
    }
}

int stack_valid_handler(const hstack_t hstack)
{
    return find_stack(hstack) != NULL;
}

unsigned int stack_size(const hstack_t hstack)
{
    struct stack* stack = find_stack(hstack);

    if (stack == NULL)
    {
        return 0;
    }

    return stack->count;
}

void
stack_push(const hstack_t hstack, const void* data_in, const unsigned int size)
{
    struct stack* stack = find_stack(hstack);

    if (stack == NULL)
    {
        return;
    }

    struct node* node;

    if (data_in == NULL || size == 0)
    {
        return;
    }

    node = malloc(sizeof *node + size);
    if (node == NULL)
    {
        return;
    }

    node->prev = stack->top;
    node->size = size;
    memcpy(node->data, data_in, size);

    stack->top = node;
    ++stack->count;
}

unsigned int
stack_pop(const hstack_t hstack, void* data_out, const unsigned int size)
{
    struct stack* stack = find_stack(hstack);
    struct node* node;
    unsigned int nodeSize;

    if (stack == NULL || data_out == NULL || size == 0)
    {
        return 0;
    }

    node = stack->top;

    if (node == NULL || size < node->size)
    {
        return 0;
    }

    nodeSize = node->size;
    memcpy(data_out, node->data, nodeSize);

    stack->top = node->prev;
    --stack->count;

    free(node);

    return nodeSize;
}

unsigned stack_top_size(const hstack_t hstack)
{
    struct stack* stack = find_stack(hstack);

    if (stack == NULL || stack->top == NULL)
    {
        return 0;
    }

    return stack->top->size;
}

hstack_t stack_get_all(void)
{
    hstack_t result = stack_new();

    if (result == -1)
    {
        return -1;
    }

    for (int i = 0; i < STACK_CAPACITY; ++i)
    {
        if (i != result && stackTable[i].reserved)
        {
            hstack_t current = i;
            unsigned int previousCount = stack_size(result);

            stack_push(result, &current, sizeof current);

            if (stack_size(result) == previousCount)
            {
                stack_free(result);
                return -1;
            }
        }
    }

    return result;
}
