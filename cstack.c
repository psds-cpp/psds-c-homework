#include "cstack.h"

#include <limits.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_CAPACITY 16u

struct node
{
    struct node* prev;
    unsigned int size;
    char data[];
};

typedef struct node* stack_t;

struct stack_entry
{
    int reserved;
    stack_t head;
    unsigned int count;
};

typedef struct stack_entry stack_entry_t;

struct stack_entries_table
{
    unsigned int capacity;
    stack_entry_t* entries;
};

static struct stack_entries_table g_table = {0u, NULL};

hstack_t stack_new(void)
{
    unsigned int slot = 0u;
    int found_slot = 0;

    if (g_table.entries != NULL)
    {
        for (unsigned int i = 0u; i < g_table.capacity; ++i)
        {
            if (!g_table.entries[i].reserved)
            {
                slot = i;
                found_slot = 1;
                break;
            }
        }
    }

    if (!found_slot)
    {
        if (g_table.capacity > (unsigned int)(INT_MAX / 2))
        {
            return -1;
        }

        unsigned int new_capacity =
            (g_table.capacity == 0u) ? INITIAL_CAPACITY : g_table.capacity * 2u;
        stack_entry_t* new_entries = (stack_entry_t*)realloc(
            g_table.entries, new_capacity * sizeof(stack_entry_t)
        );
        if (new_entries == NULL)
        {
            return -1;
        }

        for (unsigned int i = g_table.capacity; i < new_capacity; ++i)
        {
            new_entries[i].reserved = 0;
            new_entries[i].head = NULL;
            new_entries[i].count = 0u;
        }

        slot = g_table.capacity;
        g_table.entries = new_entries;
        g_table.capacity = new_capacity;
    }

    g_table.entries[slot].reserved = 1;
    g_table.entries[slot].head = NULL;
    g_table.entries[slot].count = 0u;

    return (hstack_t)slot;
}

void stack_free(const hstack_t hstack)
{
    if (!stack_valid_handler(hstack))
    {
        return;
    }

    struct node* curr = g_table.entries[hstack].head;
    while (curr != NULL)
    {
        struct node* prev = curr->prev;
        free(curr);
        curr = prev;
    }

    g_table.entries[hstack].head = NULL;
    g_table.entries[hstack].count = 0u;
    g_table.entries[hstack].reserved = 0;
}

void stack_free_all(void)
{
    if (g_table.entries == NULL)
    {
        return;
    }

    for (unsigned int i = 0u; i < g_table.capacity; ++i)
    {
        if (g_table.entries[i].reserved)
        {
            struct node* curr = g_table.entries[i].head;
            while (curr != NULL)
            {
                struct node* prev = curr->prev;
                free(curr);
                curr = prev;
            }
            g_table.entries[i].head = NULL;
            g_table.entries[i].count = 0u;
            g_table.entries[i].reserved = 0;
        }
    }

    free(g_table.entries);
    g_table.entries = NULL;
    g_table.capacity = 0u;
}

int stack_valid_handler(const hstack_t hstack)
{
    if (hstack < 0)
    {
        return 0;
    }
    if (g_table.entries == NULL)
    {
        return 0;
    }
    if ((unsigned int)hstack >= g_table.capacity)
    {
        return 0;
    }

    return g_table.entries[hstack].reserved ? 1 : 0;
}

unsigned int stack_size(const hstack_t hstack)
{
    if (!stack_valid_handler(hstack))
    {
        return 0u;
    }

    return g_table.entries[hstack].count;
}

void
stack_push(const hstack_t hstack, const void* data_in, const unsigned int size)
{
    if (!stack_valid_handler(hstack) || data_in == NULL || size == 0u)
    {
        return;
    }

    struct node* new_node = (struct node*)malloc(sizeof(struct node) + size);
    if (new_node == NULL)
    {
        return;
    }

    new_node->prev = g_table.entries[hstack].head;
    new_node->size = size;
    memcpy(new_node->data, data_in, size);

    g_table.entries[hstack].head = new_node;
    g_table.entries[hstack].count++;
}

unsigned int
stack_pop(const hstack_t hstack, void* data_out, const unsigned int size)
{
    if (!stack_valid_handler(hstack) || data_out == NULL || size == 0u)
    {
        return 0u;
    }

    struct node* top = g_table.entries[hstack].head;
    if (top == NULL || size < top->size)
    {
        return 0u;
    }

    unsigned int bytes_to_copy = top->size;
    memcpy(data_out, top->data, bytes_to_copy);

    g_table.entries[hstack].head = top->prev;
    g_table.entries[hstack].count--;
    free(top);

    return bytes_to_copy;
}

unsigned stack_top_size(const hstack_t hstack)
{
    if (!stack_valid_handler(hstack))
    {
        return 0u;
    }

    if (g_table.entries[hstack].head == NULL)
    {
        return 0u;
    }

    return g_table.entries[hstack].head->size;
}

hstack_t stack_get_all(void)
{
    unsigned int other_stacks_count = 0u;
    if (g_table.entries != NULL)
    {
        for (unsigned int i = 0u; i < g_table.capacity; ++i)
        {
            if (g_table.entries[i].reserved)
            {
                other_stacks_count++;
            }
        }
    }

    if (other_stacks_count == 0u)
    {
        return stack_new();
    }

    hstack_t* handlers =
        (hstack_t*)malloc(other_stacks_count * sizeof(hstack_t));
    if (handlers == NULL)
    {
        return -1;
    }

    unsigned int idx = 0u;
    for (unsigned int i = 0u; i < g_table.capacity; ++i)
    {
        if (g_table.entries[i].reserved)
        {
            handlers[idx] = (hstack_t)i;
            idx++;
        }
    }

    hstack_t new_stack = stack_new();
    if (new_stack < 0)
    {
        free(handlers);
        return -1;
    }

    for (unsigned int i = 0u; i < other_stacks_count; ++i)
    {
        unsigned int expected_size = i + 1u;
        stack_push(new_stack, &handlers[i], (unsigned int)sizeof(hstack_t));
        if (stack_size(new_stack) != expected_size)
        {
            stack_free(new_stack);
            free(handlers);
            return -1;
        }
    }

    free(handlers);
    return new_stack;
}
