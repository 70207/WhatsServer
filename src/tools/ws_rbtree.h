#ifndef _WS_RBTREE_H__
#define _WS_RBTREE_H__

#include <ws_types.h>

typedef struct ws_rbtree_s          ws_rbtree_t;
typedef struct ws_rbtree_node_s     ws_rbtree_node_t;


typedef struct ws_rbtree_key_s      ws_rbtree_key_t;
typedef ws_int32_t                         ws_rbtree_key_int_t;

struct ws_rbtree_key_s
{
    unsigned long long     v;
    unsigned long long     v2;
};

struct ws_rbtree_node_s
{
    ws_rbtree_key_t        key;
    ws_rbtree_node_t	  *left;
    ws_rbtree_node_t      *right;
    ws_rbtree_node_t      *parent;
    ws_uchar_t          color;
    ws_uchar_t          data;
};

typedef void (*ws_rbtree_insert_pt) (ws_rbtree_node_t *root,
    ws_rbtree_node_t *node, ws_rbtree_node_t *sentinel);

typedef ws_int32_t (*ws_rbtree_compare_pt)(ws_rbtree_key_t *p1, ws_rbtree_key_t *p2);

#define YMZ_RBTREE_TYPE_DEFAULT_NUMBER                   0
#define YMZ_RBTREE_TYPE_STR                              1

struct ws_rbtree_s {
    ws_rbtree_node_t     *root;
    ws_rbtree_node_t     *sentinel;
    ws_rbtree_insert_pt   insert;
    ws_rbtree_compare_pt  compare;
    ws_int32_t            rbtree_type;
};

#define ws_rbtree_init(tree, s, i)                                           \
    ws_rbtree_sentinel_init(s);                                              \
    (tree)->root = s;                                                         \
    (tree)->sentinel = s;                                                     \
    (tree)->compare = 0;                                                      \
    (tree)->insert = i                                                  

#define ws_rbtree_init_with_compare(tree,s,i,c)                                \
    ws_rbtree_sentinel_init(s);                                                \
    (tree)->root = s;                                                          \
    (tree)->sentinel = s;                                                      \
    (tree)->compare = c;                                                       \
    (tree)->insert = i                                                  

void ws_rbtree_insert(ws_thread_volatile ws_rbtree_t *tree,
    ws_rbtree_node_t *node);
void ws_rbtree_delete(ws_thread_volatile ws_rbtree_t *tree,
    ws_rbtree_node_t *node);
ws_int32_t ws_rbtree_get(ws_thread_volatile ws_rbtree_t *tree, ws_rbtree_key_t  key,
	ws_rbtree_node_t *sentinel, ws_rbtree_node_t **node);
void ws_rbtree_insert_value(ws_rbtree_node_t *root, ws_rbtree_node_t *node,
    ws_rbtree_node_t *sentinel);
void ws_rbtree_insert_timer_value(ws_rbtree_node_t *root,
    ws_rbtree_node_t *node, ws_rbtree_node_t *sentinel);


#define ws_rbt_red(node)               ((node)->color = 1)
#define ws_rbt_black(node)             ((node)->color = 0)
#define ws_rbt_is_red(node)            ((node)->color)
#define ws_rbt_is_black(node)          (!ws_rbt_is_red(node))
#define ws_rbt_copy_color(n1, n2)      (n1->color = n2->color)

#define ws_rbtree_sentinel_init(node)  ws_rbt_black(node)

static ws_inline ws_rbtree_node_t*
ws_rbtree_min(ws_rbtree_node_t *node, ws_rbtree_node_t *sentinel)
{
    while (node->left != sentinel) {
        node = node->left;
    }

    return node;
}

static ws_inline ws_rbtree_node_t*
ws_rbtree_max(ws_rbtree_node_t *node, ws_rbtree_node_t *sentinel)
{
	while (node->right != sentinel) {
		node = node->right;
	}

	return node;
}


#endif //_WS_RBTREE_H__
