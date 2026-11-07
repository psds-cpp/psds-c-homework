#include "cstack.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

struct Node
{
    struct Node* prev;
    unsigned long long data_size;
    char data[];
};

struct Stack
{
    struct Node* node;
    unsigned long long size;
    bool is_alive;
};

struct Storage
{
    unsigned long long size;
    unsigned long long capacity;
    struct Stack* stack;
};

static struct Storage storage = {0, 0, NULL};

hstack_t resize(void)
{
    unsigned long long new_capacity =
        storage.capacity == 0 ? 1 : storage.capacity * 2;
    struct Stack* ptr =
        realloc(storage.stack, sizeof(struct Stack) * new_capacity);
    if (ptr == NULL)
    {
        return -1;
    }
    storage.stack = ptr;
    storage.capacity = new_capacity;
    return 0;
}

hstack_t stack_new(void)
{
    if (storage.size == storage.capacity)
    {
        if (resize())
        {
            return -1;
        }
    }
    struct Stack* st = storage.stack + storage.size;
    st->size = 0;
    st->node = NULL;
    st->is_alive = true;
    return (hstack_t)storage.size++;
}

void stack_free(const hstack_t hstack)
{
    if (!stack_valid_handler(hstack))
    {
        return;
    }
    struct Stack* st = storage.stack + hstack;
    struct Node* node = st->node;
    while (node != NULL)
    {
        struct Node* prev = node->prev;
        free(node);
        node = prev;
    }
    st->node = NULL;
    st->size = 0;
    st->is_alive = false;
}

void stack_free_all(void)
{
    for (unsigned long long i = 0; i < storage.size; ++i)
    {
        stack_free((hstack_t)i);
    }
    free(storage.stack);
    storage.stack = NULL;
    storage.capacity = 0;
    storage.size = 0;
}

int stack_valid_handler(const hstack_t hstack)
{
    if (hstack < 0 || (unsigned long long)hstack >= storage.size)
    {
        return 0;
    }
    return storage.stack[hstack].is_alive ? true : false;
}

unsigned int stack_size(const hstack_t hstack)
{
    if (!stack_valid_handler(hstack))
    {
        return 0;
    }
    return (unsigned int)storage.stack[hstack].size;
}

void
stack_push(const hstack_t hstack, const void* data_in, const unsigned int size)
{
    if (data_in == NULL || size == 0 || !stack_valid_handler(hstack))
    {
        return;
    }
    struct Node* ptr = malloc(sizeof(struct Node) + size);
    if (ptr == NULL)
    {
        return;
    }
    struct Stack* st = storage.stack + hstack;
    memcpy(ptr->data, data_in, size);
    ptr->data_size = size;
    ptr->prev = st->node;
    st->node = ptr;
    ++st->size;
}

unsigned int
stack_pop(const hstack_t hstack, void* data_out, const unsigned int size)
{
    if (data_out == NULL || size == 0 || !stack_valid_handler(hstack))
    {
        return 0;
    }
    struct Stack* st = storage.stack + hstack;
    if (st->node == NULL || st->node->data_size > size)
    {
        return 0;
    }
    struct Node* node = st->node;
    unsigned int written = (unsigned int)node->data_size;
    memcpy(data_out, node->data, written);
    st->node = node->prev;
    free(node);
    --st->size;
    return written;
}

unsigned stack_top_size(const hstack_t hstack)
{
    if (!stack_valid_handler(hstack))
    {
        return 0;
    }
    struct Stack* st = storage.stack + hstack;
    if (st->node == NULL)
    {
        return 0;
    }
    return (unsigned int)st->node->data_size;
}

hstack_t stack_get_all(void)
{
    hstack_t all_hstack = stack_new();
    if (all_hstack == -1)
    {
        return -1;
    }
    for (unsigned long long i = 0; i < storage.size; ++i)
    {
        if ((hstack_t)i == all_hstack)
        {
            continue;
        }
        if (stack_valid_handler((hstack_t)i))
        {
            hstack_t t = (hstack_t)i;
            stack_push(all_hstack, &t, sizeof(t));
        }
    }
    return all_hstack;
}
