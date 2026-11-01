#include "cstack.h"

#include <stddef.h>

#include <string.h>

#include <stdlib.h>

#define UNUSED(VAR) (void)(VAR)

struct node 
{
    struct node* prev;
    unsigned int size;
    char data[];
};

typedef struct node* stack_t;

// указатель на стек
struct stack_entry
{
    int reserved;
    stack_t stack;
};

typedef struct stack_entry stack_entry_t;

// массив указателей на стеки
struct stack_entries_table
{
    unsigned int size;
    stack_entry_t* entries;
};

struct stack_entries_table s_table = {0u, NULL}; 

hstack_t stack_new(void)
{
    if (s_table.size == 0u) // создание таблицы
    {
        s_table.entries = calloc(10, sizeof(stack_entry_t));
        if (s_table.entries == NULL)
        {
            return -1;
        }
        s_table.size = 10;
    }
    for (unsigned int i = 0; i < s_table.size; ++i) // поиск свободного хэндлера
    {
        if (s_table.entries[i].reserved == 0)
        {
            s_table.entries[i].reserved = 1; 
            s_table.entries[i].stack = NULL; 
            return i;
        }
    }
    unsigned int new_size = s_table.size * 2; // выделение доп. памяти для таблицы
    stack_entry_t* tmp = realloc(s_table.entries, new_size * sizeof(stack_entry_t));
    if (tmp == NULL)
    {
        return -1;
    }
    s_table.entries = tmp;
    for (unsigned int i = s_table.size; i < new_size; ++i) // обнуление новых данных
    {
        s_table.entries[i].reserved = 0;
        s_table.entries[i].stack = NULL;
    }
    hstack_t hstack = s_table.size; // выделение места под стек в новом пространстве
    s_table.size = new_size;
    s_table.entries[hstack].reserved = 1;
    s_table.entries[hstack].stack = NULL;
    return hstack;
}

void stack_free(const hstack_t hstack)
{
    if (stack_valid_handler(hstack) == 0)
    {
        return;
    }
    struct node* node = s_table.entries[hstack].stack;
    while (node != NULL)
    {
        stack_t prev = node->prev;
        free(node);
        node = prev;
    }
    s_table.entries[hstack].reserved = 0;
    s_table.entries[hstack].stack = NULL;
}

void stack_free_all(void) 
{
    for (unsigned int i = 0; i < s_table.size; ++i){
        stack_free(i);
    }
    free(s_table.entries);
    s_table.entries = NULL;
    s_table.size = 0;
}

int stack_valid_handler(const hstack_t hstack)
{
    if (hstack >= 0 && hstack < s_table.size && s_table.entries[hstack].reserved == 1)
    {
        return 1;
    }
    return 0;
}

unsigned int stack_size(const hstack_t hstack)
{
    if (stack_valid_handler(hstack) == 0)
    {
        return 0;
    }
    unsigned int size = 0;
    stack_t node = s_table.entries[hstack].stack;
    while (node != NULL)
    {
        ++size;
        node = node->prev;
    }
    return size;
}

void
stack_push(const hstack_t hstack, const void* data_in, const unsigned int size)
{
    if (stack_valid_handler(hstack) == 0 || data_in == NULL || size == 0)
    {
        return;
    }
    stack_t node = malloc(sizeof(*node) + size);
    if (node == NULL)
    {
        return;
    }
    node->prev = s_table.entries[hstack].stack;
    node->size = size;
    memcpy(node->data, data_in, size);
    s_table.entries[hstack].stack = node;
}

unsigned int
stack_pop(const hstack_t hstack, void* data_out, const unsigned int size)
{
    if (stack_valid_handler(hstack) == 0 || data_out == NULL || size == 0)
    {
        return 0;
    }
    stack_t node = s_table.entries[hstack].stack;
    if (node == NULL || size < node->size)
    {
        return 0;
    }
    memcpy(data_out, node->data, node->size);
    unsigned int old_size = node->size;
    s_table.entries[hstack].stack = node->prev;
    free(node);
    return old_size;
}

unsigned stack_top_size(const hstack_t hstack)
{
    if (stack_valid_handler(hstack) == 0 || s_table.entries[hstack].stack == NULL)
    {
        return 0;
    }
    return s_table.entries[hstack].stack->size;
}

hstack_t stack_get_all(void)
{
    hstack_t all_stack = stack_new();
    if (all_stack == -1)
    {
        return -1;
    }
    for (unsigned int i = 0; i < s_table.size; ++i)
    {
        if (stack_valid_handler(i) && i != all_stack)
        {
            unsigned int old_size = stack_size(all_stack);
            stack_push(all_stack, &i, sizeof(hstack_t));
            if (stack_size(all_stack) == old_size)
            {
                stack_free(all_stack);
                return -1;
            }
        } 
    }
    return all_stack;
}