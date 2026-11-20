#include <inttypes.h>
#include <stdio.h>
#include <ttak/math/bigint.h>
#include <ttak/math/factor.h>
#include <ttak/mem/mem.h>
#include <ttak/timing/timing.h>

int main(void) {
    uint64_t now = ttak_get_tick_count_ns();

    /* 2^64 - 1 squared does not fit in any machine word. */
    ttak_bigint_t a, square;
    ttak_bigint_init_u64(&a, UINT64_MAX, now);
    ttak_bigint_init(&square, now);
    ttak_bigint_mul(&square, &a, &a, now);

    ttak_u128_t v;                               /* fits in 128 bits */
    if (ttak_bigint_export_u128(&square, &v))
        printf("(2^64-1)^2 = 0x%016" PRIx64 "%016" PRIx64 "\n", v.hi, v.lo);
    ttak_bigint_free(&a, now);
    ttak_bigint_free(&square, now);

    /* Factorisation: the library allocates, the caller frees. */
    ttak_prime_factor_t *f = NULL;
    size_t count = 0;
    if (ttak_factor_u64(9699690ULL, &f, &count, now) == 0) {
        printf("9699690 =");
        for (size_t i = 0; i < count; i++)
            printf(" %s%" PRIu64 "^%u", i ? "* " : "", f[i].p, f[i].a);
        putchar('\n');
        ttak_mem_free(f);
    }
    return 0;
}
