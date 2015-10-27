
#ifndef OCTSTR_H
#define OCTSTR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>

typedef struct Octstr octstr_t;

octstr_t * octstr_create(const void *buf, size_t len);
octstr_t * octstr_create_from_str(const char *str);
void octstr_destroy(octstr_t *s);
void octstr_destructor(void *ptr);
size_t octstr_len(octstr_t *s);
void octstr_erase(octstr_t *s);
octstr_t * octstr_copy(octstr_t *s);
void octstr_replace(octstr_t *dst, octstr_t *src);
void octstr_str_replace(octstr_t *dst, const void *src, size_t len);
void octstr_append(octstr_t *dst, octstr_t *src);
void octstr_str_append(octstr_t *dst, const void *src, size_t len);
void octstr_append_str(octstr_t *dst, const char *str);
void octstr_append_oct(octstr_t *s, unsigned int oct);
void octstr_fill(octstr_t *s, int c, int pos, int n);
void octstr_resize(octstr_t *s, size_t len);
unsigned int octstr_get_oct(octstr_t *s, int pos);
octstr_t * octstr_substr(octstr_t *s, int pos, int n);
size_t octstr_replace_substr(octstr_t *dst, octstr_t *src, int pos, int n);
void octstr_delete(octstr_t *s, int pos, int n);

long octstr_oct_index(octstr_t *s, long pos, unsigned int oct);
long octstr_str_index(octstr_t *s, long pos, const char *str);
long octstr_index(octstr_t *s, long pos, octstr_t *str);
long octstr_str_caseindex(octstr_t *s, long pos, const char *str);
long octstr_caseindex(octstr_t *s, long pos, octstr_t *str);

unsigned int octstr_get_bits(octstr_t *s, long pos, int n);
void octstr_set_bits(octstr_t *s, long pos, int n, unsigned long value);
long octstr_append_uintvar(octstr_t *s, unsigned long value);
long octstr_unpack_uintvar(octstr_t *s, long pos, unsigned long *value);
long octstr_append_htonl(octstr_t *s, unsigned long value, int min);
long octstr_unpack_ntohl(octstr_t *s, long pos, long len, unsigned long *value);
long octstr_tol(octstr_t *s, long *value);

char * octstr_get_cstr(octstr_t *s);
octstr_t * octstr_to_hex(octstr_t *s);
octstr_t * octstr_load_from_file(const char *path);
int octstr_save_to_file(octstr_t *s, const char *path);

octstr_t * octstr_sprintf(const char *fmt, ...);

int octstr_cmp(octstr_t *s1, octstr_t *s2);
int octstr_casecmp(octstr_t *s1, octstr_t *s2);
int octstr_str_cmp(octstr_t *s1, const char *s2);
int octstr_str_casecmp(octstr_t *s1, const char *s2);

octstr_t * octstr_strip(octstr_t *s);

octstr_t * octstr_iconv(octstr_t *s, const char *to, const char *from);

octstr_t * octstr_find_and_replace(octstr_t *s, octstr_t *what, octstr_t *replace);
octstr_t * octstr_case_find_and_replace(octstr_t *s1, octstr_t *what, octstr_t *replace);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* OCTSTR_H */
