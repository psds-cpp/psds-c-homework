#include "cstack.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

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
    stack_t stack;
};
typedef struct stack_entry stack_entry_t;

struct stack_entries_table
{
    unsigned int size;
    stack_entry_t* entries;
};

struct stack_entries_table g_table = {0u, NULL};

static stack_entry_t* get_entry(const hstack_t id)
{
    if (id < 0 || (unsigned int)id >= g_table.size ||
        g_table.entries[id].reserved == 0)
    {
        return NULL;
    }
    return &g_table.entries[id];
}

static int realloc_table(void)
{
    unsigned int new_size = g_table.size == 0 ? 2 : g_table.size * 2;
    stack_entry_t* new_entries =
        realloc(g_table.entries, new_size * sizeof *new_entries);
    if (!new_entries)
    {
        return 0;
    }
    g_table.entries = new_entries;
    for (unsigned int i = g_table.size; i < new_size; ++i)
    {
        g_table.entries[i].reserved = 0;
        g_table.entries[i].stack = NULL;
    }
    g_table.size = new_size;
    return 1;
}

hstack_t stack_new(void)
{
    stack_t head = malloc(sizeof *head);
    if (!head)
    {
        return -1;
    }
    head->prev = NULL;
    head->size = 0;

    unsigned int i = 0;
    while (i < g_table.size && g_table.entries[i].reserved != 0)
    {
        ++i;
    }
    if (i == g_table.size && !realloc_table())
    {
        free(head);
        return -1;
    }
    g_table.entries[i].reserved = 1;
    g_table.entries[i].stack = head;
    return (hstack_t)i;
}

void stack_free(const hstack_t hstack)
{
    stack_entry_t* entry = get_entry(hstack);
    if (!entry)
    {
        return;
    }
    stack_t cur = entry->stack;
    while (cur != NULL)
    {
        stack_t prev = cur->prev;
        free(cur);
        cur = prev;
    }
    entry->stack = NULL;
    entry->reserved = 0;
}

void stack_free_all(void)
{
    for (unsigned int i = 0; i < g_table.size; ++i)
    {
        if (g_table.entries[i].reserved != 0)
        {
            stack_free((hstack_t)i);
        }
    }
    free(g_table.entries);
    g_table.entries = NULL;
    g_table.size = 0;
}

int stack_valid_handler(const hstack_t hstack)
{
    return get_entry(hstack) != NULL;
}

unsigned int stack_size(const hstack_t hstack)
{
    stack_entry_t* entry = get_entry(hstack);
    if (!entry)
    {
        return 0;
    }
    unsigned int count = 0;
    for (stack_t cur = entry->stack->prev; cur != NULL; cur = cur->prev)
    {
        ++count;
    }
    return count;
}

void
stack_push(const hstack_t hstack, const void* data_in, const unsigned int size)
{
    stack_entry_t* entry = get_entry(hstack);
    if (!entry)
    {
        return;
    }
    if (size == 0 || data_in == NULL)
    {
        return;
    }
    stack_t n = malloc(sizeof(struct node) + size);
    if (!n)
    {
        return;
    }
    n->size = size;
    memcpy(n->data, data_in, size);
    n->prev = entry->stack->prev;
    entry->stack->prev = n;
}

unsigned int
stack_pop(const hstack_t hstack, void* data_out, const unsigned int size)
{
    stack_entry_t* entry = get_entry(hstack);
    if (!entry || entry->stack->prev == NULL)
    {
        return 0;
    }
    stack_t top = entry->stack->prev;
    if (data_out == NULL || size < top->size)
    {
        return 0;
    }
    memcpy(data_out, top->data, top->size);
    entry->stack->prev = top->prev;
    unsigned int result = top->size;
    free(top);
    return result;
}

unsigned int stack_top_size(const hstack_t hstack)
{
    stack_entry_t* entry = get_entry(hstack);
    if (!entry || entry->stack->prev == NULL)
    {
        return 0;
    }
    return entry->stack->prev->size;
}

hstack_t stack_get_all(void)
{
    hstack_t all = stack_new();
    if (all < 0)
    {
        return -1;
    }
    for (unsigned int i = 0; i < g_table.size; ++i)
    {
        if (g_table.entries[i].reserved != 0 && (hstack_t)i != all)
        {
            hstack_t h = (hstack_t)i;
            stack_push(all, &h, sizeof(h));
        }
    }
    return all;
}

