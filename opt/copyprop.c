#include <stdlib.h>
#include <string.h>
#include "all.h"

typedef struct block_info_def
{
    bitmap Executable;
    basic_block adjvex;
} *block_info;

struct variable_info
{
    enum { CopyTop, CopyVar, CopyBottom } CopyVal;
    varpool_node _var;
};

struct pair
{
    int x;
    int y;
};

static int compare_pairs( struct pair *arg1, struct pair *arg2 )
{
    int ret = 0 ;

    if ( arg1->x < arg2->x )
        ret = -1 ;
    else if ( arg1->x > arg2->x )
        ret = 1 ;

    return( ret );
}

static BOOL copyprop_evaluate (IRInst I, varpool_node_set set)
{
    /* state changed  */
    BOOL value_changed = FALSE;
    varpool_node dst = NULL;
    varpool_node src0;
    varpool_node vnode = NULL;

    if  (IRInstIsOutput (I, 0))
        dst = varpool_get_node (set, IRInstGetOperand (I, 0));

    switch (I->opcode)
    {
    case IRINST_OP_nop:
    case IRINST_OP_param:
    case IRINST_OP_entry:
    case IRINST_OP_exit:
    case IRINST_OP_begin_block:
    case IRINST_OP_end_block:
        /* nothing to do */
        break;

    case IRINST_OP_move:
        src0 = varpool_get_node (set, IRInstGetOperand (I, 1));
        if  (((struct variable_info *)src0->param)->CopyVal != CopyTop)
        {
            if  (((struct variable_info *)dst->param)->CopyVal == CopyVar &&
                 ((struct variable_info *)dst->param)->_var != ((struct variable_info *)src0->param)->_var)
            {
                value_changed = TRUE;
                ((struct variable_info *)dst->param)->CopyVal = CopyBottom;
                ((struct variable_info *)dst->param)->_var = dst;
            }
            else if (((struct variable_info *)dst->param)->CopyVal == CopyTop)
            {
                value_changed = TRUE;
                ((struct variable_info *)dst->param)->CopyVal = CopyVar;
                ((struct variable_info *)dst->param)->_var = ((struct variable_info *)src0->param)->_var;
            }
        }
        break;

    case IRINST_OP_ifeq:
    case IRINST_OP_ifne:
    case IRINST_OP_iflt:
    case IRINST_OP_ifge:
    case IRINST_OP_ifgt:
    case IRINST_OP_ifle:
    case IRINST_OP_goto:
        fatal ("internal compiler error");
        break;

    case IRINST_OP_load:
    case IRINST_OP_store:
    case IRINST_OP_addptr:
    case IRINST_OP_aload:
    case IRINST_OP_astore:
    case IRINST_OP_add:
    case IRINST_OP_sub:
    case IRINST_OP_mul:
    case IRINST_OP_div:
    case IRINST_OP_rem:
    case IRINST_OP_lsl:
    case IRINST_OP_lsr:
    case IRINST_OP_asr:
    case IRINST_OP_neg:
    case IRINST_OP_i2f:
    case IRINST_OP_f2i:
    case IRINST_OP_eq:
    case IRINST_OP_ne:
    case IRINST_OP_lt:
    case IRINST_OP_ge:
    case IRINST_OP_gt:
    case IRINST_OP_le:
    case IRINST_OP_not:
    case IRINST_OP_call:
    case IRINST_OP_fparam:
        if  (((struct variable_info *)dst->param)->CopyVal != CopyBottom)
        {
            value_changed = TRUE;
            ((struct variable_info *)dst->param)->CopyVal = CopyBottom;
            ((struct variable_info *)dst->param)->_var = dst;
        }
        break;

    case IRINST_OP_phi:
        do
        {
            int i;
            edge *ei;

            for(  ei=(edge *)List_First (I->bb->preds), i = 1
               ;  ei!=NULL
               ;  ei = (edge *)List_Next((void *)ei), ++i
               )
            {
                src0 = varpool_get_node (set, IRInstGetOperand(I, i));
                if  (((struct variable_info *)src0->param)->CopyVal != CopyTop &&
                     bitmap_bit_p (((block_info)(*ei)->src->param)->Executable, I->bb->index))
                    break;
            }
            if  (ei==NULL)
            {
                if  (dst->var->sdType->tdTypeKind > TYP_lastIntrins &&
                     ((struct variable_info *)dst->param)->CopyVal != CopyBottom)
                {
                    value_changed = TRUE;
                    ((struct variable_info *)dst->param)->CopyVal = CopyBottom;
                    ((struct variable_info *)dst->param)->_var = dst;
                }
                break;
            }

            for(  ei=(edge *)List_First (I->bb->preds), i = 1
               ;  ei!=NULL
               ;  ei = (edge *)List_Next((void *)ei), ++i
               )
            {
                src0 = varpool_get_node (set, IRInstGetOperand (I, i));
                if  (((struct variable_info *)src0->param)->CopyVal == CopyBottom &&
                     bitmap_bit_p (((block_info)(*ei)->src->param)->Executable, I->bb->index))
                    break;
            }
            if  (ei!=NULL)
            {
                if  (((struct variable_info *)dst->param)->CopyVal != CopyBottom)
                {
                    value_changed = TRUE;
                    ((struct variable_info *)dst->param)->CopyVal = CopyBottom;
                  ((struct variable_info *)dst->param)->_var = dst;
                }
                break;
            }

            for(  ei=(edge *)List_First (I->bb->preds), i = 1
               ;  ei!=NULL
               ;  ei = (edge *)List_Next((void *)ei), ++i
               )
            {
                src0 = varpool_get_node (set, IRInstGetOperand(I, i));
                if  (((struct variable_info *)src0->param)->CopyVal == CopyVar &&
                     bitmap_bit_p (((block_info)(*ei)->src->param)->Executable, I->bb->index))
                {
                    vnode = ((struct variable_info *)src0->param)->_var;
                }
            }
            if  (src0->var->sdType->tdTypeKind > TYP_lastIntrins &&
                 ((struct variable_info *)dst->param)->CopyVal != CopyBottom)
            {
                value_changed = TRUE;
                ((struct variable_info *)dst->param)->CopyVal = CopyBottom;
                ((struct variable_info *)dst->param)->_var = dst;
                break;
            }
            for(  ei=(edge *)List_First (I->bb->preds), i = 1
               ;  ei!=NULL
               ;  ei = (edge *)List_Next((void *)ei), ++i
               )
            {
                src0 = varpool_get_node (set, IRInstGetOperand(I, i));
                if  (((struct variable_info *)src0->param)->CopyVal == CopyVar &&
                     ((struct variable_info *)src0->param)->_var != vnode &&
                     bitmap_bit_p (((block_info)(*ei)->src->param)->Executable, I->bb->index))
                {
                    break;
                }
            }
            if  (ei==NULL)
            {
                if  (((struct variable_info *)dst->param)->CopyVal < CopyVar)
                {
                    value_changed = TRUE;
                    ((struct variable_info *)dst->param)->CopyVal = CopyVar;
                    ((struct variable_info *)dst->param)->_var = vnode;
                }
                break;
            }
            else if  (((struct variable_info *)dst->param)->CopyVal != CopyBottom)
            {
                value_changed = TRUE;
                ((struct variable_info *)dst->param)->CopyVal = CopyBottom;
                ((struct variable_info *)dst->param)->_var = dst;
            }
        }
        while (FALSE);
        break;
    }

    return value_changed;
}

static void updateInstrUses (varpool_node vnode, bitmap InstWorkList)
{
    bitmap_iterator bi;
    unsigned i;

    for (bmp_iter_set_init (&bi, vnode->use_chain, 0, &i);
         bmp_iter_set (&bi, &i);
         bmp_iter_next (&bi, &i))

        bitmap_set_bit(InstWorkList, i);
}

static void getFeasibleSuccessors (IRInst I, varpool_node_set set, bitmap succ_feasible)
{
    varpool_node src0, src1;
    IRInst target_insn;

    bitmap_clear (succ_feasible);

    switch (I->opcode)
    {
    case IRINST_OP_ifeq:
    case IRINST_OP_ifne:
        src0 = varpool_get_node (set, IRInstGetOperand(I, 0));
        src1 = varpool_get_node (set, IRInstGetOperand(I, 1));
        if  (((struct variable_info *)src0->param)->CopyVal == CopyBottom ||
             ((struct variable_info *)src1->param)->CopyVal == CopyBottom)
        {
            target_insn = InterCodeGetInstByID (I->bb->cfg->code, GetConstVal (IRInstGetOperand (I, 2)->var, 0)->cvValue.cvIval);
            bitmap_set_bit (succ_feasible, target_insn->bb->index);
            target_insn = InterCodeGetInstByID (I->bb->cfg->code, GetConstVal (IRInstGetOperand (I, 3)->var, 0)->cvValue.cvIval);
            bitmap_set_bit (succ_feasible, target_insn->bb->index);
        }
        else if (((struct variable_info *)src0->param)->CopyVal == CopyVar &&
                 ((struct variable_info *)src1->param)->CopyVal == CopyVar)
        {
            target_insn = InterCodeGetInstByID (I->bb->cfg->code, GetConstVal (IRInstGetOperand (I, (((struct variable_info *)src0->param)->_var == ((struct variable_info *)src1->param)->_var) == (I->opcode == IRINST_OP_ifeq) ? 2 : 3)->var, 0)->cvValue.cvIval);
            bitmap_set_bit (succ_feasible, target_insn->bb->index);
        }
        break;

    case IRINST_OP_goto:
        target_insn = InterCodeGetInstByID (I->bb->cfg->code, GetConstVal (IRInstGetOperand (I, 0)->var, 0)->cvValue.cvIval);
        bitmap_set_bit (succ_feasible, target_insn->bb->index);
        break;

    case IRINST_OP_iflt:
    case IRINST_OP_ifge:
    case IRINST_OP_ifgt:
    case IRINST_OP_ifle:
        target_insn = InterCodeGetInstByID (I->bb->cfg->code, GetConstVal (IRInstGetOperand (I, 2)->var, 0)->cvValue.cvIval);
        bitmap_set_bit (succ_feasible, target_insn->bb->index);
        target_insn = InterCodeGetInstByID (I->bb->cfg->code, GetConstVal (IRInstGetOperand (I, 3)->var, 0)->cvValue.cvIval);
        bitmap_set_bit (succ_feasible, target_insn->bb->index);
        break;

    default:
        fatal ("internal compiler error");
        break;
    }
}

static void copyprop_instruction (IRInst I, varpool_node_set set, bitmap InstWorkList, bitmap BBWorkList)
{
    bitmap succ_feasible;
    bitmap_iterator bi;
    unsigned int i;
    basic_block target_block;
    int k;

    switch (I->opcode)
    {
    case IRINST_OP_ifeq:
    case IRINST_OP_ifne:
    case IRINST_OP_iflt:
    case IRINST_OP_ifge:
    case IRINST_OP_ifgt:
    case IRINST_OP_ifle:
    case IRINST_OP_goto:
        succ_feasible = BITMAP_XMALLOC ();
        getFeasibleSuccessors (I, set, succ_feasible);
        for (bmp_iter_set_init (&bi, succ_feasible, 0, &i);
             bmp_iter_set (&bi, &i);
             bmp_iter_next (&bi, &i))
        {
            target_block = lookup_block_by_id (I->bb->cfg, i);
            if  (! bitmap_bit_p (((block_info)(I->bb)->param)->Executable, target_block->index))
            {
                bitmap_set_bit (((block_info)(I->bb)->param)->Executable, target_block->index);
                bitmap_set_bit (BBWorkList, target_block->index);
            }
        }
        BITMAP_XFREE (succ_feasible);
        break;

    default:
        if  (copyprop_evaluate (I, set))
            for (k = 0; k < IRInstGetNumOperands (I); k++)
                if (IRInstIsOutput (I, k))
                    updateInstrUses (varpool_get_node (set, IRInstGetOperand (I, k)), InstWorkList);
        break;
    }
}

static void copyprop_block (basic_block BB, bitmap Visited, varpool_node_set set, bitmap InstWorkList, bitmap BBWorkList)
{
    IRInst *instr;

    for(  instr=(IRInst *)List_First(BB->insns)
       ;  instr!=NULL
       ;  instr = (IRInst *)List_Next((void *)instr)
       )
        if  ((*instr)->opcode == IRINST_OP_phi)
            copyprop_instruction (*instr, set, InstWorkList, BBWorkList);

    if  (! bitmap_bit_p (Visited, BB->index))
    {
        bitmap_set_bit (Visited, BB->index);
        for(  instr=(IRInst *)List_First(BB->insns)
           ;  instr!=NULL
           ;  instr = (IRInst *)List_Next((void *)instr)
           )
            if  ((*instr)->opcode != IRINST_OP_phi)
                copyprop_instruction (*instr, set, InstWorkList, BBWorkList);
    }
}

static void
init_variable (varpool_node variable)
{
    if  (! variable->param)
    {
        variable->param = xmalloc (sizeof (struct variable_info));
        memset (variable->param, 0, sizeof (struct variable_info));
        ((struct variable_info *)variable->param)->CopyVal = CopyTop;
        ((struct variable_info *)variable->param)->_var = NULL;
        if  (variable->var->sdSymKind == SYM_VAR &&
             (variable->var->sdType->tdTypeKind > TYP_lastIntrins ||
             variable->var->sdVar.sdvConst))
        {
            ((struct variable_info *)variable->param)->CopyVal = CopyBottom;
            ((struct variable_info *)variable->param)->_var = variable;
        }
    }
}

static void
CopyPropSolver (control_flow_graph F, SymTab stab)
{
    varpool_node_set set;
    bitmap BBWorkList;
    bitmap InstWorkList;
    bitmap Visited;
    basic_block* block;
    varpool_node vnode;
    struct avl_traverser trav;
    IRInst *instr, *next;
    int i;
    LIST blocks;
    LIST Q;
    basic_block u;

    set = varpool_node_set_new(F, TRUE);
    BBWorkList = BITMAP_XMALLOC (); /* CFGWorkList  */
    InstWorkList = BITMAP_XMALLOC (); /* SSAWorkList  */
    Visited = BITMAP_XMALLOC ();

    compute_dominators (F, FALSE);

    for(  block=(basic_block *)List_First(F->basic_block_info)
       ;  block!=NULL
       ;  block = (basic_block *)List_Next((void *)block)
       )
    {
        (*block)->param = xmalloc (sizeof (struct block_info_def));
        memset ((*block)->param, 0, sizeof (struct block_info_def));
        ((block_info)(*block)->param)->Executable = BITMAP_XMALLOC ();
        ((block_info)(*block)->param)->adjvex = get_immediate_dominator (F, FALSE, *block);
    }

    /* 计算层次遍历支配者树的遍历顺序。  */
    blocks = List_Create ();
    Q = List_Create (); /* 置空的辅助队列Q */
    *(basic_block*)List_NewLast (Q, sizeof (basic_block)) = F->entry_block_ptr;
    while (!List_IsEmpty(Q))
    {
        u = *(basic_block*) List_First (Q);
        List_DeleteFirst (Q);
        *(basic_block*)List_NewLast (blocks, sizeof (basic_block)) = u;
        for(  block=(basic_block *) List_First(F->basic_block_info)
           ;  block!=NULL
           ;  block = (basic_block *) List_Next((void *)block)
           )
            if  (u == ((block_info) (*block)->param)->adjvex)
                *(basic_block*)List_NewLast (Q, sizeof (basic_block)) = *block;
    }
    List_Destroy (&Q);

    for(  vnode = (varpool_node)avl_t_first (&trav, set->nodes)
       ;  vnode != NULL
       ;  vnode = (varpool_node)avl_t_next (&trav)
       )
        init_variable (vnode);
    bitmap_set_bit (BBWorkList, F->entry_block_ptr->index);

    while (!bitmap_empty_p (BBWorkList) || !bitmap_empty_p (InstWorkList))
    {
        /* Process the instruction work list.  */
        while (!bitmap_empty_p(InstWorkList))
        {
            IRInst I = InterCodeGetInstByID (F->code, bitmap_first_set_bit (InstWorkList));
            bitmap_clear_bit (InstWorkList, I->uid);
            copyprop_instruction (I, set, InstWorkList, BBWorkList);
        }

        /* Process the basic block work list.  */
        while (!bitmap_empty_p (BBWorkList))
        {
            basic_block BB = lookup_block_by_id (F, bitmap_first_set_bit (BBWorkList));
            bitmap_clear_bit (BBWorkList, BB->index);
            copyprop_block (BB, Visited, set, InstWorkList, BBWorkList);
        }
    }

    for(  block=(basic_block *)List_First(blocks)
       ;  block!=NULL
       ;  block = (basic_block *)List_Next((void *)block)
       )
    {
        for(  instr=(IRInst *)List_First((*block)->insns)
           ;  instr!=NULL
           ;  instr = next
           )
        {
            next = (IRInst *)List_Next((void *)instr);
            for (i = IRInstGetNumOperands (*instr) - 1; i >= 0; --i)
            {
                vnode = varpool_get_node (set, IRInstGetOperand (*instr, i));
                if  (!IRInstIsOutput (*instr, i) &&
                     ((struct variable_info *) vnode->param)->CopyVal == CopyVar)
                {
#if (0)
# ifndef NDEBUG
                    fprintf (stdout,
                             "GLOBAL COPY-PROP: Replacing %s_%d in insn ",
                             stGetSymName (vnode->var), vnode->version);
                    IRInstDump (*instr, FALSE, stdout);
# endif
#endif
                    vnode = ((struct variable_info *) vnode->param)->_var;
                    IRInstSetOperand (*instr, i, vnode->var);
                    IRInstGetOperand (*instr, i)->version = vnode->version;
#if (0)
# ifndef NDEBUG
                    fprintf (stdout, " with %s_%d\n", stGetSymName (vnode->var), vnode->version);
# endif
#endif
                }
            }
        }
    }

    for(  vnode = (varpool_node)avl_t_first (&trav, set->nodes)
       ;  vnode != NULL
       ;  vnode = (varpool_node)avl_t_next (&trav)
       )
    {
        free (vnode->param);
    }

    for(  block=(basic_block *)List_First(F->basic_block_info)
       ;  block!=NULL
       ;  block = (basic_block *)List_Next((void *)block)
       )
    {
        BITMAP_XFREE (((block_info)(*block)->param)->Executable);
        free ((*block)->param);
        (*block)->param = NULL;
    }

    List_Destroy (&blocks);
    BITMAP_XFREE (Visited);
    BITMAP_XFREE (BBWorkList);
    BITMAP_XFREE (InstWorkList);
    free_varpool_node_set (set);

    /* 如果exit块不可达，则将其和entry块连接。  */
    if  (List_IsEmpty (F->exit_block_ptr->preds))
    {
        remove_edge (*(edge *)List_First(F->entry_block_ptr->succs));
        make_edge (F->entry_block_ptr, F->exit_block_ptr);
        IRInstSetOperand (*(IRInst *)List_Last (F->entry_block_ptr->insns), 0, stCreateIconNode (stab, (*(IRInst *)List_First(F->exit_block_ptr->insns))->uid));
    }

}


/* 使用SSA的复写传播。
*/
void
CopyPropPass (InterCode code, SymTab stab)
{
    control_flow_graph *F;

    for(  F=(control_flow_graph *)List_First(code->funcs)
       ;  F!=NULL
       ;  F = (control_flow_graph *)List_Next((void *)F)
       )
        CopyPropSolver (*F, stab);
}

static void
dump_bitmap_vector (FILE *file, const char *title, const char *subtitle,
                     bitmap *bmaps, struct avl_table *map)
{
    struct avl_traverser trav;
    struct pair *iter;

    fprintf (file, "%s\n", title);
    for(  iter=(struct pair *) avl_t_first (&trav, map)
       ;  iter!=NULL
       ;  iter = (struct pair *) avl_t_next (&trav)
       )
    {
        fprintf (file, "%s %d\n", subtitle, iter->x);
        dump_bitmap (file, bmaps[iter->y]);
    }

    fprintf (file, "\n");
}

/* 计算每个记录的表达式的局部属性。

   局部属性是指那些由块定义的属性，而与其他块无关。

   如果表达式的操作数DEST或SRC在块中被修改，则表达式在块中被杀死。

   如果表达式至少计算一次，则在一个块中局部可用，如果将计算
   移动到块的末尾，则表达式将包含相同的值。

   KILL和COMP是记录局部属性的目标bitmap。  */
static void
compute_local_properties (control_flow_graph cfun, struct avl_table *block_map, varpool_node_set set, bitmap ONES, bitmap *kill, bitmap *comp)
{
    basic_block* block;
    IRInst *instr;
    IRInst tmp_insn;
    bitmap temp_bitmap;
    int i, x;
    varpool_node vnode;
    bitmap_iterator bi;
    unsigned insn_index;
    bitmap_head tmp;

    /* 计算杀死集合。  */
    for(  block=(basic_block*) List_First(cfun->basic_block_info)
       ;  block!=NULL
       ;  block = (basic_block*) List_Next((void *)block)
       )
    {
        x = ((struct pair *) avl_find (block_map, &(*block)->index))->y;
        for(  instr=List_First((*block)->insns)
           ;  instr!=NULL
           ;  instr = List_Next((void *)instr)
           )
        {
            for (i = IRInstGetNumOperands (*instr) - 1; i >= 0; i--)
            {
                if  (IRInstIsOutput (*instr, i))
                {
                    vnode = varpool_get_node (set, IRInstGetOperand (*instr, i));
                    tmp.first = tmp.current = 0;
                    bitmap_ior (&tmp, vnode->use_chain, vnode->_defines);
                    for (bmp_iter_set_init (&bi, &tmp, 0, &insn_index);
                         bmp_iter_set (&bi, &insn_index);
                         bmp_iter_next (&bi, &insn_index))
                    {
                        tmp_insn = InterCodeGetInstByID (cfun->code, insn_index);
                        bitmap_set_bit (kill[x], tmp_insn->uid);
                    }
                    bitmap_clear (&tmp);
                }
            }
        }
    }

    /* 计算局部可用性。  */
    temp_bitmap = BITMAP_XMALLOC ();
    for(  block=(basic_block*) List_First(cfun->basic_block_info)
       ;  block!=NULL
       ;  block = (basic_block*) List_Next((void *)block)
       )
    {
        bitmap_clear (temp_bitmap);
        x = ((struct pair *) avl_find (block_map, &(*block)->index))->y;
        for(  instr=List_First((*block)->insns)
           ;  instr!=NULL
           ;  instr = List_Next((void *)instr)
           )
            if  (bitmap_bit_p (ONES, (*instr)->uid))
                bitmap_set_bit (comp[x], (*instr)->uid);
        for(  instr=List_Last((*block)->insns)
           ;  instr!=NULL
           ;  instr = List_Prev((void *)instr)
           )
        {
            for (i = IRInstGetNumOperands (*instr) - 1; i >= 0; i--)
            {
                if  (IRInstIsOutput(*instr, i))
                {
                    vnode = varpool_get_node (set, IRInstGetOperand (*instr, i));
                    tmp.first = tmp.current = 0;
                    bitmap_ior (&tmp, vnode->use_chain, vnode->_defines);
                    for (bmp_iter_and_compl_init (&bi, &tmp, temp_bitmap, 0,
                                                  &insn_index);
                         bmp_iter_and_compl (&bi, &insn_index);
                         bmp_iter_next (&bi, &insn_index))
                    {
                        tmp_insn = InterCodeGetInstByID (cfun->code, insn_index);
                        bitmap_clear_bit (comp[x], tmp_insn->uid);
                    }
                    bitmap_clear (&tmp);
                    bitmap_set_bit (temp_bitmap, (*instr)->uid);
                }
            }
        }
    }
    BITMAP_XFREE (temp_bitmap);
}

static struct avl_table *
find_avail_set (control_flow_graph cfun, varpool_node_set set, bitmap avin)
{
    struct avl_table *set1 = avl_create ((avl_comparison_func *) compare_pairs, NULL, NULL);
    unsigned i;
    bitmap_iterator bi;
    IRInst instr;
    struct pair *p;

    for (bmp_iter_set_init (&bi, avin, 0, &i);
         bmp_iter_set (&bi, &i);
         bmp_iter_next (&bi, &i))
    {
        instr = InterCodeGetInstByID (cfun->code, i);
        p = (struct pair *) xmalloc (sizeof (struct pair));
        p->x = varpool_get_node (set, IRInstGetOperand (instr, 0))->uid;
        p->y = varpool_get_node (set, IRInstGetOperand (instr, 1))->uid;
        free (avl_replace (set1, p));
    }
    return set1;
}

static BOOL
do_local_copyprop (basic_block block, varpool_node_set set, struct avl_table *set1)
{
    IRInst *instr;
    int i;
    varpool_node vnode;
    struct avl_traverser trav;
    struct pair *p;
    LIST _to_delete_list;
    struct pair **curs;
    BOOL changed = FALSE;

    for(  instr=(IRInst *) List_First(block->insns)
       ;  instr!=NULL
       ;  instr = (IRInst *) List_Next((void *)instr)
       )
    {
        for (i = IRInstGetNumOperands (*instr) - 1; i >= 0; i--)
        {
            if  (IRInstIsOutput (*instr, i))
            {
                vnode = varpool_get_node (set, IRInstGetOperand (*instr, i));
                _to_delete_list = List_Create ();
                for(  p = (struct pair *)avl_t_first (&trav, set1)
                   ;  p != NULL
                   ;  p = (struct pair *) avl_t_next (&trav)
                   )
                    if  (p->x == vnode->uid ||
                         p->y == vnode->uid)
                        *(struct pair **) List_NewLast (_to_delete_list, sizeof (struct pair *)) = p;
                for(  curs=(struct pair **) List_First(_to_delete_list)
                   ;  curs!=NULL
                   ;  curs = (struct pair **) List_Next((void *)curs)
                   )
                    free (avl_delete (set1, *curs));
                List_Destroy (&_to_delete_list);
            }
            else
            {
                vnode = varpool_get_node (set, IRInstGetOperand (*instr, i));
                p = (struct pair *) avl_find (set1, &vnode->uid);
                if  (p)
                {
#if (0)
# ifndef NDEBUG
                    fprintf (stdout,
                             "LOCAL COPY-PROP: Replacing %s_%d in insn ",
                             stGetSymName (vnode->var), vnode->version);
                    IRInstDump (*instr, FALSE, stdout);
# endif
#endif
                    vnode = varpool_node_set_find (set, p->y);
                    IRInstSetOperand (*instr, i, vnode->var);
                    IRInstGetOperand (*instr, i)->version = vnode->version;
                    changed = TRUE;
#if (0)
# ifndef NDEBUG
                    fprintf (stdout, " with %s_%d\n", stGetSymName (vnode->var), vnode->version);
# endif
#endif
                }
            }
        }
        if  ((*instr)->opcode == IRINST_OP_move)
        {
            p = (struct pair *) xmalloc (sizeof (struct pair));
            p->x = varpool_get_node (set, IRInstGetOperand (*instr, 0))->uid;
            p->y = varpool_get_node (set, IRInstGetOperand (*instr, 1))->uid;
            free (avl_replace (set1, p));
        }
    }

    return changed;
}

BOOL
local_copyprop_pass (basic_block block, varpool_node_set set)
{
    BOOL changed = FALSE;
    struct avl_table *set1;

    set1 = avl_create ((avl_comparison_func *) compare_pairs, NULL, NULL);
    do_local_copyprop (block, set, set1);
    avl_destroy (set1, (avl_item_func *) free);
    return changed;
}

/* Main function for the copy propagation pass.  */
static BOOL
one_copyprop_pass (control_flow_graph cfun, SymTab stab, struct avl_table *block_map)
{
    BOOL changed = FALSE;
    basic_block* block;
    IRInst *instr;
    bitmap ONES;
    int num_blocks;
    varpool_node_set set;
    struct avl_table *set1;

    /* 赋值语句的局部属性。  */
    bitmap *cprop_avloc;
    bitmap *cprop_kill;
    
    /* 赋值的全局属性(由局部属性计算得到)。  */
    bitmap *cprop_avin;
    bitmap *cprop_avout;

    num_blocks = (int) avl_count (block_map);

    /* 计算全集。  */
    ONES = BITMAP_XMALLOC ();
    for(  block=(basic_block*) List_First(cfun->basic_block_info)
       ;  block!=NULL
       ;  block = (basic_block*) List_Next((void *)block)
       )
        for(  instr=List_First((*block)->insns)
           ;  instr!=NULL
           ;  instr = List_Next((void *)instr)
           )
            if  ((*instr)->opcode == IRINST_OP_move ||
                 ((*instr)->opcode == IRINST_OP_load &&
                 IRInstGetOperand (*instr, 1)->var->sdVar.sdvConst))
                bitmap_set_bit (ONES, (*instr)->uid);

    set = varpool_node_set_new (cfun, TRUE);

    cprop_avloc = bitmap_vector_alloc (num_blocks);
    cprop_kill = bitmap_vector_alloc (num_blocks);
    compute_local_properties (cfun, block_map, set, ONES, cprop_kill, cprop_avloc);

#if (0)
# ifndef NDEBUG
    dump_bitmap_vector (stdout, "avloc", "", cprop_avloc,
                        block_map);
    dump_bitmap_vector (stdout, "kill", "", cprop_kill,
                        block_map);
# endif
#endif

    cprop_avin = bitmap_vector_alloc (num_blocks);
    cprop_avout = bitmap_vector_alloc (num_blocks);
    compute_available (cfun, block_map, ONES, cprop_avloc, cprop_kill, cprop_avout, cprop_avin);

#if (0)
# ifndef NDEBUG
    dump_bitmap_vector (stdout, "avout", "", cprop_avout, block_map);
    dump_bitmap_vector (stdout, "avin", "", cprop_avin, block_map);
# endif
#endif

    bitmap_vector_free (cprop_avloc, num_blocks);
    bitmap_vector_free (cprop_kill, num_blocks);

    for(  block=(basic_block*) List_First(cfun->basic_block_info)
       ;  block!=NULL
       ;  block = (basic_block*) List_Next((void *)block)
       )
    {
        set1 = find_avail_set (cfun, set, cprop_avin[((struct pair *) avl_find (block_map, &(*block)->index))->y]);
        (*block)->param = (void *) set1;
    }

    for(  block=(basic_block*) List_First(cfun->basic_block_info)
       ;  block!=NULL
       ;  block = (basic_block*) List_Next((void *)block)
       )
    {
        set1 = (struct avl_table *) (*block)->param;
        changed |= do_local_copyprop (*block, set, set1);
        avl_destroy (set1, (avl_item_func *) free);
        (*block)->param = NULL;
    }

    bitmap_vector_free (cprop_avin, num_blocks);
    bitmap_vector_free (cprop_avout, num_blocks);

    free_varpool_node_set (set);
    BITMAP_XFREE (ONES);
    return changed;
}

/* 不使用SSA的复写传播。
*/
void
copyprop (InterCode code, SymTab stab)
{
    control_flow_graph *F;
    struct avl_table *block_map;
    basic_block* block;
    struct pair *p;
    int num_blocks;
    BOOL updated;
    const int MAX_ITERATIONS = 100;
    int its;

    for(  F=(control_flow_graph *)List_First(code->funcs)
       ;  F!=NULL
       ;  F = (control_flow_graph *)List_Next((void *)F)
       )
    {
        /* 初始化一个从每个基本块/边到其索引的映射。  */
        block_map = avl_create ((avl_comparison_func *) compare_pairs, NULL, NULL);
        for(  block=(basic_block*) List_First((*F)->basic_block_info), num_blocks = 0
           ;  block!=NULL
           ;  block = (basic_block*) List_Next((void *)block), num_blocks++
           )
        {
            p = (struct pair *) xmalloc (sizeof (struct pair));
            p->x = (*block)->index;
            p->y = num_blocks;
            avl_insert (block_map, p);
        }

        for (its = 0, updated = TRUE;
             its < MAX_ITERATIONS && updated;
             its++)
            updated = one_copyprop_pass (*F, stab, block_map);

        avl_destroy (block_map, (avl_item_func *) free);
    }
}
