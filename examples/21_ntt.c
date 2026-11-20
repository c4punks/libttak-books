#define _XOPEN_SOURCE 700
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <ttak/math/ntt.h>

static void show(const char *label, const uint64_t *d, size_t n) {
    printf("%-12s", label);
    for (size_t i = 0; i < n; i++) printf(" %" PRIu64, d[i]);
    putchar('\n');
}

int main(void) {
    const ttak_ntt_prime_t *prime = &ttak_ntt_primes[0];
    uint64_t data[4] = { 1, 2, 3, 4 };      /* length: power of two */
    show("input:", data, 4);

    if (!ttak_ntt_transform(data, 4, prime, false)) {
        fputs("forward NTT failed\n", stderr);
        return 1;
    }
    show("forward:", data, 4);

    if (!ttak_ntt_transform(data, 4, prime, true)) {
        fputs("inverse NTT failed\n", stderr);
        return 1;
    }
    show("round trip:", data, 4);           /* back to 1 2 3 4 */
    return 0;
}
