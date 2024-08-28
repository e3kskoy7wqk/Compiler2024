#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "all.h"

#if !defined(NDEBUG)
# if (0)
# define LCM_DEBUG_INFO
# endif
#endif

static const int MAX_ITERATIONS = 50;

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

/* 拆分所有关键边，没有SSA。  */
static void
split_critical_edges (control_flow_graph cfun, SymTab stab)
{
    basic_block* block;
    edge *ei, *next_edge;
    basic_block new_block;
    IRInst jump_insn;

    for(  block=(basic_block*) List_First(cfun->basic_block_info)
       ;  block!=NULL
       ;  block = (basic_block*) List_Next((void *)block)
       )
    {
        for(  ei=(edge *) List_First((*block)->succs)
           ;  ei!=NULL
           ;  ei = next_edge
           )
        {
            next_edge = (edge *) List_Next((void *)ei);
            if (List_Card ((*ei)->src->succs) >= 2
                && List_Card ((*ei)->dest->preds) >= 2)
            {
                new_block = alloc_block ();
                link_block (cfun, new_block);
                jump_insn = IRInstEmitInst (IRINST_OP_goto, (*(IRInst *)List_First ((*ei)->dest->insns))->line, (*(IRInst *)List_First ((*ei)->dest->insns))->column);
                InterCodeAddInst (new_block, jump_insn, TRUE);
                IRInstSetOperand (jump_insn, 0, stCreateIconNode (stab, (*(IRInst *)List_First ((*ei)->dest->insns))->uid));
                make_edge (new_block, (*ei)->dest);
                make_edge ((*ei)->src, new_block);
                update_destinations (new_block, (*(IRInst *)List_First ((*ei)->dest->insns))->uid, jump_insn->uid, NULL);
                remove_edge (*ei);
            }
            else if (List_Card ((*ei)->dest->preds) >= 2
                     && (*ei)->dest == cfun->exit_block_ptr)
            {
                new_block = alloc_block ();
                link_block (cfun, new_block);
                jump_insn = IRInstEmitInst (IRINST_OP_goto, (*(IRInst *)List_First ((*ei)->dest->insns))->line, (*(IRInst *)List_First ((*ei)->dest->insns))->column);
                InterCodeAddInst (new_block, jump_insn, TRUE);
                IRInstSetOperand (jump_insn, 0, stCreateIconNode (stab, (*(IRInst *)List_First ((*ei)->dest->insns))->uid));
                for(  ei=(edge *) List_First(cfun->exit_block_ptr->preds)
                   ;  ei!=NULL
                   ;  ei = (edge *) List_First(cfun->exit_block_ptr->preds)
                   )
                {
                    make_edge ((*ei)->src, new_block);
                    remove_edge (*ei);
                }
                make_edge (new_block, cfun->exit_block_ptr);
                update_destinations (new_block, (*(IRInst *)List_First (cfun->exit_block_ptr->insns))->uid, jump_insn->uid, NULL);
                break;
            }
        }
    }
}

static BOOL
should_skip (IRInst instr)
{
    switch (instr->opcode)
    {
    case IRINST_OP_aload: case IRINST_OP_move: case IRINST_OP_le:
    case IRINST_OP_add: case IRINST_OP_not: case IRINST_OP_gt:
    case IRINST_OP_addptr: case IRINST_OP_sub: case IRINST_OP_mul:
    case IRINST_OP_div: case IRINST_OP_rem: case IRINST_OP_neg:
    case IRINST_OP_i2f: case IRINST_OP_f2i: case IRINST_OP_eq:
    case IRINST_OP_ne: case IRINST_OP_lt: case IRINST_OP_ge:
    case IRINST_OP_lsl: case IRINST_OP_lsr: case IRINST_OP_asr:
        return FALSE;

    case IRINST_OP_nop: case IRINST_OP_ifeq: case IRINST_OP_ifne:
    case IRINST_OP_iflt: case IRINST_OP_ifge: case IRINST_OP_ifgt:
    case IRINST_OP_ifle: case IRINST_OP_goto: case IRINST_OP_param: 
    case IRINST_OP_fparam: case IRINST_OP_entry: case IRINST_OP_exit: 
    case IRINST_OP_phi: case IRINST_OP_begin_block: case IRINST_OP_end_block:
    case IRINST_OP_store: case IRINST_OP_astore:
        return TRUE;

    case IRINST_OP_call:
        return !pure_function (instr->bb->cfg->code, IRInstGetOperand (instr, 1)->var);

    case IRINST_OP_load:
        return is_global_var (IRInstGetOperand (instr, 1)->var) && !IRInstGetOperand (instr, 1)->var->sdVar.sdvConst;
    }
    return FALSE;
}

/* 将位图DST设置为基本块B的前几块SRC的交集。  */
static void
bitmap_intersection_of_preds (struct avl_table *block_map, bitmap ONES, bitmap dst, bitmap *src, basic_block b)
{
    edge e;
    edge *ei;

    for(  ei=(edge *) List_First(b->preds), e = NULL
       ;  ei!=NULL
       ;  ei = (edge *) List_Next((void *)ei)
       )
    {
        e = *ei;
        if (e->src == b->cfg->entry_block_ptr)
            continue;

        bitmap_copy (dst, src[((struct pair *) avl_find (block_map, &e->src->index))->y]);
        break;
    }

    if (e == 0)
        bitmap_copy (dst, ONES);
    else
        for (ei = (edge *) List_Next((void *)ei); ei!=NULL; ei = (edge *) List_Next((void *)ei))
        {
            e = *ei;
            if (e->src == b->cfg->entry_block_ptr)
                continue;

            bitmap_and_into (dst, src[((struct pair *) avl_find (block_map, &e->src->index))->y]);
        }
}

/* 将位图DST设置为基本块B后继的SRC的交集。  */
static void
bitmap_intersection_of_succs (struct avl_table *block_map, bitmap ONES, bitmap dst, bitmap *src, basic_block b)
{
    edge e;
    edge *ei;

    for (e = NULL, ei=(edge *) List_First(b->succs); ei!=NULL; ei = (edge *) List_Next((void *)ei))
    {
        e = *ei;
        if (e->dest == b->cfg->exit_block_ptr)
            continue;

        bitmap_copy (dst, src[((struct pair *) avl_find (block_map, &e->dest->index))->y]);
        break;
    }

    if (e == 0)
        bitmap_copy (dst, ONES);
    else
        for (ei = (edge *) List_Next((void *)ei); ei!=NULL; ei = (edge *) List_Next((void *)ei))
        {
            e = *ei;
            if (e->dest == b->cfg->exit_block_ptr)
                continue;

            bitmap_and_into (dst, src[((struct pair *) avl_find (block_map, &e->dest->index))->y]);
        }
}

/* 根据AVLOC和KILL向量计算AVIN和AVOUT向量。  */
void
compute_available (control_flow_graph cfun, struct avl_table *block_map, bitmap ONES, bitmap *avloc, bitmap *kill, bitmap *avout, bitmap *avin)
{
    basic_block *worklist, *qin, *qout, *qend, bb;
    int qlen;
    basic_block* block;
    int ix, i, count;
    LIST postorder;
    edge *ei;

    /* 分配一个工作列表数组/队列。只有不在链表上的项才添加到链表。因
       此，大小受基本块数量的限制。  */
    count = List_Card (cfun->basic_block_info);
    qlen = count - 2;
    qin = qout = worklist =
      (basic_block *) xmalloc (qlen * sizeof (basic_block));

    /* 我们想要一个最大的解。  */
    for (i = 0; i < count; i++)
        bitmap_copy (avout[i], ONES);

    /* 将每个区块放到工作列表中;这是必要的，因为AVOUT的初始化是乐
       观的。使用反向后序可以减少数据流问题的迭代次数。  */
    postorder = List_Create ();
    pre_and_rev_post_order_compute (cfun, NULL, postorder, FALSE, FALSE);
    for(  block=(basic_block*) List_First(postorder)
       ;  block!=NULL
       ;  block = (basic_block*) List_Next((void *)block)
       )
    {
        bb = *block;
        *qin++ = bb;
        bb->param = bb;
    }
    List_Destroy (&postorder);

    qin = worklist;
    qend = &worklist[qlen];

    /* 标记入口块的后继块，以便我们可以在下面轻松识别它们。  */
    for(  ei=(edge *) List_First(cfun->entry_block_ptr->succs)
       ;  ei!=NULL 
       ;  ei = (edge *) List_Next((void *)ei)
       )
        (*ei)->dest->param = cfun->entry_block_ptr;

    /* 迭代，直到工作列表为空。  */
    while (qlen)
    {
        /* 从工作列表中删除第一个条目。  */
        bb = *qout++;
        qlen--;

        if (qout >= qend)
            qout = worklist;

        ix = ((struct pair *) avl_find (block_map, &bb->index))->y;

        /* 如果前一个块是入口块，那么avout的交集为空集。我们可以通过块
           结构中param字段的特殊值来识别这样的块。  */
        if ((basic_block) bb->param == cfun->entry_block_ptr)
            /* 对于继承ENTRY块的块，不要清除aux字段。这样我们就不会再把它
               添加到工作列表中了。  */
            bitmap_clear (avin[ix]);
        else
        {
            /* 清除该块的param字段，以便在必要时可以再次将其添加到工作列表中。  */
            bb->param = NULL;
            bitmap_intersection_of_preds (block_map, ONES, avin[ix], avout, bb);
        }

        if  (bitmap_ior_and_compl (avout[ix], avloc[ix],
                                    avin[ix], kill[ix]))
        /* 如果这个块的out状态改变了，那么我们需要将这个块的后继添加到工
           作列表中，如果它们还没有在工作列表中。  */
        {
            for(  ei=(edge *) List_First(bb->succs)
               ;  ei!=NULL 
               ;  ei = (edge *) List_Next((void *)ei)
               )
                if  (!(*ei)->dest->param &&
                     (*ei)->dest != cfun->exit_block_ptr)
                {
                    *qin++ = (*ei)->dest;
                    (*ei)->dest->param = *ei;
                    qlen++;

                    if (qin >= qend)
                        qin = worklist;
                }
        }
    }

    for(  block=(basic_block*) List_First(cfun->basic_block_info)
       ;  block!=NULL
       ;  block = (basic_block*) List_Next((void *)block)
       )
        (*block)->param = NULL;
    free (worklist);
}

/* 在每个块的入口和出口计算表达式的可预测性。  */
static void
compute_antinout (control_flow_graph cfun, struct avl_table *block_map, bitmap ONES, bitmap *antloc, bitmap *transp, bitmap *antin, bitmap *antout)
{
    basic_block *worklist, *qin, *qout, *qend, bb;
    int qlen;
    int ix, i, count;
    basic_block* block;
    LIST postorder;
    edge *ei;
    bitmap_head tmp;
    int changed;

    /* 分配一个工作列表数组/队列。只有不在链表上的项才添加到链表。
       因此，大小受基本块数量的限制。  */
    count = List_Card (cfun->basic_block_info);
    qin = qout = worklist = (basic_block *) xmalloc (count * sizeof (basic_block));

    /* 我们希望得到一个最大解，因此对ANTIN进行乐观初始化。  */
    for (i = 0; i < count; i++)
        bitmap_copy (antin[i], ONES);

    /* 将每个基本块放到工作列表中;这是必要的，因为上面提到了ANTIN的乐
       观初始化。  */
    postorder = List_Create ();
    pre_and_rev_post_order_compute (cfun, NULL, postorder, FALSE, FALSE);
    for(  block=(basic_block*) List_Last(postorder)
       ;  block!=NULL
       ;  block = (basic_block*) List_Prev((void *)block)
       )
    {
        bb = *block;
        *qin++ = bb;
        bb->param = bb;
    }
    List_Destroy (&postorder);

    qin = worklist;
    qend = &worklist[count - 2];
    qlen = count - 2;

    /* 标记exit块的前驱块，以便我们在下面容易识别它们。  */
    for(  ei=(edge *) List_First(cfun->exit_block_ptr->preds)
       ;  ei!=NULL 
       ;  ei = (edge *) List_Next((void *)ei)
       )
        (*ei)->src->param = cfun->exit_block_ptr;

    /* 迭代，直到工作列表为空。  */
    while (qlen)
    {
        /* 从工作列表中删除第一个条目。  */
        bb = *qout++;
        qlen--;

        if (qout >= qend)
            qout = worklist;

        ix = ((struct pair *) avl_find (block_map, &bb->index))->y;

        if (bb->param == cfun->exit_block_ptr)
            /* 不要为EXIT块的前驱块清除param字段。这样我们就不会再把它添加到
               工作列表中了。  */
            bitmap_clear (antout[ix]);
        else
        {
            /* 清除该块的param字段，以便在必要时可以再次将其添加到工作列表中。  */
            bb->param = NULL;
            bitmap_intersection_of_succs (block_map, ONES, antout[ix], antin, bb);
        }

        tmp.first = tmp.current = 0;
        bitmap_and (&tmp, transp[ix], antout[ix]);
        changed = bitmap_ior (antin[ix], &tmp, antloc[ix]);
        bitmap_clear (&tmp);
        if  (changed)
        {
            /* 如果这个块的in状态发生了变化，那么我们需要将这个块的前驱块
               添加到工作列表中(如果它们还没有在工作列表中)。  */
            for(  ei=(edge *) List_First(bb->preds)
               ;  ei!=NULL 
               ;  ei = (edge *) List_Next((void *)ei)
               )
                if (!(*ei)->src->param && (*ei)->src != cfun->entry_block_ptr)
                {
                    *qin++ = (*ei)->src;
                    (*ei)->src->param = *ei;
                    qlen++;
                    if (qin >= qend)
                        qin = worklist;
                }
        }
    }

    for(  block=(basic_block*) List_First(cfun->basic_block_info)
       ;  block!=NULL
       ;  block = (basic_block*) List_Next((void *)block)
       )
        (*block)->param = NULL;
    free (worklist);
}

/* 计算基于边的LCM的earliest向量。  */
static void
compute_earliest (control_flow_graph cfun, struct avl_table *block_map, struct avl_table *edge_map, bitmap ONES, bitmap *antin, bitmap *antout, bitmap *avout, bitmap *kill, bitmap *earliest)
{
    bitmap difference, temp_bitmap;
    int x;
    int pred, succ;
    basic_block* block;
    edge *ei;
    bitmap_head tmp;

    difference = BITMAP_XMALLOC ();
    temp_bitmap = BITMAP_XMALLOC ();

    for(  block=(basic_block*) List_First(cfun->basic_block_info)
       ;  block!=NULL
       ;  block = (basic_block*) List_Next((void *)block)
       )
    {
        for(  ei=(edge *) List_First((*block)->preds)
           ;  ei!=NULL 
           ;  ei = (edge *) List_Next((void *)ei)
           )
        {
            pred = ((struct pair *) avl_find (block_map, &(*ei)->src->index))->y;
            succ = ((struct pair *) avl_find (block_map, &(*ei)->dest->index))->y;
            x = ((struct pair *) avl_find (edge_map, &(*ei)->uid))->y;
            if ((*ei)->src == cfun->entry_block_ptr)
                bitmap_copy (earliest[x], antin[succ]);
            else
            {
                if ((*ei)->dest == cfun->exit_block_ptr)
                    bitmap_clear (earliest[x]);
                else
                {
                    bitmap_and_compl (difference, antin[succ],
                                        avout[pred]);
                    bitmap_and_compl (temp_bitmap, ONES, antout[pred]);
                    tmp.first = tmp.current = 0;
                    bitmap_ior (&tmp, kill[pred], temp_bitmap);
                    bitmap_and (earliest[x], difference,
                                          &tmp);
                    bitmap_clear (&tmp);
                }
            }
        }
    }

    BITMAP_XFREE (temp_bitmap);
    BITMAP_XFREE (difference);
}

/* later(p,s)依赖于laterin(p)的计算。
   laterin(p)依赖于later(p2,p)的计算。

     laterin(ENTRY)被定义为全0
     later(ENTRY, succs(ENTRY))使用laterin(ENTRY)定义
     laterin(succs(ENTRY))由later(ENTRY, succs(ENTRY))定义。

   如果我们以这种方式进行，从工作列表中的所有基本块开始，任何时
   候我们更改later(bb)，我们需要将succs(bb)添加到工作列表中，如果
   它们还没有在工作列表中。

   边界条件:

     我们在工作列表中填充所有正常的基本块。入口块永远不能添加到工
     作列表中，因为它永远不是任何块的后续块。我们明确地阻止将EXIT
     块添加到工作列表中。

     我们乐观地初始化LATER。这是这个例程唯一一次计算entry块外的边
     的LATER，因为entry块从未在工作列表中。因此，输入块既不使用也
     不计算LATERIN。

     由于EXIT块从未添加到工作列表中，因此我们既不使用也不计算EXIT
     块的LATERIN。到达出口块的边缘在循环内部以正常方式处理。然
     而，插入/删除计算需要LATERIN(EXIT)，所以我们必须计算它。  */
static void
compute_laterin (control_flow_graph cfun, struct avl_table *block_map, struct avl_table *edge_map, bitmap ONES, 
                 bitmap *earliest,bitmap *antloc, bitmap *later, bitmap *laterin)
{
    basic_block *worklist, *qin, *qout, *qend, bb;
    int num_edges, num_blocks, i;
    basic_block* block;
    edge *ei;
    int ix, x;
    LIST postorder;
    int qlen;

    num_blocks = (int) avl_count (block_map);
    num_edges = (int) avl_count (edge_map);

    /* 分配一个工作列表数组/队列。只有不在链表上的项才添加到链表。因
       此，大小受基本块数量的限制。  */
    qin = qout = worklist
      = (basic_block *) xmalloc (num_blocks * sizeof (basic_block));

    /* 我们想要一个最大的解，所以最初考虑所有边的LATER为真。这允许
       通过循环传播，因为传入的循环边缘将稍后设置，所以如果所有其他
       传入的循环边缘都设置了，则将为循环头部设置LATERIN。

       如果该边缘上的LATER的乐观设置不正确(例如，表达式在循环内的块
       中是ANTLOC)，那么当我们处理乐观边缘头部的块时，此算法将检测
       它。这将重新排队受影响的块。  */
    for (i = 0; i < num_edges; i++)
        bitmap_copy (later[i], ONES);

    /* 请注意，即使我们想要一个乐观的LATER设置，我们也不希望过于乐
       观。考虑入口块的出边。这条边的LATER应该始终与EARLIEST相
       同  */
    for(  ei=(edge *) List_First(cfun->entry_block_ptr->succs)
       ;  ei!=NULL 
       ;  ei = (edge *) List_Next((void *)ei)
       )
    {
        x = ((struct pair *) avl_find (edge_map, &(*ei)->uid))->y;
        bitmap_copy (later[x], earliest[x]);
    }

    /* 将所有块添加到工作列表中。鉴于上面后面的乐观初始化，这防止了
       提前退出循环。  */
    postorder = List_Create ();
    pre_and_rev_post_order_compute (cfun, NULL, postorder, FALSE, FALSE);
    for(  block=(basic_block*) List_First(postorder)
       ;  block!=NULL
       ;  block = (basic_block*) List_Next((void *)block)
       )
    {
        bb = *block;
        *qin++ = bb;
        bb->param = bb;
    }
    List_Destroy (&postorder);

    /* 请注意，我们没有使用队列中最后分配的元素，因为EXIT_BLOCK从
       未插入其中。 */
    qin = worklist;
    qend = &worklist[num_blocks - 2];
    qlen = num_blocks - 2;

    /* 迭代，直到工作列表为空。  */
    while (qlen)
    {
        /* 从工作列表中删除第一个条目。  */
        bb = *qout++;
        bb->param = NULL;
        qlen--;
        if (qout >= qend)
            qout = worklist;

        ix = ((struct pair *) avl_find (block_map, &bb->index))->y;

        /* 计算每个进入B的边的LATERIN的交集。  */
        bitmap_copy (laterin[ix], ONES);
        for(  ei=(edge *) List_First(bb->preds)
           ;  ei!=NULL 
           ;  ei = (edge *) List_Next((void *)ei)
           )
            bitmap_and_into (laterin[ix], later[(size_t) ((struct pair *) avl_find (edge_map, &(*ei)->uid))->y]);

        /* 计算所有出边的LATER。  */
        for(  ei=(edge *) List_First(bb->succs)
           ;  ei!=NULL 
           ;  ei = (edge *) List_Next((void *)ei)
           )
        {
            x = ((struct pair *) avl_find (edge_map, &(*ei)->uid))->y;
            if (bitmap_ior_and_compl (later[(size_t) x],
                                      earliest[(size_t) x],
                                      laterin[ix],
                                      antloc[ix])
                /* 如果稍后更改了出边，那么我们需要将出边的目标添加到工
                   作列表中。  */
                && (*ei)->dest != cfun->exit_block_ptr && (*ei)->dest->param == 0)
            {
                *qin++ = (*ei)->dest;
                (*ei)->dest->param = *ei;
                qlen++;
                if (qin >= qend)
                    qin = worklist;
            }
        }
    }

    /* 计算插入点和删除点需要计算出口块的LATERIN。为此，我们在
       LATERIN数组中分配了一个额外的条目。  */
    bitmap_copy (laterin[num_blocks], ONES);
    for(  ei=(edge *) List_First(cfun->exit_block_ptr->preds)
       ;  ei!=NULL 
       ;  ei = (edge *) List_Next((void *)ei)
       )
        bitmap_and_into (laterin[num_blocks], later[(size_t) ((struct pair *) avl_find (edge_map, &(*ei)->uid))->y]);

    for(  block=(basic_block*) List_First(cfun->basic_block_info)
       ;  block!=NULL
       ;  block = (basic_block*) List_Next((void *)block)
       )
    {
        (*block)->param = NULL;
        for(  ei=(edge *) List_First((*block)->preds)
           ;  ei!=NULL 
           ;  ei = (edge *) List_Next((void *)ei)
           )
            (*ei)->param = NULL;
    }
    free (worklist);
}

/* 计算基于边缘的LCM的插入点和删除点。  */
static void
compute_insert_delete (control_flow_graph cfun, struct avl_table *block_map, struct avl_table *edge_map, bitmap *antloc,
                       bitmap *later, bitmap *laterin, bitmap *insert, bitmap *del)
{
    int x;
    basic_block b;
    basic_block* block;
    edge *ei;

    for(  block=(basic_block*) List_First(cfun->basic_block_info)
       ;  block!=NULL
       ;  block = (basic_block*) List_Next((void *)block)
       )
    {
        x = ((struct pair *) avl_find (block_map, &(*block)->index))->y;
        bitmap_and_compl (del[x], antloc[x], laterin[x]);
    }

    for(  block=(basic_block*) List_First(cfun->basic_block_info)
       ;  block!=NULL
       ;  block = (basic_block*) List_Next((void *)block)
       )
        for(  ei=(edge *) List_First((*block)->preds)
           ;  ei!=NULL 
           ;  ei = (edge *) List_Next((void *)ei)
           )
        {
            x = ((struct pair *) avl_find (edge_map, &(*ei)->uid))->y;
            b = (*ei)->dest;
            if (b == cfun->exit_block_ptr)
                bitmap_and_compl (insert[x], later[x],
                                  laterin[avl_count (block_map)]);
            else
                bitmap_and_compl (insert[x], later[x], laterin[((struct pair *) avl_find (block_map, &b->index))->y]);
        }
}

bitmap *
bitmap_vector_alloc (int n_vecs)
{
    bitmap *bitmap_vector;

    bitmap_vector = (bitmap *) xmalloc (n_vecs * sizeof (bitmap));
    for (n_vecs--; n_vecs >= 0; n_vecs--)
        bitmap_vector[n_vecs] = BITMAP_XMALLOC ();
    return bitmap_vector;
}

void bitmap_vector_free (bitmap * vec, int n_vecs)
{
    for (n_vecs--; n_vecs >= 0; n_vecs--)
        BITMAP_XFREE (vec[n_vecs]);
    free (vec);
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

static IRInst
find_call (IRInst instr)
{
    IRInst *curs ;
    for(  curs=(IRInst *) InterCodeGetCursor (instr->bb->cfg->code, instr)
       ;  curs!=NULL
       ;  curs = (IRInst *) List_Next((void *)curs)
       )
        if  ((*curs)->opcode == IRINST_OP_call)
            return *curs;
    return NULL;
}

/* 计算每个记录的表达式的局部属性。

   局部属性是指那些由块定义的属性，而与其他块无关。

   如果表达式的操作数在块中没有被修改，那么表达式在块中是透明的。

   如果表达式至少计算一次，则在一个块中计算(局部可用)，如果将计算
   移动到块的末尾，则表达式将包含相同的值。

   如果一个表达式至少被计算一次，那么它在一个块中是局部可预测
   的，如果计算被移动到块的开头，那么表达式将包含相同的值。

   我们调用这个例程进行部分冗余消除和代码提升。它们都计算基本相同的信
   息，因此可以很容易地共享此代码。

   TRANSP、COMP和ANTLOC是记录局部属性的目标bitmap。如果
   为NULL，则不计算该属性。  */
static void
compute_local_properties (control_flow_graph cfun, struct avl_table *block_map, varpool_node_set set, bitmap ONES, bitmap *transp, bitmap *comp, bitmap *antloc)
{
    basic_block* block;
    IRInst *instr;
    IRInst tmp_insn;
    bitmap temp_bitmap;
    int i, x;
    varpool_node vnode;
    bitmap_iterator bi;
    unsigned insn_index;
    bitmap affected;
    bitmap_head tmp;

    temp_bitmap = BITMAP_XMALLOC ();
    affected = BITMAP_XMALLOC ();

    /* 初始化传入的所有位图。  */
    for (i = 0; i < (int) avl_count (block_map); i++)
        bitmap_copy (transp[i], ONES);

    /* 计算使用全局变量的指令。  */
    for(  block=(basic_block*) List_First(cfun->basic_block_info)
       ;  block!=NULL
       ;  block = (basic_block*) List_Next((void *)block)
       )
    {
        for(  instr=List_First((*block)->insns)
           ;  instr!=NULL
           ;  instr = List_Next((void *)instr)
           )
        {
            if  ((*instr)->opcode == IRINST_OP_call)
            {
                bitmap_set_bit (affected, (*instr)->uid);
                continue;
            }
            for (i = IRInstGetNumOperands (*instr) - 1; i >= 0; i--)
            {
                if  (!IRInstIsOutput (*instr, i) &&
                     IRInstGetOperand (*instr, i)->var->sdSymKind == SYM_VAR &&
                     is_global_var (IRInstGetOperand (*instr, i)->var) &&
                     !IRInstGetOperand (*instr, i)->var->sdVar.sdvConst)
                {
                    bitmap_set_bit (affected, (*instr)->uid);
                    continue;
                }
            }
        }
    }

    /* 计算透明性。  */
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
            if  ((*instr)->opcode == IRINST_OP_call &&
                 !pure_function (cfun->code, IRInstGetOperand (*instr, 1)->var))
                bitmap_and_compl_into (transp[x], affected);

            for (i = IRInstGetNumOperands (*instr) - 1; i >= 0; i--)
            {
                if  (IRInstIsOutput (*instr, i))
                {
                    vnode = varpool_get_node (set, IRInstGetOperand (*instr, i));
                    for (bmp_iter_set_init (&bi, vnode->use_chain, 0, &insn_index);
                         bmp_iter_set (&bi, &insn_index);
                         bmp_iter_next (&bi, &insn_index))
                    {
                        tmp_insn = InterCodeGetInstByID (cfun->code, insn_index);
                        if  (tmp_insn->opcode == IRINST_OP_param)
                            tmp_insn = find_call (tmp_insn);
                        
                        bitmap_clear_bit (transp[x], tmp_insn->uid);
                    }
                }
            }
        }
    }

    /* 计算局部预期性。  */
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
                bitmap_set_bit (antloc[x], (*instr)->uid);
        for(  instr=List_First((*block)->insns)
           ;  instr!=NULL
           ;  instr = List_Next((void *)instr)
           )
        {
            if  ((*instr)->opcode == IRINST_OP_call &&
                 !pure_function (cfun->code, IRInstGetOperand (*instr, 1)->var))
            {
                tmp.first = tmp.current = 0;
                bitmap_and_compl (&tmp, affected, temp_bitmap);
                bitmap_and_compl_into (antloc[x], &tmp);
                bitmap_clear (&tmp);
            }

            for (i = IRInstGetNumOperands (*instr) - 1; i >= 0; i--)
            {
                if  (IRInstIsOutput (*instr, i))
                {
                    vnode = varpool_get_node (set, IRInstGetOperand (*instr, i));
                    for (bmp_iter_and_compl_init (&bi, vnode->use_chain, temp_bitmap, 0,
                                                  &insn_index);
                         bmp_iter_and_compl (&bi, &insn_index);
                         bmp_iter_next (&bi, &insn_index))
                    {
                        tmp_insn = InterCodeGetInstByID (cfun->code, insn_index);
                        if  (tmp_insn->opcode == IRINST_OP_param)
                            tmp_insn = find_call (tmp_insn);

                        bitmap_clear_bit (antloc[x], tmp_insn->uid);
                    }
                    bitmap_set_bit (temp_bitmap, (*instr)->uid);
                }
            }
        }
    }

    /* 计算局部可用性。  */
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
            if  ((*instr)->opcode == IRINST_OP_call &&
                 !pure_function (cfun->code, IRInstGetOperand (*instr, 1)->var))
            {
                tmp.first = tmp.current = 0;
                bitmap_and_compl (&tmp, affected, temp_bitmap);
                bitmap_and_compl_into (comp[x], &tmp);
                bitmap_clear (&tmp);
            }

            for (i = IRInstGetNumOperands (*instr) - 1; i >= 0; i--)
            {
                if  (IRInstIsOutput(*instr, i))
                {
                    vnode = varpool_get_node (set, IRInstGetOperand (*instr, i));
                    for (bmp_iter_and_compl_init (&bi, vnode->use_chain, temp_bitmap, 0,
                                                  &insn_index);
                         bmp_iter_and_compl (&bi, &insn_index);
                         bmp_iter_next (&bi, &insn_index))
                    {
                        tmp_insn = InterCodeGetInstByID (cfun->code, insn_index);
                        if  (tmp_insn->opcode == IRINST_OP_param)
                            tmp_insn = find_call (tmp_insn);

                        bitmap_clear_bit (comp[x], tmp_insn->uid);
                    }
                    bitmap_set_bit (temp_bitmap, (*instr)->uid);
                }
            }
        }
    }

    BITMAP_XFREE (temp_bitmap);
    BITMAP_XFREE (affected);
}

static BOOL
delete_instructions (control_flow_graph cfun, SymTab stab, bitmap ONES, bitmap *del, struct avl_table *block_map, struct avl_table *map)
{
    BOOL changed = FALSE;
    basic_block* block;
    int x;
    bitmap_iterator bi;
    unsigned i;
    IRInst tmp_insn, tmp2_insn, *curs, *next_insn;
    int n_param;
    int k;
    SymDef tmpSym;
    struct pair *p;

    for(  block=(basic_block*) List_First(cfun->basic_block_info)
       ;  block!=NULL
       ;  block = (basic_block*) List_Next((void *)block)
       )
    {
        x = ((struct pair *) avl_find (block_map, &(*block)->index))->y;
        for (bmp_iter_set_init (&bi, del[x], 0, &i);
             bmp_iter_set (&bi, &i);
             bmp_iter_next (&bi, &i))
        {
            tmp_insn = InterCodeGetInstByID (cfun->code, i);
#if (1)
# ifdef LCM_DEBUG_INFO
            fprintf (stderr, "PRE: redundant insn ");
            IRInstDump (tmp_insn, FALSE, stderr);
            fprintf (stderr, " in bb %d\n",(*block)->index);
# endif
#endif
            if  (tmp_insn->opcode == IRINST_OP_call)
            {
                n_param = GetConstVal (IRInstGetOperand (tmp_insn, 2)->var, 0)->cvValue.cvIval;
                curs = (IRInst *) InterCodeGetCursor (tmp_insn->bb->cfg->code, tmp_insn);
                for(  curs=(IRInst *) List_Prev((void *)curs)
                   ;  curs!=NULL && n_param > 0
                   ;  curs = next_insn, n_param--
                   )
                {
                    next_insn = (IRInst *) List_Prev((void *)curs);
                    tmp2_insn = *curs;
                    InterCodeRemoveInst (cfun->code, tmp2_insn, NULL);
                    IRInstDelInst (tmp2_insn);
                }
            }
            bitmap_clear_bit (ONES, tmp_insn->uid);

            for (k = 0; k < IRInstGetNumOperands (tmp_insn); k++)
                if  (IRInstIsOutput (tmp_insn, k))
                    break;

            p = (struct pair *) avl_find (map, &tmp_insn->uid);
            if  (p)
            {
                tmpSym = stGetSymByID (stab, p->y);
                IRInstSetOperand (tmp_insn, 1, tmpSym);
                IRInstGetOperand (tmp_insn, 1)->version = 0;
                tmp_insn->opcode = IRINST_OP_move;
            }
            else
            {
                InterCodeRemoveInst (cfun->code, tmp_insn, NULL);
                IRInstDelInst (tmp_insn);
            }
            changed = TRUE;
        }
    }

    return changed;
}

static BOOL
insert_instructions (control_flow_graph cfun, SymTab stab, bitmap ONES, bitmap *insert, struct avl_table *edge_map, struct avl_table *map)
{
    BOOL did_insert = FALSE;
    basic_block* block;
    edge *ei;
    int x;
    bitmap_iterator bi;
    unsigned i;
    IRInst tmp_insn, tmp2_insn, *curs;
    int n_param;
    struct pair *p;
    int k;
    SymDef tmpSym;

    for(  block=(basic_block*) List_First(cfun->basic_block_info)
       ;  block!=NULL
       ;  block = (basic_block*) List_Next((void *)block)
       )
    {
        for(  ei=(edge *) List_First((*block)->preds)
           ;  ei!=NULL 
           ;  ei = (edge *) List_Next((void *)ei)
           )
        {
            x = ((struct pair *) avl_find (edge_map, &(*ei)->uid))->y;
            for (bmp_iter_set_init (&bi, insert[x], 0, &i);
                 bmp_iter_set (&bi, &i);
                 bmp_iter_next (&bi, &i))
            {
                tmp_insn = InterCodeGetInstByID (cfun->code, i);
#if (1)
# ifdef LCM_DEBUG_INFO
                fprintf (stderr, "PRE: edge (%d,%d), ",
                     (*ei)->src->index,(*ei)->dest->index);
                fprintf (stderr, "copy expression ");
                IRInstDump (tmp_insn, FALSE, stderr);
                fprintf (stderr, "\n");
# endif
#endif
                if  (tmp_insn->opcode == IRINST_OP_call)
                {
                    n_param = GetConstVal (IRInstGetOperand (tmp_insn, 2)->var, 0)->cvValue.cvIval;
                    for(  curs=(IRInst *) InterCodeGetCursor (tmp_insn->bb->cfg->code, tmp_insn)
                       ;  curs!=NULL && n_param > 0
                       ;  curs = (IRInst *) List_Prev((void *)curs), n_param--
                       )
                        ;
                    for(
                       ;  curs!=NULL && (*curs)->opcode != IRINST_OP_call
                       ;  curs = (IRInst *) List_Next((void *)curs)
                       )
                    {
                        tmp2_insn = IRInstCopy (*curs);
                        InterCodeInsertBefore ((*ei)->src, (IRInst *) List_Last ((*ei)->src->insns), tmp2_insn, TRUE, NULL);
                        bitmap_set_bit (ONES, tmp2_insn->uid);
                    }
                }

                for (k = 0; k < IRInstGetNumOperands (tmp_insn); k++)
                    if  (IRInstIsOutput (tmp_insn, k))
                        break;

                /* 对于程序计算的每个表达式，创建一个新的临时变量。  */
                p = (struct pair *) avl_find (map, &tmp_insn->uid);
                if  (p == NULL &&
                     k == 0 &&
                     (tmp_insn->opcode != IRINST_OP_call ||
                     IRInstGetOperand (tmp_insn, 0)->var->sdType->tdTypeKind != TYP_VOID))
                {
                    tmpSym = duplicate_symbol (IRInstGetOperand (tmp_insn, k)->var);
                    p = (struct pair *) xmalloc (sizeof (*p));
                    p->x = tmp_insn->uid;
                    p->y = tmpSym->uid;
                    avl_insert (map, p);
                }
                tmp_insn = IRInstCopy (tmp_insn);
                if  (p)
                {
                    tmpSym = stGetSymByID (stab, p->y);
                    IRInstSetOperand (tmp_insn, k, tmpSym);
                    IRInstGetOperand (tmp_insn, k)->version = 0;
                }
                InterCodeInsertBefore ((*ei)->src, (IRInst *) List_Last ((*ei)->src->insns), tmp_insn, TRUE, NULL);
                bitmap_set_bit (ONES, tmp_insn->uid);
                did_insert = TRUE;
            }
        }
    }
    return did_insert;
}

static BOOL
Lazy_Code_Motion (control_flow_graph cfun, SymTab stab, bitmap ONES, struct avl_table *block_map, struct avl_table *edge_map)
{
    BOOL updated;
    bitmap *avout, *avin;
    bitmap *antin, *antout, *earliest;
    bitmap *later, *laterin;
    /* 这些位图将保存每个基本块的局部数据流属性。  */
    bitmap *kill, *avloc, *antloc, *transp;
    bitmap *insert, *del;
    varpool_node_set set;
    int num_edges, num_blocks, i;
    bitmap_head tmp;
    struct avl_table *map;
    basic_block* block;
    int its;

    num_blocks = (int) avl_count (block_map);
    num_edges = (int) avl_count (edge_map);
    set = varpool_node_set_new (cfun, FALSE /* For arrays.  */);

    for(  block=(basic_block*) List_First(cfun->basic_block_info)
       ;  block!=NULL
       ;  block = (basic_block*) List_Next((void *)block)
       )
        for (its = 0, updated = TRUE; its < MAX_ITERATIONS && updated; its++)
            updated = local_copyprop_pass (*block, set);

    transp = bitmap_vector_alloc (num_blocks);
    avloc = bitmap_vector_alloc (num_blocks);
    antloc = bitmap_vector_alloc (num_blocks);
    kill = bitmap_vector_alloc (num_blocks);
    compute_local_properties (cfun, block_map, set, ONES, transp, avloc, antloc);

    /* 使用以下公式计算每个基本块的kill:

       ~(TRANSP | COMP)
    */
    for (i = 0; i < num_blocks; i++)
    {
        tmp.first = tmp.current = 0;
        bitmap_ior (&tmp, transp[i], avloc[i]);
        bitmap_and_compl (kill[i], ONES, &tmp);
        bitmap_clear (&tmp);
    }

#if (0)
# ifndef NDEBUG
    dump_bitmap_vector (stdout, "transp", "", transp,
                        block_map);
    dump_bitmap_vector (stdout, "antloc", "", antloc,
                        block_map);
    dump_bitmap_vector (stdout, "avloc", "", avloc,
                        block_map);
    dump_bitmap_vector (stdout, "kill", "", kill,
                        block_map);
# endif
#endif

    /* 计算全局可用性。  */
    avout = bitmap_vector_alloc (num_blocks);
    avin = bitmap_vector_alloc (num_blocks);
    compute_available (cfun, block_map, ONES, avloc, kill, avout, avin);

#if (0)
# ifndef NDEBUG
    dump_bitmap_vector (stdout, "avout", "", avout, block_map);
    dump_bitmap_vector (stdout, "avin", "", avin, block_map);
# endif
#endif

    bitmap_vector_free (avin, num_blocks);

    /* 计算全局可预测性。  */
    antout = bitmap_vector_alloc (num_blocks);
    antin = bitmap_vector_alloc (num_blocks);
    compute_antinout (cfun, block_map, ONES, antloc, transp, antin, antout);

#if (0)
# ifndef NDEBUG
    dump_bitmap_vector (stdout, "antin", "", antin,
                        block_map);
    dump_bitmap_vector (stdout, "antout", "", antout,
                        block_map);
# endif
#endif

    /* 计算最早插入点。  */
    earliest = bitmap_vector_alloc (num_edges);
    compute_earliest (cfun, block_map, edge_map, ONES, antin, antout, avout, kill, earliest);

#if (0)
# ifndef NDEBUG
    dump_bitmap_vector (stdout, "earliest", "", earliest, edge_map);
# endif
#endif

    bitmap_vector_free (antin, num_blocks);
    bitmap_vector_free (antout, num_blocks);
    bitmap_vector_free (avout, num_blocks);

    later = bitmap_vector_alloc (num_edges);
    /* 在laterin向量中为出口块分配一个额外的元素。  */
    laterin = bitmap_vector_alloc (num_blocks + 1);
    compute_laterin (cfun, block_map, edge_map, ONES, earliest, antloc, later, laterin);
#if (0)
# ifndef NDEBUG
    dump_bitmap_vector (stdout, "laterin", "", laterin,
                        block_map);
    dump_bitmap_vector (stdout, "later", "", later, edge_map);
# endif
#endif

    bitmap_vector_free (earliest, num_edges);

    insert = bitmap_vector_alloc (num_edges);
    del = bitmap_vector_alloc (num_blocks);
    compute_insert_delete (cfun, block_map, edge_map, antloc, later, laterin, insert, del);

    bitmap_vector_free (laterin, num_blocks + 1);
    bitmap_vector_free (later, num_edges);

#if (0)
# ifndef NDEBUG
    dump_bitmap_vector (stdout, "pre_insert_map", "", insert, edge_map);
    dump_bitmap_vector (stdout, "pre_delete_map", "", del,
                        block_map);
# endif
#endif

    map = avl_create ((avl_comparison_func *) compare_pairs, NULL, NULL);
    updated = insert_instructions (cfun, stab, ONES, insert, edge_map, map);
    updated |= delete_instructions (cfun, stab, ONES, del, block_map, map);
    avl_destroy (map, (avl_item_func *) free);

    bitmap_vector_free (insert, num_edges);
    bitmap_vector_free (del, num_blocks);

    bitmap_vector_free (transp, num_blocks);
    bitmap_vector_free (avloc, num_blocks);
    bitmap_vector_free (antloc, num_blocks);
    bitmap_vector_free (kill, num_blocks);

    free_varpool_node_set (set);

    return updated;
}

static void
runOnFunction (control_flow_graph cfun, SymTab stab)
{
    BOOL updated;
    struct avl_table *block_map, *edge_map;
    struct pair *p;
    basic_block* block;
    IRInst *instr;
    bitmap ONES;
    int num_edges, num_blocks;
    edge *ei;
    int its;

    /* 初始化一个从每个基本块/边到其索引的映射。  */
    block_map = avl_create ((avl_comparison_func *) compare_pairs, NULL, NULL);
    edge_map = avl_create ((avl_comparison_func *) compare_pairs, NULL, NULL);
    for(  block=(basic_block*) List_First(cfun->basic_block_info), num_blocks = 0, num_edges = 0
       ;  block!=NULL
       ;  block = (basic_block*) List_Next((void *)block), num_blocks++
       )
    {
        p = (struct pair *) xmalloc (sizeof (struct pair));
        p->x = (*block)->index;
        p->y = num_blocks;
        avl_insert (block_map, p);
        for(  ei=(edge *) List_First((*block)->preds)
           ;  ei!=NULL 
           ;  ei = (edge *) List_Next((void *)ei), num_edges++
           )
        {
            p = (struct pair *) xmalloc (sizeof (struct pair));
            p->x = (*ei)->uid;
            p->y = num_edges;
            avl_insert (edge_map, p);
        }
    }

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
            if  (!should_skip (*instr))
                bitmap_set_bit (ONES, (*instr)->uid);

    for (its = 0, updated = TRUE; its < MAX_ITERATIONS && updated; its++)
        updated = Lazy_Code_Motion (cfun, stab, ONES, block_map, edge_map);

    avl_destroy (block_map, (avl_item_func *) free);
    avl_destroy (edge_map, (avl_item_func *) free);

    BITMAP_XFREE (ONES);
}

void
LazyCodeMotion (InterCode code, SymTab stab)
{
    control_flow_graph *F;

    for(  F=(control_flow_graph *)List_First(code->funcs)
       ;  F!=NULL
       ;  F = (control_flow_graph *)List_Next((void *)F)
       )
    {
        /* 创建循环前置结点。  */
        flow_loops_find (*F);
#if (1)
# ifdef LCM_DEBUG_INFO
        flow_loops_dump ((*F)->loops, stdout);
# endif
#endif
        create_preheaders (*F, stab);
        flow_loops_free (&(*F)->loops);

        /* 拆分关键边。  */
        split_critical_edges (*F, stab);

        runOnFunction (*F, stab);

        cleanup_cfg (*F, stab);
    }
}
