#include <stdio.h>
#include <assert.h>

#include "bitmap.h"

void print_bitmap(const char *fmt, ullong *x, ullong s)
{
    ullong w = s >> ULLONG_SHIFT;
    for (ullong i = w-1; i < w; i--) {
        printf(fmt, x[i]);
    }
    printf("\n");
}

void test_bitmap_scan(const char *fmt, ullong *x, ullong s, ullong o, ullong l)
{
    ullong r = bitmap_scan(x, s, o, l);
    printf("bitmap_scan %-3lld %-3lld %-3lld %-3lld ", s, o, l, r);
    print_bitmap(fmt, x, s);
}

void t1()
{
    ullong t[2] = { 0x0000000000ffff00LL, 0x00FFFF0000000000LL };
    test_bitmap_scan("%016llx ", t, 128, 0, 8);
    test_bitmap_scan("%016llx ", t, 128, 0, 16);
    test_bitmap_scan("%016llx ", t, 128, 32, 8);
}

void t2()
{
    ullong t[2] = { 0x0000000000ffff00LL, 0x00FFFF0000000000LL };
    test_bitmap_scan("%016llx ", t, 128, 0, 80);
    test_bitmap_scan("%016llx ", t, 128, 0, 96);
}

int main()
{
    t1();
    t2();
}
