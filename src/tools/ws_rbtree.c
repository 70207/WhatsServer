#include <ws_std.h>


#include <ws_thread.h>
#include <ws_types.h>
#include <ws_rbtree.h>


static ws_inline void ws_rbtree_left_rotate(ws_rbtree_node_t **root,
    ws_rbtree_node_t *sentinel, ws_rbtree_node_t *node);
static ws_inline void ws_rbtree_right_rotate(ws_rbtree_node_t **root,
    ws_rbtree_node_t *sentinel, ws_rbtree_node_t *node);



void
ws_rbtree_insert(ws_thread_volatile ws_rbtree_t *tree,
    ws_rbtree_node_t *node)
{
    ws_rbtree_node_t  **root, *temp, *sentinel;

    /* a binary tree insert */

    root = (ws_rbtree_node_t **) &tree->root;
    sentinel = tree->sentinel;

    if (*root == sentinel) {
        node->parent = NULL;
        node->left = sentinel;
        node->right = sentinel;
        ws_rbt_black(node);
        *root = node;

        return;
    }

    tree->insert(*root, node, sentinel);

    /* re-balance tree */

    while (node != *root && ws_rbt_is_red(node->parent)) {

        if (node->parent == node->parent->parent->left) {
            temp = node->parent->parent->right;

            if (ws_rbt_is_red(temp)) {
                ws_rbt_black(node->parent);
                ws_rbt_black(temp);
                ws_rbt_red(node->parent->parent);
                node = node->parent->parent;

            } else {
                if (node == node->parent->right) {
                    node = node->parent;
                    ws_rbtree_left_rotate(root, sentinel, node);
                }

                ws_rbt_black(node->parent);
                ws_rbt_red(node->parent->parent);
                ws_rbtree_right_rotate(root, sentinel, node->parent->parent);
            }

        } else {
            temp = node->parent->parent->left;

            if (ws_rbt_is_red(temp)) {
                ws_rbt_black(node->parent);
                ws_rbt_black(temp);
                ws_rbt_red(node->parent->parent);
                node = node->parent->parent;

            } else {
                if (node == node->parent->left) {
                    node = node->parent;
                    ws_rbtree_right_rotate(root, sentinel, node);
                }

                ws_rbt_black(node->parent);
                ws_rbt_red(node->parent->parent);
                ws_rbtree_left_rotate(root, sentinel, node->parent->parent);
            }
        }
    }

    ws_rbt_black(*root);
}



void
ws_rbtree_insert_value(ws_rbtree_node_t *temp, ws_rbtree_node_t *node,
    ws_rbtree_node_t *sentinel)
{
    ws_rbtree_node_t  **p;

    for ( ;; ) {

        if (node->key.v < temp->key.v) {
            p = &temp->left;
        }
        else if (node->key.v == temp->key.v && node->key.v2 < temp->key.v2) {
            p = &temp->left;
        }
        else {
            p = &temp->right;
        }
      
        if (*p == sentinel) {
            break;
        }

        temp = *p;
    }

    *p = node;
    node->parent = temp;
    node->left = sentinel;
    node->right = sentinel;
    ws_rbt_red(node);
}


void
ws_rbtree_insert_timer_value(ws_rbtree_node_t *temp, ws_rbtree_node_t *node,
    ws_rbtree_node_t *sentinel)
{
    ws_rbtree_node_t  **p;

    for ( ;; ) {

        /*
         * Timer values
         * 1) are spread in small range, usually several minutes,
         * 2) and overflow each 49 days, if milliseconds are stored in 32 bits.
         * The comparison takes into account that overflow.
         */

        /*  node->key < temp->key */
        if (node->key.v < temp->key.v) {
            p = &temp->left;
        }
        else if (node->key.v == temp->key.v && node->key.v2 < temp->key.v2) {
            p = &temp->left;
        }
        else {
            p = &temp->right;
        }

        if (*p == sentinel) {
            break;
        }

        temp = *p;
    }

    *p = node;
    node->parent = temp;
    node->left = sentinel;
    node->right = sentinel;
    ws_rbt_red(node);
}

ws_int32_t ws_rbtree_get(ws_thread_volatile ws_rbtree_t *tree, ws_rbtree_key_t  key,
	ws_rbtree_node_t *sentinel, ws_rbtree_node_t **node)
{
	ws_rbtree_node_t			*nn, *parent;
	ws_int32_t							 value;

	nn = tree->root;
	*node = 0;
	while (nn != sentinel){
		parent = nn;

        if (tree->compare) {
            value = tree->compare(&nn->key, &key);
            if (value > 0) {
                nn = nn->left;
            }
            else if (value == 0) {
                *node = nn;
                return YMZ_OK;
            }
            else {
                nn = nn->right;
            }
        }
        else {
            if (nn->key.v > key.v) {
                nn = nn->left;
            }
            else if (nn->key.v == key.v && nn->key.v2 > key.v2) {
                nn = nn->left;
            }
            else if (nn->key.v < key.v) {
                nn = nn->right;
            }
            else if (nn->key.v == key.v && nn->key.v2 < key.v2) {
                nn = nn->right;
            }
            else {
                *node = nn;
                return YMZ_OK;
            }
        }

       
	}

	return YMZ_ERROR;
}

void
ws_rbtree_delete(ws_thread_volatile ws_rbtree_t *tree,
    ws_rbtree_node_t *node)
{
    ws_uint32_t         red;
    ws_rbtree_node_t  **root, *sentinel, *subst, *temp, *w;

    /* a binary tree delete */

    root = (ws_rbtree_node_t **) &tree->root;
    sentinel = tree->sentinel;

    if (node->left == sentinel) {
        temp = node->right;
        subst = node;

    } else if (node->right == sentinel) {
        temp = node->left;
        subst = node;

    } else {
        subst = ws_rbtree_min(node->right, sentinel);

        if (subst->left != sentinel) {
            temp = subst->left;
        } else {
            temp = subst->right;
        }
    }

    if (subst == *root) {
        *root = temp;
        ws_rbt_black(temp);

        /* DEBUG stuff */
        node->left = NULL;
        node->right = NULL;
        node->parent = NULL;
        node->key.v = 0;
        node->key.v2 = 0;

        return;
    }

    red = ws_rbt_is_red(subst);

    if (subst == subst->parent->left) {
        subst->parent->left = temp;

    } else {
        subst->parent->right = temp;
    }

    if (subst == node) {

        temp->parent = subst->parent;

    } else {

        if (subst->parent == node) {
            temp->parent = subst;

        } else {
            temp->parent = subst->parent;
        }

        subst->left = node->left;
        subst->right = node->right;
        subst->parent = node->parent;
        ws_rbt_copy_color(subst, node);

        if (node == *root) {
            *root = subst;

        } else {
            if (node == node->parent->left) {
                node->parent->left = subst;
            } else {
                node->parent->right = subst;
            }
        }

        if (subst->left != sentinel) {
            subst->left->parent = subst;
        }

        if (subst->right != sentinel) {
            subst->right->parent = subst;
        }
    }

    /* DEBUG stuff */
    node->left = NULL;
    node->right = NULL;
    node->parent = NULL;
    node->key.v = 0;
    node->key.v2 = 0;

    if (red) {
        return;
    }

    /* a delete fixup */

    while (temp != *root && ws_rbt_is_black(temp)) {

        if (temp == temp->parent->left) {
            w = temp->parent->right;

            if (ws_rbt_is_red(w)) {
                ws_rbt_black(w);
                ws_rbt_red(temp->parent);
                ws_rbtree_left_rotate(root, sentinel, temp->parent);
                w = temp->parent->right;
            }

            if (ws_rbt_is_black(w->left) && ws_rbt_is_black(w->right)) {
                ws_rbt_red(w);
                temp = temp->parent;

            } else {
                if (ws_rbt_is_black(w->right)) {
                    ws_rbt_black(w->left);
                    ws_rbt_red(w);
                    ws_rbtree_right_rotate(root, sentinel, w);
                    w = temp->parent->right;
                }

                ws_rbt_copy_color(w, temp->parent);
                ws_rbt_black(temp->parent);
                ws_rbt_black(w->right);
                ws_rbtree_left_rotate(root, sentinel, temp->parent);
                temp = *root;
            }

        } else {
            w = temp->parent->left;

            if (ws_rbt_is_red(w)) {
                ws_rbt_black(w);
                ws_rbt_red(temp->parent);
                ws_rbtree_right_rotate(root, sentinel, temp->parent);
                w = temp->parent->left;
            }

            if (ws_rbt_is_black(w->left) && ws_rbt_is_black(w->right)) {
                ws_rbt_red(w);
                temp = temp->parent;

            } else {
                if (ws_rbt_is_black(w->left)) {
                    ws_rbt_black(w->right);
                    ws_rbt_red(w);
                    ws_rbtree_left_rotate(root, sentinel, w);
                    w = temp->parent->left;
                }

                ws_rbt_copy_color(w, temp->parent);
                ws_rbt_black(temp->parent);
                ws_rbt_black(w->left);
                ws_rbtree_right_rotate(root, sentinel, temp->parent);
                temp = *root;
            }
        }
    }

    ws_rbt_black(temp);
}


static ws_inline void
ws_rbtree_left_rotate(ws_rbtree_node_t **root, ws_rbtree_node_t *sentinel,
    ws_rbtree_node_t *node)
{
    ws_rbtree_node_t  *temp;

    temp = node->right;
    node->right = temp->left;

    if (temp->left != sentinel) {
        temp->left->parent = node;
    }

    temp->parent = node->parent;

    if (node == *root) {
        *root = temp;

    } else if (node == node->parent->left) {
        node->parent->left = temp;

    } else {
        node->parent->right = temp;
    }

    temp->left = node;
    node->parent = temp;
}


static ws_inline void
ws_rbtree_right_rotate(ws_rbtree_node_t **root, ws_rbtree_node_t *sentinel,
    ws_rbtree_node_t *node)
{
    ws_rbtree_node_t  *temp;

    temp = node->left;
    node->left = temp->right;

    if (temp->right != sentinel) {
        temp->right->parent = node;
    }

    temp->parent = node->parent;

    if (node == *root) {
        *root = temp;

    } else if (node == node->parent->right) {
        node->parent->right = temp;

    } else {
        node->parent->left = temp;
    }

    temp->right = node;
    node->parent = temp;
}

