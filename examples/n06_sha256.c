#include <stdio.h>
#include <string.h>
#include <ttak/security/sha256.h>

static void hex(const uint8_t *d, size_t n, char *out) {
    for (size_t i = 0; i < n; i++) sprintf(out + 2 * i, "%02x", d[i]);
}

int main(void) {
    uint8_t digest[SHA256_BLOCK_SIZE];
    char out[2 * SHA256_BLOCK_SIZE + 1];

    /* Streaming API: init, update as often as needed, final. */
    SHA256_CTX ctx;
    sha256_init(&ctx);
    sha256_update(&ctx, (const uint8_t *)"a", 1);
    sha256_update(&ctx, (const uint8_t *)"bc", 2);   /* = "abc" */
    sha256_final(&ctx, digest);
    hex(digest, sizeof digest, out);

    /* FIPS 180-2 test vector for "abc" */
    const char *expected =
        "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad";
    printf("sha256(\"abc\") = %s\n%s\n", out,
           strcmp(out, expected) == 0 ? "matches FIPS 180-2" : "MISMATCH");
    return strcmp(out, expected) != 0;
}
