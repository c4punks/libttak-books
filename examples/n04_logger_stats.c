#include <stdio.h>
#include <ttak/log/logger.h>
#include <ttak/stats/stats.h>

/* A sink is just a function: stdout here, syslog or a file elsewhere. */
static void stdout_sink(ttak_log_level_t level, const char *msg) {
    static const char *names[] = { "DEBUG", "INFO", "WARN", "ERROR" };
    printf("[%s] %s\n", names[level], msg);
}

int main(void) {
    ttak_logger_t log;
    ttak_logger_init(&log, stdout_sink, TTAK_LOG_INFO);   /* drop DEBUG */

    ttak_stats_t lat;
    ttak_stats_init(&lat, 0, 100);         /* histogram range: 0..100 ms */

    const uint64_t samples[] = { 12, 18, 25, 31, 40, 44, 52, 95 };
    for (size_t i = 0; i < sizeof samples / sizeof *samples; i++) {
        ttak_stats_record(&lat, samples[i]);
        /* below the INFO threshold: filtered out */
        ttak_logger_log(&log, TTAK_LOG_DEBUG, "sample %zu", i);
    }

    ttak_logger_log(&log, TTAK_LOG_INFO, "mean latency %.2f ms",
                    ttak_stats_mean(&lat));
    if (ttak_stats_mean(&lat) > 30.0)
        ttak_logger_log(&log, TTAK_LOG_WARN, "latency budget exceeded");

    ttak_stats_print_ascii(&lat);
    return 0;
}
