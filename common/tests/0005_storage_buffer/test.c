#undef NDEBUG
#include <stdio.h>
#include <assert.h>

#include "buffer.h"

void t1()
{
    storage_buffer sb;
    storage_buffer_init(&sb, 0);
    int sum = 0, csum = 0;
    for (int i = 0; i < 256; i++) {
        sum += i;
        size_t o = storage_buffer_alloc(&sb, sizeof(int), _Alignof(int));
        *(int*)storage_buffer_get(&sb, o) = i;
    }
    storage_buffer_resize(&sb, 512 * sizeof(int));
    for (int i = 0; i < 256; i++) {
        csum += *(int*)storage_buffer_get(&sb, i * sizeof(int));
    }
    assert(sum == csum);
    storage_buffer_destroy(&sb);
}

int main()
{
    t1();
}
