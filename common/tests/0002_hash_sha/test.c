#undef NDEBUG
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#include "sha256.h"
#include "sha512.h"
#include "sha3.h"

static const uint8_t empty_hash_sha256[32] = {
    0xe3, 0xb0, 0xc4, 0x42, 0x98, 0xfc, 0x1c, 0x14,
    0x9a, 0xfb, 0xf4, 0xc8, 0x99, 0x6f, 0xb9, 0x24,
    0x27, 0xae, 0x41, 0xe4, 0x64, 0x9b, 0x93, 0x4c,
    0xa4, 0x95, 0x99, 0x1b, 0x78, 0x52, 0xb8, 0x55
};

static const uint8_t empty_hash_sha384[48] = {
    0x38, 0xb0, 0x60, 0xa7, 0x51, 0xac, 0x96, 0x38,
    0x4c, 0xd9, 0x32, 0x7e, 0xb1, 0xb1, 0xe3, 0x6a,
    0x21, 0xfd, 0xb7, 0x11, 0x14, 0xbe, 0x07, 0x43,
    0x4c, 0x0c, 0xc7, 0xbf, 0x63, 0xf6, 0xe1, 0xda,
    0x27, 0x4e, 0xde, 0xbf, 0xe7, 0x6f, 0x65, 0xfb,
    0xd5, 0x1a, 0xd2, 0xf1, 0x48, 0x98, 0xb9, 0x5b
};

static const uint8_t empty_hash_sha512[64] = {
    0xcf, 0x83, 0xe1, 0x35, 0x7e, 0xef, 0xb8, 0xbd,
    0xf1, 0x54, 0x28, 0x50, 0xd6, 0x6d, 0x80, 0x07,
    0xd6, 0x20, 0xe4, 0x05, 0x0b, 0x57, 0x15, 0xdc,
    0x83, 0xf4, 0xa9, 0x21, 0xd3, 0x6c, 0xe9, 0xce,
    0x47, 0xd0, 0xd1, 0x3c, 0x5d, 0x85, 0xf2, 0xb0,
    0xff, 0x83, 0x18, 0xd2, 0x87, 0x7e, 0xec, 0x2f,
    0x63, 0xb9, 0x31, 0xbd, 0x47, 0x41, 0x7a, 0x81,
    0xa5, 0x38, 0x32, 0x7a, 0xf9, 0x27, 0xda, 0x3e
};

static const uint8_t empty_hash_sha512_256[32] = {
    0xc6, 0x72, 0xb8, 0xd1, 0xef, 0x56, 0xed, 0x28,
    0xab, 0x87, 0xc3, 0x62, 0x2c, 0x51, 0x14, 0x06,
    0x9b, 0xdd, 0x3a, 0xd7, 0xb8, 0xf9, 0x73, 0x74,
    0x98, 0xd0, 0xc0, 0x1e, 0xce, 0xf0, 0x96, 0x7a
};

static const uint8_t empty_hash_sha3_224[28] = {
    0x6b, 0x4e, 0x03, 0x42, 0x36, 0x67, 0xdb, 0xb7,
    0x3b, 0x6e, 0x15, 0x45, 0x4f, 0x0e, 0xb1, 0xab,
    0xd4, 0x59, 0x7f, 0x9a, 0x1b, 0x07, 0x8e, 0x3f,
    0x5b, 0x5a, 0x6b, 0xc7
};

static const uint8_t empty_hash_sha3_256[32] = {
    0xa7, 0xff, 0xc6, 0xf8, 0xbf, 0x1e, 0xd7, 0x66,
    0x51, 0xc1, 0x47, 0x56, 0xa0, 0x61, 0xd6, 0x62,
    0xf5, 0x80, 0xff, 0x4d, 0xe4, 0x3b, 0x49, 0xfa,
    0x82, 0xd8, 0x0a, 0x4b, 0x80, 0xf8, 0x43, 0x4a
};

static const uint8_t empty_hash_sha3_384[48] = {
    0x0c, 0x63, 0xa7, 0x5b, 0x84, 0x5e, 0x4f, 0x7d,
    0x01, 0x10, 0x7d, 0x85, 0x2e, 0x4c, 0x24, 0x85,
    0xc5, 0x1a, 0x50, 0xaa, 0xaa, 0x94, 0xfc, 0x61,
    0x99, 0x5e, 0x71, 0xbb, 0xee, 0x98, 0x3a, 0x2a,
    0xc3, 0x71, 0x38, 0x31, 0x26, 0x4a, 0xdb, 0x47,
    0xfb, 0x6b, 0xd1, 0xe0, 0x58, 0xd5, 0xf0, 0x04
};

static const uint8_t empty_hash_sha3_512[64] = {
    0xa6, 0x9f, 0x73, 0xcc, 0xa2, 0x3a, 0x9a, 0xc5,
    0xc8, 0xb5, 0x67, 0xdc, 0x18, 0x5a, 0x75, 0x6e,
    0x97, 0xc9, 0x82, 0x16, 0x4f, 0xe2, 0x58, 0x59,
    0xe0, 0xd1, 0xdc, 0xc1, 0x47, 0x5c, 0x80, 0xa6,
    0x15, 0xb2, 0x12, 0x3a, 0xf1, 0xf5, 0xf9, 0x4c,
    0x11, 0xe3, 0xe9, 0x40, 0x2c, 0x3a, 0xc5, 0x58,
    0xf5, 0x00, 0x19, 0x9d, 0x95, 0xb6, 0xd3, 0xe3,
    0x01, 0x75, 0x85, 0x86, 0x28, 0x1d, 0xcd, 0x26
};

static const uint8_t empty_hash_shake128_256[32] = {
    0x7f, 0x9c, 0x2b, 0xa4, 0xe8, 0x8f, 0x82, 0x7d,
    0x61, 0x60, 0x45, 0x50, 0x76, 0x05, 0x85, 0x3e,
    0xd7, 0x3b, 0x80, 0x93, 0xf6, 0xef, 0xbc, 0x88,
    0xeb, 0x1a, 0x6e, 0xac, 0xfa, 0x66, 0xef, 0x26
};

static const uint8_t empty_hash_shake256_512[64] = {
    0x46, 0xb9, 0xdd, 0x2b, 0x0b, 0xa8, 0x8d, 0x13,
    0x23, 0x3b, 0x3f, 0xeb, 0x74, 0x3e, 0xeb, 0x24,
    0x3f, 0xcd, 0x52, 0xea, 0x62, 0xb8, 0x1b, 0x82,
    0xb5, 0x0c, 0x27, 0x64, 0x6e, 0xd5, 0x76, 0x2f,
    0xd7, 0x5d, 0xc4, 0xdd, 0xd8, 0xc0, 0xf2, 0x00,
    0xcb, 0x05, 0x01, 0x9d, 0x67, 0xb5, 0x92, 0xf6,
    0xfc, 0x82, 0x1c, 0x49, 0x47, 0x9a, 0xb4, 0x86,
    0x40, 0x29, 0x2e, 0xac, 0xb3, 0xb7, 0xc4, 0xbe
};

void test_sha256()
{
    sha256_ctx ctx;
    uint8_t buf[32];
    sha256_init(&ctx);
    sha256_update(&ctx, NULL, 0);
    sha256_final(&ctx, buf);
    assert(memcmp(buf, empty_hash_sha256, sizeof(buf)) == 0);
}

void test_sha384()
{
    sha512_ctx ctx;
    uint8_t buf[48];
    sha384_init(&ctx);
    sha512_update(&ctx, NULL, 0);
    sha512_final(&ctx, buf);
    assert(memcmp(buf, empty_hash_sha384, sizeof(buf)) == 0);
}

void test_sha512()
{
    sha512_ctx ctx;
    uint8_t buf[64];
    sha512_init(&ctx);
    sha512_update(&ctx, NULL, 0);
    sha512_final(&ctx, buf);
    assert(memcmp(buf, empty_hash_sha512, sizeof(buf)) == 0);
}

void test_sha512_256()
{
    sha512_ctx ctx;
    uint8_t buf[32];
    sha512_256_init(&ctx);
    sha512_update(&ctx, NULL, 0);
    sha512_final(&ctx, buf);
    assert(memcmp(buf, empty_hash_sha512_256, sizeof(buf)) == 0);
}

void test_sha3_224()
{
    sha3_ctx ctx;
    uint8_t buf[28];
    sha3_224_init(&ctx);
    sha3_update(&ctx, NULL, 0);
    sha3_final(&ctx, buf);
    assert(memcmp(buf, empty_hash_sha3_224, sizeof(buf)) == 0);
}

void test_sha3_256()
{
    sha3_ctx ctx;
    uint8_t buf[32];
    sha3_256_init(&ctx);
    sha3_update(&ctx, NULL, 0);
    sha3_final(&ctx, buf);
    assert(memcmp(buf, empty_hash_sha3_256, sizeof(buf)) == 0);
}

void test_sha3_384()
{
    sha3_ctx ctx;
    uint8_t buf[48];
    sha3_384_init(&ctx);
    sha3_update(&ctx, NULL, 0);
    sha3_final(&ctx, buf);
    assert(memcmp(buf, empty_hash_sha3_384, sizeof(buf)) == 0);
}

void test_sha3_512()
{
    sha3_ctx ctx;
    uint8_t buf[64];
    sha3_512_init(&ctx);
    sha3_update(&ctx, NULL, 0);
    sha3_final(&ctx, buf);
    assert(memcmp(buf, empty_hash_sha3_512, sizeof(buf)) == 0);
}

void test_shake128_256()
{
    sha3_ctx ctx;
    uint8_t buf[32];
    shake128_init(&ctx);
    shake_absorb_bits(&ctx, NULL, 0);
    shake_final(&ctx);
    shake_squeeze_bits(&ctx, buf, 256);
    assert(memcmp(buf, empty_hash_shake128_256, sizeof(buf)) == 0);
}

void test_shake256_512()
{
    sha3_ctx ctx;
    uint8_t buf[64];
    shake256_init(&ctx);
    shake_absorb_bits(&ctx, NULL, 0);
    shake_final(&ctx);
    shake_squeeze_bits(&ctx, buf, 512);
    assert(memcmp(buf, empty_hash_shake256_512, sizeof(buf)) == 0);
}

static const uint8_t test_shake128_hash_msg1[1] = {
    0x0e
};
static const uint8_t test_shake128_hash_out1[16] = {
  0xfa, 0x99, 0x6d, 0xaf, 0xaa, 0x20, 0x8d, 0x72,
  0x28, 0x7c, 0x23, 0xbc, 0x4e, 0xd4, 0xbf, 0xd5
};

static const uint8_t test_shake128_hash_msg2[16] = {
  0x57, 0x24, 0xec, 0xf8, 0xda, 0x91, 0x09, 0xb8,
  0x98, 0xa4, 0x28, 0x93, 0x9e, 0x08, 0xdb, 0x24
};
static const uint8_t test_shake128_hash_out2[64] = {
  0x74, 0xec, 0x9a, 0xe4, 0xe1, 0xcb, 0x43, 0x10,
  0x39, 0xd4, 0xd1, 0xbb, 0xa0, 0x01, 0xcb, 0x22,
  0xb8, 0x8c, 0x13, 0x19, 0x65, 0x3f, 0xcb, 0x17,
  0xa2, 0xfa, 0xc2, 0x8b, 0x6a, 0x27, 0xb5, 0xdf,
  0xbb, 0xfa, 0x88, 0x69, 0xe8, 0xd3, 0x49, 0xcf,
  0x3d, 0x9c, 0xc0, 0x3c, 0x05, 0xa7, 0x39, 0x98,
  0xa1, 0x0f, 0x0b, 0xb8, 0x3e, 0x5a, 0x54, 0xb5,
  0x52, 0x54, 0xcc, 0xbc, 0x5e, 0x21, 0x56, 0x6d
};

static const uint8_t test_shake256_hash_msg1[1] = {
    0x0f
};
static const uint8_t test_shake256_hash_out1[32] = {
  0xaa, 0xbb, 0x07, 0x48, 0x8f, 0xf9, 0xed, 0xd0,
  0x5d, 0x6a, 0x60, 0x3b, 0x77, 0x91, 0xb6, 0x0a,
  0x16, 0xd4, 0x50, 0x93, 0x60, 0x8f, 0x1b, 0xad,
  0xc0, 0xc9, 0xcc, 0x9a, 0x91, 0x54, 0xf2, 0x15
};

static const uint8_t test_shake256_hash_msg2[32] = {
  0xdc, 0x88, 0x6d, 0xf3, 0xf6, 0x9c, 0x49, 0x51,
  0x3d, 0xe3, 0x62, 0x7e, 0x94, 0x81, 0xdb, 0x58,
  0x71, 0xe8, 0xee, 0x88, 0xeb, 0x9f, 0x99, 0x61,
  0x15, 0x41, 0x93, 0x0a, 0x8b, 0xc8, 0x85, 0xe0
};
static const uint8_t test_shake256_hash_out2[128] = {
  0x00, 0x64, 0x8a, 0xfb, 0xc5, 0xe6, 0x51, 0x64,
  0x9d, 0xb1, 0xfd, 0x82, 0x93, 0x6b, 0x00, 0xdb,
  0xbc, 0x12, 0x2f, 0xb4, 0xc8, 0x77, 0x86, 0x0d,
  0x38, 0x5c, 0x49, 0x50, 0xd5, 0x6d, 0xe7, 0xe0,
  0x96, 0xd6, 0x13, 0xd7, 0xa3, 0xf2, 0x7e, 0xd8,
  0xf2, 0x63, 0x34, 0xb0, 0xcc, 0xc1, 0x40, 0x7b,
  0x41, 0xdc, 0xcb, 0x23, 0xdf, 0xaa, 0x52, 0x98,
  0x18, 0xd1, 0x12, 0x5c, 0xd5, 0x34, 0x80, 0x92,
  0x52, 0x43, 0x66, 0xb8, 0x5f, 0xab, 0xb9, 0x7c,
  0x6c, 0xd1, 0xe6, 0x06, 0x6f, 0x45, 0x9b, 0xcc,
  0x56, 0x6d, 0xa8, 0x7e, 0xc9, 0xb7, 0xba, 0x36,
  0x79, 0x2d, 0x11, 0x8a, 0xc3, 0x9a, 0x4c, 0xce,
  0xf6, 0x19, 0x2b, 0xbf, 0x3a, 0x54, 0xaf, 0x18,
  0xe5, 0x7b, 0x0c, 0x14, 0x61, 0x01, 0xf6, 0xae,
  0xaa, 0x82, 0x2b, 0xc4, 0xb4, 0xc9, 0x70, 0x8b,
  0x09, 0xf0, 0xb3, 0xba, 0xb4, 0x1b, 0xcc, 0xe9
};

#define test_shake_xof(init,input,output) {              \
    sha3_ctx ctx;                                        \
    uint8_t buf[sizeof(output)];                         \
    init(&ctx);                                          \
    shake_absorb_bits(&ctx, input, sizeof(input) << 3);  \
    shake_final(&ctx);                                   \
    shake_squeeze_bits(&ctx, buf, sizeof(output) << 3);  \
    assert(memcmp(buf, output, sizeof(output)) == 0);    \
}

void test_shake128_xof()
{
    test_shake_xof(shake128_init,test_shake128_hash_msg1,test_shake128_hash_out1);
    test_shake_xof(shake128_init,test_shake128_hash_msg2,test_shake128_hash_out2);
}

void test_shake256_xof()
{
    test_shake_xof(shake256_init,test_shake256_hash_msg1,test_shake256_hash_out1);
    test_shake_xof(shake256_init,test_shake256_hash_msg2,test_shake256_hash_out2);
}

static const uint8_t test_sha3_512_msg1[1] = {
    0xe5
};
static const uint8_t test_sha3_512_out1[64] = {
  0x15, 0x02, 0x40, 0xba, 0xf9, 0x5f, 0xb3, 0x6f,
  0x8c, 0xcb, 0x87, 0xa1, 0x9a, 0x41, 0x76, 0x7e,
  0x7a, 0xed, 0x95, 0x12, 0x50, 0x75, 0xa2, 0xb2,
  0xdb, 0xba, 0x6e, 0x56, 0x5e, 0x1c, 0xe8, 0x57,
  0x5f, 0x2b, 0x04, 0x2b, 0x62, 0xe2, 0x9a, 0x04,
  0xe9, 0x44, 0x03, 0x14, 0xa8, 0x21, 0xc6, 0x22,
  0x41, 0x82, 0x96, 0x4d, 0x8b, 0x55, 0x7b, 0x16,
  0xa4, 0x92, 0xb3, 0x80, 0x6f, 0x4c, 0x39, 0xc1
};

static const uint8_t test_sha3_512_msg2[145] = {
  0x5c, 0x2b, 0x3c, 0xfb, 0x98, 0x24, 0x8a, 0xa4,
  0x23, 0x2f, 0xa1, 0xfd, 0x83, 0x10, 0x41, 0x90,
  0xac, 0x35, 0x34, 0xd7, 0xbb, 0x98, 0x96, 0x47,
  0xe6, 0xe7, 0x75, 0x8d, 0xb3, 0xe3, 0x9a, 0x17,
  0x69, 0x06, 0xa5, 0xd6, 0xc4, 0x68, 0x60, 0x42,
  0xed, 0xe3, 0x4d, 0x9f, 0xa9, 0x75, 0x81, 0x7f,
  0xd1, 0xe7, 0xb5, 0x89, 0x02, 0xac, 0x9b, 0x78,
  0x95, 0x0c, 0x6e, 0x67, 0xd2, 0x82, 0xcd, 0x4b,
  0x60, 0x62, 0xc9, 0xc1, 0x00, 0xbe, 0x3d, 0x0a,
  0x7c, 0x8b, 0x5e, 0x53, 0xd9, 0x00, 0x92, 0x3a,
  0xd4, 0x3f, 0x86, 0x8f, 0x8d, 0x37, 0x7a, 0x57,
  0x7d, 0xc9, 0xe0, 0x17, 0xad, 0x29, 0x26, 0x4f,
  0x2a, 0x88, 0xd0, 0x44, 0xb4, 0xf0, 0xc9, 0x02,
  0xb6, 0x6f, 0xfb, 0xa2, 0x7b, 0x77, 0x1d, 0xc9,
  0x9e, 0x85, 0xcc, 0x71, 0xc2, 0x01, 0xde, 0xec,
  0x84, 0x65, 0xf3, 0x25, 0xae, 0xa2, 0x4c, 0x2a,
  0xad, 0xc0, 0xe1, 0xe7, 0x46, 0xc4, 0xad, 0x34,
  0x84, 0xd6, 0x4d, 0x13, 0xf5, 0xa3, 0xc9, 0x32,
  0x00
};
static const uint8_t test_sha3_512_out2[64] = {
  0xfc, 0x6c, 0x2b, 0x31, 0x3a, 0x4e, 0xce, 0x52,
  0xc4, 0xd4, 0x68, 0x8d, 0xaf, 0x43, 0x32, 0x62,
  0xe9, 0x5d, 0x96, 0x80, 0xae, 0xc5, 0x8e, 0x2b,
  0x3d, 0x7c, 0x04, 0x37, 0x70, 0x32, 0x2e, 0xe6,
  0xae, 0x11, 0x1e, 0x1d, 0x34, 0xfe, 0x53, 0x41,
  0x04, 0x88, 0x37, 0x77, 0x62, 0x24, 0xe5, 0xb6,
  0x80, 0x59, 0x10, 0x48, 0x1a, 0xa2, 0x76, 0x64,
  0x19, 0xce, 0x55, 0x48, 0xbe, 0xa5, 0x3e, 0x5a
};

#define test_sha3_var(init,input,input_len,output) {     \
    sha3_ctx ctx;                                        \
    uint8_t buf[sizeof(output)];                         \
    init(&ctx);                                          \
    sha3_update_bits(&ctx, input, input_len);            \
    sha3_final(&ctx, buf);                               \
    assert(memcmp(buf, output, sizeof(output)) == 0);    \
}

void test_sha3_512_var()
{
    test_sha3_var(sha3_512_init,test_sha3_512_msg1,8,test_sha3_512_out1);
    test_sha3_var(sha3_512_init,test_sha3_512_msg2,1153,test_sha3_512_out2);
}

void run_tests()
{
    test_sha256();
    test_sha384();
    test_sha512();
    test_sha512_256();
    test_sha3_224();
    test_sha3_256();
    test_sha3_384();
    test_sha3_512();
    test_shake128_256();
    test_shake256_512();
    test_shake128_xof();
    test_shake256_xof();
    test_sha3_512_var();
}

int main(int argc, char **argv)
{
    run_tests();
}
