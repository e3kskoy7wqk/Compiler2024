#include <string.h>
#include <stdlib.h>
#include <time.h>
#if ! defined (_WIN32)
#include <unistd.h>
#endif
#include "all.h"

#if !defined(NDEBUG)
# if (1)
# define DEBUG_SSA
# endif
#endif

/* Per-ssa version information (induction variable descriptions, etc.).  */
struct version_info
{
    int count;
};

struct variable_info
{
    bitmap def_site;
    struct version_info *info;
    LIST stack;
#if (0)
    bitmap phiCongruenceClass;
    bitmap unresolvedNeighborMap;
#endif
};
//varinfo_t vi, newvi;

typedef struct block_info_def
{
    /* dominance frontiers.  */
    bitmap df;
    /* This bitmap contains the variables which are set before they
       are used in a basic block.  */
    bitmap kills;
    bitmap phi;
    basic_block adjvex;
} *block_info;
//block_info bi

struct pair
{
    int x;
    int y;
};

static int compare( struct pair *arg1, struct pair *arg2, void *d )
{
    if ( arg1->x < arg2->x )
        return( -1 );
    else if ( arg1->x > arg2->x )
        return( 1 );
    else if ( arg1->y < arg2->y )
        return( -1 );
    else if ( arg1->y > arg2->y )
        return( 1 );

    return( 0 );
}

static int
compare_pairs (struct pair *p1, struct pair *p2)
{
    if ( p1->x < p2->x )
        return( -1 );
    else if ( p1->x > p2->x )
        return( 1 );

    return( 0 );
}

static int
compare_integers (int *sn1, int *sn2, void *p)
{
    return (*sn1 < *sn2) ? (-1) : (*sn1 > *sn2);
}

void
compute_dominators (control_flow_graph cfun, BOOL reverse)
{
    LIST rpo = List_Create ();
    bitmap work = BITMAP_XMALLOC ();
    basic_block *bb;
    basic_block *b2;
    BOOL first = TRUE;
    BOOL changed;
    edge *ei;

    reverse = !!reverse;
    pre_and_rev_post_order_compute (cfun, NULL, rpo, TRUE, reverse);

    for(  bb=(basic_block *)List_First(rpo)
        ;  bb!=NULL
        ;  bb = (basic_block *)List_Next((void *)bb)
        )
    {
        bitmap_clear ((*bb)->dom[reverse]);
        if (first)
        {
            bitmap_set_bit ((*bb)->dom[reverse], (*bb)->index);
            first = FALSE;
        }
        else
        {
            for(  b2=(basic_block *)List_First(rpo)
                ;  b2!=NULL
                ;  b2 = (basic_block *)List_Next((void *)b2)
                )
                bitmap_set_bit ((*bb)->dom[reverse], (*b2)->index);
        }
    }

    changed = TRUE;
    while (changed)
    {
        changed = FALSE;
        first = TRUE;

        for(  bb=(basic_block *)List_First(rpo)
            ;  bb!=NULL
            ;  bb = (basic_block *)List_Next((void *)bb)
            )
        {
            if  (first)
            {
                first = FALSE;
            }
            else
            {
                BOOL first = TRUE;
                for(  ei=(edge *)List_First((reverse) ? (*bb)->succs : (*bb)->preds)
                   ;  ei!=NULL
                   ;  ei = (edge *)List_Next((void *)ei)
                   )
                {
                    if (first)
                    {
                        bitmap_copy (work, (reverse) ? (*ei)->dest->dom[reverse] : (*ei)->src->dom[reverse]);
                        first = FALSE;
                    }
                    else
                        bitmap_and_into (work, (reverse) ? (*ei)->dest->dom[reverse] : (*ei)->src->dom[reverse]);
                }
                bitmap_set_bit (work, (*bb)->index);

                if  (! bitmap_equal_p (work, (*bb)->dom[reverse]))
                {
                    bitmap_copy ((*bb)->dom[reverse], work);
                    changed = TRUE;
                }
            }
        }
    }

    List_Destroy (&rpo);
    BITMAP_XFREE (work);
}

/* Return the immediate dominator of basic block BB.  */
basic_block
get_immediate_dominator (control_flow_graph cfun, BOOL reverse, basic_block bb)
{
    basic_block      *v, n;
    edge      *curs;
    LIST Queue;
    bitmap visited;
    basic_block res = NULL;
    /* adjList: A list of vertex pointers indicating the vertices
     * that are adjacent to this vertex. */
    LIST   adjList;

    reverse = !!reverse;

    /* Start a BFS from u: add u to the queue */
    /* printf("Using as root\n"); */
    Queue = List_Create ();
    visited = BITMAP_XMALLOC ();
    bitmap_set_bit (visited, bb->index);
    *(basic_block *)List_NewLast (Queue, sizeof (basic_block)) = bb;

    /* Process the vertices in the queue */
    for(; ! List_IsEmpty (Queue); List_DeleteFirst (Queue) ) {

        v = (basic_block *)List_First (Queue);
        /* printf("Dequeued %d\n",(*v)->index); */
        if(bitmap_bit_p(bb->dom[reverse], (*v)->index) && bb != *v) {
            res = *v;
            break;
        }

        adjList = (reverse) ? (*v)->succs : (*v)->preds;
        /* Examine each of v's neighbours */
        for(  curs=(edge *)List_First(adjList)
           ;  curs!=NULL
           ;  curs = (edge *)List_Next((void *)curs)
           ) {

            /* Examine neighbour n */
            n = (reverse) ? (*curs)->dest : (*curs)->src;
            if(bitmap_bit_p(visited, n->index)) {
                continue;       /* We've already visited n */
            }

            bitmap_set_bit (visited, n->index);
        
           *(basic_block *)List_NewLast (Queue, sizeof (basic_block)) = n;
        }

    }

    List_Destroy (&Queue);
    BITMAP_XFREE (visited);
    return res;
}

/* 计算支配边界，参考 Harvey, Ferrante 等人的算法。

   这个算法可以在 Timothy Harvey 的博士论文中找到，网址为
   http://www.cs.rice.edu/~harv/dissertation.pdf 在迭代支配算法一节中。

   首先，我们识别每个合并点 j（任何有多于一个入边的节点都是合并点）。

   然后，我们检查 j 的每个前驱 p，并从 p 开始沿支配树向上走。

   我们在达到 j 的直接支配者时停止走，j 在走过的每个节点的支配边界中，但不包括 j 的直接支配
   者。直观地说，j 的其余支配者也被 j 的前驱所共享。
   因为它们支配 j，所以它们不会在它们的支配边界中包含 j。

   这个算法所触及的节点数等于支配边界的大小，不多也不少。
*/
static void
compute_dominance_frontiers (control_flow_graph cfun)
{
    edge *ei;
    basic_block *b;

    for(  b=(basic_block *)List_First(cfun->basic_block_info)
       ;  b!=NULL
       ;  b = (basic_block *)List_Next((void *)b)
       )
    {
        if (List_Card ((*b)->preds) >= 2)
        {
            for(  ei=(edge *)List_First((*b)->preds)
               ;  ei!=NULL
               ;  ei = (edge *)List_Next((void *)ei)
               )
            {
                basic_block runner = (*ei)->src;
                basic_block domsb;
                if (runner == cfun->entry_block_ptr)
                    continue;

                domsb = ((block_info)(*b)->param)->adjvex;
                while (runner != domsb)
                {
                    if (!bitmap_set_bit (((block_info)runner->param)->df,
                                         (*b)->index))
                        break;
                    runner = ((block_info)runner->param)->adjvex;
                }
            }
        }
    }
}

static void
insert_phi_nodes (control_flow_graph cfun, LIST globals, varpool_node_set set)
{
    basic_block *bb;
    IRInst *stmt;
    unsigned i1, i2, n;
    int index;
    bitmap_iterator bi1, bi2;
    bitmap vars = BITMAP_XMALLOC ();
    bitmap work = BITMAP_XMALLOC ();
    basic_block bb1, bb2;
    IRInst phi;
    IRInst first;
    void *curs;
    varpool_node vnode;

    /* 活跃变量分析。  */
    compute_liveness (cfun, set);

    /* 计算在每个基本块定值的所有变量的集合。  */
    for(  bb=(basic_block *)List_First(cfun->basic_block_info)
        ;  bb!=NULL
        ;  bb = (basic_block *)List_Next((void *)bb)
        )
    {
        for(  stmt=(IRInst *)List_First((*bb)->insns)
            ;  stmt!=NULL
            ;  stmt = (IRInst *)List_Next((void *)stmt)
            )
        {
            for (index = 0; index < IRInstGetNumOperands (*stmt); index++)
            {
                if  (IRInstIsOutput (*stmt, index))
                {
                    vnode = varpool_get_node (set, IRInstGetOperand (*stmt, index));
                    bitmap_set_bit (((block_info) (*bb)->param)->kills, vnode->uid);
                }
            }
            if  ((*stmt)->opcode == IRINST_OP_call)
            {
                for(  curs=List_First(globals)
                   ;  curs!=NULL
                   ;  curs = List_Next((void *)curs)
                   )
                    bitmap_set_bit (((block_info) (*bb)->param)->kills, (*(varpool_node *)curs)->uid);
            }
        }
    }

    for(  bb=(basic_block *)List_First(cfun->basic_block_info)
        ;  bb!=NULL
        ;  bb = (basic_block *)List_Next((void *)bb)
        )
    {
        for (bmp_iter_set_init (&bi1, ((block_info) (*bb)->param)->kills, 0, &i1);
             bmp_iter_set (&bi1, &i1);
             bmp_iter_next (&bi1, &i1))
        {
            vnode = varpool_node_set_find (set, i1);
            bitmap_set_bit (((struct variable_info *)vnode->param)->def_site, (*bb)->index);
            bitmap_set_bit (vars, vnode->uid);
        }
    }

    for (bmp_iter_set_init (&bi1, vars, 0, &i1);
         bmp_iter_set (&bi1, &i1);
         bmp_iter_next (&bi1, &i1))
    {
        vnode = varpool_node_set_find (set, i1);
        bitmap_copy (work, ((struct variable_info *)vnode->param)->def_site);
        while   (! bitmap_empty_p (work))
        {
            n = bitmap_first_set_bit(work);
            bitmap_clear_bit(work, n);
            bb1 = lookup_block_by_id(cfun, n);
            for (bmp_iter_set_init (&bi2, ((block_info) bb1->param)->df, 0, &i2);
                 bmp_iter_set (&bi2, &i2);
                 bmp_iter_next (&bi2, &i2))
            {
                bb2 = lookup_block_by_id(cfun, i2);
                if  (! bitmap_bit_p (((block_info) bb2->param)->phi, i1) &&
                          bitmap_bit_p (bb2->live_in, i1))
                {
                    first = *(IRInst *) List_First (bb2->insns);
                    phi = IRInstEmitInst (IRINST_OP_phi, first->line, first->column);
                    for(  curs=List_First(bb2->preds), index = 0
                       ;  curs!=NULL
                       ;  curs = List_Next((void *)curs), index++
                       )
                    {
                        IRInstSetOperand (phi, index, vnode->var);
                        IRInstGetOperand(phi, index)->version = vnode->version;
                    }
                    IRInstSetOperand (phi, index, vnode->var);
                    IRInstGetOperand(phi, index)->version = vnode->version;
                    InterCodeInsertAfter (bb2, NULL, phi, TRUE, NULL);
                    bitmap_set_bit (((block_info) bb2->param)->phi, i1);
                    /* 注意：虎书的此处是错的。  */
                    if  (! bitmap_bit_p (((block_info) bb2->param)->kills, i1))
                    {
                        bitmap_set_bit (work, i2);
                    }
                }
            }
        }
    }

    /* 释放分配的内存。  */
    BITMAP_XFREE (vars);
    BITMAP_XFREE (work);
}

static void
rewrite_blocks (basic_block entry, control_flow_graph cfun, LIST globals, varpool_node_set set)
{
    IRInst *stmt;
    int index;
    ssa_name name;
    basic_block *bbi;
    edge *ei1, *ei2;
    varpool_node * curs;
    varpool_node vnode;

    for(  stmt=(IRInst *)List_First(entry->insns)
        ;  stmt!=NULL
        ;  stmt = (IRInst *)List_Next((void *)stmt)
        )
    {
        if  ((*stmt)->opcode != IRINST_OP_phi)
        {
            for (index = 0; index < IRInstGetNumOperands (*stmt); index++)
            {
                name = IRInstGetOperand (*stmt, index);
                vnode = varpool_get_node(set, name);
                if  (! IRInstIsOutput (*stmt, index) && vnode)
                {
                    name->new_version = *(int *) List_Last(((struct variable_info *) vnode->param)->stack);
                }
            }
        }

        for (index = 0; index < IRInstGetNumOperands (*stmt); index++)
        {
            name = IRInstGetOperand (*stmt, index);
            if  (IRInstIsOutput (*stmt, index))
            {
                vnode = varpool_get_node(set, name);
                ((struct version_info *) vnode->var->param)->count++;
                *(int *) List_NewLast (((struct variable_info *) vnode->param)->stack, sizeof (int)) = ((struct version_info *) vnode->var->param)->count;
                name->new_version = ((struct version_info *) vnode->var->param)->count;
            }
        }

        if  ((*stmt)->opcode == IRINST_OP_call)
        {
            for(  curs=(varpool_node *)List_First(globals)
               ;  curs!=NULL
               ;  curs = (varpool_node *)List_Next((void *)curs)
               )
            {
                ((struct version_info *) (*curs)->var->param)->count++;
                *(int *) List_NewLast (((struct variable_info *) (*curs)->param)->stack, sizeof (int)) = ((struct version_info *) (*curs)->var->param)->count;
            }
        }
    }

    for(  ei1=(edge *)List_First(entry->succs)
       ;  ei1!=NULL
       ;  ei1 = (edge *)List_Next((void *)ei1)
       )
    {
        index = 0;
        for(  ei2=(edge *)List_First((*ei1)->dest->preds)
           ;  ei2!=NULL
           ;  ei2 = (edge *)List_Next((void *)ei2)
           )
        {
            index++;
            if  ((*ei2)->src == entry)
            {
                break;
            }
        }
        for(  stmt=(IRInst *)List_First((*ei1)->dest->insns)
            ;  stmt!=NULL
            ;  stmt = (IRInst *)List_Next((void *)stmt)
            )
        {
            if  ((*stmt)->opcode == IRINST_OP_phi)
            {
                name = IRInstGetOperand (*stmt, index);
                vnode = varpool_get_node(set, name);
                name->new_version = *(int *) List_Last(((struct variable_info *) vnode->param)->stack);
            }
        }
    }

    for(  bbi=(basic_block *)List_First(cfun->basic_block_info)
        ;  bbi!=NULL
        ;  bbi = (basic_block *)List_Next((void *)bbi)
        )
    {
        if  (((block_info)(*bbi)->param)->adjvex == entry)
        {
            rewrite_blocks (*bbi, cfun, globals, set);
        }
    }

    for(  stmt=(IRInst *)List_First(entry->insns)
        ;  stmt!=NULL
        ;  stmt = (IRInst *)List_Next((void *)stmt)
        )
    {
        for (index = 0; index < IRInstGetNumOperands (*stmt); index++)
        {
            name = IRInstGetOperand (*stmt, index);
            if  (IRInstIsOutput (*stmt, index))
            {
                vnode = varpool_get_node(set, name);
                List_DeleteLast (((struct variable_info *) vnode->param)->stack);
            }
        }
        if  ((*stmt)->opcode == IRINST_OP_call)
        {
            for(  curs=(varpool_node *)List_First(globals)
               ;  curs!=NULL
               ;  curs = (varpool_node *)List_Next((void *)curs)
               )
                List_DeleteLast (((struct variable_info *) (*curs)->param)->stack);
        }
    }
}

/* SSA构建器的主要入口点。重命名过程分为三个主要阶段：

   1- 计算支配边界和直接支配结点，用于在支配边界处插入PHI结点并按支配树的顺序重命名函数。

   2- 在支配边界处插入PHI节点（insert_phi_nodes）。

   3- 重命名程序中的所有块（rewrite_blocks）和语句。  */
void
build_ssa (InterCode code, SymTab stab)
{
    control_flow_graph *func;
    basic_block *bb;
    LIST globals = List_Create ();
    void * curs;
    LIST allocated;
    varpool_node_set set;
    struct avl_traverser trav;
    varpool_node vnode;
    IRInst *instr;
    int index;
    ssa_name name;

    for(  func=(control_flow_graph *)List_First(code->funcs)
        ;  func!=NULL
        ;  func = (control_flow_graph *)List_Next((void *)func)
        )
    {
        /* Initialize dominance frontier.  */
        allocated = List_Create ();
        List_Clear (globals);
        for(  bb=(basic_block *)List_First((*func)->basic_block_info)
           ;  bb!=NULL
           ;  bb = (basic_block *)List_Next((void *)bb)
           )
        {
            (*bb)->param = xmalloc (sizeof (struct block_info_def));
            memset ((*bb)->param, '\0', sizeof (struct block_info_def));
            ((block_info) (*bb)->param)->df = BITMAP_XMALLOC ();
            ((block_info) (*bb)->param)->kills = BITMAP_XMALLOC ();
            ((block_info) (*bb)->param)->phi = BITMAP_XMALLOC ();
        }

        /* 构建SSA之前，先将数组和全局变量的版本号归零。因为不同版本变量对应同一内存位置。  */
        for(  bb=(basic_block *)List_First((*func)->basic_block_info)
           ;  bb!=NULL
           ;  bb = (basic_block *)List_Next((void *)bb)
           )
        {
            for(  instr=(IRInst *)List_First((*bb)->insns)
               ;  instr!=NULL
               ;  instr = (IRInst *)List_Next((void *)instr)
               )
            {
                for (index = 0; index < IRInstGetNumOperands (*instr); index++)
                {
                    name = IRInstGetOperand (*instr, index);
                    if  (name->var->sdSymKind == SYM_VAR &&
                         (name->var->sdType->tdTypeKind > TYP_lastIntrins ||
                         (is_global_var (name->var) &&
                         !name->var->sdIsImplicit &&
                         !name->var->sdVar.sdvConst)))
                        name->version = 0;
                }
            }
        }

        set = varpool_node_set_new (*func, TRUE);
        for(  vnode = (varpool_node)avl_t_first (&trav, set->nodes)
           ;  vnode != NULL
           ;  vnode = (varpool_node)avl_t_next (&trav)
           )
        {
            if  (is_global_var (vnode->var) &&
                 !vnode->var->sdIsImplicit &&
                 vnode->var->sdSymKind == SYM_VAR &&
                 !vnode->var->sdVar.sdvConst)
            {
                *(varpool_node *) List_NewLast (globals, sizeof (varpool_node)) = vnode;
            }
            if  (! vnode->var->param)
            {
                vnode->var->param = xmalloc (sizeof (struct version_info));
                ((struct version_info *) vnode->var->param)->count = 0;
                *(SymDef *) List_NewLast (allocated, sizeof (SymDef)) = vnode->var;
            }
            vnode->param = xmalloc (sizeof (struct variable_info));
            ((struct variable_info *) vnode->param)->def_site = BITMAP_XMALLOC ();
            ((struct variable_info *) vnode->param)->info = (struct version_info *) vnode->var->param;
            ((struct variable_info *) vnode->param)->stack = List_Create ();
            *(int *) List_NewLast (((struct variable_info *) vnode->param)->stack, sizeof (int)) = 0;
        }

        /* 1- Compute dominance frontiers.  */
        compute_dominators (*func, FALSE);
        for(  bb=(basic_block *)List_First((*func)->basic_block_info)
           ;  bb!=NULL
           ;  bb = (basic_block *)List_Next((void *)bb)
           )
            ((block_info)(*bb)->param)->adjvex = get_immediate_dominator (*func, FALSE, *bb);
        compute_dominance_frontiers (*func);

        /* 2- Insert PHI nodes at dominance frontiers of definition blocks.  */
        insert_phi_nodes (*func, globals, set);

        /* 3- Rename all the blocks.  */
        rewrite_blocks ((*func)->entry_block_ptr, *func, globals, set);
        for(  bb=(basic_block *)List_First((*func)->basic_block_info)
           ;  bb!=NULL
           ;  bb = (basic_block *)List_Next((void *)bb)
           )
        {
            for(  instr=(IRInst *)List_First((*bb)->insns)
               ;  instr!=NULL
               ;  instr = (IRInst *)List_Next((void *)instr)
               )
            {
                for (index = 0; index < IRInstGetNumOperands (*instr); index++)
                {
                    name = IRInstGetOperand (*instr, index);
                    name->version = name->new_version;
                    name->new_version = 0;
                }
            }
        }

        /* Free allocated memory.  */
        for(  bb=(basic_block *)List_First((*func)->basic_block_info)
            ;  bb!=NULL
            ;  bb = (basic_block *)List_Next((void *)bb)
            )
        {
            BITMAP_XFREE (((block_info) (*bb)->param)->df);
            BITMAP_XFREE (((block_info) (*bb)->param)->kills);
            BITMAP_XFREE (((block_info) (*bb)->param)->phi);
            free ((*bb)->param);
            (*bb)->param = NULL;
        }
        for(  curs=(SymDef *)List_First(allocated)
           ;  curs!=NULL
           ;  curs = (SymDef *)List_Next((void *)curs)
           )
        {
            free ((*(SymDef *)curs)->param);
            (*(SymDef *)curs)->param = NULL;
        }
        for(  vnode = (varpool_node)avl_t_first (&trav, set->nodes)
           ;  vnode != NULL
           ;  vnode = (varpool_node)avl_t_next (&trav)
           )
        {
            BITMAP_XFREE (((struct variable_info *) vnode->param)->def_site);
            List_Destroy (&((struct variable_info *) vnode->param)->stack);
            free (vnode->param);
        }
        free_varpool_node_set (set);
        List_Destroy (&allocated);
    }

    List_Destroy (&globals);
}

static void
init_variable (varpool_node variable)
{
    if  (! variable->param)
    {
        variable->param = xmalloc (sizeof (struct variable_info));
        ((struct variable_info *) variable->param)->stack = List_Create ();
    }
}

static varpool_node
emit_copy_insn (IRInst *Curs, varpool_node_set set, SymTab stab, varpool_node dst, varpool_node src, int line, int column)
{
    SymDef sym;
    IRInst insn;
    ssa_name name;
    struct ssa_name name_buf = {0};
    varpool_node retval = NULL;

    if  (!dst)
    {
        name_buf.var = stDeclareSym (stab, NULL, SYM_VAR);
        name_buf.var->sdIsImplicit = TRUE;
        name_buf.var->sdType = CopyType (src->var->sdType);
        name_buf.version = 1;
        retval = varpool_node_set_add (set, &name_buf);
        init_variable (retval);
        dst = retval;
    }

    if  ((dst->var->sdVar.sdvLocal || is_global_var (dst->var)) &&
         (src->var->sdVar.sdvLocal || is_global_var (src->var)) &&
         !src->var->sdVar.sdvConst)
    {
        insn = IRInstEmitInst (IRINST_OP_load, line, column);
        sym = stDeclareSym (stab, NULL, SYM_VAR);
        sym->sdIsImplicit = TRUE;
        sym->sdType = CopyType (src->var->sdType);
        IRInstSetOperand (insn, 0, sym);
        name = IRInstGetOperand (insn, 0);
        name->version = 1;
        IRInstSetOperand (insn, 1, src->var);
        IRInstGetOperand (insn, 1)->version = src->version;
        InterCodeInsertBefore ((*Curs)->bb, Curs, insn, TRUE, set);
        src = varpool_node_set_add (set, name);
        init_variable (src);
        bitmap_set_bit (src->_defines, insn->uid);
    }

    if      (src->var->sdVar.sdvLocal ||
             is_global_var (src->var) ||
             src->var->sdVar.sdvConst)
        insn = IRInstEmitInst (IRINST_OP_load, line, column);
    else if (dst->var->sdVar.sdvLocal ||
             is_global_var (dst->var))
        insn = IRInstEmitInst (IRINST_OP_store, line, column);
    else
        insn = IRInstEmitInst (IRINST_OP_move, line, column);
    InterCodeInsertBefore ((*Curs)->bb, Curs, insn, TRUE, set);

    IRInstSetOperand (insn, 1, src->var);
    IRInstGetOperand (insn, 1)->version = src->version;
    IRInstSetOperand (insn, 0, dst->var);
    IRInstGetOperand (insn, 0)->version = dst->version;

    if  (retval)
        bitmap_set_bit (retval->_defines, insn->uid);

    return retval;
}

static void
schedule_copies (basic_block entry, varpool_node_set set, SymTab stab, bitmap pushed)
{
    struct avl_table *copy_set;
    struct avl_table *map;
    bitmap used_by_another;
    struct avl_table *worklist;
    IRInst *stmt;
    int index;
    varpool_node src, dest;
    edge *ei1, *ei2;
    struct pair *p;
    struct pair *q;
    struct pair *r;
    struct avl_traverser trav;
    varpool_node retval;
    IRInst temp;

    copy_set = avl_create ((avl_comparison_func *) compare, NULL, NULL);
    map = avl_create ((avl_comparison_func *) compare_pairs, NULL, NULL);
    used_by_another = BITMAP_XMALLOC ();
    worklist = avl_create ((avl_comparison_func *) compare, NULL, NULL);

    /* Pass One: Initialize the data structures.  */
    for(  ei1=(edge *)List_First(entry->succs)
       ;  ei1!=NULL
       ;  ei1 = (edge *)List_Next((void *)ei1)
       )
    {
        index = 0;
        for(  ei2=(edge *)List_First((*ei1)->dest->preds)
           ;  ei2!=NULL
           ;  ei2 = (edge *)List_Next((void *)ei2)
           )
        {
            index++;
            if  ((*ei2)->src == entry)
            {
                break;
            }
        }
        for(  stmt=(IRInst *)List_First((*ei1)->dest->insns)
            ;  stmt!=NULL
            ;  stmt = (IRInst *)List_Next((void *)stmt)
            )
        {
            if  ((*stmt)->opcode == IRINST_OP_phi)
            {
                src = varpool_get_node (set, IRInstGetOperand (*stmt, index));
                dest = varpool_get_node (set, IRInstGetOperand (*stmt, 0));

                p = (struct pair *) xmalloc (sizeof (*p));
                p->x = src->uid; p->y = dest->uid;
                free (avl_replace (copy_set, p));
                /* map[src] <- src  */
                p = (struct pair *) xmalloc (sizeof (*p));
                p->x = src->uid; p->y = src->uid;
                free (avl_replace (map, p));
                /* map[dest] <- dest  */
                p = (struct pair *) xmalloc (sizeof (*p));
                p->x = dest->uid; p->y = dest->uid;
                free (avl_replace (map, p));
                /* used_by_another[src] <- TRUE  */
                bitmap_set_bit(used_by_another, src->uid);
            }
        }
    }

    /* Pass Two: Set up the worklist of initial copies.  */
    for(  p = (struct pair *) avl_t_first (&trav, copy_set)
       ;  p != NULL
       ;  p = (struct pair *) avl_t_next (&trav)
       )
        if  (!bitmap_bit_p (used_by_another, p->y))
           avl_insert (worklist, p);
    for(  p = (struct pair *) avl_t_first (&trav, worklist)
       ;  p != NULL
       ;  p = (struct pair *) avl_t_next (&trav)
       )
        avl_delete(copy_set, p);

    /* Pass Three: Iterate over the worklist, inserting copies.  */
    while (avl_count (worklist) || avl_count (copy_set))
    {
        while (avl_count (worklist))
        {
            p = (struct pair *) avl_t_first (&trav, worklist);
            avl_delete (worklist, p);
            if  (bitmap_bit_p (entry->live_out, p->y) &&
                 varpool_node_set_find (set, p->y)->var->sdType->tdTypeKind <= TYP_lastIntrins)
            {
                /* Insert a copy from dest to a new temp t at phi-node defining dest.  */
                temp = InterCodeGetInstByID (entry->cfg->code, bitmap_first_set_bit (varpool_node_set_find (set, p->y)->_defines));
                retval = emit_copy_insn (InterCodeGetCursor (entry->cfg->code, temp), set, stab, NULL, varpool_node_set_find (set, p->y), temp->line, temp->column);
                /* push(t,Stacks[dest]).  */
                *(int *) List_NewLast (((struct variable_info *) varpool_node_set_find (set, p->y)->param)->stack, sizeof (int)) = retval->uid;
                bitmap_set_bit(pushed, p->y);
            }
            /* Insert a copy operation from map[src] to dest at the end of b.  */
            temp = *(IRInst *)List_Last (entry->insns);
            q = (struct pair *) avl_find (map, p);
            if  (varpool_node_set_find (set, p->y)->var->sdType->tdTypeKind <= TYP_lastIntrins)
                emit_copy_insn ((IRInst *)List_Last (entry->insns), set, stab, varpool_node_set_find (set, p->y), varpool_node_set_find (set, q->y), temp->line, temp->column);
            /* map[src] <- dest  */
            q->y = p->y;

            for(  q = (struct pair *) avl_t_first (&trav, copy_set)
               ;  q != NULL
               ;  q = (struct pair *) avl_t_next (&trav)
               )
                if  (q->y == p->x &&
                     q->y != q->x)
                {
                    r = (struct pair *) xmalloc (sizeof (*r));
                    memcpy (r, q, sizeof (*q));
                    free (avl_replace (worklist, r));
                }

            free (p);
        }
        if (avl_count (copy_set))
        {
            p = (struct pair *) avl_t_first (&trav, copy_set);
            avl_delete (copy_set, p);
            if  (varpool_node_set_find (set, p->y)->var->sdType->tdTypeKind <= TYP_lastIntrins)
            {
                /* Insert a copy operation from dest to a new temp t at the end of b.  */
                temp = *(IRInst *)List_Last (entry->insns);
                q = (struct pair *) avl_find (map, &p->y);
                retval = emit_copy_insn ((IRInst *)List_Last (entry->insns), set, stab, NULL, varpool_node_set_find (set, p->y), temp->line, temp->column);
                /* map[dest] <- t  */
                q->y = retval->uid;
            }
            free (avl_replace (worklist, p));
        }
    }

    avl_destroy (copy_set, (avl_item_func *) free);
    avl_destroy (map, (avl_item_func *) free);
    BITMAP_XFREE (used_by_another);
    avl_destroy (worklist, (avl_item_func *) free);
}


static void
insert_copies (basic_block entry, control_flow_graph cfun, varpool_node_set set, SymTab stab, bitmap orig_insns)
{
    IRInst *stmt;
    int index;
    varpool_node vnode;
    basic_block *bbi;
    bitmap pushed;
    bitmap_iterator bi;
    unsigned i;

    pushed = BITMAP_XMALLOC ();

    for(  stmt=(IRInst *)List_First(entry->insns)
        ;  stmt!=NULL
        ;  stmt = (IRInst *)List_Next((void *)stmt)
        )
    {
        for (index = 0; index < IRInstGetNumOperands (*stmt); index++)
        {
            vnode = varpool_get_node (set, IRInstGetOperand (*stmt, index));
            if  (! IRInstIsOutput (*stmt, index) &&
                  IRInstGetOperand (*stmt, index)->var->sdSymKind == SYM_VAR &&
                  ! IRInstGetOperand (*stmt, index)->var->sdVar.sdvConst &&
                  ! List_IsEmpty (((struct variable_info *) vnode->param)->stack) &&
                  bitmap_bit_p(orig_insns, (*stmt)->uid))
            {
                /* Replace all uses u with Stacks[u].  */
                vnode = varpool_node_set_find (set, *(int *) List_Last(((struct variable_info *) vnode->param)->stack));
                IRInstSetOperand (*stmt, index, vnode->var);
                IRInstGetOperand (*stmt, index)->version = vnode->version;
            }
        }
    }

    schedule_copies (entry, set, stab, pushed);

    for(  bbi=(basic_block *)List_First(cfun->basic_block_info)
        ;  bbi!=NULL
        ;  bbi = (basic_block *)List_Next((void *)bbi)
        )
    {
        if  (((block_info)(*bbi)->param)->adjvex == entry)
        {
            insert_copies (*bbi, cfun, set, stab, orig_insns);
        }
    }

    for (bmp_iter_set_init (&bi, pushed, 0, &i);
         bmp_iter_set (&bi, &i);
         bmp_iter_next (&bi, &i))
    {
        vnode = varpool_node_set_find (set, i);
        List_DeleteLast (((struct variable_info *) vnode->param)->stack);
    }

    BITMAP_XFREE (pushed);
}

/* Eliminate all the phi nodes.  */
static void
eliminate_phi (control_flow_graph cfun, varpool_node_set set, SymTab stab)
{
    struct avl_traverser trav;
    varpool_node vnode;
    IRInst *next_insn;
    basic_block *bb;
    IRInst *insn;
    IRInst ptr;
    bitmap orig_insns;
#if defined(DEBUG_SSA)
    clock_t this_clock = -1;
    clock_t prev_clock;
#endif

    orig_insns = BITMAP_XMALLOC ();

    for(  bb=(basic_block *)List_First(cfun->basic_block_info)
        ;  bb!=NULL
        ;  bb = (basic_block *)List_Next((void *)bb)
        )
    {
        for(  insn=(IRInst *)List_First((*bb)->insns)
            ;  insn!=NULL
            ;  insn = (IRInst *)List_Next((void *)insn)
            )
        {
            bitmap_set_bit(orig_insns, (*insn)->uid);
        }
    }

#if defined(DEBUG_SSA)
    prev_clock = clock();    
#endif
    /* 活跃变量分析。  */
    compute_liveness (cfun, set);
#if defined(DEBUG_SSA)
    this_clock = clock();
    fprintf (stdout, "  %s:delay=%ld\n", "compute_liveness", this_clock - prev_clock);
#endif

    for(  vnode = (varpool_node)avl_t_first (&trav, set->nodes)
       ;  vnode != NULL
       ;  vnode = (varpool_node)avl_t_next (&trav)
       )
    {
        init_variable (vnode);
    }

#if defined(DEBUG_SSA)
    prev_clock = clock();    
#endif
    insert_copies (cfun->entry_block_ptr, cfun, set, stab, orig_insns);
#if defined(DEBUG_SSA)
    this_clock = clock();
    fprintf (stdout, "  %s:delay=%ld\n", "insert_copies", this_clock - prev_clock);
#endif

    for(  vnode = (varpool_node)avl_t_first (&trav, set->nodes)
       ;  vnode != NULL
       ;  vnode = (varpool_node)avl_t_next (&trav)
       )
    {
        if  (NULL != vnode->param)
        {
            List_Destroy (&((struct variable_info *) vnode->param)->stack);
            free (vnode->param);
            vnode->param = NULL;
        }
    }

    for(  bb=(basic_block *)List_First(cfun->basic_block_info)
        ;  bb!=NULL
        ;  bb = (basic_block *)List_Next((void *)bb)
        )
    {
        for(  insn=(IRInst *)List_First((*bb)->insns)
            ;  insn!=NULL
            ;  insn = next_insn
            )
        {
            next_insn = (IRInst *)List_Next((void *)insn);
            if  ((*insn)->opcode == IRINST_OP_phi)
            {
                ptr = *insn;
                InterCodeRemoveInst(cfun->code, *insn, NULL);
                IRInstDelInst(ptr);
            }
        }
    }

    BITMAP_XFREE (orig_insns);
}

/* SSA形式的消去，参考 briggs, cooper 等人的算法。

   Practical Improvements to the Construction
   and Destruction of Static Single Assignment
   Form
*/
void
remove_ssa_form (InterCode code, SymTab stab)
{
    varpool_node_set set;
    control_flow_graph *func;
    basic_block *bb;
    struct avl_traverser trav;
    varpool_node vnode;
    struct avl_table *map;
    struct pair *p;
    SymDef sym;
    IRInst *instr;
    int i;

    for(  func=(control_flow_graph *)List_First(code->funcs)
       ;  func!=NULL
       ;  func = (control_flow_graph *)List_Next((void *)func)
       )
    {
        /* Initialize dominance frontier.  */
        for(  bb=(basic_block *)List_First((*func)->basic_block_info)
           ;  bb!=NULL
           ;  bb = (basic_block *)List_Next((void *)bb)
           )
        {
            (*bb)->param = xmalloc (sizeof (struct block_info_def));
            memset ((*bb)->param, '\0', sizeof (struct block_info_def));
            ((block_info) (*bb)->param)->df = BITMAP_XMALLOC ();
        }

        /* Compute dominance frontiers.  */
        compute_dominators (*func, FALSE);

        for(  bb=(basic_block *)List_First((*func)->basic_block_info)
           ;  bb!=NULL
           ;  bb = (basic_block *)List_Next((void *)bb)
           )
            ((block_info)(*bb)->param)->adjvex = get_immediate_dominator (*func, FALSE, *bb);

        set = varpool_node_set_new (*func, TRUE);
        eliminate_phi (*func, set, stab);

        /* Free allocated memory.  */
        free_varpool_node_set (set);
        for(  bb=(basic_block *)List_First((*func)->basic_block_info)
           ;  bb!=NULL
           ;  bb = (basic_block *)List_Next((void *)bb)
           )
        {
            BITMAP_XFREE (((block_info) (*bb)->param)->df);
            free ((*bb)->param);
            (*bb)->param = NULL;
        }

        set = varpool_node_set_new (*func, TRUE);
        map = avl_create ((avl_comparison_func *) compare_pairs, NULL, NULL);
        for(  vnode = (varpool_node)avl_t_first (&trav, set->nodes)
           ;  vnode != NULL
           ;  vnode = (varpool_node)avl_t_next (&trav)
           )
        {
            if  (! vnode->var->param)
            {
                vnode->var->param = xmalloc (sizeof (struct version_info));
                ((struct version_info *) vnode->var->param)->count = vnode->version;
            }
            else if (! is_global_var (vnode->var) &&
                     vnode->var->sdSymKind == SYM_VAR &&
                     vnode->var->sdType->tdTypeKind <= TYP_lastIntrins &&
                     ! vnode->var->sdVar.sdvConst)
            {
                sym = duplicate_symbol (vnode->var);
                p = (struct pair*)xmalloc (sizeof (struct pair));
                p->x = vnode->uid;
                p->y = sym->uid;
                avl_insert (map, p);
            }
        }
        for(  bb=(basic_block *)List_First((*func)->basic_block_info)
           ;  bb!=NULL
           ;  bb = (basic_block *)List_Next((void *)bb)
           )
        {
            for(  instr=(IRInst *) List_First((*bb)->insns)
               ;  instr!=NULL
               ;  instr = (IRInst *) List_Next((void *)instr)
               )
            {
                for (i = IRInstGetNumOperands (*instr) - 1; i >= 0; i--)
                {
                    vnode = varpool_get_node(set, IRInstGetOperand (*instr, i));
                    p = (struct pair *) avl_find (map, &vnode->uid);
                    if  (p)
                    {
                        IRInstSetOperand (*instr, i, stGetSymByID (stab, p->y));
                        IRInstGetOperand (*instr, i)->version = ((struct version_info *) vnode->var->param)->count;
                    }
                }
            }
        }
        for(  vnode = (varpool_node)avl_t_first (&trav, set->nodes)
           ;  vnode != NULL
           ;  vnode = (varpool_node)avl_t_next (&trav)
           )
        {
            free (vnode->var->param);
            vnode->var->param = NULL;
        }
        free_varpool_node_set (set);
        avl_destroy (map, (avl_item_func *) free);
    }
}

/* SSA形式的消去，适用于Conventional SSA。
*/
void
rewrite_out_of_ssa (InterCode code, SymTab stab)
{
    control_flow_graph *F;
    IRInst *instr, *next_insn, tmp;
    basic_block* block ;
    int i, k;
    edge *ei;
    ssa_name name;

    for(  F=(control_flow_graph *)List_First(code->funcs)
       ;  F!=NULL
       ;  F = (control_flow_graph *)List_Next((void *)F)
       )
    {
        for(  block=(basic_block *)List_First((*F)->basic_block_info)
           ;  block!=NULL
           ;  block = (basic_block *)List_Next((void *)block)
           )
        {
            for(  instr=(IRInst *) List_Last((*block)->insns)
               ;  instr!=NULL 
               ;  instr = next_insn
               )
            {
                next_insn = (IRInst *) List_Prev((void *)instr);
                if  ((*instr)->opcode == IRINST_OP_phi)
                {
                    for (i = 1; i < IRInstGetNumOperands (*instr); i++)
                    {
                        name = IRInstGetOperand (*instr, i);
                        if  (name->var->sdSymKind == SYM_VAR &&
                             name->var->sdType->tdTypeKind <= TYP_lastIntrins &&
                             name->var->sdVar.sdvConst)
                        {
                            for(  ei=(edge *) List_First((*block)->preds), k = i - 1
                               ;  ei!=NULL && k > 0
                               ;  ei = (edge *) List_Next((void *)ei), k--
                               )
                                ;
                            tmp = IRInstEmitInst ((IRInstGetOperand (*instr, 0)->var->sdVar.sdvLocal || is_global_var (IRInstGetOperand (*instr, 0)->var)) ? IRINST_OP_store : IRINST_OP_load, (*(IRInst *)List_Last ((*ei)->src->insns))->line, (*(IRInst *)List_Last ((*ei)->src->insns))->column);
                            IRInstSetOperand (tmp, 1, name->var);
                            IRInstSetOperand (tmp, 0, IRInstGetOperand (*instr, 0)->var);
                            InterCodeInsertBefore ((*ei)->src, (IRInst *) List_Last ((*ei)->src->insns), tmp, TRUE, NULL);
                        }
                    }
                    tmp = *instr;
                    InterCodeRemoveInst (code, tmp, NULL);
                    IRInstDelInst (tmp);
                }
                else
                {
                    for (i = 0; i < IRInstGetNumOperands (*instr); i++)
                    {
                        IRInstGetOperand(*instr, i)->version = 0;
                    }
                }
            }
        }
    }
}
