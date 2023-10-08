/*
 * sha3.c
 *
 * an implementation of Secure Hash Algorithm 3 (Keccak) based on:
 * The Keccak SHA-3 submission. Submission to NIST (Round 3), 2011
 * by Guido Bertoni, Joan Daemen, Michaël Peeters and Gilles Van Assche
 *
 * Copyright (c) 2013, Aleksey Kravchenko <rhash.admin@gmail.com>
 * Copyright (c) 2019, Michael Clark <michaeljclark@mac.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "string.h"

#include "bytes.h"
#include "sha3.h"

/* macro to expand Keccak 7-term GF(2) round constant:
 *
 * (ax^63 + bx^31 + cx^15 + dx^7 + ex^3 + fx + g)
 *
 * K(c) -> forall (b in 0...6) |= c[b] << (1 << b) ;
 */
#define T(c,b) 0b##c##ull>>b<<63>>(64-(1 << b))
#define K(c) T(c,0)|T(c,1)|T(c,2)|T(c,3)|T(c,4)|T(c,5)|T(c,6)

/* SHA3 (Keccak) constants for 24 rounds */
static uint64_t keccak_round_constants[24] = {
    K(0000001), K(0011010), K(1011110), K(1110000),
    K(0011111), K(0100001), K(1111001), K(1010101),
    K(0001110), K(0001100), K(0110101), K(0100110),
    K(0111111), K(1001111), K(1011101), K(1010011),
    K(1010010), K(1001000), K(0010110), K(1100110),
    K(1111001), K(1011000), K(0100001), K(1110100),
};

/* rotate pattern is lifted by compiler into rotate instructions */
static inline uint64_t rol64(uint64_t x, int d)
{
    return (x << d) | (x >> (64-d));
}

/* Keccak theta() transformation */
static void keccak_theta(uint64_t A[25])
{
    uint64_t C[5] = {
        A[0] ^ A[5] ^ A[10] ^ A[15] ^ A[20],
        A[1] ^ A[6] ^ A[11] ^ A[16] ^ A[21],
        A[2] ^ A[7] ^ A[12] ^ A[17] ^ A[22],
        A[3] ^ A[8] ^ A[13] ^ A[18] ^ A[23],
        A[4] ^ A[9] ^ A[14] ^ A[19] ^ A[24]
    };

    uint64_t D[5] = {
        rol64(C[1], 1) ^ C[4],
        rol64(C[2], 1) ^ C[0],
        rol64(C[3], 1) ^ C[1],
        rol64(C[4], 1) ^ C[2],
        rol64(C[0], 1) ^ C[3]
    };

    for (size_t i = 0; i < 25; i += 5) {
        A[i + 0] ^= D[0];
        A[i + 1] ^= D[1];
        A[i + 2] ^= D[2];
        A[i + 3] ^= D[3];
        A[i + 4] ^= D[4];
    }
}

/* Keccak rho() transformation */
static void keccak_rho(uint64_t A[25])
{
    A[ 1] = rol64(A[ 1],  1);
    A[ 2] = rol64(A[ 2], 62);
    A[ 3] = rol64(A[ 3], 28);
    A[ 4] = rol64(A[ 4], 27);
    A[ 5] = rol64(A[ 5], 36);
    A[ 6] = rol64(A[ 6], 44);
    A[ 7] = rol64(A[ 7],  6);
    A[ 8] = rol64(A[ 8], 55);
    A[ 9] = rol64(A[ 9], 20);
    A[10] = rol64(A[10],  3);
    A[11] = rol64(A[11], 10);
    A[12] = rol64(A[12], 43);
    A[13] = rol64(A[13], 25);
    A[14] = rol64(A[14], 39);
    A[15] = rol64(A[15], 41);
    A[16] = rol64(A[16], 45);
    A[17] = rol64(A[17], 15);
    A[18] = rol64(A[18], 21);
    A[19] = rol64(A[19],  8);
    A[20] = rol64(A[20], 18);
    A[21] = rol64(A[21],  2);
    A[22] = rol64(A[22], 61);
    A[23] = rol64(A[23], 56);
    A[24] = rol64(A[24], 14);
}

/* Keccak pi() transformation */
static void keccak_pi(uint64_t A[25])
{
    uint64_t A1;
    A1 = A[1];
    A[ 1] = A[ 6];
    A[ 6] = A[ 9];
    A[ 9] = A[22];
    A[22] = A[14];
    A[14] = A[20];
    A[20] = A[ 2];
    A[ 2] = A[12];
    A[12] = A[13];
    A[13] = A[19];
    A[19] = A[23];
    A[23] = A[15];
    A[15] = A[ 4];
    A[ 4] = A[24];
    A[24] = A[21];
    A[21] = A[ 8];
    A[ 8] = A[16];
    A[16] = A[ 5];
    A[ 5] = A[ 3];
    A[ 3] = A[18];
    A[18] = A[17];
    A[17] = A[11];
    A[11] = A[ 7];
    A[ 7] = A[10];
    A[10] = A1;
    /* note: A[ 0] is left as is */
}

static inline void ChiStep(uint64_t A[25], size_t i)
{
    uint64_t C[5];
    C[0] = A[0 + i] ^ ~A[1 + i] & A[2 + i];
    C[1] = A[1 + i] ^ ~A[2 + i] & A[3 + i];
    C[2] = A[2 + i] ^ ~A[3 + i] & A[4 + i];
    C[3] = A[3 + i] ^ ~A[4 + i] & A[0 + i];
    C[4] = A[4 + i] ^ ~A[0 + i] & A[1 + i];
    A[0 + i] = C[0];
    A[1 + i] = C[1];
    A[2 + i] = C[2];
    A[3 + i] = C[3];
    A[4 + i] = C[4];
}

/* Keccak chi() transformation */
static void keccak_chi(uint64_t A[25])
{
    ChiStep(A,0);
    ChiStep(A,5);
    ChiStep(A,10);
    ChiStep(A,15);
    ChiStep(A,20);
}

static void keccak_iota(uint64_t A[25], size_t round)
{
    /* apply iota(state, round) */
    A[0] ^= keccak_round_constants[round];
}

static void keccak_permutation(uint64_t A[25])
{
    for (size_t round = 0; round < 24; round++)
    {
        keccak_theta(A);
        keccak_rho(A);
        keccak_pi(A);
        keccak_chi(A);
        keccak_iota(A, round);
    }
}

static void keccak_transform(sha3_ctx* ctx, const unsigned char *buf)
{
    size_t block_size = ctx->block_size;
    for (size_t i = 0; i < block_size/8; i++)
    {
        ctx->state[i] ^= le64(((uint64_t*)buf)[i]);
    }
    keccak_permutation(ctx->state);
}

static void keccak_init(sha3_ctx* ctx, unsigned bits)
{
    /* NB: The Keccak capacity parameter = bits * 2 */
    unsigned rate = 1600 - bits * 2;

    memset(ctx, 0, sizeof(sha3_ctx));
    ctx->block_size = rate / 8;
    //assert(rate <= 1600 && (rate % 64) == 0);
}

void sha3_224_init(sha3_ctx* ctx) { keccak_init(ctx, 224); }
void sha3_256_init(sha3_ctx* ctx) { keccak_init(ctx, 256); }
void sha3_384_init(sha3_ctx* ctx) { keccak_init(ctx, 384); }
void sha3_512_init(sha3_ctx* ctx) { keccak_init(ctx, 512); }

void shake128_init(shake_ctx* ctx) { keccak_init(ctx, 128); }
void shake256_init(shake_ctx* ctx) { keccak_init(ctx, 256); }

/* Update SHA-3 hash internal state with msg bytes */
void sha3_update(sha3_ctx* ctx, const void *data, size_t nbytes)
{
    sha3_update_bits(ctx, data, nbytes << 3);
}

/* Update SHA-3 hash internal state with msg bytes */
void sha3_update_bits(sha3_ctx* ctx, const void *data, size_t nbits)
{
    size_t len = (nbits + 7) >> 3;
    size_t block_size = ctx->block_size;
    size_t len_buf = (ctx->size >> 3) % block_size;

    ctx->size += nbits;

    /* process blocks as they get full */
    if (len_buf > 0) {
        size_t left = block_size - len_buf;
        if (len < left) {
            left = len;
        }
        memcpy(len_buf + ctx->buf, data, left);
        len_buf = (len_buf + left) % block_size;
        len -= left;
        data = ((const char *)data + left);
        if (len_buf > 0) {
            return;
        }
        keccak_transform(ctx, ctx->buf);
    }
    while (len >= block_size) {
        keccak_transform(ctx, data);
        data = ((const char *)data + block_size);
        len -= block_size;
    }
    if (len > 0) {
        memcpy(ctx->buf, data, len);
    }
}

static inline void crypto_memcpy_le64(char *dest, const char *src, const size_t length)
{
    /* copy 64-bit host integers to little endian destination */
    const size_t tail = length & 7;
    const char *end = dest + length - tail;
    while (dest < end){
        *(uint64_t*)dest = le64(*(uint64_t*)src);
        src += sizeof(uint64_t);
        dest += sizeof(uint64_t);
    }
    /* copy tail bytes who may need their order swapping */
    if (tail) {
        uint64_t bits = le64(*(uint64_t*)src);
        for (size_t i = 0; i < tail; i++) {
            *dest++ = (char)(bits >> (i << 3));
        }
    }
}

/* Finalize SHA-3 hash and copy to result buffer */
void sha3_final(sha3_ctx* ctx, unsigned char* result)
{
    size_t digest_length = 100 - ctx->block_size / 2;
    size_t block_size = ctx->block_size;
    size_t len = (ctx->size >> 3) % block_size;
    size_t rbits = ctx->size & 7;

    /* Pad with 0x06, then zeroes, then 0x80 */
    memset((char*)ctx->buf + len, 0, block_size - len);
    ((char*)ctx->buf)[len] |= (0x06 << rbits);
    ((char*)ctx->buf)[block_size - 1] |= 0x80;

    /* process final block */
    keccak_transform(ctx, ctx->buf);

    /* copy output */
    crypto_memcpy_le64(result, (const char*)ctx->state, digest_length);
}

void shake_absorb_bits(shake_ctx* ctx, const void *data, size_t nbits)
{
    sha3_update_bits(ctx, data, nbits);
}

void shake_final(shake_ctx* ctx)
{
    size_t block_size = ctx->block_size;
    size_t len = (ctx->size >> 3) % block_size;
    size_t rbits = ctx->size & 7;

    /* Pad with 0x1F, then zeroes, then 0x80 */
    memset((char*)ctx->buf + len, 0, block_size - len);
    ((char*)ctx->buf)[len] |= (0x1F << rbits);
    ((char*)ctx->buf)[block_size - 1] |= 0x80;

    /* process final block */
    keccak_transform(ctx, ctx->buf);
}

void shake_squeeze_bits(shake_ctx* ctx, unsigned char* result, size_t nbits)
{
    size_t noctets = nbits >> 3;
    while (noctets > 0) {
        size_t chunk_len = noctets < ctx->block_size ? noctets : ctx->block_size;
        crypto_memcpy_le64(result, (const char*)ctx->state, chunk_len);
        noctets -= chunk_len;
        if (noctets == 0) break;
        keccak_permutation(ctx->state);
    }
}
