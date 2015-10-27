
#ifndef TREE_H
#define TREE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Tree tree_t;
typedef int (*tree_cmp_t)(const void *, const void *);

tree_t * tree_create(tree_cmp_t cmp);
void tree_destroy(tree_t *tree, void (*destructor)(void *));
long tree_size(tree_t *tree);
void * tree_find(tree_t *tree, void *key);
void * tree_insert(tree_t *tree, void *key, void *value);
void * tree_delete(tree_t *tree, void *key);

void * tree_fold(tree_t *tree, void * (*fun)(void *, void *, void *), void *acc);
void * tree_foldl(tree_t *tree, void * (*fun)(void *, void *, void *), void *acc);
void * tree_foldr(tree_t *tree, void * (*fun)(void *, void *, void *), void *acc);

typedef struct TreeInfo {
    long size;
    long height;
    long min_height;
    long black_height;
    long red_number;
    long black_number;
} tree_info_t;

tree_info_t * tree_info(tree_t *tree, tree_info_t *info);
int tree_check_integrity(tree_t *tree);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TREE_H */
