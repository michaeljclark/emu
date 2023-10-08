#undef NDEBUG
#include <stdio.h>
#include <assert.h>

#include "buffer.h"

void t1()
{
    array_buffer ab;
    array_buffer_init(&ab, sizeof(int), 0);
    int sum = 0, csum = 0;
    for (int i = 0; i < 256; i++) {
        sum += i;
        array_buffer_add(&ab, sizeof(int), &i);
    }
    array_buffer_resize(&ab, sizeof(int), 512);
    for (int i = 0; i < 256; i++) {
        csum += *(int*)array_buffer_get(&ab, sizeof(int), i);
    }
    assert(sum == csum);
    array_buffer_destroy(&ab);
}

int main()
{
    t1();
}
