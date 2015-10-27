
#ifndef BITS_H
#define BITS_H

#ifdef __cplusplus
extern "C" {
#endif

unsigned long bits_get(const void *buf, long pos, int n);
void bits_set(void *buf, long pos, int n, unsigned long value);
int bits_size(int n);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* BITS_H */
