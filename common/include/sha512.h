#pragma once

#include "stdint.h"
#include "stddef.h"

#ifdef __cplusplus
extern "C" {
#endif

#define sha512_block_size    128
#define sha512_256_hash_size  32
#define sha512_384_hash_size  48
#define sha512_hash_size      64

struct sha512_ctx {
    uint64_t chain[8];
    uint8_t block[sha512_block_size];
    uint64_t nbytes;
    uint64_t digestlen;
};

typedef struct sha512_ctx sha512_ctx;

void sha512_init(sha512_ctx *ctx);
void sha384_init(sha512_ctx *ctx);
void sha512_256_init(sha512_ctx *ctx);
void sha512_update(sha512_ctx *ctx, const void *data, size_t len);
void sha512_final(sha512_ctx *ctx, unsigned char *result);

#ifdef __cplusplus
}
#endif