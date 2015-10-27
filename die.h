
#ifndef DIE_H
#define DIE_H

void die(const char *fmt, ...);
void die_status(int status, const char *fmt, ...);
void die_full(const char *file, long line, const char *func,
    int status, const char *fmt, ...);

#define die_trace(fmt, ...) \
    (die_full(__FILE__, (long)__LINE__, __func__, EXIT_FAILURE, fmt, ##__VA_ARGS__))

#endif /* DIE_H */
