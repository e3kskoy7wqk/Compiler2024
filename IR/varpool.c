#include <stdlib.h>
#include <string.h>
#include "all.h"

static int
compare_ids (varpool_node a, varpool_node b, void *p)
{
    if (a->uid < b->uid)
        return -1;
    else if (a->uid > b->uid)
        return 1;
    return 0;
}

int compare_varpool_node( varpool_node a, varpool_node b, void *no_backend )
{
    if  (a->var->sdType->tdTypeKind != b->var->sdType->tdTypeKind)
        return a->var->sdType->tdTypeKind - b->var->sdType->tdTypeKind;

    if  (a->var->sdType->tdTypeKind <= TYP_lastIntrins)
    {
        if      (a->var->sdVar.sdvConst != b->var->sdVar.sdvConst)
        {
            return  a->var->sdVar.sdvConst - b->var->sdVar.sdvConst;
        }
        else if (a->var->sdVar.sdvConst && b->var->sdVar.sdvConst)
        {
            return compare_constant (GetConstVal (a->var, 0), GetConstVal (b->var, 0), NULL);
        }
    }

    if  (a->var->uid != b->var->uid || 
         (!no_backend && a->var->sdType->tdTypeKind == TYP_ARRAY))
        return a->var->uid - b->var->uid;

    return a->version - b->version;
}

varpool_node
varpool_get_node (varpool_node_set set, ssa_name decl)
{
    struct varpool_node vnode;
    vnode.var = decl->var;
    vnode.version = decl->version;
    return (varpool_node) avl_find (set->nodes, &vnode);
}

void varpool_node_set_update (control_flow_graph cfun, varpool_node_set set, BOOL no_backend)
{
    basic_block *bb;
    IRInst *Cursor;
    int i, count;
    varpool_node vnode;

    for(  bb=(basic_block *)List_First(cfun->basic_block_info)
        ;  bb!=NULL
        ;  bb = (basic_block *)List_Next((void *)bb)
        )
    {
        for(  Cursor=(IRInst *)List_First((*bb)->insns)
            ;  Cursor!=NULL
            ;  Cursor = (IRInst *)List_Next((void *)Cursor)
            )
        {
            count = IRInstGetNumOperands(*Cursor);
            for (i = 0; i < count; ++i)
            {
                vnode = varpool_node_set_add(set, IRInstGetOperand (*Cursor, i));
                bitmap_set_bit(IRInstIsOutput(*Cursor, i) ? vnode->_defines : vnode->use_chain, (*Cursor)->uid);
            }
        }
    }
}

/* Create a new varpool.  */
varpool_node_set
varpool_node_set_new (control_flow_graph cfun, BOOL no_backend)
{   
    varpool_node_set set;

    set = (varpool_node_set) xmalloc (sizeof (*set));
    memset (set, '\0', sizeof (*set));
    set->nodes = avl_create ((avl_comparison_func *)compare_varpool_node, (void *) (no_backend ? varpool_node_set_new : FALSE), NULL);
    set->map = avl_create ((avl_comparison_func *)compare_ids, NULL, NULL);

    if  (cfun)
    {
        varpool_node_set_update (cfun, set, no_backend);
    }

    return set;
}   

void
free_varpool_node_set (varpool_node_set set)
{
    varpool_node iter;
    struct avl_traverser trav;

    for(  iter = (varpool_node)avl_t_first (&trav, set->nodes)
       ;  iter != NULL
       ;  iter = (varpool_node)avl_t_next (&trav)
       )
    {
        List_Destroy (&iter->addr);
        List_Destroy (&iter->value);
        BITMAP_XFREE (iter->_defines);
        BITMAP_XFREE (iter->use_chain);
    }

    avl_destroy (set->nodes, (avl_item_func *) free);
    avl_destroy (set->map, NULL);
    free (set);
}

/* Add varpool_node NODE to varpool_node_set SET.  */
varpool_node
varpool_node_set_add (varpool_node_set set, ssa_name node)
{
    varpool_node vnode = varpool_get_node (set, node);
    if  (! vnode)
    {
        vnode = (varpool_node) xmalloc (sizeof (*vnode));
        memset (vnode, '\0', sizeof (*vnode));
        vnode->addr = List_Create();
        vnode->value = List_Create();
        vnode->_defines = BITMAP_XMALLOC ();
        vnode->use_chain = BITMAP_XMALLOC ();
        vnode->var = node->var;
        vnode->version = node->version;
        avl_insert (set->nodes, vnode);
        vnode->uid = ++set->count;
        avl_insert (set->map, vnode);
    }
    return vnode;
}

/* Find NODE in SET and return an iterator to it if found.  A null iterator
   is returned if NODE is not in SET.  */
varpool_node
varpool_node_set_find (varpool_node_set set, int id)
{
    struct varpool_node buf;
    buf.uid = id;
    return (varpool_node) avl_find (set->map, &buf);
}

