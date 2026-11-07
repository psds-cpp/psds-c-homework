#include "cstack.h"
#include <limits.h>
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
    int exists;
    unsigned int count;
    stack_t top;
    int prev;
    int next;
};

typedef struct stack_entry stack_entry_t;

struct stack_entries_table
{
    unsigned int size;
    stack_entry_t* entries;
};

static struct stack_entries_table g_table = {0u, NULL};
static int g_free_head = -1;
static int g_active_head = -1;

static int grow_table(void)
{
    const unsigned int cap = g_table.size;
    const unsigned int max_cap = (unsigned int)INT_MAX;
    unsigned int new_cap = 0;
    stack_entry_t* ptr = NULL;

    if (cap >= max_cap)
    {
        return 0;
    }
    if (cap == 0)
    {
        new_cap = 1;
    }
    else
    {
        new_cap = (cap > max_cap / 2) ? max_cap : cap * 2;
    }
    const size_t bytes = new_cap * sizeof(stack_entry_t);
    ptr = (stack_entry_t*)realloc(g_table.entries, bytes);
    if (!ptr)
    {
        return 0;
    }
    g_table.entries = ptr;

    for (unsigned int i = new_cap; i > cap; --i)
    {
        stack_entry_t* entry = &g_table.entries[i - 1];
        entry->exists = 0;
        entry->count = 0;
        entry->top = NULL;
        entry->prev = -1;
        entry->next = g_free_head;
        g_free_head = (int)(i - 1);
    }
    g_table.size = new_cap;
    return 1;
}

static void link_active(const hstack_t hstack)
{
    stack_entry_t* entry = &g_table.entries[hstack];
    entry->prev = -1;
    entry->next = g_active_head;
    if (g_active_head != -1)
    {
        g_table.entries[g_active_head].prev = hstack;
    }
    g_active_head = hstack;
}

static void unlink_active(const hstack_t hstack)
{
    const stack_entry_t* entry = &g_table.entries[hstack];
    if (entry->prev != -1)
    {
        g_table.entries[entry->prev].next = entry->next;
    }
    else
    {
        g_active_head = entry->next;
    }
    if (entry->next != -1)
    {
        g_table.entries[entry->next].prev = entry->prev;
    }
}

static void release_nodes(stack_entry_t* entry)
{
    while (entry->top)
    {
        stack_t prev = entry->top->prev;
        free(entry->top);
        entry->top = prev;
    }
    entry->count = 0;
}

static int
push_node(const hstack_t hstack, const void* data_in, const unsigned int size)
{
    stack_entry_t* entry = &g_table.entries[hstack];
    stack_t node = (stack_t)malloc(sizeof(struct node) + size);

    if (!node)
    {
        return 0;
    }
    node->prev = entry->top;
    node->size = size;
    memcpy(node->data, data_in, size);
    entry->top = node;
    ++entry->count;
    return 1;
}

hstack_t stack_new(void)
{
    if (g_free_head == -1 && !grow_table())
    {
        return -1;
    }
    const hstack_t hstack = g_free_head;
    stack_entry_t* entry = &g_table.entries[hstack];
    g_free_head = entry->next;
    entry->exists = 1;
    entry->count = 0;
    entry->top = NULL;
    link_active(hstack);
    return hstack;
}

int stack_valid_handler(const hstack_t hstack)
{
    if (hstack < 0 || (unsigned int)hstack >= g_table.size)
    {
        return 0;
    }
    return g_table.entries[hstack].exists ? 1 : 0;
}

void stack_free(const hstack_t hstack)
{
    if (!stack_valid_handler(hstack))
    {
        return;
    }
    stack_entry_t* entry = &g_table.entries[hstack];
    release_nodes(entry);
    unlink_active(hstack);
    entry->exists = 0;
    entry->prev = -1;
    entry->next = g_free_head;
    g_free_head = hstack;
}

void stack_free_all(void)
{
    if (g_active_head == -1)
    {
        return;
    }
    for (hstack_t hstack = g_active_head; hstack != -1;
         hstack = g_table.entries[hstack].next)
    {
        release_nodes(&g_table.entries[hstack]);
    }
    free(g_table.entries);
    g_table.entries = NULL;
    g_table.size = 0u;
    g_free_head = -1;
    g_active_head = -1;
}

unsigned int stack_size(const hstack_t hstack)
{
    return stack_valid_handler(hstack) ? g_table.entries[hstack].count : 0u;
}

void
stack_push(const hstack_t hstack, const void* data_in, const unsigned int size)
{
    if (size == 0 || !data_in || !stack_valid_handler(hstack))
    {
        return;
    }
    (void)push_node(hstack, data_in, size);
}

unsigned int stack_top_size(const hstack_t hstack)
{
    if (!stack_valid_handler(hstack) || !g_table.entries[hstack].top)
    {
        return 0u;
    }
    return g_table.entries[hstack].top->size;
}

unsigned int
stack_pop(const hstack_t hstack, void* data_out, const unsigned int size)
{
    const unsigned int top_size = stack_top_size(hstack);
    if (top_size == 0u || !data_out || size < top_size)
    {
        return 0u;
    }
    stack_entry_t* entry = &g_table.entries[hstack];
    stack_t node = entry->top;
    memcpy(data_out, node->data, top_size);
    entry->top = node->prev;
    --entry->count;
    free(node);
    return top_size;
}

hstack_t stack_get_all(void)
{
    const hstack_t result = stack_new();
    if (result < 0)
    {
        return -1;
    }
    for (hstack_t hstack = g_active_head; hstack != -1;
         hstack = g_table.entries[hstack].next)
    {
        if (hstack == result)
        {
            continue;
        }
        if (!push_node(result, &hstack, (unsigned int)sizeof(hstack_t)))
        {
            stack_free(result);
            return -1;
        }
    }
    return result;
}
