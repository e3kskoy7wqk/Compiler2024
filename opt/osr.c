#include <stdlib.h>
#include <string.h>
#if ! defined (_WIN32)
#include <unistd.h>
#endif
#include "all.h"

#define THRESHOLD 256
#define MAX_ITERATIONS 50
/* #define Tarjan */

#ifndef min
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif

typedef struct block_info_def
{
    /* SCC information.  */
    int dfsnum;
    int low;
    BOOL visited;
    BOOL on_sccstack;
} *block_info;

struct variable_info
{
    bitmap affect;
    bitmap constants;
};

struct entry
{
    enum IRInstOperator opcode;
    varpool_node src0, src1, dest;
};


/* Strongly Connected Component visitation info.  */
struct scc_info
{
    int current_index;
    LIST scc_stack;
    LIST _bitmaps;
};

#if defined(Tarjan)
static void
scc_visit (struct scc_info *si, basic_block block)
{
    edge* ei;
    bitmap scc;
    basic_block block2;

    *(basic_block *) List_NewLast (si->scc_stack, sizeof (basic_block)) = block;
    ((block_info) block->param)->dfsnum = ((block_info) block->param)->low = ++si->current_index;
    ((block_info) block->param)->visited = TRUE;
    ((block_info) block->param)->on_sccstack = TRUE;
    for(  ei=(edge*) List_First(block->succs)
       ;  ei!=NULL
       ;  ei = (edge*) List_Next((void *)ei)
       )
    {
        if  (!((block_info) (*ei)->dest->param)->visited)
        {
            scc_visit (si, (*ei)->dest);
            ((block_info) block->param)->low = min (((block_info) block->param)->low, ((block_info) (*ei)->dest->param)->low);
        }
        else if (((block_info) block->param)->dfsnum > ((block_info) (*ei)->dest->param)->dfsnum &&
                 ((block_info) (*ei)->dest->param)->on_sccstack)
        {
            ((block_info) block->param)->low = min (((block_info) block->param)->low, ((block_info) (*ei)->dest->param)->dfsnum);
        }
    }
    if  (((block_info) block->param)->dfsnum == ((block_info) block->param)->low)
    {
        scc = BITMAP_XMALLOC ();
        do
        {
            block2 = *(basic_block *) List_Last (si->scc_stack);
            List_DeleteLast (si->scc_stack);
            bitmap_set_bit (scc, block2->index);
            ((block_info) block2->param)->on_sccstack = FALSE;
        }
        while (block2 != block);
        *(bitmap *) List_NewLast (si->_bitmaps, sizeof (bitmap)) = scc;
    }
}
#endif /* defined(Tarjan) */

static int
compare_vars (varpool_node a, varpool_node b)
{
    int ret = 0 ;

    if ( a->uid < b->uid )
        ret = -1 ;
    else if ( a->uid > b->uid )
        ret = 1 ;

    return( ret );
}

static int
compare_entries (struct entry *first, struct entry *second)
{
    if ( first->opcode < second->opcode )
        return -1 ;
    else if ( first->opcode > second->opcode )
        return 1 ;
    if ( first->src0->uid < second->src0->uid )
        return -1 ;
    else if ( first->src0->uid > second->src0->uid )
        return 1 ;

    if  (first->src1)
    {
        if ( first->src1->uid < second->src1->uid )
            return -1 ;
        else if ( first->src1->uid > second->src1->uid )
            return 1 ;
    }

    return( 0 );
}

static varpool_node
find (struct avl_table *temp_htab, enum IRInstOperator opcode, varpool_node src0, varpool_node src1)
{
    struct entry e;
    struct entry *n;

    e.opcode = opcode;
    e.src0 = src0;
    e.src1 = src1;
    n = avl_find (temp_htab, &e);
    return n == NULL ? NULL : n->dest;
}

static void
insert (struct avl_table *temp_htab, enum IRInstOperator opcode, varpool_node src0, varpool_node src1, varpool_node dest)
{
    struct entry *n;
    n = (struct entry *) xmalloc (sizeof (*n));
    n->dest = dest;
    n->opcode = opcode;
    n->src0 = src0;
    n->src1 = src1;
    free (avl_replace (temp_htab, n));
}

static void
findivars (control_flow_graph fn, varpool_node_set set, bitmap scr, struct avl_table *rc, struct avl_table *iv, bitmap ivnodes)
{
    bitmap_iterator bi;
    unsigned block_no;
    basic_block bb;
    IRInst *instr;
    varpool_node vnode;
    BOOL changed;
    int i;

    for (bmp_iter_set_init (&bi, scr, 0, &block_no);
         bmp_iter_set (&bi, &block_no);
         bmp_iter_next (&bi, &block_no))
    {
        bb = lookup_block_by_id (fn, block_no);
        for(  instr=(IRInst *) List_First(bb->insns)
           ;  instr!=NULL
           ;  instr = (IRInst *) List_Next((void *)instr)
           )
        {
            if  ((*instr)->opcode == IRINST_OP_add ||
                 (*instr)->opcode == IRINST_OP_sub ||
                 (*instr)->opcode == IRINST_OP_move)
            {
                vnode = varpool_get_node (set, IRInstGetOperand (*instr, 0));
                bitmap_set_bit (ivnodes, (*instr)->uid);
                avl_insert (iv, vnode);
            }
        }
    }
    changed = TRUE;
    while (changed)
    {
        changed = FALSE;
        for (bmp_iter_set_init (&bi, scr, 0, &block_no);
             bmp_iter_set (&bi, &block_no);
             bmp_iter_next (&bi, &block_no))
        {
            bb = lookup_block_by_id (fn, block_no);
            for(  instr=(IRInst *) List_First(bb->insns)
               ;  instr!=NULL
               ;  instr = (IRInst *) List_Next((void *)instr)
               )
            {
                if  (bitmap_bit_p (ivnodes, (*instr)->uid) &&
                     avl_find (iv, varpool_get_node (set, IRInstGetOperand (*instr, 0))))
                {
                    for (i = 0; i < IRInstGetNumOperands (*instr); i++)
                    {
                        if  (! IRInstIsOutput (*instr, i) &&
                             ! avl_find (iv, varpool_get_node (set, IRInstGetOperand (*instr, i))) &&
                             ! avl_find (rc, varpool_get_node (set, IRInstGetOperand (*instr, i))))
                        {
                            avl_delete (iv, varpool_get_node (set, IRInstGetOperand (*instr, 0)));
                            bitmap_clear_bit (ivnodes, (*instr)->uid);
                            changed = TRUE;
                            break;
                        }
                    }
                }
            }
        }
    }
}

static void
findcands (control_flow_graph fn, varpool_node_set set, bitmap scr, struct avl_table *rc, struct avl_table *iv, bitmap cands)
{
    bitmap_iterator bi;
    unsigned block_no;
    basic_block bb;
    IRInst *instr;

    for (bmp_iter_set_init (&bi, scr, 0, &block_no);
         bmp_iter_set (&bi, &block_no);
         bmp_iter_next (&bi, &block_no))
    {
        bb = lookup_block_by_id (fn, block_no);
        for(  instr=(IRInst *) List_First(bb->insns)
           ;  instr!=NULL
           ;  instr = (IRInst *) List_Next((void *)instr)
           )
        {
            if  (((*instr)->opcode == IRINST_OP_mul &&
                 avl_find (iv, varpool_get_node (set, IRInstGetOperand (*instr, 1))) &&
                 avl_find (rc, varpool_get_node (set, IRInstGetOperand (*instr, 2)))) ||
                 ((*instr)->opcode == IRINST_OP_mul &&
                 avl_find (rc, varpool_get_node (set, IRInstGetOperand (*instr, 1))) &&
                 avl_find (iv, varpool_get_node (set, IRInstGetOperand (*instr, 2)))))
            {
                bitmap_set_bit (cands, (*instr)->uid);
            }
        }
    }
}

static void
findaffect (control_flow_graph fn, varpool_node_set set, bitmap scr, struct avl_table *iv, struct avl_table *rc)
{
    bitmap ONES;
    varpool_node vnode;
    BOOL changed;
    struct avl_traverser trav;
    bitmap_iterator bi;
    unsigned block_no;
    basic_block bb;
    IRInst *instr;
    unsigned var;
    bitmap work;
    int i;
    BOOL first;

    ONES = BITMAP_XMALLOC ();
    for(  vnode = (varpool_node)avl_t_first (&trav, iv)
       ;  vnode != NULL
       ;  vnode = (varpool_node)avl_t_next (&trav)
       )
    {
        bitmap_set_bit (((struct variable_info *) vnode->param)->affect, vnode->uid);
        bitmap_set_bit (ONES, vnode->uid);
    }
    for (bmp_iter_set_init (&bi, scr, 0, &block_no);
         bmp_iter_set (&bi, &block_no);
         bmp_iter_next (&bi, &block_no))
    {
        bb = lookup_block_by_id (fn, block_no);
        for(  instr=(IRInst *) List_First(bb->insns)
           ;  instr!=NULL
           ;  instr = (IRInst *) List_Next((void *)instr)
           )
        {
            if  (IRInstGetNumOperands (*instr) > 1 &&
                 IRInstIsOutput (*instr, 0) &&
                 avl_find (iv, varpool_get_node (set, IRInstGetOperand (*instr, 0))))
            {
                vnode = varpool_get_node (set, IRInstGetOperand(*instr, 0));
                for (i = 0; i < IRInstGetNumOperands (*instr); i++)
                {
                    if  (! IRInstIsOutput (*instr, i))
                    {
                        bitmap_set_bit (((struct variable_info *) vnode->param)->affect, varpool_get_node (set, IRInstGetOperand (*instr, i))->uid);
                    }
                }
            }
        }
    }
    work = BITMAP_XMALLOC ();
    do
    {
        changed = FALSE;
        for(  vnode = (varpool_node)avl_t_first (&trav, iv)
           ;  vnode != NULL
           ;  vnode = (varpool_node)avl_t_next (&trav)
           )
        {
            first = TRUE;
            for (bmp_iter_and_init (&bi, ONES, ((struct variable_info *)vnode->param)->affect, 0,
                                    &var);
                 bmp_iter_and (&bi, &var);
                 bmp_iter_next (&bi, &var))
            {
                if (first)
                {
                    bitmap_copy (work, ((struct variable_info *) varpool_node_set_find (set, var)->param)->affect);
                    first = FALSE;
                }
                else
                    bitmap_ior_into (work, ((struct variable_info *) varpool_node_set_find (set, var)->param)->affect);
            }
            if (bitmap_ior_into (((struct variable_info *) vnode->param)->affect, work))
                changed = TRUE;
        }
    }
    while (changed);
    BITMAP_XFREE (work);
    BITMAP_XFREE (ONES);
}

static void
init_block (basic_block  block)
{
    block->param = xmalloc (sizeof (struct block_info_def));
    memset (block->param, '\0', sizeof (struct block_info_def));
}

static int
streduce (control_flow_graph fn, varpool_node_set set, SymTab stab, basic_block prolog, bitmap scr, struct avl_table *rc)
{
    struct avl_traverser trav;
    struct avl_table *iv;
    struct avl_table *u;
    struct avl_table *temp_htab;
    bitmap ivnodes;
    bitmap cands;
    varpool_node vnode;
    varpool_node x, y, c;
    bitmap_iterator bi, bi2;
    unsigned var;
    unsigned insn_uid;
    unsigned block_no;
    IRInst insn1, insn2;
    struct ssa_name name;
    basic_block bb;
    IRInst *curs;
    int i;
    int retval;

    /* 寻找归纳变量 */ 
    ivnodes = BITMAP_XMALLOC ();
    iv = avl_create ((avl_comparison_func *) compare_vars, NULL, NULL);
    findivars (fn, set, scr, rc, iv, ivnodes);

    /* 寻找削减的候选者*/
    cands = BITMAP_XMALLOC ();
    findcands (fn, set, scr, rc, iv, cands);
    retval = bitmap_count_bits (cands);
    if  (retval > THRESHOLD || avl_count (iv) > THRESHOLD)
        goto fail;

    u = avl_create ((avl_comparison_func *) compare_vars, NULL, NULL);
    for(  vnode = (varpool_node)avl_t_first (&trav, iv)
       ;  vnode != NULL
       ;  vnode = (varpool_node)avl_t_next (&trav)
       )
        avl_insert (u, vnode);
    for(  vnode = (varpool_node)avl_t_first (&trav, rc)
       ;  vnode != NULL
       ;  vnode = (varpool_node)avl_t_next (&trav)
       )
        avl_insert (u, vnode);
    for(  vnode = (varpool_node)avl_t_first (&trav, set->nodes)
       ;  vnode != NULL
       ;  vnode = (varpool_node)avl_t_next (&trav)
       )
    {
        vnode->param = xmalloc (sizeof (struct block_info_def));
        ((struct variable_info *) vnode->param)->affect = BITMAP_XMALLOC ();
        ((struct variable_info *) vnode->param)->constants = BITMAP_XMALLOC ();
    }

    /* 找到影响关系*/
    findaffect (fn, set, scr, iv, rc);

    /* 现在遍历候选者，创建临时变量*/
    temp_htab = avl_create ((avl_comparison_func *) compare_entries, NULL, NULL);
    for (bmp_iter_set_init (&bi, cands, 0, &insn_uid);
         bmp_iter_set (&bi, &insn_uid);
         bmp_iter_next (&bi, &insn_uid))
    {
        insn1 = InterCodeGetInstByID (fn->code, insn_uid);
        if  ((insn1->opcode == IRINST_OP_mul &&
             avl_find (iv, varpool_get_node (set, IRInstGetOperand (insn1, 1))) &&
             avl_find (rc, varpool_get_node (set, IRInstGetOperand (insn1, 2)))))
        {
            x = varpool_get_node (set, IRInstGetOperand (insn1, 1));
            c = varpool_get_node (set, IRInstGetOperand (insn1, 2));
        }
        else if ((insn1->opcode == IRINST_OP_mul &&
                 avl_find (rc, varpool_get_node (set, IRInstGetOperand (insn1, 1))) &&
                 avl_find (iv, varpool_get_node (set, IRInstGetOperand (insn1, 2)))))
        {
            x = varpool_get_node (set, IRInstGetOperand (insn1, 2));
            c = varpool_get_node (set, IRInstGetOperand (insn1, 1));
        }
        else
        {
            fatal ("internal compiler error");
            x = NULL;
            c = NULL;
        }

        for (bmp_iter_set_init (&bi2, ((struct variable_info *) x->param)->affect, 0, &var);
             bmp_iter_set (&bi2, &var);
             bmp_iter_next (&bi2, &var))
        {
            y = varpool_node_set_find (set, var);
            bitmap_set_bit (((struct variable_info *) y->param)->constants, c->uid);
        }
    }

    for(  x = (varpool_node)avl_t_first (&trav, u)
       ;  x != NULL
       ;  x = (varpool_node)avl_t_next (&trav)
       )
    {
        for (bmp_iter_set_init (&bi, ((struct variable_info *) x->param)->constants, 0, &var);
             bmp_iter_set (&bi, &var);
             bmp_iter_next (&bi, &var))
        {
            c = varpool_node_set_find (set, var);
            if  (!find (temp_htab, IRINST_OP_mul, x, c))
            {
                name.var = stDeclareSym (stab, NULL, SYM_VAR);
                name.var->sdIsImplicit = TRUE;
                name.var->sdType = CopyType (x->var->sdType);
                name.version = 0;
                insert (temp_htab, IRINST_OP_mul, x, c, varpool_node_set_add (set, &name));

                /* initialization in prolog */
                insn2 = IRInstEmitInst (IRINST_OP_mul, (*(IRInst *)List_Last (prolog->insns))->line, (*(IRInst *)List_Last (prolog->insns))->column);
                IRInstSetOperand (insn2, 1, x->var);
                IRInstGetOperand (insn2, 1)->version = x->version;
                IRInstSetOperand (insn2, 2, c->var);
                IRInstGetOperand (insn2, 2)->version = c->version;
                IRInstSetOperand (insn2, 0, name.var);
                IRInstGetOperand (insn2, 0)->version = name.version;
                InterCodeInsertBefore (prolog, (IRInst *)List_Last (prolog->insns), insn2, TRUE, set);

                /* double entries for canst * canst  */
                if  (avl_find (rc, x))
                    insert (temp_htab, IRINST_OP_mul, c, x, find (temp_htab, IRINST_OP_mul, x, c));
            }
        }
    }

    for (bmp_iter_set_init (&bi, scr, 0, &block_no);
         bmp_iter_set (&bi, &block_no);
         bmp_iter_next (&bi, &block_no))
    {
        bb = lookup_block_by_id (fn, block_no);
        for(  curs=(IRInst *) List_First(bb->insns)
           ;  curs!=NULL
           ;  curs = (IRInst *) List_Next((void *)curs)
           )
        {
            if  (IRInstGetNumOperands (*curs) > 1 &&
                 IRInstIsOutput (*curs, 0) &&
                 avl_find (iv, varpool_get_node (set, IRInstGetOperand (*curs, 0))))
            {
                for (bmp_iter_set_init (&bi2, ((struct variable_info *) varpool_get_node (set, IRInstGetOperand (*curs, 0))->param)->constants, 0, &var);
                     bmp_iter_set (&bi2, &var);
                     bmp_iter_next (&bi2, &var))
                {
                    c = varpool_node_set_find (set, var);
                    insn2 = IRInstEmitInst ((*curs)->opcode, (*curs)-> line, (*curs)-> column);
                    for (i = 0; i < IRInstGetNumOperands (*curs); i++)
                    {
                        vnode = varpool_get_node (set, IRInstGetOperand (*curs, i));
                        vnode = find (temp_htab, IRINST_OP_mul, vnode, c);
                        IRInstSetOperand (insn2, i, vnode->var);
                        IRInstGetOperand (insn2, i)->version = vnode->version;
                    }
                    InterCodeInsertAfter (bb, curs, insn2, TRUE, set);
                }
            }
        }
    }

    /* 现在用store操作替换候选者*/
    for (bmp_iter_set_init (&bi, cands, 0, &insn_uid);
         bmp_iter_set (&bi, &insn_uid);
         bmp_iter_next (&bi, &insn_uid))
    {
        insn1 = InterCodeGetInstByID (fn->code, insn_uid);
        if  ((insn1->opcode == IRINST_OP_mul &&
             avl_find (iv, varpool_get_node (set, IRInstGetOperand (insn1, 1))) &&
             avl_find (rc, varpool_get_node (set, IRInstGetOperand (insn1, 2)))))
        {
            x = varpool_get_node (set, IRInstGetOperand (insn1, 1));
            c = varpool_get_node (set, IRInstGetOperand (insn1, 2));
        }
        else if ((insn1->opcode == IRINST_OP_mul &&
                 avl_find (rc, varpool_get_node (set, IRInstGetOperand (insn1, 1))) &&
                 avl_find (iv, varpool_get_node (set, IRInstGetOperand (insn1, 2)))))
        {
            x = varpool_get_node (set, IRInstGetOperand (insn1, 2));
            c = varpool_get_node (set, IRInstGetOperand (insn1, 1));
        }
        else
        {
            fatal ("internal compiler error");
            x = NULL;
            c = NULL;
        }

        insn1->opcode = IRINST_OP_move;
        vnode = find (temp_htab, IRINST_OP_mul, x, c);
        IRInstSetOperand (insn1, 1, vnode->var);
        IRInstGetOperand (insn1, 1)->version = vnode->version;
    }

    /* 线性函数判断替代 */
    /* Note: could overflow.  */
#if 0
    for (bmp_iter_set_init (&bi, scr, 0, &block_no);
         bmp_iter_set (&bi, &block_no);
         bmp_iter_next (&bi, &block_no))
    {
        bb = lookup_block_by_id (fn, block_no);
        insn1 = *(IRInst *) List_Last (bb->insns);

        if  (insn1->opcode == IRINST_OP_goto)
            continue;

        if  (avl_find (iv, varpool_get_node (set, IRInstGetOperand (insn1, 0))) &&
             avl_find (rc, varpool_get_node (set, IRInstGetOperand (insn1, 1))))
        {
            x = varpool_get_node (set, IRInstGetOperand (insn1, 0));
            y = varpool_get_node (set, IRInstGetOperand (insn1, 1));
        }
        else if (avl_find (rc, varpool_get_node (set, IRInstGetOperand (insn1, 0))) &&
                 avl_find (iv, varpool_get_node (set, IRInstGetOperand (insn1, 1))))
        {
            x = varpool_get_node (set, IRInstGetOperand (insn1, 1));
            y = varpool_get_node (set, IRInstGetOperand (insn1, 0));
        }
        else
            continue;

        if  (bitmap_empty_p (((struct variable_info *) x->param)->constants))
            continue;
        c = varpool_node_set_find (set, bitmap_first_set_bit (((struct variable_info *) x->param)->constants));

        if  (!find (temp_htab, IRINST_OP_mul, y, c))
        {
            name.var = stDeclareSym (stab, NULL, SYM_VAR);
            name.var->sdIsImplicit = TRUE;
            name.var->sdType = CopyType (y->var->sdType);
            name.version = 0;
            insert (temp_htab, IRINST_OP_mul, y, c, varpool_node_set_add (set, &name));

            insn2 = IRInstEmitInst (IRINST_OP_mul, (*(IRInst *)List_Last (prolog->insns))->line, (*(IRInst *)List_Last (prolog->insns))->column);
            IRInstSetOperand (insn2, 1, y->var);
            IRInstGetOperand (insn2, 1)->version = y->version;
            IRInstSetOperand (insn2, 2, c->var);
            IRInstGetOperand (insn2, 2)->version = c->version;
            IRInstSetOperand (insn2, 0, name.var);
            IRInstGetOperand (insn2, 0)->version = name.version;
            InterCodeInsertBefore (prolog, (IRInst *)List_Last (prolog->insns), insn2, TRUE, set);

            /* double entries for canst * canst  */
            if  (avl_find (rc, y))
                insert (temp_htab, IRINST_OP_mul, c, y, find (temp_htab, IRINST_OP_mul, y, c));
        }

        vnode = varpool_get_node (set, IRInstGetOperand (insn1, 0));
        vnode = find (temp_htab, IRINST_OP_mul, vnode, c);
        IRInstSetOperand (insn1, 0, vnode->var);
        IRInstGetOperand (insn1, 0)->version = vnode->version;
        vnode = varpool_get_node (set, IRInstGetOperand (insn1, 1));
        vnode = find (temp_htab, IRINST_OP_mul, vnode, c);
        IRInstSetOperand (insn1, 1, vnode->var);
        IRInstGetOperand (insn1, 1)->version = vnode->version;
    }
#endif /* 0 */

    for(  vnode = (varpool_node)avl_t_first (&trav, set->nodes)
       ;  vnode != NULL
       ;  vnode = (varpool_node)avl_t_next (&trav)
       )
    {
        if  (vnode->param)
        {
            BITMAP_XFREE (((struct variable_info *) vnode->param)->affect);
            BITMAP_XFREE (((struct variable_info *) vnode->param)->constants);
            free (vnode->param);
            vnode->param = NULL;
        }
    }
    BITMAP_XFREE (ivnodes);
    BITMAP_XFREE (cands);
    avl_destroy (iv, NULL);
    avl_destroy (u, NULL);
    avl_destroy (temp_htab, (avl_item_func *) free);
    return retval;

fail:
    BITMAP_XFREE (ivnodes);
    BITMAP_XFREE (cands);
    avl_destroy (iv, NULL);
    return 0;
}

/* 为prolog生成指令。  */
static basic_block
generate_prolog (SymTab stab, bitmap scc, basic_block header)
{
    edge *ei;
    basic_block new_block;
    edge *next_edge;
    IRInst jump_insn;

    new_block = alloc_block ();
    link_block (header->cfg, new_block);
    jump_insn = IRInstEmitInst (IRINST_OP_goto, (*(IRInst *)List_First (header->insns))->line, (*(IRInst *)List_First (header->insns))->column);
    InterCodeAddInst (new_block, jump_insn, TRUE);
    IRInstSetOperand (jump_insn, 0, stCreateIconNode(stab, (*(IRInst *)List_First (header->insns))->uid));

    for(  ei=(edge *) List_First(header->preds)
       ;  ei!=NULL
       ;  ei = next_edge
       )
    {
        next_edge = (edge *) List_Next((void *)ei);
        if  (!bitmap_bit_p (scc, (*ei)->src->index))
        {
            make_edge ((*ei)->src, new_block);
            remove_edge (*ei);
        }
    }
    make_edge (new_block, header);

    update_destinations (new_block, (*(IRInst *)List_First (header->insns))->uid, jump_insn->uid, NULL);

    return new_block;
}

void
OSR (InterCode code, SymTab stab)
{
    control_flow_graph *F;
    basic_block* block;
    struct scc_info si;
    bitmap *bm;
    struct avl_table *rc;
    IRInst *instr;
    basic_block bb;
    bitmap_iterator bi;
    unsigned block_no;
    varpool_node_set set;
    varpool_node vnode;
    int i;
    int count;
    LIST rpo;
    basic_block prolog;
    int its;
#if !defined(Tarjan)
    struct loop **loop;
    LIST blocks;
#endif /* !defined(Tarjan) */

    for(  F=(control_flow_graph *)List_First(code->funcs)
       ;  F!=NULL
       ;  F = (control_flow_graph *)List_Next((void *)F)
       )
    {
        for(  block=(basic_block*) List_First((*F)->basic_block_info)
           ;  block!=NULL
           ;  block = (basic_block*) List_Next((void *)block)
           )
            init_block (*block);

#if defined(Tarjan)
        /* Tarjan's strongly connected component finding algorithm.  */
        memset (&si, '\0', sizeof (si));
        si.scc_stack = List_Create ();
        si._bitmaps = List_Create ();
        scc_visit (&si, (*F)->entry_block_ptr);
        List_Destroy (&si.scc_stack);
        compute_dominators (*F, FALSE);
#else /* !defined(Tarjan) */
        flow_loops_find (*F);
/*      flow_loops_dump ((*F)->loops, stdout); */
        si._bitmaps = List_Create ();
        for(  loop=(struct loop **)List_Last((*F)->loops->larray)
           ;  loop!=NULL
           ;  loop = (struct loop **) List_Prev((void *)loop)
           )
        {
            if ((*loop)->header == (*F)->entry_block_ptr)
                continue;
            blocks = get_loop_body (*loop);
            bm = (bitmap *) List_NewLast (si._bitmaps, sizeof (bitmap));
            *bm = BITMAP_XMALLOC ();
            for(  block=(basic_block*) List_First(blocks)
               ;  block!=NULL
               ;  block = (basic_block*) List_Next((void *)block)
               )
                bitmap_set_bit (*bm, (*block)->index);
            List_Destroy (&blocks);
        }
        flow_loops_free (&(*F)->loops);
#endif /* defined(Tarjan) */

        set = varpool_node_set_new (*F, TRUE);
        rpo = List_Create ();
        pre_and_rev_post_order_compute (*F, NULL, rpo, TRUE, FALSE);

        for(  bm=(bitmap *) List_First(si._bitmaps)
           ;  bm!=NULL
           ;  bm = (bitmap *) List_Next((void *)bm)
           )
        {
            /* 寻找强连通分量的头结点。  */
            for(  block=(basic_block*) List_First(rpo)
               ;  block!=NULL
               ;  block = (basic_block*) List_Next((void *)block)
               )
                if  (bitmap_bit_p (*bm, (*block)->index) &&
                     is_loop_head (*F, *block))
                    break;
            if  (!block)
                continue;

            prolog = generate_prolog (stab, *bm, *block);
            init_block (prolog);

            for (its = 0, count = 1;
                 its < MAX_ITERATIONS && count;
                 its++)
            {
                /* 计算区域常量。  */
                rc = avl_create ((avl_comparison_func *) compare_vars, NULL, NULL);
                for (bmp_iter_set_init (&bi, *bm, 0, &block_no);
                     bmp_iter_set (&bi, &block_no);
                     bmp_iter_next (&bi, &block_no))
                {
                    bb = lookup_block_by_id (*F, block_no);
                    for(  instr=(IRInst *) List_First(bb->insns)
                       ;  instr!=NULL
                       ;  instr = (IRInst *) List_Next((void *)instr)
                       )
                    {
                        for (i = 0; i < IRInstGetNumOperands (*instr); i++)
                        {
                            vnode = varpool_get_node (set, IRInstGetOperand (*instr, i));
                            if  (vnode != NULL &&
                                 vnode->var->sdSymKind == SYM_VAR &&
#if 0
                                 1)
#else
                                 /* That's right  */
                                 !vnode->var->sdVar.sdvConst)
#endif
                                avl_insert (rc, vnode);
                        }
                    }
                }
                for (bmp_iter_set_init (&bi, *bm, 0, &block_no);
                     bmp_iter_set (&bi, &block_no);
                     bmp_iter_next (&bi, &block_no))
                {
                    bb = lookup_block_by_id (*F, block_no);
                    for(  instr=(IRInst *) List_First(bb->insns)
                       ;  instr!=NULL
                       ;  instr = (IRInst *) List_Next((void *)instr)
                       )
                    {
                        for (i = 0; i < IRInstGetNumOperands (*instr); i++)
                        {
                            if  (IRInstIsOutput (*instr, i))
                            {
                                vnode = varpool_get_node (set, IRInstGetOperand (*instr, i));
                                avl_delete (rc, vnode);
                            }
                        }
                    }
                }

                /* 强度削减。  */
                count = streduce (*F, set, stab, prolog, *bm, rc);

                /* 执行清理。  */
                avl_destroy (rc, NULL);
            }
        }

        /* 执行清理。  */
        for(  bm=(bitmap *) List_First(si._bitmaps)
           ;  bm!=NULL
           ;  bm = (bitmap *) List_Next((void *)bm)
           )
            BITMAP_XFREE (*bm);
        List_Destroy (&si._bitmaps);
        for(  block=(basic_block*) List_First((*F)->basic_block_info)
           ;  block!=NULL
           ;  block = (basic_block*) List_Next((void *)block)
           )
        {
            free ((*block)->param);
            (*block)->param = NULL;
        }
        List_Destroy (&rpo);
        free_varpool_node_set (set);

        cleanup_cfg (*F, stab);
    }
}
