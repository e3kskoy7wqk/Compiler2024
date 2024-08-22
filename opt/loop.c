#include <stdlib.h>
#include <string.h>
#include "all.h"

/* 分配并返回新的循环结构。  */
static struct loop *
alloc_loop (void)
{
    struct loop *loop = (struct loop *) malloc (sizeof (*loop));
    static int counter = 0;

    memset (loop, 0, sizeof *loop);
    loop->num = ++counter;
    loop->superloops = List_Create ();
    loop->inner = List_Create ();

    return loop;
}

/* 回收分配给循环的数据。  */
static void
flow_loop_free (struct loop *loop)
{
    List_Destroy (&loop->inner);
    List_Destroy (&loop->superloops);
    free (loop);
}

/* 释放所有为循环分配的内存。  */
void
flow_loops_free (struct loops **loops)
{
    if ((*loops)->larray)
    {
        struct loop **loop;

        /* 释放循环描述符。  */
        for(  loop=(struct loop **)List_First((*loops)->larray)
           ;  loop!=NULL
           ;  loop = (struct loop **) List_Next((void *)loop)
           )
        {
            if (!*loop)
                continue;

            flow_loop_free (*loop);
        }

        List_Destroy (&(*loops)->larray);
    }
    free (*loops);
    *loops = NULL;
}

/* 初始化loops结构。  */
static void
init_loops_structure (control_flow_graph fn)
{
    struct loop *root;

    fn->loops = (struct loops *) malloc (sizeof (*fn->loops));
    memset (fn->loops, 0, sizeof *fn->loops);
    fn->loops->larray = List_Create ();

    /* 包含整个函数的假循环。  */
    root = alloc_loop ();
    root->num_nodes = List_Card (fn->basic_block_info);
    root->header = fn->entry_block_ptr;
    root->latch = fn->exit_block_ptr;
    fn->entry_block_ptr->loop_father = root;
    fn->exit_block_ptr->loop_father = root;

    *(struct loop **)List_NewLast(fn->loops->larray, sizeof (struct loop *)) = root;
    fn->loops->tree_root = root;
}

/* 返回HEADER是否为循环头。  */
BOOL
is_loop_head (control_flow_graph cfun, basic_block header)
{
    edge *ei;

    /* 查找回边，其中前趋被此块所支配。一个自然的循环只有一个入
       口节点(header)，它支配着循环中的所有节点。它还有一条单独的
       回边，从latch节点回到header。  */
    for(  ei=List_First(header->preds)
       ;  ei!=NULL
       ;  ei = List_Next((void *)ei)
       )
    {
        basic_block latch = (*ei)->src;
        if (latch != cfun->entry_block_ptr
            && bitmap_bit_p(latch->dom[0], header->index))
            return TRUE;
    }

  return FALSE;
}

/* 记录loop的向量，其直接的superloop是FATHER。  */
static void
establish_preds (struct loop *loop, struct loop *father)
{
    struct loop **ploop;

    List_Destroy (&loop->superloops);
    loop->superloops = List_Create ();
    for(  ploop=(struct loop **) List_First(father->superloops)
       ;  ploop!=NULL
       ;  ploop = (struct loop **) List_Next((void *)ploop)
       )
        *(struct loop **)List_NewLast (loop->superloops, sizeof (struct loop *)) = *ploop;
    *(struct loop **)List_NewLast (loop->superloops, sizeof (struct loop *)) = father;
  
    for(  ploop=(struct loop **) List_First(loop->inner)
       ;  ploop!=NULL
       ;  ploop = (struct loop **) List_Next((void *)ploop)
       )
        establish_preds (*ploop, loop);
}

/* 将循环添加到循环层次树中，其中父亲是添加的循环的父亲。如果循
   环有一些子循环，注意它们的pred字段将被正确初始化。  */
static void
flow_loop_tree_node_add (struct loop *father, struct loop *loop)
{
    *(struct loop **)List_NewFirst (father->inner, sizeof (struct loop *)) = loop;

    establish_preds (loop, father);
}

/* 使用header查找循环中包含的节点。返回循环中节点的数目。  */
static int
flow_loop_nodes_find (basic_block header, struct loop *loop)
{
    LIST stack = List_Create ();
    int num_nodes = 1;
    edge *latch_ei;

    header->loop_father = loop;

    for(  latch_ei=(edge *) List_First(loop->header->preds)
       ;  latch_ei!=NULL
       ;  latch_ei = (edge *) List_Next((void *)latch_ei)
       )
    {
        if ((*latch_ei)->src->loop_father == loop
            || !bitmap_bit_p ((*latch_ei)->src->dom[0], loop->header->index))
            continue;

        num_nodes++;
        *(basic_block *) List_NewLast (stack, sizeof (basic_block)) = (*latch_ei)->src;
        (*latch_ei)->src->loop_father = loop;

        while (!List_IsEmpty (stack))
        {
            basic_block node;
            edge *ei;

            node = *(basic_block *) List_Last (stack);
            List_DeleteLast (stack);

            for(  ei=(edge *) List_First(node->preds)
               ;  ei!=NULL
               ;  ei = (edge *) List_Next((void *)ei)
               )
            {
                basic_block ancestor = (*ei)->src;

                if (ancestor->loop_father != loop)
                {
                    ancestor->loop_father = loop;
                    num_nodes++;
                    *(basic_block *) List_NewLast (stack, sizeof (basic_block)) = ancestor;
                }
            }
        }
    }
    List_Destroy (&stack);

    return num_nodes;
}

/* 如果LOOP的节点是OUTER的子集，则返回非零值。  */
BOOL
is_flow_loop_nested (const struct loop *outer, const struct loop *loop)
{
    struct loop **ploop;

    for(  ploop=(struct loop **) List_First(loop->superloops)
       ;  ploop!=NULL
       ;  ploop = (struct loop **) List_Next((void *)ploop)
       )
        if  (*ploop == outer)
            return TRUE;

    return (FALSE);
}

/* Return nonzero if basic block BB belongs to LOOP.  */
BOOL
is_flow_bb_inside_loop (const struct loop *loop, basic_block bb)
{
    struct loop *source_loop;

    if (bb == bb->cfg->entry_block_ptr
        || bb == bb->cfg->exit_block_ptr)
      return FALSE;

    source_loop = bb->loop_father;
    return loop == source_loop || is_flow_loop_nested (loop, source_loop);
}

struct loops *
flow_loops_find (control_flow_graph cfun)
{
    LIST rc_order;
    basic_block *header;
    LIST larray;
    struct loop **loop;

    /* 确保dominator被计算出来。  */
    compute_dominators (cfun, FALSE);

    init_loops_structure (cfun); 

    /* 计算CFG的深度优先搜索顺序，以便在找到内部自然循环之前找到外部自然循环。  */
    rc_order = List_Create ();
    pre_and_rev_post_order_compute (cfun, NULL, rc_order, TRUE, FALSE);

    /* 以相反的完成顺序收集所有循环头，并分配循环结构。  */
    larray = List_Create ();
    for(  header=(basic_block *) List_First(rc_order)
       ;  header!=NULL
       ;  header = (basic_block *)List_Next((void *)header)
       )
    {
        if  (is_loop_head (cfun, *header))
        {
            struct loop *loop = alloc_loop ();
            *(struct loop **)List_NewLast(cfun->loops->larray, sizeof (struct loop *)) = loop;
            loop->header = *header;
            /* 重置latch，我们在下面重新计算它。  */
            loop->latch = NULL;
            *(struct loop **)List_NewLast(larray, sizeof (struct loop *)) = loop;
        }
        /* 在开始时使块成为循环根节点的一部分。  */
        (*header)->loop_father = cfun->loops->tree_root;
    }

    List_Destroy (&rc_order);

    /* 现在遍历找到的循环，将它们插入循环树。  */
    for(  loop=(struct loop **)List_First(larray)
       ;  loop!=NULL
       ;  loop = (struct loop **) List_Next((void *)loop)
       )
    {
        basic_block header = (*loop)->header;
        edge *ei;

        flow_loop_tree_node_add (header->loop_father, *loop);
        (*loop)->num_nodes = flow_loop_nodes_find ((*loop)->header, *loop);

        for(  ei=(edge *) List_First(header->preds)
           ;  ei!=NULL
           ;  ei = (edge *) List_Next((void *)ei)
           )
        {
            basic_block latch = (*ei)->src;

            if (is_flow_bb_inside_loop (*loop, latch))
            {
                if ((*loop)->latch != NULL)
                {
                    /* 存在一个以上的latch。  */
                    (*loop)->latch = NULL;
                    break;
                }
                (*loop)->latch = latch;
            }
        }
    }

    List_Destroy (&larray);

    return cfun->loops;
}

/* 返回循环latch边的列表。  */
LIST 
get_loop_latch_edges (const struct loop *loop)
{
    edge *ei;
    LIST ret = List_Create ();

    for(  ei=(edge *) List_First(loop->header->preds)
       ;  ei!=NULL
       ;  ei = (edge *) List_Next((void *)ei)
       )
    {
        if (bitmap_bit_p ((*ei)->src->dom[0], loop->header->index))
            *(edge *)List_NewLast (ret, sizeof (edge)) = *ei;
    }

    return ret;
}

/* 获取循环的基本块。header是第0个块，其余按dfs顺序从latch的边缘方向。
   特别地，if header != latch, latch是第1个块。  */
LIST 
get_loop_body (const struct loop *loop)
{
    basic_block *bb;
    LIST body;

    body = List_Create();

    if (loop->latch && loop->latch == loop->latch->cfg->exit_block_ptr)
    {
        /* 包含整个函数的假循环。  */
        *(basic_block *) List_NewLast (body, sizeof (basic_block)) = loop->header;
        *(basic_block *) List_NewLast (body, sizeof (basic_block)) = loop->latch->cfg->exit_block_ptr;
        for(  bb=(basic_block *)List_First(loop->latch->cfg->basic_block_info)
           ;  bb!=NULL
           ;  bb = (basic_block *)List_Next((void *)bb)
           )
            if  (*bb != loop->latch->cfg->exit_block_ptr &&
                 *bb != loop->latch->cfg->entry_block_ptr)
                *(basic_block *) List_NewLast (body, sizeof (basic_block)) = *bb;
    }
    else
    {
        LIST st;
        bitmap visited;
        basic_block lbb;
        int tv = 0;

        st = List_Create ();
        visited = BITMAP_XMALLOC ();
        *(basic_block *)List_NewLast(st, sizeof (basic_block)) = loop->header;
        *(basic_block *) List_NewLast (body, sizeof (basic_block)) = loop->header;
        tv++;
        bitmap_set_bit (visited, loop->header->index);
        while (! List_IsEmpty (st) && tv < loop->num_nodes)
        {
            edge *ei;
            lbb = *(basic_block *)List_Last (st);
            List_DeleteLast(st);
            for(  ei=(edge *) List_First(lbb->preds)
               ;  ei!=NULL && tv < loop->num_nodes
               ;  ei = (edge *) List_Next((void *)ei)
               )
                if (!bitmap_bit_p (visited, (*ei)->src->index) && 
                    (*ei)->src != loop->header &&
                    bitmap_bit_p((*ei)->src->dom[0], loop->header->index))
                {
                    *(basic_block *)List_NewLast(st, sizeof (basic_block)) = (*ei)->src;
                    *(basic_block *)List_NewLast(body, sizeof (basic_block)) = (*ei)->src;
                    tv++;
                    bitmap_set_bit (visited, (*ei)->src->index);
                }
        }
        List_Destroy (&st);
        BITMAP_XFREE (visited);
    }

    return body;
}

/* 将loop指定的循环信息转储到文件中.  */
void
flow_loop_dump (const struct loop *loop, FILE *file)
{
    LIST latches;
    edge *ei;
    LIST bbs;
    basic_block *bb;

    if (! loop || ! loop->header)
        return;

    fprintf (file, ";;\n;; Loop %d\n", loop->num);

    fprintf (file, ";;  header %d, ", loop->header->index);
    if (loop->latch)
        fprintf (file, "latch %d\n", loop->latch->index);
    else
    {
        fprintf (file, "multiple latches:");
        latches = get_loop_latch_edges (loop);
        for(  ei=(edge *) List_First(latches)
           ;  ei!=NULL
           ;  ei = (edge *) List_Next((void *)ei)
           )
            fprintf (file, " %d", (*ei)->src->index);
        List_Destroy (&latches);
        fprintf (file, "\n");
    }

    fprintf (file, ";;  depth %d, outer %ld\n",
             List_Card (loop->superloops), (long) (List_Last(loop->superloops)
                                        ? (*(struct loop **)List_Last (loop->superloops))->num : -1));

    fprintf (file, ";;  nodes:");
    bbs = get_loop_body (loop);
    for(  bb=(basic_block *) List_First(bbs)
       ;  bb!=NULL
       ;  bb = (basic_block *) List_Next((void *)bb)
       )
       fprintf (file, " %d", (*bb)->index);
    List_Destroy (&bbs);
    fprintf (file, "\n");

}

/* 将循环信息转储到文件中。  */
void
flow_loops_dump (struct loops *current_loops, FILE *file)
{
    struct loop **loop;

    if (!current_loops || ! file)
        return;

    fprintf (file, ";; %d loops found\n", List_Card (current_loops->larray));

    for(  loop=(struct loop **)List_First(current_loops->larray)
       ;  loop!=NULL
       ;  loop = (struct loop **) List_Next((void *)loop)
       )
    {
        flow_loop_dump (*loop, file);
    }
    fprintf (file, "\n");
}

/* 返回循环的出口边的列表。  */
static LIST
get_loop_exit_edges (const struct loop *loop)
{
    LIST edges = List_Create ();
    LIST body;
    edge *ei;
    basic_block *curs;

    body = get_loop_body (loop);
    for(  curs=(basic_block *) List_First(body)
       ;  curs!=NULL
       ;  curs = (basic_block *) List_Next((void *)curs)
       )
    {
        for(  ei=(edge *) List_First((*curs)->succs)
           ;  ei!=NULL
           ;  ei = (edge *) List_Next((void *)ei)
           )
        {
            if  (!is_flow_bb_inside_loop (loop, (*ei)->dest))
            {
                *(edge *)List_NewLast(edges, sizeof (edge)) = *ei;
            }
        }
    }
    List_Destroy (&body);

    return edges;
}

/* 返回循环的单个出口，如果循环没有出口或有多个出口，则返回NULL。  */
edge
single_exit (const struct loop *loop)
{
    LIST edges = get_loop_exit_edges (loop);
    edge exit = NULL;
    if  (List_Card(edges) == 1)
        exit = *(edge *) List_First (edges);
    List_Destroy (&edges);
    return exit;
}

void
copy_loop_headers (control_flow_graph cfun)
{
    struct loop **loop;
    basic_block header;
    edge exit;
    LIST headers;
    basic_block bb, *curs;
    LIST latches;
    edge *latch = NULL;
    edge e;
    basic_block new_bb;
    basic_block prev_bb, prev_bb_1;
    edge *ei;
    edge *next_edge;
    int orig_insn;

    for(  loop=(struct loop **)List_First(cfun->loops->larray)
       ;  loop!=NULL
       ;  loop = (struct loop **) List_Next((void *)loop)
       )
    {
        header = (*loop)->header;

        if (header == cfun->entry_block_ptr)
            continue;

        if (List_Card (header->succs) == 1)
            continue;
        if (is_flow_bb_inside_loop (*loop, (*(edge *) List_First (header->succs))->dest)
            && is_flow_bb_inside_loop (*loop, (*(edge *) List_Last (header->succs))->dest))
            continue;

        /* 获取循环的出口边。  */
        exit = single_exit (*loop);

        /* 如果循环有多个出边，则放弃。  */
        if (! exit)
            continue;

        /* 找到所有要复制的循环头。  */
        headers = List_Create ();
        bb = exit->src;
        while (bb != header)
        {
            *(basic_block *)List_NewLast (headers, sizeof (basic_block)) = bb;
            bb = get_immediate_dominator (cfun, FALSE, bb);
        }
        *(basic_block *)List_NewLast (headers, sizeof (basic_block)) = header;

        latches = get_loop_latch_edges (*loop);
        for(  latch=(edge *) List_First(latches)
           ;  latch!=NULL
           ;  latch = (edge *) List_Next((void *)latch)
           )
        {
            e = *latch;
            prev_bb = NULL;
            prev_bb_1 = NULL;
            for(  curs=(basic_block *) List_First(headers)
               ;  curs!=NULL
               ;  curs = (basic_block *) List_Next((void *)curs)
               )
            {
                orig_insn = (*(IRInst *) List_First(e->dest->insns))->uid;
                new_bb = split_edge (e);
                e = *(edge *) List_First(new_bb->preds);
                /* 清除新创建的基本块的所有后继边。  */
                for(  ei=(edge *)List_First(new_bb->succs)
                   ;  ei!=NULL
                   ;  ei = next_edge
                   )
                {
                    next_edge = (edge *) List_Next((void *)ei);
                    if  (prev_bb != (*ei)->dest)
                        remove_edge (*ei);
                }
                /* 新建基本块的后继恰好为原基本块的后继。  */
                for(  ei=(edge *)List_First((*curs)->succs)
                   ;  ei!=NULL
                   ;  ei = (edge *)List_Next((void *)ei)
                   )
                {
                    if  (prev_bb_1 != (*ei)->dest)
                    {
                        make_edge (new_bb, (*ei)->dest);
                    }
                }
                copy_block (new_bb, *curs);
                new_bb->loop_father = (*curs)->loop_father;
                update_destinations (new_bb, orig_insn, (*(IRInst *) List_First(new_bb->insns))->uid, NULL);
                if  ((*(IRInst *) List_Last(new_bb->insns))->opcode == IRINST_OP_goto)
                {
                    orig_insn = GetConstVal(IRInstGetOperand(*(IRInst *) List_Last(new_bb->insns), 0)->var, 0)->cvValue.cvIval;
                    update_destinations (prev_bb, orig_insn, (*(IRInst *) List_First(prev_bb->insns))->uid, NULL);
                }
                prev_bb = new_bb;
                prev_bb_1 = *curs;
            }
        }
        List_Destroy (&latches);
        List_Destroy (&headers);
    }
}

/* 为循环创建一个循环前置结点，没有SSA.  */
static basic_block
create_preheader (struct loop *loop, SymTab stab)
{
    LIST latches = get_loop_latch_edges (loop);
    edge *ei;
    edge *ei2;
    basic_block new_block = NULL;
    edge *next_edge;
    IRInst jump_insn;

    new_block = alloc_block ();
    link_block (loop->header->cfg, new_block);
    jump_insn = IRInstEmitInst (IRINST_OP_goto, (*(IRInst *)List_First (loop->header->insns))->line, (*(IRInst *)List_First (loop->header->insns))->column);
    InterCodeAddInst (new_block, jump_insn, TRUE);
    IRInstSetOperand (jump_insn, 0, stCreateIconNode(stab, (*(IRInst *)List_First (loop->header->insns))->uid));

    for(  ei=(edge *) List_First(loop->header->preds)
       ;  ei!=NULL
       ;  ei = next_edge
       )
    {
        next_edge = (edge *) List_Next((void *)ei);
        for(  ei2=(edge *) List_First(latches)
           ;  ei2!=NULL
           ;  ei2 = (edge *) List_Next((void *)ei2)
           )
            if ((*ei2)->uid == (*ei)->uid)
                break;
        if  (ei2!=NULL)
            continue;
        make_edge ((*ei)->src, new_block);
        remove_edge (*ei);
    }
    make_edge (new_block, loop->header);
    List_Destroy (&latches);

    update_destinations (new_block, (*(IRInst *)List_First (loop->header->insns))->uid, jump_insn->uid, NULL);

    return new_block;
}

/* 为每个循环创建循环前置结点，没有SSA。  */
void
create_preheaders (control_flow_graph cfun, SymTab stab)
{
    struct loop **loop;

    if (!cfun->loops)
        return;

    for(  loop=(struct loop **)List_First(cfun->loops->larray)
       ;  loop!=NULL
       ;  loop = (struct loop **) List_Next((void *)loop)
       )
    {
        if ((*loop)->header == cfun->entry_block_ptr)
            continue;
        create_preheader (*loop, stab);
    }
}

/* 返回BB所属循环的循环深度。  */
int
loop_depth (basic_block bb)
{
    return bb->loop_father ? List_Card (bb->loop_father->superloops) : 0;
}

