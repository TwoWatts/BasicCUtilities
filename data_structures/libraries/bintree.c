#include "bintree.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef bintree_t* btree_t;
typedef bintree_key_t bkey_t;
typedef bintree_entry_t bentry_t;
typedef bintree_count_t bcount_t;
typedef bentry_t* node_t;
typedef unsigned int bindex_t;

#define LEFT_CHILD_INDEX(x)  (((x)<<1)+1)
#define RIGHT_CHILD_INDEX(x) (((x)<<1)+2)
#define GET_NODE(a, n) &a->data[(n)]
#define SWAP_NODE(a, b) { bentry_t c; c = a; a = b; b = c; }
#define GET_NODE_INDEX(a, n) (bindex_t)(n - GET_NODE(a, 0))

typedef enum
{
    ERR_NULL_INSTANCE = -1,
    ERR_INV_MAX_ENTRY = -2,
    ERR_INV_MEM_ALLOC = -3,
    ERR_MALLOC_FAIL   = -4,
    ERR_INVALID_KEY   = -5,
    ERR_INV_ENTRY_CNT = -6,
    ERR_SEARCH_BOUNDS = -7,
    ERR_IMPOSSIBLE    = -8,
    BINTREE_SUCCESS   =  0,
} BINTREE_STATUS;

void
bintree_clear_entries(ins)
btree_t const ins;
{
    size_t i;
    if ( ins )
    {
        if ( ins->data )
        {
            for ( i = 0; i < ins->max_entry_count-1; ++i )
            {
                ins->data[i] = (bentry_t)
                {
                    .bf    = 0,
                    .key   = 0,
                    .value = 0,
                    .left  = GET_NODE(ins, LEFT_CHILD_INDEX(i)),
                    .right = GET_NODE(ins, RIGHT_CHILD_INDEX(i))
                };
            }
            if ( ins->max_entry_count )
            {
                ins->data[ins->max_entry_count - 1] = (bentry_t)
                {
                    .bf    = 0,
                    .key   = 0,
                    .value = 0,
                    .left  = NULL,
                    .right = NULL
                };
            }
        }
        ins->entry_count = 0;
    }
}

void
bintree_free(ins)
bintree_t * const ins;
{
    free(ins->data);
}

void
bintree_print(ins)
btree_t ins;
{
    node_t node;
    bcount_t i, c;
    printf("------------------------------------------\n");
    for ( i = 0, c = 0; c < ins->entry_count; ++i )
    {
        node = GET_NODE(ins, i);
        if ( node->key.field.valid )
        {
            c++;
            printf("%u: {K: %llu, V: %llu} {L: %u, R: %u, BF: %d}\n", 
            i, node->key.field.value, node->value, GET_NODE_INDEX(ins, node->left), GET_NODE_INDEX(ins, node->right), node->bf);
        }
    }
    printf("------------------------------------------\n");
}

static void
bintree_visit(ins, cb)
btree_t ins;
node_t root;
{
    
}

void
bintree_visualize(ins)
btree_t ins;
{
    bindex_t i, j, head, tail, layer_count;
    node_t stack[ins->entry_count];
    node_t node;

    layer_count = 1;
    memset(stack, NULL, ins->entry_count);
    stack[0] = ins->root;
    head = 0;
    tail = ins->entry_count - 1;

    // For each layer.
    for ( i = 0; i < ins->layers; ++i )
    {
        if ( !(i & 1) )
        {
            head = 0;
            // For each node in layer.
            for ( j = 0; j < layer_count; ++j )
            {
                if ( !stack[head] )
                {
                    break;
                }
                if ( stack[head]->left.key.field.valid )
                {
                    stack[tail--] = stack[head]->left;
                }
                if ( stack[head]->right.key.field.valid )
                {
                    stack[tail--] = stack[head]->right;
                }
                stack[head] = NULL;
                head++;
            }
        }
        else
        {
            tail = ins->entry_count - 1;
            // For each node in layer.
            for ( j = 0; j < layer_count; ++j )
            {
                if ( !stack[tail] )
                {
                    break;
                }
                if ( stack[tail]->left.key.field.valid )
                {
                    stack[head++] = stack[tail]->left;
                }
                if ( stack[tail]->right.key.field.valid )
                {
                    stack[head++] = stack[tail]->right;
                }
                stack[tail] = NULL;
                tail--;
            }
        }
        layer_count <<= 1
    }

}

int
bintree_init(ins, max_entry_count)
btree_t const ins;
const size_t max_entry_count;
{
    // Compute Memory Allocation
    bcount_t heap = 1, layers = 1, temp = 1;
    while ( heap < max_entry_count )
    {
        temp *= 2;
        heap += temp;
        layers++;
    }
    temp *= 2;
    heap += temp;
    layers++;
    heap *= sizeof(bentry_t);

    // Error Checks
    if ( !ins )
        return ERR_NULL_INSTANCE;
    if ( max_entry_count == 0 )
        return ERR_INV_MAX_ENTRY;
    if ( (heap < max_entry_count) )
        return ERR_INV_MEM_ALLOC;
    if ( !(ins->data = malloc(heap)) )
        return ERR_MALLOC_FAIL;

    // Instantiate Binary Tree
    ins->mem_alloc       = heap;
    ins->layers          = layers;
    ins->max_entry_count = max_entry_count;
    ins->root            = &ins->data[0];
    bintree_clear_entries(ins);
    printf("sizeof(bintree_entry_t) = %zu\n", sizeof(bintree_entry_t));
    printf("mem_alloc = %u bytes\n", ins->mem_alloc);
    printf("layers = %u\n", ins->layers);
    printf("entry count = %u\n", ins->entry_count);
    printf("max entry count = %u\n", ins->max_entry_count);
    printf("theoretical max entry count = %llu\n", ins->mem_alloc / sizeof(bintree_entry_t));
    return BINTREE_SUCCESS;
}

static inline void
bintree_right_rotate(ins, p)
btree_t ins;
node_t p;
{
//          z                                      y 
//         / \                                   /   \
//        y   T4      Right Rotate (z)          x      z
//       / \          - - - - - - - - ->      /  \    /  \ 
//      x   T3                               T1  T2  T3  T4
//     / \
//   T1   T2
    node_t c = p->left;
    p->left  = c->right;
    c->right = p;
    if ( p == ins->root )
        ins->root = c;
}

static inline void
bintree_left_rotate(ins, p)
btree_t ins;
node_t p;
{
//   z                                y
//  /  \                            /   \ 
// T1   y     Left Rotate(z)       z      x
//     /  \   - - - - - - - ->    / \    / \
//    T2   x                     T1  T2 T3  T4
//        / \
//      T3  T4
    node_t c = p->right;
    p->right = c->left;
    c->left  = p;
    if ( p == ins->root )
        ins->root = c;
}

static void
balance_tree(ins, i, callstack)
btree_t ins;
bindex_t i;
node_t *callstack;
{
    // Iterate through the callstack in order to balance the tree.
    while ( i > 2 )
    {
        node_t x, y, z;
        x = callstack[i-1];
        y = callstack[i-2];
        z = callstack[i-3];

        // Z is unbalanced on right;
        if ( z-> bf > 1 )
        {
            if ( y == z->right )
            {
                if ( x == y->right )
                {
                    bintree_left_rotate(ins, z);
                    break;
                }
                else
                {
                    bintree_right_rotate(ins, y);
                    bintree_left_rotate(ins, z);
                    break;
                }
            }
        }
        // Z is unbalanced on left.
        else if ( z->bf < 1 )
        {
            if ( y == z->left )
            {
                if ( x == y->left )
                {
                    bintree_right_rotate(ins, z);
                    break;
                }
                else
                {
                    bintree_left_rotate(ins, y);
                    bintree_right_rotate(ins, z);
                    break;
                }
            }
        }
        i--;
    }
}

bentry_t*
bintree_search(ins, key)
const bintree_t * const ins;
bkey_t key;
{
    node_t node = ins->root;

    if ( key.field.valid != 0 )
        return NULL;
    else
        key.field.valid = 1;

    while ( node )
    {
        if ( !node->key.field.valid )
            break;
        if ( key.v > node->key.v )
            node = node->right;
        else if ( key.v < node->key.v ) 
            node = node->left;
        else
            return node;
    }
    return NULL;
}

int
bintree_insert(ins, key, item)
btree_t const ins;
bkey_t key;
const size_t item;
{
    bindex_t i = 0;
    node_t node = ins->root;
    node_t callstack[ins->layers];

    if ( key.field.valid != 0 )
        return ERR_INVALID_KEY;
    else
        key.field.valid = 1;

    if ( ins->entry_count == ins->max_entry_count )
        return ERR_INV_ENTRY_CNT;

    while ( node )
    {
        if ( !node->key.field.valid )
        {
            // If it's an invalid index,
            // this slot is available.
            break;
        }

        if ( key.v > node->key.v )
        {
            node->bf++;
            callstack[i++] = node;
            if ( node->right )
            {
                node = node->right;
            }
            else
            {
                // A null child indicates that
                // the search is about to go out-of-bounds.
                return ERR_SEARCH_BOUNDS;
            }
        }
        else if ( key.v < node->key.v ) 
        {
            node->bf--;
            callstack[i++] = node;
            if ( node->left )
            {
                node = node->left;
            }
            else
            {
                // A null child indicates that
                // the search is about to go out-of-bounds.
                return ERR_SEARCH_BOUNDS;
            }
        }
        else
        {
            // If key matches an existing key,
            // replace existing node.
            node->value = item;
            return BINTREE_SUCCESS;
        }
    }

    node->bf = 0;
    node->key.v = key.v;
    node->value = item;
    ins->entry_count++;
    callstack[i++] = node;

    printf("Balancing...\n");
    balance_tree(ins, i, callstack);
    bintree_print(ins);

    return BINTREE_SUCCESS;
}

int
bintree_remove(ins, key)
btree_t ins;
bkey_t key;
{
    if ( key.field.valid != 0 )
        return ERR_INVALID_KEY;
    else
        key.field.valid = 1;

    if ( ins->entry_count == 0 )
        return ERR_INV_ENTRY_CNT;

    node_t node = ins->root;

    while ( node )
    {
        if ( !node->key.field.valid )
        {
            // If it's an invalid index,
            // there's a dead end.
            return ERR_SEARCH_BOUNDS;
        }

        if ( key.v > node->key.v )
        {
            if ( node->right )
            {
                node = node->right;
            }
            else
            {
                // A null child indicates that
                // the search is about to go out-of-bounds.
                return ERR_SEARCH_BOUNDS;
            }
        }
        else if ( key.v < node->key.v ) 
        {
            if ( node->left )
            {
                node = node->left;
            }
            else
            {
                // A null child indicates that
                // the search is about to go out-of-bounds.
                return ERR_SEARCH_BOUNDS;
            }
        }
        else
        {
            // If key matches an existing key,
            // remove existing key.
            node->key.field.valid = 0;
            ins->entry_count--;
            return BINTREE_SUCCESS;
        }
    }
    return ERR_IMPOSSIBLE;
}

