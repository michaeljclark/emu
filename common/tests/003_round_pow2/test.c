#undef NDEBUG
#include <stdio.h>
#include <assert.h>

#include "bits.h"

void t1()
{
    assert(rupgtpow2_u32(7) == 8);
    assert(rupgtpow2_u32(8) == 8);
    assert(rupgtpow2_u32(9) == 16);
    assert(rupgepow2_u32(7) == 8);
    assert(rupgepow2_u32(8) == 16);
    assert(rupgepow2_u32(9) == 16);
    assert(rdnltpow2_u32(7) == 4);
    assert(rdnltpow2_u32(8) == 8);
    assert(rdnltpow2_u32(9) == 8);
    assert(rdnlepow2_u32(7) == 4);
    assert(rdnlepow2_u32(8) == 4);
    assert(rdnlepow2_u32(9) == 8);
}

void t2()
{
    assert(rupgtpow2_u64(7LL << 32) == 8LL << 32);
    assert(rupgtpow2_u64(8LL << 32) == 8LL << 32);
    assert(rupgtpow2_u64(9LL << 32) == 16LL << 32);
    assert(rupgepow2_u64(7LL << 32) == 8LL << 32);
    assert(rupgepow2_u64(8LL << 32) == 16LL << 32);
    assert(rupgepow2_u64(9LL << 32) == 16LL << 32);
    assert(rdnltpow2_u64(7LL << 32) == 4LL << 32);
    assert(rdnltpow2_u64(8LL << 32) == 8LL << 32);
    assert(rdnltpow2_u64(9LL << 32) == 8LL << 32);
    assert(rdnlepow2_u64(7LL << 32) == 4LL << 32);
    assert(rdnlepow2_u64(8LL << 32) == 4LL << 32);
    assert(rdnlepow2_u64(9LL << 32) == 8LL << 32);
}

void t3()
{
    assert(rupgtpow2(7) == 8);
    assert(rupgtpow2(8) == 8);
    assert(rupgtpow2(9) == 16);
    assert(rupgepow2(7) == 8);
    assert(rupgepow2(8) == 16);
    assert(rupgepow2(9) == 16);
    assert(rdnltpow2(7) == 4);
    assert(rdnltpow2(8) == 8);
    assert(rdnltpow2(9) == 8);
    assert(rdnlepow2(7) == 4);
    assert(rdnlepow2(8) == 4);
    assert(rdnlepow2(9) == 8);
}

void t4()
{
    assert(rupgtpow2(7LL << 32) == 8LL << 32);
    assert(rupgtpow2(8LL << 32) == 8LL << 32);
    assert(rupgtpow2(9LL << 32) == 16LL << 32);
    assert(rupgepow2(7LL << 32) == 8LL << 32);
    assert(rupgepow2(8LL << 32) == 16LL << 32);
    assert(rupgepow2(9LL << 32) == 16LL << 32);
    assert(rdnltpow2(7LL << 32) == 4LL << 32);
    assert(rdnltpow2(8LL << 32) == 8LL << 32);
    assert(rdnltpow2(9LL << 32) == 8LL << 32);
    assert(rdnlepow2(7LL << 32) == 4LL << 32);
    assert(rdnlepow2(8LL << 32) == 4LL << 32);
    assert(rdnlepow2(9LL << 32) == 8LL << 32);
}

int main()
{
    t1();
    t2();
    t3();
    t4();
}
