#include "cstack.h"

#include <stddef.h>

#define UNUSED(VAR) (void)(VAR)

hstack_t stack_new(void)
{
    return -1;
}

void stack_free(const hstack_t hstack)
{
    UNUSED(hstack);
}

void stack_free_all(void) {}

int stack_valid_handler(const hstack_t hstack)
{
    UNUSED(hstack);
    return 0;
}


unsigned int stack_size(const hstack_t hstack)
{
    UNUSED(hstack);
    return 0;
}

void
stack_push(const hstack_t hstack, const void* data_in, const unsigned int size)
{
    UNUSED(hstack);
    UNUSED(data_in);
    UNUSED(size);
}

unsigned int
stack_pop(const hstack_t hstack, void* data_out, const unsigned int size)
{
    UNUSED(hstack);
    UNUSED(data_out);
    UNUSED(size);
    return 0;
}

unsigned stack_top_size(const hstack_t hstack)
{
    UNUSED(hstack);
    return 0;
}

hstack_t stack_get_all(void)
{
    return -1;
}
