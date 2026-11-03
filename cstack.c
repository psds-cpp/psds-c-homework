#include "cstack.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

struct linked_list_node
{
    struct linked_list_node* prev;
    unsigned int size_data;
    char data[];
};

typedef struct linked_list_node linked_list_node_t;

struct stack
{
    linked_list_node_t* node;
    unsigned int size;
    int is_alive;
};

typedef struct stack stack_t;

struct stack_storage
{
    unsigned int size;
    unsigned int capacity;
    stack_t* stacks;
};

static struct stack_storage storage = {0, 0, NULL};

int resize(void)
{
    stack_t* ptr = malloc(sizeof(stack_t) * storage.capacity * 2);
    if (ptr == NULL)
    {
        return -1;
    }
    memcpy(ptr, storage.stacks, sizeof(stack_t) * storage.size);
    free(storage.stacks);
    storage.stacks = ptr;
    storage.capacity *= 2;
    return 0;
}

hstack_t stack_new(void)
{
    if (storage.stacks == NULL)
    {
        storage.capacity = 2;
        storage.stacks = malloc(sizeof(stack_t) * 2);
        if (storage.stacks == NULL)
        {
            return -1;
        }
    }
    if (storage.size == storage.capacity)
    {
        int error = resize();
        if (error == -1)
        {
            return -1;
        }
    }
    stack_t* stack = storage.stacks + storage.size;
    stack->size = 0;
    stack->node = NULL;
    stack->is_alive = 1;
    return storage.size++;
}

void stack_free(const hstack_t hstack)
{
    if (!stack_valid_handler(hstack))
    {
        return;
    }
    stack_t* stack = storage.stacks + hstack;
    stack->size = 0;
    stack->is_alive = 0;
    linked_list_node_t* node = stack->node;
    while (node != NULL)
    {
        linked_list_node_t* del_node = node;
        node = node->prev;
        free(del_node);
    }
    stack->node = NULL;
}

void stack_free_all(void)
{
    for (unsigned int i = 0; i < storage.size; ++i)
    {
        stack_free(i);
    }
    free(storage.stacks);
    storage.stacks = NULL;
    storage.capacity = 0;
    storage.size = 0;
}

int stack_valid_handler(const hstack_t hstack)
{
    unsigned int u_hstack = hstack;
    if (hstack >= 0 && u_hstack < storage.size)
    {
        if (storage.stacks[hstack].is_alive)
        {
            return 1;
        }
        return 0;
    }
    return 0;
}

unsigned int stack_size(const hstack_t hstack)
{
    if (stack_valid_handler(hstack))
    {
        return storage.stacks[hstack].size;
    }
    return 0;
}

void
stack_push(const hstack_t hstack, const void* data_in, const unsigned int size)
{
    if (data_in == NULL || size == 0 || !stack_valid_handler(hstack))
    {
        return;
    }
    linked_list_node_t* ptr = malloc(sizeof(linked_list_node_t) + size);
    if (ptr == NULL)
    {
        return;
    }
    stack_t* stack = storage.stacks + hstack;
    memcpy(ptr->data, data_in, size);
    ptr->size_data = size;
    ptr->prev = stack->node;
    stack->node = ptr;
    ++stack->size;
}

unsigned int
stack_pop(const hstack_t hstack, void* data_out, const unsigned int size)
{
    if (data_out == NULL || size == 0 || !stack_valid_handler(hstack))
    {
        return 0;
    }
    stack_t* stack = storage.stacks + hstack;
    if (stack->node == NULL || stack->node->size_data > size)
    {
        return 0;
    }
    linked_list_node_t* node = stack->node;
    unsigned int size_writ_data = node->size_data;
    memcpy(data_out, node->data, size_writ_data);
    stack->node = node->prev;
    free(node);
    --stack->size;
    return size_writ_data;
}

unsigned stack_top_size(const hstack_t hstack)
{
    if (stack_valid_handler(hstack) && storage.stacks[hstack].node != NULL)
    {
        return storage.stacks[hstack].node->size_data;
    }
    return 0;
}

hstack_t stack_get_all(void)
{
    hstack_t all_hstack = stack_new();
    if (all_hstack == -1)
    {
        return -1;
    }
    if (storage.size == 0)
    {
        return 0;
    }
    for (unsigned int i = 0; i < storage.size - 1; ++i)
    {
        hstack_t t = i;
        if (stack_valid_handler(i))
        {
            stack_push(all_hstack, &t, sizeof(hstack_t));
        }
    }
    return all_hstack;
}
