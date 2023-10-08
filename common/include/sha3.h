#pragma once

#include "stdint.h"
#include "stddef.h"

#ifdef __cplusplus
extern "C" {
#endif

#define sha3_224_hash_size  28
#define sha3_256_hash_size  32
#define sha3_384_hash_size  48
#define sha3_512_hash_size  64
#define sha3_max_permutation_words 25
#define sha3_max_permutation_size 200

/**
 * SHA3 Algorithm context.
 */
typedef struct sha3_ctx
{
    uint64_t state[sha3_max_permutation_words];
    uint64_t size;
    uint64_t block_size;
    uint8_t buf[sha3_max_permutation_size];
} sha3_ctx;

/* methods for calculating the hash function */

void sha3_224_init(sha3_ctx* ctx);
void sha3_256_init(sha3_ctx* ctx);
void sha3_384_init(sha3_ctx* ctx);
void sha3_512_init(sha3_ctx* ctx);
void sha3_update(sha3_ctx* ctx, const void *data, size_t len);
void sha3_update_bits(sha3_ctx* ctx, const void *data, size_t nbits);
void sha3_final(sha3_ctx* ctx, unsigned char* result);

typedef sha3_ctx shake_ctx;

void shake128_init(shake_ctx* ctx);
void shake256_init(shake_ctx* ctx);
void shake_absorb_bits(shake_ctx* ctx, const void *data, size_t nbits);
void shake_final(shake_ctx* ctx);
void shake_squeeze_bits(shake_ctx* ctx, unsigned char* result, size_t nbits);

#ifdef __cplusplus
}
#endif