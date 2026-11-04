#include "cstack.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_CAPACITY 10

// Stack Using Linked List
struct node
{
    const struct node* prev;
    unsigned int size;
    char data[]; // Flexible Array Member (FAM)
};
typedef struct node* stack_t;

struct stack_entry
{
    int reserved;
    stack_t stack;
    unsigned int count;
};
typedef struct stack_entry stack_entry_t;

struct stack_entries_table
{
    unsigned int size;
    stack_entry_t* entries;
};

static struct stack_entries_table g_table = {0u, NULL};

static int handler_valid(const hstack_t hstack)
{
    return hstack >= 0                            // negatives first
           && (unsigned int)hstack < g_table.size // range-check against size
           &&
           g_table.entries[hstack].reserved; // sure the slot is actually in use
}
hstack_t stack_new(void)
{
    for (unsigned int i = 0; i < g_table.size; ++i)
    {
        if (!g_table.entries[i].reserved)
        {
            g_table.entries[i].reserved = 1;
            g_table.entries[i].stack = NULL;
            return (hstack_t)i;
        }
    }

    unsigned int old_size = g_table.size;
    unsigned int new_size =
        (old_size == 0) ? INITIAL_CAPACITY
                        : old_size * 2; // alialgmass/dynamic-array-internals

    stack_entry_t* new_entries =
        realloc(g_table.entries, new_size * sizeof(stack_entry_t));
    if (new_entries == NULL)
    {
        return -1;
    }
    memset(
        new_entries + old_size, 0, (new_size - old_size) * sizeof(stack_entry_t)
    );
    g_table.entries = new_entries;
    g_table.size = new_size;

    g_table.entries[old_size].reserved = 1;
    g_table.entries[old_size].stack = NULL;

    return (hstack_t)old_size;
}
void stack_free(const hstack_t hstack)
{
    if (!handler_valid(hstack))
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
    g_table.entries[hstack].count = 0;
}

void stack_free_all(void)
{
    for (unsigned int i = 0; i < g_table.size; i++)
    {
        if (g_table.entries[i].reserved)
        {
            const struct node* current = g_table.entries[i].stack;
            while (current != NULL)
            {
                const struct node* prev = current->prev;
                free((void*)current);
                current = prev;
            }
            g_table.entries[i].stack = NULL;
            g_table.entries[i].reserved = 0;
            g_table.entries[i].count = 0;
        }
    }
    free(g_table.entries);
    g_table.entries = NULL;
    g_table.size = 0;
}

int stack_valid_handler(const hstack_t hstack)
{
    return handler_valid(hstack) ? 1 : 0;
}

unsigned int stack_size(const hstack_t hstack)
{
    if (!handler_valid(hstack))
    {
        return 0;
    }
    return g_table.entries[hstack].count;
    ;
}

void
stack_push(const hstack_t hstack, const void* data_in, const unsigned int size)
{
    if (!handler_valid(hstack) || data_in == NULL || size == 0)
    {
        return;
    }
    struct node* new_node = malloc(sizeof(struct node) + size);
    if (new_node == NULL)
    {
        return;
    }
    new_node->prev = g_table.entries[hstack].stack;
    new_node->size = size;
    memcpy(new_node->data, data_in, size);

    g_table.entries[hstack].stack = new_node;
    g_table.entries[hstack].count++;
}

unsigned int
stack_pop(const hstack_t hstack, void* data_out, const unsigned int size)
{
    if (!handler_valid(hstack) || data_out == NULL || size == 0)
    {
        return 0;
    }
    stack_t top = g_table.entries[hstack].stack;
    if (top == NULL)
    {
        return 0;
    }

    if (size < top->size)
    {
        return 0;
    }

    memcpy(data_out, top->data, top->size);
    unsigned int bytes_copied = top->size;

    g_table.entries[hstack].stack = (stack_t)top->prev;
    free(top);
    g_table.entries[hstack].count--;

    return bytes_copied;
}

unsigned stack_top_size(const hstack_t hstack)
{
    if (!handler_valid(hstack))
    {
        return 0;
    }

    stack_t top = g_table.entries[hstack].stack;
    if (top == NULL)
    {
        return 0;
    }
    return top->size;
}

hstack_t stack_get_all(void)
{
    hstack_t new_stack = stack_new();
    if (new_stack == -1)
    {
        return -1;
    }

    for (unsigned int i = 0; i < g_table.size; i++)
    {
        if (g_table.entries[i].reserved && (hstack_t)i != new_stack)
        {
            hstack_t handler = (hstack_t)i;
            unsigned int size_before = stack_size(new_stack);
            stack_push(new_stack, &handler, sizeof(hstack_t));

            if (stack_size(new_stack) != size_before + 1)
            {
                stack_free(new_stack);
                return -1;
            }
        }
    }

    return new_stack;
}
