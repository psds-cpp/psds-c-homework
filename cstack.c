#include "cstack.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define MAX_STACKS 16

struct node
{
    struct node* prev;
    unsigned int size;
    char data[];
};

struct stack_info
{
    struct node* top;
    unsigned int count;
    int occupied;
};

static struct stack_info stacks[MAX_STACKS];

hstack_t stack_new(void)
{
    for (hstack_t i = 0; i < MAX_STACKS; ++i)
    {
        if (!stacks[i].occupied)
        {
            stacks[i].occupied = 1;
            return i;
        }
    }
    return -1;
}

void stack_free(const hstack_t hstack)
{
    if (!stack_valid_handler(hstack))
    {
        return;
    }
    while (stacks[hstack].top != NULL)
    {
        struct node* current = stacks[hstack].top;
        stacks[hstack].top = current->prev;
        free(current);
    }
    stacks[hstack].count = 0;
    stacks[hstack].occupied = 0;
}

void stack_free_all(void)
{
    for (hstack_t i = 0; i < MAX_STACKS; ++i)
    {
        stack_free(i);
    }
}

int stack_valid_handler(const hstack_t hstack)
{
    return hstack >= 0 && hstack < MAX_STACKS && stacks[hstack].occupied;
}

unsigned int stack_size(const hstack_t hstack)
{
    return stack_valid_handler(hstack) ? stacks[hstack].count : 0;
}

void
stack_push(const hstack_t hstack, const void* data_in, const unsigned int size)
{
    if (!stack_valid_handler(hstack) || data_in == NULL || size == 0)
    {
        return;
    }
    struct node* node = malloc(sizeof(struct node) + size);
    if (node == NULL)
    {
        return;
    }
    node->size = size;
    memcpy(node->data, data_in, size);
    node->prev = stacks[hstack].top;
    stacks[hstack].top = node;
    ++stacks[hstack].count;
}

unsigned int
stack_pop(const hstack_t hstack, void* data_out, const unsigned int size)
{
    if (!stack_valid_handler(hstack) || data_out == NULL ||
        stacks[hstack].top == NULL || size < stacks[hstack].top->size)
    {
        return 0;
    }
    struct node* node = stacks[hstack].top;
    const unsigned int node_size = node->size;
    memcpy(data_out, node->data, node_size);
    stacks[hstack].top = node->prev;
    --stacks[hstack].count;
    free(node);
    return node_size;
}

unsigned stack_top_size(const hstack_t hstack)
{
    if (!stack_valid_handler(hstack) || stacks[hstack].top == NULL)
    {
        return 0;
    }
    return stacks[hstack].top->size;
}

hstack_t stack_get_all(void)
{
    const hstack_t result = stack_new();
    if (result < 0)
    {
        return -1;
    }
    for (hstack_t i = 0; i < MAX_STACKS; ++i)
    {
        if (i != result && stack_valid_handler(i))
        {
            const unsigned int before = stack_size(result);
            stack_push(result, &i, sizeof i);
            if (stack_size(result) == before)
            {
                stack_free(result);
                return -1;
            }
        }
    }
    return result;
}