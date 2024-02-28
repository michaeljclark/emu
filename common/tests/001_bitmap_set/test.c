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

void test_bitmap_set(const char *fmt, ullong *x, ullong s, ullong o, ullong l, int v)
{
    bitmap_set(x, s, o, l, v);
    printf("bitmap_set %-3lld %-3lld %-3lld ", s, o, l);
    print_bitmap(fmt, x, s);
}

void t1()
{
    ullong t[1] = { 0 };
    test_bitmap_set("%016llx ", t, 64, 24, 16, 1);
}

void t2()
{
    ullong t[2] = { 0 };
    test_bitmap_set("%016llx ", t, 128, 56, 16, 1);
}

void t3()
{
    ullong t[3] = { 0 };
    test_bitmap_set("%016llx ", t, 192, 0, 8, 1);
    test_bitmap_set("%016llx ", t, 192, 184, 8, 1);
    test_bitmap_set("%016llx ", t, 192, 56, 80, 1);
}

void t4()
{
    ullong t[1] = { (ullong)-1LL };
    test_bitmap_set("%016llx ", t, 64, 24, 16, 0);
}

void t5()
{
    ullong t[2] = { (ullong)-1LL , (ullong)-1LL  };
    test_bitmap_set("%016llx ", t, 128, 56, 16, 0);
}

void t6()
{
    ullong t[3] = { (ullong)-1LL , (ullong)-1LL , (ullong)-1LL };
    test_bitmap_set("%016llx ", t, 192, 0, 8, 0);
    test_bitmap_set("%016llx ", t, 192, 184, 8, 0);
    test_bitmap_set("%016llx ", t, 192, 56, 80, 0);
}

int main()
{
    t1();
    t2();
    t3();
    t4();
    t5();
    t6();
}
