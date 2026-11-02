#include "cstack.h"

#include <stdlib.h>
#include <string.h>

struct node;
typedef struct node* stack_t;

struct stack_entry;
typedef struct stack_entry stack_entry_t;

struct node
{
    const struct node* prev;
    unsigned int size;
    char data[];
};

struct stack_entry
{
    int reserved;
    stack_t top;
};

static stack_entry_t* g_entries = NULL;
static unsigned int g_entries_count = 0;

static stack_t* get_top(const hstack_t hstack);
static void free_stack_nodes(stack_t top);
static int find_free_slot(void);

hstack_t stack_new(void)
{
    int slot = find_free_slot();
    if (slot < 0)
    {
        return -1;
    }

    g_entries[slot].reserved = 1;
    g_entries[slot].top = NULL;
    return (hstack_t)slot;
}

void stack_free(const hstack_t hstack)
{
    if (hstack < 0 || (unsigned int)hstack >= g_entries_count)
    {
        return;
    }
    if (!g_entries[hstack].reserved)
    {
        return;
    }

    free_stack_nodes(g_entries[hstack].top);
    g_entries[hstack].top = NULL;
    g_entries[hstack].reserved = 0;
}

void stack_free_all(void)
{
    for (unsigned int i = 0; i < g_entries_count; ++i)
    {
        if (g_entries[i].reserved)
        {
            free_stack_nodes(g_entries[i].top);
            g_entries[i].top = NULL;
            g_entries[i].reserved = 0;
        }
    }

    free(g_entries);
    g_entries = NULL;
    g_entries_count = 0;
}

int stack_valid_handler(const hstack_t hstack)
{
    return get_top(hstack) != NULL;
}

unsigned int stack_size(const hstack_t hstack)
{
    stack_t* top = get_top(hstack);
    if (top == NULL)
    {
        return 0;
    }

    unsigned int count = 0;
    for (stack_t n = *top; n != NULL; n = (stack_t)n->prev)
    {
        ++count;
    }
    return count;
}

void
stack_push(const hstack_t hstack, const void* data_in, const unsigned int size)
{
    stack_t* top = get_top(hstack);
    if (top == NULL || data_in == NULL || size == 0)
    {
        return;
    }

    stack_t node = malloc(sizeof(*node) + size);
    if (node == NULL)
    {
        return;
    }

    node->prev = *top;
    node->size = size;
    memcpy(node->data, data_in, size);
    *top = node;
}

unsigned int
stack_pop(const hstack_t hstack, void* data_out, const unsigned int size)
{
    stack_t* top = get_top(hstack);
    if (top == NULL || *top == NULL || data_out == NULL || size == 0)
    {
        return 0;
    }

    stack_t node = *top;
    if (size < node->size)
    {
        return 0;
    }

    memcpy(data_out, node->data, node->size);
    unsigned int result = node->size;

    *top = (stack_t)node->prev;
    free(node);
    return result;
}

unsigned stack_top_size(const hstack_t hstack)
{
    stack_t* top = get_top(hstack);
    if (top == NULL || *top == NULL)
    {
        return 0;
    }
    return (*top)->size;
}

hstack_t stack_get_all(void)
{
    unsigned int n = 0;
    for (unsigned int i = 0; i < g_entries_count; ++i)
    {
        if (g_entries[i].reserved)
        {
            ++n;
        }
    }

    hstack_t* handlers = NULL;
    if (n > 0)
    {
        handlers = malloc(n * sizeof(*handlers));
        if (handlers == NULL)
        {
            return -1;
        }

        unsigned int j = 0;
        for (unsigned int i = 0; i < g_entries_count; ++i)
        {
            if (g_entries[i].reserved)
            {
                handlers[j++] = (hstack_t)i;
            }
        }
    }

    hstack_t result = stack_new();
    if (result < 0)
    {
        free(handlers);
        return -1;
    }

    for (unsigned int i = 0; i < n; i++)
    {
        stack_push(result, &handlers[i], (unsigned int)sizeof(handlers[i]));
        if (stack_size(result) != i + 1)
        {
            stack_free(result);
            free(handlers);
            return -1;
        }
    }

    free(handlers);
    return result;
}

static stack_t* get_top(const hstack_t hstack)
{
    if (hstack < 0 || (unsigned int)hstack >= g_entries_count)
    {
        return NULL;
    }
    if (!g_entries[hstack].reserved)
    {
        return NULL;
    }
    return &g_entries[hstack].top;
}

static void free_stack_nodes(stack_t top)
{
    while (top != NULL)
    {
        stack_t prev = (stack_t)top->prev;
        free(top);
        top = prev;
    }
}

static int find_free_slot(void)
{
    for (unsigned int i = 0; i < g_entries_count; ++i)
    {
        if (!g_entries[i].reserved)
        {
            return (int)i;
        }
    }

    unsigned int old_count = g_entries_count;
    unsigned int new_count = old_count == 0 ? 10u : old_count * 2u;

    stack_entry_t* new_entries =
        realloc(g_entries, new_count * sizeof(*new_entries));
    if (new_entries == NULL)
    {
        return -1;
    }

    for (unsigned int i = old_count; i < new_count; i++)
    {
        new_entries[i].reserved = 0;
        new_entries[i].top = NULL;
    }

    g_entries = new_entries;
    g_entries_count = new_count;
    return (int)old_count;
}
