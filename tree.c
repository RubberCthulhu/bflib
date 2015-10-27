
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tree.h"

struct TreeNode {
    tree_t *tree;
    struct TreeNode *parent;
    struct TreeNode *left;
    struct TreeNode *right;
    enum {
        RED,
        BLACK
    } color;
    void *key;
    void *value;
};

#define IS_RED(node)       ((node) != NULL && (node)->color == RED)
#define IS_BLACK(node)     ((node) == NULL || (node)->color == BLACK)

struct Tree {
    struct TreeNode *root;
    tree_cmp_t cmp;
    long size;
};

static void tree_destroy_subtree(struct TreeNode *node, void (*destructor)(void *));
static struct TreeNode * tree_find_node(tree_t *tree, void *key);

static void tree_insert1(struct TreeNode *node);
static void tree_insert2(struct TreeNode *node);
static void tree_insert3(struct TreeNode *node);
static void tree_insert4(struct TreeNode *node);
static void tree_insert5(struct TreeNode *node);

static void tree_delete1(struct TreeNode *node);
static void tree_delete2(struct TreeNode *node);
static void tree_delete3(struct TreeNode *node);
static void tree_delete4(struct TreeNode *node);
static void tree_delete5(struct TreeNode *node);
static void tree_delete6(struct TreeNode *node);

static void tree_rotate_left(struct TreeNode *node);
static void tree_rotate_right(struct TreeNode *node);

static void * tree_node_foldl(struct TreeNode *node, void * (*fun)(void *, void *, void *), void *acc);
static void * tree_node_foldr(struct TreeNode *node, void * (*fun)(void *, void *, void *), void *acc);

static struct TreeNode * tree_node_create(void *key, void *value);
static void tree_node_destroy(struct TreeNode *node);

static struct TreeNode * tree_node_grandparent(struct TreeNode *node);
static struct TreeNode * tree_node_uncle(struct TreeNode *node);
static struct TreeNode * tree_node_sibling(struct TreeNode *node);
static struct TreeNode * tree_node_max(struct TreeNode *node);
static struct TreeNode * tree_node_min(struct TreeNode *node);

static tree_info_t * tree_node_info(struct TreeNode *node, tree_info_t *info);
static int tree_node_check_integrity(struct TreeNode *node);

tree_t * tree_create(tree_cmp_t cmp)
{
    tree_t *tree;

    tree = malloc(sizeof(tree_t));
    memset(tree, 0, sizeof(*tree));

    tree->cmp = cmp;

    return tree;
}

void tree_destroy(tree_t *tree, void (*destructor)(void *))
{
    if( tree->root )
        tree_destroy_subtree(tree->root, destructor);
    free(tree);
}

static void tree_destroy_subtree(struct TreeNode *node, void (*destructor)(void *))
{
    if( node->left )
        tree_destroy_subtree(node->left, destructor);
    if( node->right )
        tree_destroy_subtree(node->right, destructor);

    if( destructor )
        destructor(node->value);

    tree_node_destroy(node);
}

long tree_size(tree_t *tree)
{
    return tree->size;
}

void * tree_find(tree_t *tree, void *key)
{
    struct TreeNode *node;

    if( (node = tree_find_node(tree, key)) )
        return node->value;

    return NULL;
}

static struct TreeNode * tree_find_node(tree_t *tree, void *key)
{
    struct TreeNode *node;
    int cmp;

    node = tree->root;
    while( node ) {
        cmp = tree->cmp(key, node->key);
        if( cmp == 0 )
            return node;
        else if( cmp < 0 )
            node = node->left;
        else
            node = node->right;
    }

    return node;
}

void * tree_insert(tree_t *tree, void *key, void *value)
{
    struct TreeNode **node, *parent;
    int cmp;

    node = &tree->root;
    parent = NULL;
    while( *node ) {
        parent = *node;
        cmp = tree->cmp(key, (*node)->key);
        if( cmp < 0 )
            node = &(*node)->left;
        else if( cmp > 0 )
            node = &(*node)->right;
        else
            return (*node)->value;
    }

    *node = tree_node_create(key, value);
    (*node)->tree = tree;
    (*node)->parent = parent;
    (*node)->color = RED;

    tree_insert1(*node);
    tree->size++;

    return value;
}

static void tree_insert1(struct TreeNode *node)
{
    if( node->parent == NULL )
        node->color = BLACK;
    else
        tree_insert2(node);
}

static void tree_insert2(struct TreeNode *node)
{
    // node->parent != NULL because we know it from tree_insert1().
    if( IS_BLACK(node->parent) )
        // node->color == RED. Everything's ok.
        return;
    else
        // node->color == RED and parent->color == RED.
        tree_insert3(node);
}

static void tree_insert3(struct TreeNode *node)
{
    struct TreeNode *u, *g;

    u = tree_node_uncle(node);
    // node->color == RED (initially)
    //   and parent->color == RED (known from tree_insert2())
    //   and uncle->color == RED.
    if( IS_RED(u) ) {
        // If uncle is RED it can't be NULL.
        u->color = BLACK;
        // node->parent != NULL. We know it from tree_insert1().
        node->parent->color = BLACK;

        // g is valid because there is a valid uncle.
        g = tree_node_grandparent(node);
        g->color = RED;
        tree_insert1(g);
    }
    else {
        tree_insert4(node);
    }
}

static void tree_insert4(struct TreeNode *node)
{
    struct TreeNode *g;

    g = tree_node_grandparent(node);

    // node->color == RED (still)
    //   and parent->color == RED (tree_insert2() => tree_insert3())
    //   and uncle->color == BLACK (known from tree_insert3()).
    // grand is valid because parent->color == RED so the grand must exist.
    if( node == node->parent->right && node->parent == g->left ) {
        tree_rotate_left(node->parent);
        // Take the former parent.
        node = node->left;
    }
    else if( node == node->parent->left && node->parent == g->right ) {
        tree_rotate_right(node->parent);
        // Take the former parent.
        node = node->right;
    }

    // Now we are working with former node's parent.
    tree_insert5(node);
}

static void tree_insert5(struct TreeNode *node)
{
    struct TreeNode *g;

    g = tree_node_grandparent(node);

    // Now node = parent and parent = node.
    // Both node and parent are left or right for their parents.
    // grand is valid - it's the same as in (tree_insert4()).
    // node->color == RED and parent->color == RED (as in tree_insert4())
    //   and uncle->color == BLACK (it's the same as in tree_insert4())
    //   and grand->color == BLACK (it was a parent of RED node).
    node->parent->color = BLACK;
    g->color = RED;
    if( node == node->parent->left && node->parent == g->left )
        tree_rotate_right(g);
    else if( node == node->parent->right && node->parent == g->right )
        tree_rotate_left(g);
}

void * tree_delete(tree_t *tree, void *key)
{
    struct TreeNode *node, *heir;
    void *value;

    if( !(node = tree_find_node(tree, key)) )
        return NULL;

    value = node->value;
    if( node->left )
        heir = tree_node_max(node->left);
    else if( node->right )
        heir = tree_node_min(node->right);
    else
        heir = NULL;

    if( heir ) {
        node->key = heir->key;
        node->value = heir->value;
        node = heir;
    }

    // It follows from above that node has at most 1 non-null branch.
    heir = node->left ? node->left : node->right;
    if( heir ) {
        node->key = heir->key;
        node->value = heir->value;

        if( IS_BLACK(heir) ) {
            if( IS_RED(node) )
                node->color = BLACK;
            else
                // heir->color == BLACK and node->color == BLACK.
                // Hence correction is required.
                tree_delete1(node);
        }

        node = heir;
    }
    else {
        if( IS_BLACK(node) )
            // Hence correction is required.
            tree_delete1(node);
    }

    if( !node->parent )
        tree->root = NULL;
    else if( node == node->parent->left )
        node->parent->left = NULL;
    else
        node->parent->right = NULL;

    tree_node_destroy(node);
    tree->size--;

    return value;
}

static void tree_delete1(struct TreeNode *node)
{
    // node->color == BLACK (known from tree_delete()).
    // If node is root then nothing has to be done.
    if( node->parent )
        tree_delete2(node);
}

static void tree_delete2(struct TreeNode *node)
{
    struct TreeNode *s;

    // node->color == BLACK.
    // node has a valid parent (tree_delete1()).

    s = tree_node_sibling(node);
    if( IS_RED(s) ) {
        // If s->color == RED it is valid (non-null).
        node->parent->color = RED;
        s->color = BLACK;
        if( node == node->parent->left )
            tree_rotate_left(node->parent);
        else
            tree_rotate_right(node->parent);
    }

    tree_delete3(node);
}

static void tree_delete3(struct TreeNode *node)
{
    struct TreeNode *s;

    // node->color == BLACK.
    // node has a valid parent (tree_delete1(), tree_delete2()).

    s = tree_node_sibling(node);
    // Actually if node->color == BLACK (and it is BLACK)
    //   there must be a valid sibling for node.
    if( IS_BLACK(node->parent)
        && s && IS_BLACK(s) && IS_BLACK(s->left) && IS_BLACK(s->right) ) {
            s->color = RED;
            // node->parent->color == BLACK.
            tree_delete1(node->parent);
    }
    else {
        tree_delete4(node);
    }
}

static void tree_delete4(struct TreeNode *node)
{
    struct TreeNode *s;

    // node->color == BLACK.
    // node has a valid parent (tree_delete1(), tree_delete2()).

    s = tree_node_sibling(node);
    // Actually if node->color == BLACK (and it is BLACK)
    //   there must be a valid sibling for it.
    if( IS_RED(node->parent)
        && s && IS_BLACK(s) && IS_BLACK(s->left) && IS_BLACK(s->right) ) {
            s->color = RED;
            node->parent->color = BLACK;
    }
    else {
        tree_delete5(node);
    }
}

static void tree_delete5(struct TreeNode *node)
{
    struct TreeNode *s;

    // node->color == BLACK.
    // node has a valid parent (tree_delete1(), tree_delete2()).

    s = tree_node_sibling(node);
    // Actually if node->color == BLACK (and it is BLACK)
    //   there must be a valid sibling for it.
    if( IS_BLACK(s) ) {
        if( node == node->parent->left
            && s && IS_BLACK(s->right) && IS_RED(s->left) ) {
                s->color = RED;
                // s->left is valid because it's red.
                s->left->color = BLACK;
                tree_rotate_right(s);
        }
        else if( node == node->parent->right
            && IS_BLACK(s->left) && IS_RED(s->right) ) {
                s->color = RED;
                // s->right is valid because it's red.
                s->right->color = BLACK;
                tree_rotate_left(s);
        }
    }

    tree_delete6(node);
}

static void tree_delete6(struct TreeNode *node)
{
    struct TreeNode *s;

    // node->color == BLACK.
    // node has a valid parent (previous cases).

    s = tree_node_sibling(node);
    // Actually if node->color == BLACK (and it is BLACK)
    //   there must be a valid sibling for it.
    // s->color == BLACK (tree_delete2()).
    s->color = node->parent->color;
    node->parent->color = BLACK;

    // s must have children otherwise the tree would be unbalanced.
    // If node == node->parent->left
    //   then s->right->color == RED (from tree_delete5)
    //   and s->right must have both valid black children to keep tree balanced.
    if( node == node->parent->left ) {
        s->right->color = BLACK;
        tree_rotate_left(node->parent);
    }
    // If node == node->parent->right
    //   then s->left->color == RED (from tree_delete5)
    //   and s->left must have both valid black children to keep tree balanced.
    else {
        s->left->color = BLACK;
        tree_rotate_right(node->parent);
    }
}

static void tree_rotate_left(struct TreeNode *node)
{
    if( node->parent ) {
        if( node == node->parent->left )
            node->parent->left = node->right;
        else
            node->parent->right = node->right;
    }
    else {
        node->tree->root = node->right;
    }

    node->right->parent = node->parent;
    node->parent = node->right;
    node->right = node->right->left;
    node->parent->left = node;
    if( node->right )
        node->right->parent = node;
}

static void tree_rotate_right(struct TreeNode *node)
{
    if( node->parent ) {
        if( node == node->parent->right )
            node->parent->right = node->left;
        else
            node->parent->left = node->left;
    }
    else {
        node->tree->root = node->left;
    }

    node->left->parent = node->parent;
    node->parent = node->left;
    node->left = node->left->right;
    node->parent->right = node;
    if( node->left )
        node->left->parent = node;
}

void * tree_fold(tree_t *tree, void * (*fun)(void *, void *, void *), void *acc)
{
    return tree_foldl(tree, fun, acc);
}

void * tree_foldl(tree_t *tree, void * (*fun)(void *, void *, void *), void *acc)
{
    return tree_node_foldl(tree->root, fun, acc);
}

static void * tree_node_foldl(struct TreeNode *node, void * (*fun)(void *, void *, void *), void *acc)
{
    if( node ) {
        acc = tree_node_foldl(node->left, fun, acc);
        acc = fun(node->key, node->value, acc);
        acc = tree_node_foldl(node->right, fun, acc);
    }

    return acc;
}

void * tree_foldr(tree_t *tree, void * (*fun)(void *, void *, void *), void *acc)
{
    return tree_node_foldr(tree->root, fun, acc);
}

static void * tree_node_foldr(struct TreeNode *node, void * (*fun)(void *, void *, void *), void *acc)
{
    if( node ) {
        acc = tree_node_foldr(node->right, fun, acc);
        acc = fun(node->key, node->value, acc);
        acc = tree_node_foldr(node->left, fun, acc);
    }

    return acc;
}

static struct TreeNode * tree_node_create(void *key, void *value)
{
    struct TreeNode *node;

    node = malloc(sizeof(struct TreeNode));
    memset(node, 0, sizeof(*node));

    node->key = key;
    node->value = value;
    node->color = RED;

    return node;
}

static void tree_node_destroy(struct TreeNode *node)
{
    free(node);
}

static struct TreeNode * tree_node_grandparent(struct TreeNode *node)
{
    if( node ) {
        if( node->parent )
            return node->parent->parent;
    }

    return NULL;
}

static struct TreeNode * tree_node_uncle(struct TreeNode *node)
{
    struct TreeNode *g;

    if( (g = tree_node_grandparent(node)) ) {
        if( node->parent == g->left )
            return g->right;
        else
            return g->left;
    }

    return NULL;
}

static struct TreeNode * tree_node_sibling(struct TreeNode *node)
{
    if( node ) {
        if( node->parent ) {
            if( node == node->parent->left )
                return node->parent->right;
            else
                return node->parent->left;
        }
    }

    return NULL;
}

static struct TreeNode * tree_node_max(struct TreeNode *node)
{
    if( !node )
        return NULL;

    while( node->right ) {
        node = node->right;
    }

    return node;
}

static struct TreeNode * tree_node_min(struct TreeNode *node)
{
    if( !node )
        return NULL;

    while( node->left ) {
        node = node->left;
    }

    return node;
}

tree_info_t * tree_info(tree_t *tree, tree_info_t *info)
{
    return tree_node_info(tree->root, info);
}

static tree_info_t * tree_node_info(struct TreeNode *node, tree_info_t *info)
{
    tree_info_t left_info, right_info;

    memset(info, 0, sizeof(*info));
    if( node ) {
        tree_node_info(node->left, &left_info);
        tree_node_info(node->right, &right_info);

        info->size = left_info.size + right_info.size + 1;

        info->height = left_info.height > right_info.height
            ? left_info.height + 1 : right_info.height + 1;

        info->min_height = left_info.min_height <= right_info.min_height
            ? left_info.min_height + 1 : right_info.min_height + 1;

        info->black_height = (left_info.black_height > right_info.black_height
            ? left_info.black_height : right_info.black_height)
            + (IS_BLACK(node) ? 1 : 0);

        info->red_number = (IS_RED(node) ? 1 : 0)
            + left_info.red_number + right_info.red_number;

        info->black_number = (IS_BLACK(node) ? 1 : 0)
            + left_info.black_number + right_info.black_number;
    }

    return info;
}

int tree_check_integrity(tree_t *tree)
{
    if( !IS_BLACK(tree->root) )
        return 0;

    return tree_node_check_integrity(tree->root);
}

static int tree_node_check_integrity(struct TreeNode *node)
{
    tree_info_t left_info, right_info;
    long height, min_height;

    if( !node )
        return 1;

    if( node->left && node->left->parent != node )
        return 0;

    if( node->right && node->right->parent != node )
        return 0;

    if( IS_RED(node) && !(IS_BLACK(node->parent) && IS_BLACK(node->left) && IS_BLACK(node->right)) )
        return 0;

    tree_node_info(node->left, &left_info);
    tree_node_info(node->right, &right_info);
    if( left_info.black_height != right_info.black_height )
        return 0;

    height = left_info.height > right_info.height
        ? left_info.height + 1 : right_info.height + 1;
    min_height = left_info.min_height <= right_info.min_height
        ? left_info.min_height + 1 : right_info.min_height + 1;
    if( !(height <= min_height*2) )
        return 0;

    return tree_node_check_integrity(node->left)
        && tree_node_check_integrity(node->right);
}
