// Leveled log lines on stderr. Dev builds are console programs, so the lines
// show in the terminal that started them. HERON_LOG picks the level (error,
// warn, info, debug, trace); the default is debug in dev builds, warn
// otherwise. Lines printed per output chunk belong at trace only.
#pragma once

#include <stddef.h>

typedef enum { LOG_ERROR, LOG_WARN, LOG_INFO, LOG_DEBUG, LOG_TRACE } LogLevel;

void log_init(void);
// Run as a hook, heron-c must stay quiet: only errors then.
void log_set_level(LogLevel level);
int log_enabled(LogLevel level);

// The UCRT printf understands C99 formats such as %zu; gcc calls that
// checker gnu_printf, clang (used by clangd) calls it printf.
#if defined(__clang__)
__attribute__((format(printf, 3, 4)))
#elif defined(__GNUC__)
__attribute__((format(gnu_printf, 3, 4)))
#endif
void log_write(LogLevel level, const char *scope, const char *fmt, ...);

// Logs bytes with control characters escaped (\e, \r, \n, \xNN), to see
// exactly what a program printed. Long data is cut at 1500 bytes.
void log_bytes(LogLevel level, const char *scope, const char *label, const void *data, size_t len);

#define log_error(scope, ...) log_write(LOG_ERROR, scope, __VA_ARGS__)
#define log_warn(scope, ...) log_write(LOG_WARN, scope, __VA_ARGS__)
#define log_info(scope, ...) log_write(LOG_INFO, scope, __VA_ARGS__)
#define log_debug(scope, ...) log_write(LOG_DEBUG, scope, __VA_ARGS__)
#define log_trace(scope, ...) log_write(LOG_TRACE, scope, __VA_ARGS__)
