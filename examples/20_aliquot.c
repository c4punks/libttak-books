#define _XOPEN_SOURCE 700
#include <inttypes.h>
#include <stdio.h>
#include <ttak/math/sum_divisors.h>

/* Aliquot sequence: a(k+1) = s(a(k)), s = sum of proper divisors.
 * Stops at 1 (prime reached), at a repeat (cycle), or after a cap. */
static void aliquot(uint64_t start, int max_steps) {
    uint64_t seen[64];
    int len = 0;
    uint64_t n = start;

    printf("%" PRIu64, n);
    while (len < max_steps && len < 64) {
        seen[len++] = n;
        uint64_t next = 0;
        if (!ttak_sum_proper_divisors_u64(n, &next)) {
            printf(" -> (overflow)");
            break;
        }
        printf(" -> %" PRIu64, next);
        if (next <= 1) break;
        int cycle = 0;
        for (int i = 0; i < len; i++) cycle |= (seen[i] == next);
        if (cycle) { printf("  (cycle)"); break; }
        n = next;
    }
    putchar('\n');
}

int main(void) {
    aliquot(12, 20);    /* terminates: ... -> 3 -> 1 */
    aliquot(28, 20);    /* perfect number: 28 -> 28 */
    aliquot(220, 20);   /* amicable pair: 220 <-> 284 */
    aliquot(12496, 20); /* sociable chain of length 5 */
    return 0;
}
