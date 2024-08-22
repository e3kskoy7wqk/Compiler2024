#include <stdlib.h>
#include <string.h>
#include "all.h"

typedef struct HTNode
{
    int weight;
    struct HTNode *parent,*lchild,*rchild;
    int prio;
    int index;
    BOOL is_root;
    varpool_node vnode;
}HTNode,*HuffmanTree; /* 动态分配数组存储赫夫曼树 */

static int compare( const HTNode **arg1, const HTNode **arg2 )
{
    int ret = 0 ;

    if ( (*arg1)->prio < (*arg2)->prio )
        ret = -1 ;
    else if ( (*arg1)->prio > (*arg2)->prio )
        ret = 1 ;

    return( ret );
}

static int
get_priority (enum IRInstOperator node)
{
    int pri= -1;

    switch (node)
    {
    case IRINST_OP_nop:
    case IRINST_OP_load:
    case IRINST_OP_aload:
    case IRINST_OP_store:
    case IRINST_OP_astore:
    case IRINST_OP_move:
    case IRINST_OP_addptr:
    case IRINST_OP_sub:
    case IRINST_OP_div:
    case IRINST_OP_rem:
    case IRINST_OP_lsl:
    case IRINST_OP_asr:
    case IRINST_OP_lsr:
    case IRINST_OP_neg:
    case IRINST_OP_i2f:
    case IRINST_OP_f2i:
    case IRINST_OP_ifeq:
    case IRINST_OP_ifne:
    case IRINST_OP_iflt:
    case IRINST_OP_ifge:
    case IRINST_OP_ifgt:
    case IRINST_OP_ifle:
    case IRINST_OP_not:
    case IRINST_OP_goto:
    case IRINST_OP_call:
    case IRINST_OP_param:
    case IRINST_OP_fparam:
    case IRINST_OP_entry:
    case IRINST_OP_exit:
    case IRINST_OP_phi:
    case IRINST_OP_begin_block:
    case IRINST_OP_end_block:
    case IRINST_OP_eq:
    case IRINST_OP_ne:
    case IRINST_OP_lt:
    case IRINST_OP_ge:
    case IRINST_OP_gt:
    case IRINST_OP_le:
        pri= -1;
        break;

    case IRINST_OP_add:
        pri= 14;
        break;

    case IRINST_OP_mul:
        pri= 15;
        break;
    }

    return pri;
}

static void
BalanceTree (SymTab stab, varpool_node_set set, basic_block block, LIST new_code, HTNode * root);

static int
do_flatten (SymTab stab, varpool_node_set set, basic_block block, LIST new_code, HTNode * p, LIST leaves)
{
    HTNode *arg0, *arg1;
    IRInst instr;

    if  (p->vnode->var->sdVar.sdvConst)
    {
        p->weight = 0;
        *(HTNode **) List_NewLast (leaves, sizeof (HTNode *)) = p;
    }
    else if (p->rchild == NULL && p->lchild == NULL)
    {
        p->weight = 1;
        *(HTNode **) List_NewLast (leaves, sizeof (HTNode *)) = p;
    }
    else if (p->is_root)
    {
        BalanceTree (stab, set, block, new_code, p);
        *(HTNode **) List_NewLast (leaves, sizeof (HTNode *)) = p;
    }
    else
    {
        instr = InterCodeGetInstByID (block->cfg->code, bitmap_first_set_bit (p->vnode->_defines));
        arg0 = (HTNode *) varpool_get_node (set, IRInstGetOperand (instr, 1))->param;
        arg1 = (HTNode *) varpool_get_node (set, IRInstGetOperand (instr, 2))->param;
        do_flatten (stab, set, block, new_code, arg0, leaves);
        do_flatten (stab, set, block, new_code, arg1, leaves);
    }
    return p->weight;
}

static void
init_variable (varpool_node variable)
{
    static int count = 0;
    if  (!variable->param)
    {
        variable->param = (HTNode *) xmalloc (sizeof (HTNode));
        memset (variable->param, 0, sizeof (HTNode));
        ((HTNode *) variable->param)->vnode = variable;
        ((HTNode *) variable->param)->weight = -1;
        ((HTNode *) variable->param)->index = ++count;
    }
}

static void
BalanceTree (SymTab stab, varpool_node_set set, basic_block block, LIST new_code, HTNode * root)
{
    LIST leaves;
    HTNode *arg0, *arg1, *arg2;
    IRInst instr;
    HTNode **minval;
    HTNode **curs;
    struct ssa_name name;
    varpool_node new_var = {0};
    IRInst new_insn;

    if  (root->weight >= 0)
        return;

    leaves = List_Create ();

    instr = InterCodeGetInstByID (block->cfg->code, bitmap_first_set_bit (root->vnode->_defines));
    arg1 = (HTNode *) varpool_get_node (set, IRInstGetOperand (instr, 1))->param;
    arg2 = (HTNode *) varpool_get_node (set, IRInstGetOperand (instr, 2))->param;
    root->weight = do_flatten (stab, set, block, new_code, arg1, leaves) + do_flatten (stab, set, block, new_code, arg2, leaves);

    while (!List_IsEmpty (leaves))
    {
        for(  curs=(HTNode **) List_First(leaves), minval = NULL
           ;  curs!=NULL
           ;  curs = (HTNode **) List_Next((void *)curs)
           )
            if  (minval == NULL ||
                 (*minval)->weight > (*curs)->weight)
                minval = curs;
        arg1 = *minval;
        List_Delete (minval);

        for(  curs=(HTNode **) List_First(leaves), minval = NULL
           ;  curs!=NULL
           ;  curs = (HTNode **) List_Next((void *)curs)
           )
            if  (minval == NULL ||
                 (*minval)->weight > (*curs)->weight)
                minval = curs;
        arg2 = *minval;
        List_Delete (minval);

        if  (List_IsEmpty (leaves))
        {
            arg0 = root;
        }
        else
        {
            name.var = stDeclareSym (stab, NULL, SYM_VAR);
            name.var->sdIsImplicit = TRUE;
            name.var->sdType = CopyType (root->vnode->var->sdType);
            name.version = 1;
            new_var = varpool_node_set_add (set, &name);
            init_variable (new_var);
            arg0 = (HTNode *) new_var->param;
        }
        new_insn = IRInstEmitInst (instr->opcode, instr->line, instr->column);
        IRInstSetOperand (new_insn, 0, arg0->vnode->var);
        IRInstGetOperand (new_insn, 0)->version = arg0->vnode->version;
        IRInstSetOperand (new_insn, 1, arg1->vnode->var);
        IRInstGetOperand (new_insn, 1)->version = arg1->vnode->version;
        IRInstSetOperand (new_insn, 2, arg2->vnode->var);
        IRInstGetOperand (new_insn, 2)->version = arg2->vnode->version;
        *(IRInst *) List_NewLast (new_code, sizeof (IRInst)) = new_insn;
        arg0->weight = arg1->weight + arg2->weight;
        if  (!List_IsEmpty (leaves))
            *(HTNode **) List_NewLast (leaves, sizeof (HTNode *)) = arg0;
    }

    List_Destroy (&leaves);
}

static void
FindRoots (SymTab stab, basic_block block, varpool_node_set set)
{
    IRInst *instr, *next_insn;
    LIST roots;
    int i, k = 0, len;
    HTNode **lst;
    HTNode **p;
    varpool_node arg0, arg1, arg2;
    struct avl_traverser trav;
    IRInst *begin, *end;
    bitmap region;
    LIST new_code;
    IRInst tmp_insn;

    begin = NULL;
    end = NULL;
    region = BITMAP_XMALLOC ();

    /* 寻找可以进行树高平衡的连续指令块。  */
    for(  instr=(IRInst *) List_First(block->insns)
       ;  instr!=NULL
       ;  instr = (IRInst *) List_Next((void *)instr)
       )
    {
        if  (get_priority ((*instr)->opcode) == -1 ||
#if 0
             0)
#else
             IRInstGetOperand (*instr, 0)->var->sdType->tdTypeKind == TYP_FLOAT)
#endif
        {
            if  (begin != NULL &&
                 begin != end)
            {
                /* 初始化。  */
                end = (IRInst *) List_Next((void *)end);
                roots = List_Create ();
                for(  instr=begin
                   ;  instr!=end
                   ;  instr = (IRInst *) List_Next((void *)instr)
                   )
                {
                    for (i = IRInstGetNumOperands (*instr) - 1; i >= 0; i--)
                    {
                        arg0 = varpool_get_node (set, IRInstGetOperand (*instr, i));
                        init_variable (arg0);
                    }
                }

                /* 识别根指令。  */
                for(  instr=begin
                   ;  instr!=end
                   ;  instr = (IRInst *) List_Next((void *)instr)
                   )
                {
                    arg0 = varpool_get_node (set, IRInstGetOperand (*instr, 0));
                    arg1 = varpool_get_node (set, IRInstGetOperand (*instr, 1));
                    arg2 = varpool_get_node (set, IRInstGetOperand (*instr, 2));
                    ((HTNode *) arg0->param)->lchild = (HTNode *) arg1->param;
                    ((HTNode *) arg0->param)->rchild = (HTNode *) arg2->param;
                    ((HTNode *) arg1->param)->parent = (HTNode *) arg0->param;
                    ((HTNode *) arg2->param)->parent = (HTNode *) arg0->param;
                    if  (bitmap_count_bits (arg0->use_chain) > 1 ||
                         (bitmap_count_bits (arg0->use_chain) == 1 &&
                         (!InterCodeGetInstByID (block->cfg->code, bitmap_first_set_bit (arg0->use_chain)) ||
                         (*instr)->opcode != InterCodeGetInstByID (block->cfg->code, bitmap_first_set_bit (arg0->use_chain))->opcode ||
                         !bitmap_bit_p (region, InterCodeGetInstByID (block->cfg->code, bitmap_first_set_bit (arg0->use_chain))->uid))))
                    {
                        ((HTNode *) arg0->param)->prio = get_priority ((*instr)->opcode);
                        ((HTNode *) arg0->param)->is_root = TRUE;
                        *(HTNode **) List_NewLast (roots, sizeof (HTNode *)) = (HTNode *) arg0->param;
                    }
                }

                /* 排序。  */
                len = List_Card (roots);
                lst = (HTNode **)xmalloc (sizeof (HTNode *) * len);
                for(  p=(HTNode **) List_First(roots), i = 0
                   ;  p!=NULL
                   ;  p = (HTNode **) List_Next((void *)p), i++
                   )
                    lst[i] = *p;
                qsort( (void *)lst, (size_t)len, sizeof( HTNode * ), (int (*)(const void*, const void*))compare );
                List_Destroy (&roots);

                new_code = List_Create ();
                for (k = 0; k < len; ++k)
                    BalanceTree (stab, set, block, new_code, lst[k]);

                for(  instr=(IRInst *) List_First(new_code)
                   ;  instr!=NULL
                   ;  instr = (IRInst *) List_Next((void *)instr)
                   )
                    InterCodeInsertBefore (block, begin, *instr, TRUE, NULL);

                List_Destroy (&new_code);

                for(  instr=begin
                   ;  instr!=end
                   ;  instr = next_insn
                   )
                {
                    next_insn = (IRInst *) List_Next((void *)instr);
                    tmp_insn = *instr;
                    InterCodeRemoveInst (block->cfg->code, tmp_insn, NULL);
                    IRInstDelInst (tmp_insn);
                }

                /* 执行清理。  */
                for(  arg0 = (varpool_node)avl_t_first (&trav, set->nodes)
                   ;  arg0 != NULL
                   ;  arg0 = (varpool_node)avl_t_next (&trav)
                   )
                    if  (arg0->param)
                    {
                        free (arg0->param);
                        arg0->param = NULL;
                    }
                free ((void *) lst);
            }
            begin = NULL;
            end = NULL;
            bitmap_clear (region);
        }
        else
        {
            if  (begin == NULL)
                begin = instr;
                
            bitmap_set_bit (region, (*instr)->uid);
            end = instr;
        }
    }

    BITMAP_XFREE (region);
}

void
treeheight (InterCode code, SymTab stab)
{
    control_flow_graph *F;
    basic_block* block;
    varpool_node_set set;

    for(  F=(control_flow_graph *)List_First(code->funcs)
       ;  F!=NULL
       ;  F = (control_flow_graph *)List_Next((void *)F)
       )
    {
        set = varpool_node_set_new (*F, TRUE);
        for(  block=(basic_block *)List_First((*F)->basic_block_info)
           ;  block!=NULL
           ;  block = (basic_block *)List_Next((void *)block)
           )
        {
            FindRoots (stab, *block, set);
        }
        free_varpool_node_set (set);
    }
}
