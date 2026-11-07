#include "cstack.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define UNUSED(VAR) (void)(VAR)

struct node
{
    const struct node* prev;
    unsigned int size;
    char data[];
};
typedef struct node* stack_t;

struct stack_entry
{
    int reserved; // 1 - стек занят, 0 - нет
    stack_t stack;
};
typedef struct stack_entry stack_entry_t;

struct stack_entries_table
{
    unsigned int size;
    stack_entry_t* entries;
};

static struct stack_entries_table g_table = {0u, NULL};

hstack_t stack_new(void)
{
    for (unsigned int i = 0u; i < g_table.size; ++i)
    {
        if (!g_table.entries[i].reserved)
        {
            g_table.entries[i].reserved = 1;
            g_table.entries[i].stack = NULL;
            return (hstack_t)i;
        }
    }

    unsigned int new_size = (g_table.size == 0u) ? 10u : (g_table.size + 10u);
    stack_entry_t* new_entries = (stack_entry_t*)realloc(
        g_table.entries, new_size * sizeof(stack_entry_t)
    );

    if (new_entries == NULL)
    {
        return -1;
    }

    for (unsigned int i = g_table.size; i < new_size; ++i)
    {
        new_entries[i].reserved = 0;
        new_entries[i].stack = NULL;
    }

    g_table.entries = new_entries;
    unsigned int allocated_index = g_table.size;
    g_table.size = new_size;

    g_table.entries[allocated_index].reserved = 1;
    g_table.entries[allocated_index].stack = NULL;

    return (hstack_t)allocated_index;
}

void stack_free(const hstack_t hstack)
{
    if (!stack_valid_handler(hstack))
    {
        return;
    }

    const struct node* current = g_table.entries[hstack].stack;
    while (current != NULL)
    {
        const struct node* prev = current->prev;
        free((void*)current);
        current = prev;
    }

    g_table.entries[hstack].stack = NULL;
    g_table.entries[hstack].reserved = 0;
}

void stack_free_all(void)
{
    if (g_table.entries == NULL)
    {
        return;
    }

    for (unsigned int i = 0u; i < g_table.size; ++i)
    {
        if (g_table.entries[i].reserved)
        {
            stack_free((hstack_t)i);
        }
    }

    free(g_table.entries);
    g_table.entries = NULL;
    g_table.size = 0u;
}

int stack_valid_handler(const hstack_t hstack)
{
    if (hstack < 0 || g_table.entries == NULL)
    {
        return 0;
    }
    if ((unsigned int)hstack >= g_table.size)
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

    unsigned int count = 0u;
    const struct node* current = g_table.entries[hstack].stack;
    while (current != NULL)
    {
        ++count;
        current = current->prev;
    }
    return count;
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

    new_node->prev = g_table.entries[hstack].stack;
    new_node->size = size;
    memcpy(new_node->data, data_in, size);

    g_table.entries[hstack].stack = new_node;
}

unsigned int
stack_pop(const hstack_t hstack, void* data_out, const unsigned int size)
{
    if (!stack_valid_handler(hstack) || data_out == NULL || size == 0u)
    {
        return 0u;
    }

    struct node* top = g_table.entries[hstack].stack;
    if (top == NULL || size < top->size)
    {
        return 0u;
    }

    unsigned int bytes_to_copy = top->size;
    memcpy(data_out, top->data, bytes_to_copy);

    g_table.entries[hstack].stack = (stack_t)top->prev;
    free(top);

    return bytes_to_copy;
}

unsigned int stack_top_size(const hstack_t hstack)
{
    if (!stack_valid_handler(hstack))
    {
        return 0u;
    }

    const struct node* top = g_table.entries[hstack].stack;
    if (top == NULL)
    {
        return 0u;
    }

    return top->size;
}

hstack_t stack_get_all(void)
{
    unsigned int old_table_size = g_table.size;

    hstack_t all_stack = stack_new();
    if (all_stack < 0)
    {
        return -1;
    }

    for (unsigned int i = 0u; i < old_table_size; ++i)
    {
        if (g_table.entries[i].reserved && (hstack_t)i != all_stack)
        {
            hstack_t h = (hstack_t)i;
            unsigned int prev_count = stack_size(all_stack);
            stack_push(all_stack, &h, sizeof(hstack_t));

            if (stack_size(all_stack) == prev_count)
            {
                stack_free(all_stack);
                return -1;
            }
        }
    }

    return all_stack;
}
