
#ifndef UINTVAR_H
#define UINTVAR_H

#ifdef __cplusplus
extern "C" {
#endif

#define UINTVAR32_LEN 5

int uintvar_pack(void *buf, long len, unsigned long value);
int uintvar_unpack(unsigned long *value, const void *buf, long len);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* UINTVAR_H */
