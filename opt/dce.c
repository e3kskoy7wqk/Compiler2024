#include <string.h>
#include <stdlib.h>
#include "all.h"

/* 死代码消除。

   引用:

     Building an Optimizing Compiler,
     Robert Morgan, Butterworth-Heinemann, 1998, Section 8.9.

     高级编译器设计与实现,
     Steven Muchnick, Morgan Kaufmann, 1997, Section 18.10.

   死代码消除是指删除对程序输出没有影响的语句。“无效语句”不会
   影响程序的输出，而“必要语句”可能会影响输出。

   该算法由3个阶段组成:
   1. 将所有已知必要的语句标记为必要的，例如大多数函数调用、将值
      写入内存等;
   2. 传播必要语句，例如，在必要语句中给操作数赋值的语句;和
   3. 移除无效语句。  */

typedef struct block_info_def
{
    basic_block adjvex;
    /* dominance frontiers.  */
    LIST df;
    bitmap control_dependence_map;
} *block_info;

static void
mark_stmt_necessary (IRInst stmt, bitmap worklist)
{
/*  fprintf (stderr, "Marking useful stmt: ");
    IRInstDump (stmt, FALSE, stderr);
    fprintf (stderr, "\n"); */
    bitmap_set_bit (worklist, stmt->uid);
}

/* Find the immediate postdominator PDOM of the specified basic block BLOCK.
   This function is necessary because some blocks have negative numbers.  */
static basic_block
find_pdom (basic_block block)
{
    if (block == block->cfg->exit_block_ptr)
        return block->cfg->exit_block_ptr;
    else
    {
        basic_block bb = ((block_info) block->param)->adjvex;
        if (! bb)
            return block->cfg->exit_block_ptr;
        return bb;
    }
}

static void
find_control_dependence (control_flow_graph cfun, edge e)
{
    basic_block current_block;
    basic_block ending_block;

    if (e->src == cfun->entry_block_ptr)
        ending_block = (*(edge *) List_First(cfun->entry_block_ptr->succs))->dest;
    else
        ending_block = find_pdom (e->src);

    for (current_block = e->dest;
         current_block != ending_block
         && current_block != cfun->exit_block_ptr;
         current_block = find_pdom (current_block))
    {
        bitmap_set_bit (((block_info) current_block->param)->control_dependence_map, e->uid);
    }
}

/* Mark control dependent edges of BB as necessary.  We have to do this only
   once for each basic block so we set the appropriate bit after we're done.

   When IGNORE_SELF is true, ignore BB in the list of control dependences.  */
static void
mark_control_dependent_edges_necessary (basic_block bb, BOOL ignore_self, struct avl_table *edge_list, bitmap stmt_necessary, bitmap worklist)
{
    bitmap_iterator bi;
    unsigned edge_number;
/*  BOOL skipped = FALSE; */
    struct edge_def buffer;
    basic_block cd_bb;

    if (bb == bb->cfg->entry_block_ptr)
        return;

    for (bmp_iter_set_init (&bi, ((block_info) bb->param)->control_dependence_map, 0, &edge_number);
         bmp_iter_set (&bi, &edge_number);
         bmp_iter_next (&bi, &edge_number))
    {
        buffer.uid = edge_number;
        cd_bb = ((edge) avl_find (edge_list, &buffer))->src;

        if (ignore_self && cd_bb == bb)
        {
/*          skipped = TRUE; */
            continue;
        }

        if  (bitmap_set_bit (stmt_necessary, (*(IRInst *)List_Last(cd_bb->insns))->uid) &&
             worklist != NULL)
            mark_stmt_necessary (*(IRInst *)List_Last(cd_bb->insns), worklist);
    }

/*if (!skipped)
    bitmap_set_bit (visited_control_parents, bb->index); */
}

/* 找出明显需要的语句。  */
static void
find_obviously_necessary_stmts (control_flow_graph cfun, bitmap stmt_necessary)
{
    basic_block *block;
    IRInst *instr;
    int i;
    ssa_name var;

    for(  block=(basic_block *)List_First(cfun->basic_block_info)
       ;  block!=NULL
       ;  block = (basic_block *)List_Next((void *)block)
       )
    {
        for(  instr=(IRInst *) List_First ((*block)->insns)
           ;  instr!=NULL
           ;  instr = (IRInst *) List_Next ((void *)instr)
           )
        {
            for (i = 0; i < IRInstGetNumOperands (*instr); ++i) 
            {
                var = IRInstGetOperand(*instr, i);
                if  (IRInstIsOutput (*instr, i) &&
                     /* 指令设置了全局变量。  */
                     (is_global_var (var->var) ||
                     /* 指令设置了作为参数传递的数组中的值。  */
                     (var->var->sdType->tdTypeKind > TYP_lastIntrins &&
                     var->var->sdVar.sdvArgument)))
                {
                    mark_stmt_necessary (*instr, stmt_necessary);
                }
            }

            if  ((*instr)->opcode == IRINST_OP_goto ||
                 (*instr)->opcode == IRINST_OP_exit ||
                 (*instr)->opcode == IRINST_OP_entry ||
                 (*instr)->opcode == IRINST_OP_fparam)
            {
                mark_stmt_necessary (*instr, stmt_necessary);
            }

            if  ((*instr)->opcode == IRINST_OP_call &&
                 strcmp (stGetSymName(IRInstGetOperand(*instr, 1)->var), "memset") != 0 &&
                 !pure_function(cfun->code, IRInstGetOperand(*instr, 1)->var))
            {
                IRInst *curs;
                int count = GetConstVal(IRInstGetOperand(*instr, 2)->var, 0)->cvValue.cvIval;

                for(  curs=(IRInst *) List_Prev((void *)instr)
                   ;  curs!=NULL && count > 0
                   ;  curs = (IRInst *) List_Prev((void *)curs)
                   )
                {
                    if  ((*curs)->opcode == IRINST_OP_param)
                    {
                        mark_stmt_necessary (*curs, stmt_necessary);
                        count--;
                    }
                }
                mark_stmt_necessary (*instr, stmt_necessary);
            }
        }
    }
}

static void
handle_call (IRInst *call, bitmap stmt_necessary, bitmap worklist)
{
    int count;

    mark_stmt_necessary (*call, stmt_necessary);
    count = GetConstVal(IRInstGetOperand(*call, 2)->var, 0)->cvValue.cvIval;

    for(  call=(IRInst *) List_Prev((void *)call)
       ;  call!=NULL && count > 0
       ;  call = (IRInst *) List_Prev((void *)call)
       )
    {
        if  ((*call)->opcode == IRINST_OP_param)
        {
            if  (bitmap_set_bit (stmt_necessary, (*call)->uid))
            {
                mark_stmt_necessary (InterCodeGetInstByID((*call)->bb->cfg->code, (*call)->uid), worklist);
            }
            mark_stmt_necessary (*call, stmt_necessary);
            count--;
        }
    }
}

static void
propagate_necessity (control_flow_graph cfun, bitmap stmt_necessary, bitmap worklist, varpool_node_set set, struct avl_table *edge_list)
{
    IRInst stmt;
    int i;
    varpool_node vnode;
    int first;
    IRInst last_insn;
    edge *ei;
    basic_block arg_bb;
    int x;

    while   (! bitmap_empty_p (worklist))
    {
        stmt = InterCodeGetInstByID (cfun->code, bitmap_first_set_bit (worklist));
        bitmap_clear_bit (worklist, stmt->uid);

/*      fprintf (stderr, "processing: ");
        IRInstDump (stmt, FALSE, stderr);
        fprintf (stderr, "\n"); */

        for (i = 0; i < IRInstGetNumOperands (stmt); ++i) 
        {
            if  (! IRInstIsOutput (stmt, i))
            {
                vnode = varpool_get_node (set, IRInstGetOperand (stmt, i));
                if  (! bitmap_empty_p(vnode->_defines))
                {
                    first = bitmap_first_set_bit (vnode->_defines);
                    if  (bitmap_set_bit (stmt_necessary, first))
                    {
                        mark_stmt_necessary (InterCodeGetInstByID(cfun->code, first), worklist);
                    }
                }
                if  (stmt->opcode == IRINST_OP_phi)
                {
                    for(  ei=(edge *) List_First(stmt->bb->preds), x = 1
                       ;  ei!=NULL && x<i
                       ;  ei = (edge *) List_Next((void *)ei), x++
                       )
                        ;
                    arg_bb = (*ei)->src;
                    if  (((block_info) arg_bb->param)->adjvex != stmt->bb)
                    {
                        last_insn = *(IRInst *)List_Last (arg_bb->insns);
                        if  (bitmap_set_bit (stmt_necessary, last_insn->uid))
                        {
                            bitmap_set_bit (worklist, last_insn->uid);
                        }
                    }
                    else if (arg_bb != cfun->entry_block_ptr)
                    {
                        mark_control_dependent_edges_necessary (arg_bb, TRUE, edge_list, stmt_necessary, worklist);
                    }
                }
            }
        }

        /* 标记IRINST_OP_addptr对应的函数调用。  */
        if  (stmt->opcode == IRINST_OP_addptr)
        {
            bitmap_iterator bi;
            unsigned int bb_index;
            IRInst tmp;
            IRInst *curs;

            vnode = varpool_get_node (set, IRInstGetOperand (stmt, 0));
            for (bmp_iter_set_init (&bi, vnode->use_chain, 0, &bb_index);
                 bmp_iter_set (&bi, &bb_index);
                 bmp_iter_next (&bi, &bb_index))
            {
                tmp = InterCodeGetInstByID (cfun->code, bb_index);
                if  (tmp->opcode == IRINST_OP_param)
                {
                    for(  curs=InterCodeGetCursor(cfun->code, tmp)
                       ;  curs!=NULL && (*curs)->opcode != IRINST_OP_call
                       ;  curs = (IRInst *) List_Next((void *)curs)
                       )
                        ;
                    handle_call (curs, stmt_necessary, worklist);
                }
            }
        }

        if  (stmt->opcode == IRINST_OP_call)
            handle_call (InterCodeGetCursor (cfun->code, stmt), stmt_necessary, worklist);

        if  (stmt->opcode == IRINST_OP_goto)
            continue;

        if (stmt->bb != cfun->entry_block_ptr)
            mark_control_dependent_edges_necessary (stmt->bb, FALSE, edge_list, stmt_necessary, worklist);
    }
}

static void
eliminate_unnecessary_stmts (control_flow_graph cfun, bitmap stmt_necessary, varpool_node_set set, SymTab stab)
{
    basic_block *block;
    IRInst *instr;
    IRInst *next_insn;
    IRInst tmp_insn;
    basic_block dominator;
    edge *ei;

    for(  block=(basic_block *)List_First(cfun->basic_block_info)
       ;  block!=NULL
       ;  block = (basic_block *)List_Next((void *)block)
       )
    {
        for(  instr=(IRInst *) List_First ((*block)->insns)
           ;  instr!=NULL
           ;  instr = next_insn
           )
        {
            next_insn = (IRInst *) List_Next ((void *)instr);
            if  (! bitmap_bit_p (stmt_necessary, (*instr)->uid))
            {
                if  (next_insn != NULL)
                {
                    tmp_insn = *instr;
/*                  fprintf (stderr, "Deleting : ");
                    IRInstDump (tmp_insn, FALSE, stderr);
                    fprintf (stderr, "\n"); */
                    InterCodeRemoveInst (cfun->code, tmp_insn, set);
                    IRInstDelInst (tmp_insn);
                }
                else
                {
                    dominator = (*instr)->bb;
                    do
                    {
                        dominator = ((block_info) dominator->param)->adjvex;
                    }
                    while (! bitmap_bit_p(stmt_necessary, (*(IRInst *)List_Last (dominator->insns))->uid));

                    for(  ei=List_First((*instr)->bb->succs)
                       ;  ei!=NULL
                       ;  ei = List_First((*instr)->bb->succs)
                       )
                        remove_edge (*ei);
                    make_edge ((*instr)->bb, dominator);

                    tmp_insn = IRInstEmitInst (IRINST_OP_goto, (*instr)->line, (*instr)->column);
                    IRInstSetOperand (tmp_insn, 0, stCreateIconNode (stab, (*(IRInst *)List_First (dominator->insns))->uid));
                    InterCodeAddInst ((*instr)->bb, tmp_insn, TRUE);
                    tmp_insn = *instr;
                    InterCodeRemoveInst (cfun->code, tmp_insn, set);
                    IRInstDelInst (tmp_insn);
                }
            }
        }
    }
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
    basic_block *b, *curs;

    for(  b=(basic_block *)List_First(cfun->basic_block_info)
       ;  b!=NULL
       ;  b = (basic_block *)List_Next((void *)b)
       )
    {
        ((block_info)(*b)->param)->adjvex = get_immediate_dominator (cfun, TRUE, *b);
    }

    for(  b=(basic_block *)List_First(cfun->basic_block_info)
       ;  b!=NULL
       ;  b = (basic_block *)List_Next((void *)b)
       )
    {
        if (List_Card ((*b)->succs) >= 2)
        {
            for(  ei=(edge *)List_First((*b)->succs)
               ;  ei!=NULL
               ;  ei = (edge *)List_Next((void *)ei)
               )
            {
                basic_block runner = (*ei)->dest;
                basic_block domsb;
                if (runner == cfun->entry_block_ptr)
                    continue;

                domsb = ((block_info)(*b)->param)->adjvex;
                while (runner != domsb)
                {
                    for(  curs=(basic_block *) List_First(((block_info)runner->param)->df) 
                       ;  curs!=NULL
                       ;  curs = (basic_block *) List_Next((void *)curs)
                       )
                        if  (*curs == *b)
                            break;
                    if  (curs!=NULL)
                        break;
                    *(basic_block*)List_NewLast (((block_info)runner->param)->df, sizeof (basic_block)) = *b;
                    runner = ((block_info)runner->param)->adjvex;
                }
            }
        }
    }
}

static int compare( edge arg1, edge arg2 )
{
    return arg1->uid - arg2->uid;
}

void
perform_ssa_dce (InterCode code, SymTab stab)
{
    control_flow_graph *F;
    basic_block *block;
    bitmap stmt_necessary;
    bitmap worklist;
    varpool_node_set set;
    edge *ei;
    struct avl_table *edge_list;

    for(  F=(control_flow_graph *)List_First(code->funcs)
       ;  F!=NULL
       ;  F = (control_flow_graph *)List_Next((void *)F)
       )
    {
        compute_dominators (*F, TRUE);
        stmt_necessary = BITMAP_XMALLOC ();
        worklist = BITMAP_XMALLOC ();
        set = varpool_node_set_new (*F, TRUE);
        edge_list = avl_create ((avl_comparison_func *)compare, NULL, NULL);

        for(  block=(basic_block *)List_First((*F)->basic_block_info)
           ;  block!=NULL
           ;  block = (basic_block *)List_Next((void *)block)
           )
        {
            (*block)->param = xmalloc (sizeof (struct block_info_def));
            memset ((*block)->param, 0, sizeof (struct block_info_def));
            ((block_info) (*block)->param)->adjvex = get_immediate_dominator (*F, TRUE, *block);
            ((block_info) (*block)->param)->df = List_Create ();
            ((block_info) (*block)->param)->control_dependence_map = BITMAP_XMALLOC ();
        }

        /* 计算反向支配边界。  */
        compute_dominance_frontiers (*F);

        for(  block=(basic_block *)List_First((*F)->basic_block_info)
           ;  block!=NULL
           ;  block = (basic_block *)List_Next((void *)block)
           )
        {
            for(  ei=(edge *) List_First((*block)->succs)
               ;  ei!=NULL
               ;  ei = (edge *) List_Next((void *)ei)
               )
            {
                avl_insert (edge_list, *ei);
                find_control_dependence(*F, *ei);
            }
        }

        find_obviously_necessary_stmts (*F, stmt_necessary);
        bitmap_copy (worklist, stmt_necessary);

        propagate_necessity (*F, stmt_necessary, worklist, set, edge_list);
        eliminate_unnecessary_stmts (*F, stmt_necessary, set, stab);

        for(  block=(basic_block *)List_First((*F)->basic_block_info)
           ;  block!=NULL
           ;  block = (basic_block *)List_Next((void *)block)
           )
        {
            List_Destroy (&((block_info) (*block)->param)->df);
            BITMAP_XFREE (((block_info) (*block)->param)->control_dependence_map);
            free ((*block)->param);
            (*block)->param = NULL;
        }

        /* 死基本块删除。  */
        cleanup_cfg (*F, stab);

        BITMAP_XFREE (stmt_necessary);
        BITMAP_XFREE (worklist);
        free_varpool_node_set (set);
        avl_destroy (edge_list, NULL);
    }
}
