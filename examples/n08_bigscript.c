#include <stdio.h>
#include <string.h>
#include <ttak/math/sum_divisors.h>
#include <ttak/script/bigscript.h>
#include <ttak/timing/timing.h>

/* The host passes seed and sn = s(seed); the script decides.
 * s() is a builtin: the sum of proper divisors. */
static const char *SRC =
    "fn main(seed, sn) {\n"
    "  if (sn == seed)    { return 1; }   // perfect: s(n) = n\n"
    "  if (s(sn) == seed) { return 2; }   // amicable: s(s(n)) = n\n"
    "  return 0;\n"
    "}\n";

int main(void) {
    uint64_t now = ttak_get_tick_count_ns();
    ttak_bigscript_error_t err = { TTAK_BIGSCRIPT_ERR_NONE, NULL };

    ttak_bigscript_program_t *prog = ttak_bigscript_compile(SRC, NULL, NULL,
                                                            &err, now);
    ttak_bigscript_vm_t *vm = ttak_bigscript_vm_create(NULL, now);
    if (!prog || !vm) {
        fprintf(stderr, "compile failed: %s\n",
                err.message ? err.message : "?");
        return 1;
    }

    static const char *kind[] = { "-", "perfect", "amicable" };
    const uint64_t seeds[] = { 6, 10, 220, 284, 496 };
    for (size_t i = 0; i < 5; i++) {
        uint64_t s = 0;
        ttak_sum_proper_divisors_u64(seeds[i], &s);

        ttak_bigint_t seed, sn;
        ttak_bigint_init_u64(&seed, seeds[i], now);
        ttak_bigint_init_u64(&sn, s, now);

        ttak_bigscript_value_t out;
        memset(&out, 0, sizeof out);
        if (ttak_bigscript_eval_seed(prog, vm, &seed, &sn, &out, &err, now)) {
            uint64_t r = 0;
            ttak_bigint_export_u64(&out.value.v.i, &r);
            printf("%4llu: s(n)=%4llu -> %s\n", (unsigned long long)seeds[i],
                   (unsigned long long)s, r < 3 ? kind[r] : "?");
        }
        ttak_bigscript_value_free(&out, now);
        ttak_bigint_free(&seed, now);
        ttak_bigint_free(&sn, now);
    }

    ttak_bigscript_vm_free(vm, now);
    ttak_bigscript_program_free(prog, now);
    return 0;
}
