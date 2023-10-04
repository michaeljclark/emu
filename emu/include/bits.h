#pragma once

#include "types.h"
#if defined (_MSC_VER)
#include <intrin.h>
#endif

#define _clz_defined 1
#define _ctz_defined 2
#define _popcnt_defined 4

#if defined (__GNUC__)
static inline uint clz_u32(uint val) { return val == 0 ? 32 : __builtin_clz(val); }
static inline uint clz_u64(ullong val) { return val == 0 ? 64 : __builtin_clzll(val); }
static inline uint ctz_u32(uint val) { return val == 0 ? 32 : __builtin_ctz(val); }
static inline uint ctz_u64(ullong val) { return val == 0 ? 64 : __builtin_ctzll(val); }
static inline uint popcnt_u32(uint val) { return val == 0 ? 32 : __builtin_popcount(val); }
static inline uint popcnt_u64(ullong val) { return val == 0 ? 64 : __builtin_popcountll(val); }
#define _bits_defined (_clz_defined | _ctz_defined | _popcnt_defined)
#elif defined (_MSC_VER) && defined (_M_X64)
static inline uint clz_u32(uint val) { return (int)_lzcnt_u32(val); }
static inline uint clz_u64(ullong val) { return (int)_lzcnt_u64(val); }
static inline uint ctz_u32(uint val) { return (int)_tzcnt_u32(val); }
static inline uint ctz_u64(ullong val) { return (int)_tzcnt_u64(val); }
static inline uint popcnt_u32(uint val) { return (int)__popcnt(val); }
static inline uint popcnt_u64(ullong val) { return (int)__popcnt64(val); }
#define _bits_defined (_clz_defined | _ctz_defined | _popcnt_defined)
#elif defined (_MSC_VER) && defined (_M_IX86)
static inline uint clz_u32(uint val) { uint long count; return val == 0 ? 32 : (_BitScanReverse(&count, val) ^ 31); }
static inline uint clz_u64(ullong val) { uint long count; return val == 0 ? 64 : (_BitScanReverse64(&count, val) ^ 63); }
static inline uint ctz_u32(uint val) { uint long count; return val == 0 ? 32 :_BitScanForward(&count, val); }
static inline uint ctz_u64(ullong val) { uint long count; return val == 0 ? 64 : _BitScanForward64(&count, val); }
#define _bits_defined (_clz_defined | _ctz_defined)
#else
#define _bits_defined 0
#endif

/*
 * algorithms from stanford bit twiddling hacks
 */

#if (_bits_defined & _popcnt_defined) != _popcnt_defined
static inline uint popcnt_u32(uint val)
{
    val = (val & 0x55555555) + ((val >> 1) & 0x55555555);
    val = (val & 0x33333333) + ((val >> 2) & 0x33333333);
    val = (val & 0x0F0F0F0F) + ((val >> 4) & 0x0F0F0F0F);
    val = (val & 0x00FF00FF) + ((val >> 8) & 0x00FF00FF);
    val = (val & 0x0000FFFF) + ((val >>16) & 0x0000FFFF);
    return (uint)val;
}
static inline uint popcnt_u64(ullong val)
{
    val = (val & 0x5555555555555555ULL) + ((val >>  1) & 0x5555555555555555ULL);
    val = (val & 0x3333333333333333ULL) + ((val >>  2) & 0x3333333333333333ULL);
    val = (val & 0x0F0F0F0F0F0F0F0FULL) + ((val >>  4) & 0x0F0F0F0F0F0F0F0FULL);
    val = (val & 0x00FF00FF00FF00FFULL) + ((val >>  8) & 0x00FF00FF00FF00FFULL);
    val = (val & 0x0000FFFF0000FFFFULL) + ((val >> 16) & 0x0000FFFF0000FFFFULL);
    return (uint)((uint)(val) + (uint)(val >> 32));
}
#endif

#if (_bits_defined & _clz_defined) != _clz_defined
static inline uint clz_u32(uint x)
{
    x = x | (x >> 1);
    x = x | (x >> 2);
    x = x | (x >> 4);
    x = x | (x >> 8);
    x = x | (x >>16);
    return popcnt_u32(~x);
}

static inline uint clz_u64(ullong x)
{
    x = x | (x >> 1);
    x = x | (x >> 2);
    x = x | (x >> 4);
    x = x | (x >> 8);
    x = x | (x >>16);
    x = x | (x >>32);
    return popcnt_u64(~x);
}
#endif

#if (_bits_defined & _ctz_defined) != _ctz_defined
static inline uint ctz_u32(uint v)
{
    uint c = 32;
    v &= -(int)v;
    if (v) c--;
    if (v & 0x0000FFFF) c -= 16;
    if (v & 0x00FF00FF) c -= 8;
    if (v & 0x0F0F0F0F) c -= 4;
    if (v & 0x33333333) c -= 2;
    if (v & 0x55555555) c -= 1;
    return c;
}

static inline uint ctz_u64(ullong v)
{
    uint c = 64;
    v &= -(llong)v;
    if (v) c--;
    if (v & 0x00000000FFFFFFFFULL) c -= 32;
    if (v & 0x0000FFFF0000FFFFULL) c -= 16;
    if (v & 0x00FF00FF00FF00FFULL) c -= 8;
    if (v & 0x0F0F0F0F0F0F0F0FULL) c -= 4;
    if (v & 0x3333333333333333ULL) c -= 2;
    if (v & 0x5555555555555555ULL) c -= 1;
    return c;
}
#endif

static inline uint ispow2_u32(uint v) { return v && !(v & (v-1)); }
static inline uint ispow2_u64(ullong v) { return v && !(v & (v-1)); }

/* C11 generics */
#if __STDC_VERSION__ >= 201112L
#define clz(X) _Generic((X), uint: clz_u32, int: clz_u32, ullong: clz_u64, llong: clz_u64)(X)
#define ctz(X) _Generic((X), uint: ctz_u32, int: ctz_u32, ullong: ctz_u64, llong: ctz_u64)(X)
#define popcnt(X) _Generic((X), uint: ctz_u32, int: popcnt_u32, ullong: ctz_u64, llong: popcnt_u64)(X)
#define ispow2(X) _Generic((X), uint: ispow2_u32, int: ispow2_u32, ullong: ispow2_u64, llong: ispow2_u64)(X)
#elif defined __cplusplus
/* C++ template specializations */
template <typename T> uint clz(T v);
template <typename T> uint ctz(T v);
template <typename T> uint popcnt(T v);
template <typename T> uint ispow2(T v);
template <> uint clz<int>(int v) { return clz_u32(v); }
template <> uint ctz<int>(int v) { return ctz_u32(v); }
template <> uint popcnt<int>(int v) { return popcnt_u32(v); }
template <> uint ispow2<int>(int v) { return ispow2_u32(v); }
template <> uint clz<uint>(uint v) { return clz_u32(v); }
template <> uint ctz<uint>(uint v) { return ctz_u32(v); }
template <> uint popcnt<uint>(uint v) { return popcnt_u32(v); }
template <> uint ispow2<uint>(uint v) { return ispow2_u32(v); }
template <> uint clz<llong>(llong v) { return clz_u64(v); }
template <> uint ctz<llong>(llong v) { return ctz_u64(v); }
template <> uint popcnt<llong>(llong v) { return popcnt_u64(v); }
template <> uint ispow2<llong>(llong v) { return ispow2_u64(v); }
template <> uint clz<ullong>(ullong v) { return clz_u64(v); }
template <> uint ctz<ullong>(ullong v) { return ctz_u64(v); }
template <> uint popcnt<ullong>(ullong v) { return popcnt_u64(v); }
template <> uint ispow2<ullong>(ullong v) { return ispow2_u64(v); }
#endif
