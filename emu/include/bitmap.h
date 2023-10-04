#pragma once

#include "types.h"
#include "bits.h"

#define ULLONG_SHIFT 6
#define ULLONG_SIZE 64
#define ULLONG_MASK 63

static inline ullong minull(ullong a, ullong b) { return a < b ? a : b; }

/*
 * bitmap_scan - scan bitmap for contiguous span of zero bits
 *
 * @param s - size in bits of the bitmap
 * @param o - offset in bits to start scanning
 * @param l - length in bits of range to scan
 */
ullong bitmap_scan(ullong *x, ullong s, ullong o, ullong l)
{
    ullong v = 0, i = o, c = 0, lo = 0, ll = 0, no, nl, b0, b1;

    do {
        /* fetch next word if there are no bits */
        if (c == 0 && i < o + s) {
            v = x[(i % s) >> ULLONG_SHIFT] >> (i & ULLONG_MASK);
            c = ULLONG_SIZE - (i & ULLONG_MASK);
            i += ULLONG_SIZE - (i & ULLONG_MASK);
        }

        /* find range of used bits */
        b1 = minull(ctz(~v), c);
        v >>= b1;
        c -= b1;

        /* fetch next word if there are no bits */
        if (c == 0 && i < o + s) {
            v = x[(i % s) >> ULLONG_SHIFT] >> (i & ULLONG_MASK);
            c = ULLONG_SIZE - (i & ULLONG_MASK);
            i += ULLONG_SIZE - (i & ULLONG_MASK);
        }

        /* find range of free bits */
        b0 = minull(ctz(v), c);
        v >>= b0;
        c -= b0;

        /* create bit address */
        no = i - c - b0;
        nl = b0;

        /* merge with range in last round or emit */
        if (lo + ll == no) {
            no = lo;
            nl += ll;
        } else if (ll >= l) {
            return lo;
        }

        /* save next range */
        lo = no;
        ll = nl;
    }
    while (c > 0 || i < o + s);

    return ll >= l ? lo : (ullong)-1LL;
}

/*
 * bitmap_set - set contiguous span of bits in bitmap
 *
 * @param s - size in bits of the bitmap
 * @param o - offset in bits of range to set
 * @param l - length in bits of range to set
 * @param v - value to set (zero or one)
 */
void bitmap_set(ullong *x, ullong s, ullong o, ullong l, int v)
{
    ullong sw = (o) >> ULLONG_SHIFT, ew = (o+l-1) >> ULLONG_SHIFT;

    ullong cv = ((ullong)-1LL >> (-(llong)l & ULLONG_MASK)) << (o & ULLONG_MASK);
    ullong sv = ((ullong)-1LL << (o & ULLONG_MASK));
    ullong ev = ((ullong)-1LL >> ((-(llong)o-l) & ULLONG_MASK));

    if (v) {
        ullong w = sw;
        if (w == ew) {
            x[w] |= cv;
        } else {
            x[w++] |= sv;
            while (w < ew) x[w++] = (ullong)-1LL;
            x[w] |= ev;
        }
    } else {
        ullong w = sw;
        if (w == ew) {
            x[w] &= ~cv;
        } else {
            x[w++] &= ~sv;
            while (w < ew) x[w++] = 0LL;
            x[w] &= ~ev;
        }
    }
}
