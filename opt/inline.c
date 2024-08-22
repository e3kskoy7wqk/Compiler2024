#include <stdlib.h>
#include <string.h>
#include "all.h"

#define THRESHOLD 666
#define max_expanded 3

typedef struct ArcBox
{
    int tailvex,headvex; /* 该弧的尾和头顶点的位置 */
    struct ArcBox *hlink,*tlink; /* 分别为弧头相同和弧尾相同的弧的链域 */
    bitmap insns;
}ArcBox; /* 弧结点 */
typedef struct
{
    ArcBox *firstin,*firstout; /* 分别指向该顶点第一条入弧和出弧 */
    SymDef func;
    int count; /* 递归函数调用自身被内联的次数 */
}VexNode; /* 顶点结点 */
typedef struct
{
    VexNode *xlist; /* 表头向量(数组) */
    int allocated;
    int vexnum,arcnum; /* 有向图的当前顶点数和弧数 */
    struct avl_table *map;
}OLGraph;

struct pair
{
    int x;
    int y;
};

static int compare( struct pair *arg1, struct pair *arg2 )
{
    int ret = 0 ;

    if ( arg1->x < arg2->x )
        ret = -1 ;
    else if ( arg1->x > arg2->x )
        ret = 1 ;

    return( ret );
}

static int InsertVex(OLGraph *G, SymDef func)
{  /* 初始条件: 有向图G存在,v和有向图G中顶点有相同特征 */
   /* 操作结果: 在有向图G中增添新顶点v(不增添与顶点相关的弧,留待InsertArc()去做) */
    struct pair tmp;
    struct pair *p;

    tmp.x = func->uid;
    p = avl_find (G->map, &tmp);
    if  (p)
        return p->y;

    if ((*G). vexnum >= G->allocated) 
    {
        int new_size = (*G). vexnum * 2 + 1;
        (*G). xlist = (VexNode *) xrealloc ((*G). xlist, new_size * sizeof (VexNode));
        G->allocated = new_size;
    }
    (*G). xlist[(*G). vexnum].firstin=(*G). xlist[(*G). vexnum].firstout=NULL;
    (*G). xlist[(*G). vexnum].func= func;
    (*G). xlist[(*G). vexnum].count= 0;
    p = (struct pair *) xmalloc (sizeof (struct pair));
    p->x = func->uid;
    p->y = (*G). vexnum;
    avl_insert (G->map, p);
    (*G). vexnum++;
    return (*G). vexnum - 1;
}

static void InsertArc(OLGraph *G,int i,int j, IRInst instr)
{  /* 初始条件: 有向图G存在,v和w是G中两个顶点 */
   /* 操作结果: 在G中增添弧<i,j> */
    ArcBox *p;
    p=G->xlist[i].firstout;
    while(p)
    {
        if  (p->headvex == j)
        {
            bitmap_set_bit (p->insns, instr->uid);
            return;
        }

        p=p->hlink;
    }
    p=(ArcBox *)xmalloc(sizeof(ArcBox)); /* 生成新结点 */
    p->insns = BITMAP_XMALLOC ();
    bitmap_set_bit (p->insns, instr->uid);
    p->tailvex=i; /* 给新结点赋值 */
    p->headvex=j;
    p->hlink=(*G). xlist[j].firstin; /* 插在入弧和出弧的链头 */
    p->tlink=(*G). xlist[i].firstout;
    (*G). xlist[j].firstin=(*G). xlist[i].firstout=p;
    (*G). arcnum++; /* 弧数加1 */
}

static void CreateDG(OLGraph *G)
{  /* 采用十字链表存储表示,构造有向图G。算法7.3 */
    memset (G, '\0', sizeof (*G));
    G->map = avl_create ((avl_comparison_func *) compare, NULL, NULL);
}

static void DestroyGraph(OLGraph *G)
{  /* 初始条件: 有向图G存在 */
   /* 操作结果: 销毁有向图G */
    int j;
    ArcBox *p,*q;
    for(j=0;j<(*G).vexnum;j++) /* 对所有顶点 */
    {
        p=(*G).xlist[j].firstout; /* 仅处理出弧 */
        while(p)
        {
            q=p;
            p=p->tlink;
            BITMAP_XFREE (q->insns);
            free(q);
        }
    }
    free (G->xlist);
    avl_destroy (G->map, (avl_item_func *) free);
    (*G).arcnum=0;
    (*G).vexnum=0;
    (*G).allocated=0;
}

static void
Build (OLGraph *G, InterCode code)
{
    control_flow_graph *F;
    basic_block *block;
    IRInst *instr;
    SymDef caller, callee;

    CreateDG (G);

    for(  F=(control_flow_graph *)List_First(code->funcs)
       ;  F!=NULL
       ;  F = (control_flow_graph *)List_Next((void *)F)
       )
    {
        caller = IRInstGetOperand((*(IRInst *) List_First((*F)->entry_block_ptr->insns)), 0)->var;
        for(  block=(basic_block *)List_First((*F)->basic_block_info)
           ;  block!=NULL
           ;  block = (basic_block *)List_Next((void *)block)
           )
        {
            for(  instr=(IRInst *) List_First ((*block)->insns)
               ;  instr!=NULL
               ;  instr = (IRInst *) List_Next ((void *)instr)
               )
            {
                if  ((*instr)->opcode == IRINST_OP_call)
                {
                    callee = IRInstGetOperand(*instr, 1)->var;
                    InsertArc (G, InsertVex(G, caller), InsertVex(G, callee), *instr);
                }
            }
        }
    }
}

static BOOL DFS(OLGraph *G,int i, bitmap visited, int target) /* DFSTraverse()调用 */
{
    ArcBox *p;
    bitmap_set_bit(visited, i); /* 访问标志数组置1(已被访问) */
    p=G->xlist[i].firstout; /* p指向第i个顶点的出度 */
    while(p) /* p没到表尾 */
    {
        if(target == p->headvex)
            return TRUE;
        p=p->tlink; /* 查找下一个结点 */
    }
    p=G->xlist[i].firstout; /* p指向第i个顶点的出度 */
    while(p&&bitmap_bit_p(visited, p->headvex)) /* p没到表尾且该弧的头顶点已被访问 */
        p=p->tlink; /* 查找下一个结点 */
    if(p&&!bitmap_bit_p(visited, p->headvex)) /* 该弧的头顶点未被访问 */
        return DFS(G,p->headvex, visited, target); /* 递归调用DFS() */
    return FALSE;
}

static BOOL DFSTraverse(OLGraph *G, int target)
{  /* 初始条件: 有向图G存在,v是G中某个顶点,Visit是顶点的应用函数 */
   /* 操作结果: 从第1个顶点起,按深度优先递归遍历有向图G,并对每个顶点调用 */
   /*           函数Visit一次且仅一次。一旦Visit()失败,则操作失败 */
    bitmap visited = BITMAP_XMALLOC ();
    BOOL result;
    result = DFS (G,target, visited, target);
    BITMAP_XFREE (visited);
    return result;
}

static VexNode* GetVex(OLGraph *G,int v)
{  /* 初始条件:有向图G存在,v是G中某个顶点的序号。操作结果:返回v的值 */
   return &G->xlist[v];
}

static ArcBox *DeleteArc(OLGraph *G,int i,int j, IRInst instr)
{  /* 初始条件: 有向图G存在,v和w是G中两个顶点 */
   /* 操作结果: 在G中删除弧<v,w> */
    ArcBox *p1,*p2;

    p2=(*G). xlist[i].firstout; /* 将弧结点从出弧链表中删去 */
    while(p2&&p2->headvex!=j) /* 向后找 */
    {
        p2=p2->tlink;
    }
    bitmap_clear_bit (p2->insns, instr->uid);

    if(! bitmap_empty_p(p2->insns))
        return p2;

    p2=(*G). xlist[i].firstout; /* 将弧结点从出弧链表中删去 */
    if(p2&&p2->headvex==j) /* 第1个结点为待删除结点 */
          (*G). xlist[i].firstout=p2->tlink;
    else
    {
        while(p2&&p2->headvex!=j) /* 向后找 */
        {
            p1=p2;
            p2=p2->tlink;
        }
        if(p2) /* 没到表尾 */
        p1->tlink=p2->tlink;
    }
    p2=(*G). xlist[j].firstin; /* 将弧结点从入弧链表中删去 */
    if(p2&&p2->tailvex==i)
        (*G). xlist[j].firstin=p2->hlink;
    else
    {
        while(p2&&p2->tailvex!=i)
        {
            p1=p2;
            p2=p2->hlink;
        }
        if(p2) /* 没到表尾 */
            p1->hlink=p2->hlink;
    }
    BITMAP_XFREE (p2->insns);
    free(p2);
    (*G). arcnum--; /* 弧数减1 */

    return NULL;
}

static control_flow_graph
copy_function (control_flow_graph node, SymTab stab)
{
    control_flow_graph function;
    struct avl_table *block_map;
    struct avl_table *var_map;
    basic_block *curs;
    basic_block block;
    struct pair *p;
    edge *ei;
    varpool_node_set set;
    varpool_node vnode;
    struct avl_traverser trav;
    SymDef tmp_var;
    IRInst *instr;
    int i;

    block_map = avl_create ((avl_comparison_func *) compare, NULL, NULL);
    function = alloc_flow (node->code);

    /* 复制基本块。  */
    for(  curs=(basic_block *) List_First(node->basic_block_info)
       ;  curs!=NULL
       ;  curs = (basic_block *) List_Next((void *)curs)
       )
    {
        block = alloc_block ();
        link_block (function, block);
        copy_block (block, *curs);
        p = (struct pair *) xmalloc (sizeof (struct pair));
        p->x = (*curs)->index;
        p->y = block->index;
        avl_insert (block_map, p);
    }

    /* 连接新创建的基本块。  */
    for(  curs=(basic_block *) List_First(node->basic_block_info)
       ;  curs!=NULL
       ;  curs = (basic_block *) List_Next((void *)curs)
       )
    {
        block = lookup_block_by_id (function, ((struct pair *) avl_find(block_map, &(*curs)->index))->y);
        for(  ei=(edge *) List_First((*curs)->succs)
           ;  ei!=NULL
           ;  ei = (edge *) List_Next((void *)ei)
           )
        {
            make_edge (block, lookup_block_by_id (function, ((struct pair *) avl_find(block_map, &(*ei)->dest->index))->y));
        }
    }

    /* 更新跳转的目标指令。  */
    for(  curs=(basic_block *) List_First(node->basic_block_info)
       ;  curs!=NULL
       ;  curs = (basic_block *) List_Next((void *)curs)
       )
    {
        block = lookup_block_by_id (function, ((struct pair *) avl_find(block_map, &(*curs)->index))->y);
        update_destinations (block, (*(IRInst *)List_First((*curs)->insns))->uid, (*(IRInst *)List_First(block->insns))->uid, NULL);
    }

    function->entry_block_ptr = lookup_block_by_id (function, ((struct pair *) avl_find(block_map, &node->entry_block_ptr->index))->y);
    function->exit_block_ptr = lookup_block_by_id (function, ((struct pair *) avl_find(block_map, &node->exit_block_ptr->index))->y);
    avl_destroy (block_map, (avl_item_func *) free);

    /* 映射符号。  */
    if  (stab)
    {
        var_map = avl_create ((avl_comparison_func *) compare, NULL, NULL);
        set = varpool_node_set_new (function, TRUE);
        for(  vnode = (varpool_node) avl_t_first (&trav, set->nodes)
           ;  vnode != NULL
           ;  vnode = (varpool_node) avl_t_next (&trav)
           )
        {
            if  (vnode->var->sdSymKind == SYM_VAR &&
                 !vnode->var->sdVar.sdvConst &&
                 !is_global_var (vnode->var))
            {
                tmp_var = duplicate_symbol (vnode->var);
                p = (struct pair *) xmalloc (sizeof (struct pair));
                p->x = vnode->uid;
                p->y = tmp_var->uid;
                avl_insert (var_map, p);
            }
        }

        for(  curs=(basic_block *) List_First(function->basic_block_info)
           ;  curs!=NULL
           ;  curs = (basic_block *) List_Next((void *)curs)
           )
        {
            for(  instr=(IRInst *) List_First((*curs)->insns)
               ;  instr!=NULL 
               ;  instr = (IRInst *) List_Next((void *)instr)
               )
            {
                for (i = IRInstGetNumOperands(*instr) - 1;
                 i >= 0;
                 i--)
                {
                    vnode = varpool_get_node (set, IRInstGetOperand (*instr, i));
                    p = (struct pair *) avl_find (var_map, &vnode->uid);
                    if  (p)
                    {
                        IRInstSetOperand (*instr, i, stGetSymByID (stab, p->y));
                        IRInstGetOperand (*instr, i)->version = 0;
                    }
                }
            }
        }

        free_varpool_node_set (set);
        avl_destroy (var_map, (avl_item_func *) free);
    }

    return function;
}

static ArcBox * 
inline_call (OLGraph *G, IRInst call, VexNode *caller, VexNode *callee, SymTab stab)
{
    edge *ei;
    IRInst *insn1, *insn2;
    IRInst *next_insn;
    IRInst tmp_insn, tmp2_insn;
    struct avl_table *block_map;
    control_flow_graph *F;
    basic_block *curs;
    basic_block entry, exit;
    basic_block block;
    struct pair *p;
    int count;
    ssa_name name;
    ssa_name offset;
    ssa_name replace;
    SymDef tmp_var;
    ArcBox *retval;
    control_flow_graph orig_func;

    /* 找到要被内联的函数。  */
    for(  F=(control_flow_graph *)List_First(call->bb->cfg->code->funcs)
       ;  F!=NULL
       ;  F = (control_flow_graph *)List_Next((void *)F)
       )
        if  (IRInstGetOperand (*(IRInst *)List_First ((*F)->entry_block_ptr->insns), 0)->var == callee->func)
            break;

    /* 复制将被内联的函数。  */
    orig_func = copy_function (*F, caller->func == callee->func ? stab : NULL);

    /* 以函数调用为界，将原基本块拆分成两个。  */
    entry = call->bb;
    exit = alloc_block ();
    link_block (entry->cfg, exit);
    for(  ei=(edge *) List_First(entry->succs)
       ;  ei!=NULL
       ;  ei = (edge *) List_First(entry->succs)
       )
    {
        make_edge (exit, (*ei)->dest);
        remove_edge (*ei);
    }
    for(  insn1= (IRInst *) List_Next (InterCodeGetCursor(entry->cfg->code, call))
       ;  insn1!=NULL
       ;  insn1 = next_insn
       )
    {
        next_insn = (IRInst *) List_Next((void *)insn1);
        tmp_insn = *insn1;
        InterCodeRemoveInst (entry->cfg->code, tmp_insn, NULL);
        InterCodeAddInst (exit, tmp_insn, FALSE);
    }

    /* 复制基本块。  */
    block_map = avl_create ((avl_comparison_func *) compare, NULL, NULL);
    for(  curs=(basic_block *) List_First(orig_func->basic_block_info)
       ;  curs!=NULL
       ;  curs = (basic_block *) List_Next((void *)curs)
       )
    {
        if      (*curs == orig_func->entry_block_ptr)
        {
            block = entry;
        }
        else if (*curs == orig_func->exit_block_ptr)
        {
            block = exit;
        }
        else
        {
            block = alloc_block ();
            link_block (entry->cfg, block);
            copy_block (block, *curs);
        }
        p = (struct pair *) xmalloc (sizeof (struct pair));
        p->x = (*curs)->index;
        p->y = block->index;
        avl_insert (block_map, p);
    }

    /* 连接新创建的基本块。  */
    for(  curs=(basic_block *) List_First(orig_func->basic_block_info)
       ;  curs!=NULL
       ;  curs = (basic_block *) List_Next((void *)curs)
       )
    {
        block = lookup_block_by_id (entry->cfg, ((struct pair *) avl_find(block_map, &(*curs)->index))->y);
        for(  ei=(edge *) List_First((*curs)->succs)
           ;  ei!=NULL
           ;  ei = (edge *) List_Next((void *)ei)
           )
        {
            make_edge (block, lookup_block_by_id (entry->cfg, ((struct pair *) avl_find(block_map, &(*ei)->dest->index))->y));
        }
    }

    /* 添加跳转指令。  */
    tmp_insn = IRInstEmitInst(IRINST_OP_goto, call->line, call->column);
    IRInstSetOperand(tmp_insn, 0, stCreateIconNode(stab, (*(IRInst *) List_First((*(edge *) List_First(entry->succs))->dest->insns))->uid));
    InterCodeAddInst (entry, tmp_insn, TRUE);

    /* 更新跳转的目标指令。  */
    for(  curs=(basic_block *) List_First(orig_func->basic_block_info)
       ;  curs!=NULL
       ;  curs = (basic_block *) List_Next((void *)curs)
       )
    {
        if  (*curs != orig_func->entry_block_ptr)
        {
            block = lookup_block_by_id (entry->cfg, ((struct pair *) avl_find(block_map, &(*curs)->index))->y);
            update_destinations (block, (*(IRInst *)List_First((*curs)->insns))->uid, (*(IRInst *)List_First(block->insns))->uid, NULL);
        }
    }

    /* 处理param指令。  */
    count = GetConstVal(IRInstGetOperand(call, 2)->var, 0)->cvValue.cvIval;
    for(  insn1=(IRInst *) InterCodeGetCursor(entry->cfg->code, call)
       ;  insn1!=NULL && count > 0
       ;  insn1 = (IRInst *) List_Prev((void *)insn1), --count
       )
        ;
    for(  insn2 = (IRInst *) List_Next (List_First (orig_func->entry_block_ptr->insns))
       ;  (*insn1)->opcode != IRINST_OP_call
       ;  insn1 = next_insn, insn2 = (IRInst *) List_Next ((void *)insn2)
       )
    {
        next_insn = (IRInst *) List_Next((void *)insn1);
        if  (IRInstGetOperand (*insn1, 0)->var->sdType->tdTypeKind > TYP_lastIntrins)
        {
            /* 获取要修改的数组名称和偏移量。  */
            tmp_insn = *insn1;
            name = IRInstGetOperand (*insn1, 0);
            for(  insn1 = (IRInst *) List_Prev((void *)insn1)
               ;  insn1!=NULL 
               ;  insn1 = (IRInst *) List_Prev((void *)insn1)
               )
            {
                if  ((*insn1)->opcode == IRINST_OP_addptr &&
                     name->var == IRInstGetOperand (*insn1, 0)->var &&
                     name->version == IRInstGetOperand (*insn1, 0)->version)
                {
                    break;
                }
            }
            replace = IRInstGetOperand (*insn1, 1);
            name = IRInstGetOperand (*insn2, 0);
            offset = IRInstGetOperand (*insn1, 2);

            /* 删除param指令。  */
            InterCodeRemoveInst (entry->cfg->code, tmp_insn, NULL);
            IRInstDelInst (tmp_insn);
            tmp2_insn = *insn1;

            /* 对内联函数中所有的数组引用进行修改。  */
            for(  curs=(basic_block *) List_First(orig_func->basic_block_info)
               ;  curs!=NULL
               ;  curs = (basic_block *) List_Next((void *)curs)
               )
            {
                if  (*curs != orig_func->entry_block_ptr &&
                     *curs != orig_func->exit_block_ptr)
                {
                    block = lookup_block_by_id (entry->cfg, ((struct pair *) avl_find(block_map, &(*curs)->index))->y);
                    for(  insn1=(IRInst *) List_First(block->insns)
                       ;  insn1!=NULL
                       ;  insn1 = (IRInst *) List_Next((void *)insn1)
                       )
                    {
                        if  ((*insn1)->opcode == IRINST_OP_addptr &&
                             IRInstGetOperand (*insn1, 1)->var == name->var &&
                             IRInstGetOperand (*insn1, 1)->version == name->version)
                        {
                            tmp_insn = IRInstEmitInst (IRINST_OP_add, (*insn1)->line, (*insn1)->column);
                            tmp_var = stDeclareSym (stab, NULL, SYM_VAR);
                            tmp_var->sdIsImplicit = TRUE;
                            tmp_var->sdType = stAllocTypDef (TYP_INT);
                            IRInstSetOperand (tmp_insn, 0, tmp_var);
                            IRInstSetOperand (tmp_insn, 1, IRInstGetOperand (*insn1, 2)->var);
                            IRInstGetOperand (tmp_insn, 1)->version = IRInstGetOperand (*insn1, 2)->version;
                            IRInstSetOperand (tmp_insn, 2, offset->var);
                            IRInstGetOperand (tmp_insn, 2)->version = offset->version;
                            IRInstSetOperand (*insn1, 2, tmp_var);
                            IRInstGetOperand (*insn1, 2)->version = 0;
                            IRInstSetOperand (*insn1, 1, replace->var);
                            IRInstGetOperand (*insn1, 1)->version = replace->version;
                            IRInstSetOperand (*insn1, 3, replace->var);
                            IRInstGetOperand (*insn1, 3)->version = replace->version;
                            InterCodeInsertBefore (block, insn1, tmp_insn, TRUE, NULL);
                        }
                        else if ((*insn1)->opcode == IRINST_OP_aload &&
                                 IRInstGetOperand (*insn1, 1)->var == name->var &&
                                 IRInstGetOperand (*insn1, 1)->version == name->version)
                        {
                            tmp_insn = IRInstEmitInst (IRINST_OP_add, (*insn1)->line, (*insn1)->column);
                            tmp_var = stDeclareSym (stab, NULL, SYM_VAR);
                            tmp_var->sdIsImplicit = TRUE;
                            tmp_var->sdType = stAllocTypDef (TYP_INT);
                            IRInstSetOperand (tmp_insn, 0, tmp_var);
                            IRInstSetOperand (tmp_insn, 1, IRInstGetOperand (*insn1, 2)->var);
                            IRInstGetOperand (tmp_insn, 1)->version = IRInstGetOperand (*insn1, 2)->version;
                            IRInstSetOperand (tmp_insn, 2, offset->var);
                            IRInstGetOperand (tmp_insn, 2)->version = offset->version;
                            IRInstSetOperand (*insn1, 2, tmp_var);
                            IRInstGetOperand (*insn1, 2)->version = 0;
                            IRInstSetOperand (*insn1, 1, replace->var);
                            IRInstGetOperand (*insn1, 1)->version = replace->version;
                            InterCodeInsertBefore (block, insn1, tmp_insn, TRUE, NULL);
                        }
                        else if ((*insn1)->opcode == IRINST_OP_astore &&
                                 IRInstGetOperand (*insn1, 0)->var == name->var &&
                                 IRInstGetOperand (*insn1, 0)->version == name->version)
                        {
                            tmp_insn = IRInstEmitInst (IRINST_OP_add, (*insn1)->line, (*insn1)->column);
                            tmp_var = stDeclareSym (stab, NULL, SYM_VAR);
                            tmp_var->sdIsImplicit = TRUE;
                            tmp_var->sdType = stAllocTypDef (TYP_INT);
                            IRInstSetOperand (tmp_insn, 0, tmp_var);
                            IRInstSetOperand (tmp_insn, 1, IRInstGetOperand (*insn1, 1)->var);
                            IRInstGetOperand (tmp_insn, 1)->version = IRInstGetOperand (*insn1, 1)->version;
                            IRInstSetOperand (tmp_insn, 2, offset->var);
                            IRInstGetOperand (tmp_insn, 2)->version = offset->version;
                            IRInstSetOperand (*insn1, 1, tmp_var);
                            IRInstGetOperand (*insn1, 1)->version = 0;
                            IRInstSetOperand (*insn1, 0, replace->var);
                            IRInstGetOperand (*insn1, 0)->version = replace->version;
                            IRInstSetOperand (*insn1, 3, replace->var);
                            IRInstGetOperand (*insn1, 3)->version = replace->version;
                            InterCodeInsertBefore (block, insn1, tmp_insn, TRUE, NULL);
                        }
                    }
                }
            }

            /* 删除param指令对应的addptr指令。  */
            InterCodeRemoveInst (entry->cfg->code, tmp2_insn, NULL);
            IRInstDelInst (tmp2_insn);
        }
        else
        {
            /* 将param指令转换为赋值。  */
            (*insn1)->opcode = IRINST_OP_store;
            IRInstSetOperand (*insn1, 1, IRInstGetOperand (*insn1, 0)->var);
            IRInstGetOperand (*insn1, 1)->version = IRInstGetOperand (*insn1, 0)->version;
            IRInstSetOperand (*insn1, 0, IRInstGetOperand (*insn2, 0)->var);
            IRInstGetOperand (*insn1, 0)->version = IRInstGetOperand (*insn2, 0)->version;
        }
    }

    /* 复制返回值。  */
    if  (IRInstGetOperand (call, 0)->var->sdType->tdTypeKind != TYP_VOID)
    {
        tmp_insn = IRInstEmitInst (IRINST_OP_move, call->line, call->column);
        IRInstSetOperand (tmp_insn, 0, IRInstGetOperand (call, 0)->var);
        IRInstGetOperand (tmp_insn, 0)->version = IRInstGetOperand (call, 0)->version;
        IRInstSetOperand (tmp_insn, 1, IRInstGetOperand(*(IRInst *)List_Last (orig_func->exit_block_ptr->insns), 0)->var);
        IRInstGetOperand (tmp_insn, 1)->version = IRInstGetOperand(*(IRInst *)List_Last (orig_func->exit_block_ptr->insns), 0)->version;
        InterCodeInsertAfter (exit, NULL, tmp_insn, TRUE, NULL);
    }

    /* 更新函数调用图。  */
    retval = DeleteArc (G, InsertVex (G, caller->func), InsertVex (G, callee->func), call);
    for(  curs=(basic_block *) List_First(orig_func->basic_block_info)
       ;  curs!=NULL
       ;  curs = (basic_block *) List_Next((void *)curs)
       )
    {
        if  (*curs != orig_func->entry_block_ptr &&
             *curs != orig_func->exit_block_ptr)
        {
            block = lookup_block_by_id (entry->cfg, ((struct pair *) avl_find(block_map, &(*curs)->index))->y);
            for(  insn1=(IRInst *) List_First(block->insns)
               ;  insn1!=NULL
               ;  insn1 = (IRInst *) List_Next((void *)insn1)
               )
            {
                if  ((*insn1)->opcode == IRINST_OP_call)
                {
                    InsertArc (G, InsertVex(G, IRInstGetOperand (*(IRInst *)List_First(orig_func->entry_block_ptr->insns), 0)->var), InsertVex (G, IRInstGetOperand(*insn1, 1)->var), *insn1);
                }
            }
        }
    }

    /* 删除函数调用指令。  */
    InterCodeRemoveInst(entry->cfg->code, call, NULL);
    IRInstDelInst(call);
 
    avl_destroy (block_map, (avl_item_func *) free);
    free_cfg (orig_func->code, orig_func);

    return retval;
}

static int
number_instructions (InterCode code, SymDef node)
{
    basic_block *curs;
    int count = 0;
    control_flow_graph *F;

    for(  F=(control_flow_graph *)List_First(code->funcs)
       ;  F!=NULL
       ;  F = (control_flow_graph *)List_Next((void *)F)
       )
        if  (IRInstGetOperand (*(IRInst *)List_First ((*F)->entry_block_ptr->insns), 0)->var == node)
            break;

    for(  curs=(basic_block *) List_First((*F)->basic_block_info)
       ;  curs!=NULL
       ;  curs = (basic_block *) List_Next((void *)curs)
       )
        count += List_Card ((*curs)->insns);
    return count;
}

void
inline_transform (InterCode code, SymTab stab)
{
    OLGraph g;
    int i;
    IRInst instr;
    ArcBox *p;
    control_flow_graph *curs;
    control_flow_graph *next;

    Build (&g, code);

    for (i = 0; i < g.vexnum; i++)
    {
        /* 函数没有实现。  */
        if  (! GetVex (&g, i)->func->sdIsDefined)
            continue;

#if 0
        /* 递归函数不可内联。  */
        if  (DFSTraverse (&g, i))
            continue;
#endif

/*      printf("%s: ", stGetSymName (GetVex (&g, i)->func)); */
        p=GetVex (&g, i)->firstin;

        while(p)
        {
/*          printf("%s ",stGetSymName (GetVex (&g, p->tailvex)->func)); */
            if  (GetVex (&g, i)->func == GetVex (&g, p->tailvex)->func &&
                 (GetVex (&g, i)->count >= max_expanded ||
                 number_instructions (code, GetVex (&g, i)->func) >= THRESHOLD))
                break;

            while(p)
            {
                if  (GetVex (&g, i)->func == GetVex (&g, p->tailvex)->func &&
                     (GetVex (&g, i)->count >= max_expanded ||
                     number_instructions (code, GetVex (&g, i)->func) >= THRESHOLD))
                    break;

                if  (GetVex (&g, i)->func == GetVex (&g, p->tailvex)->func)
                    GetVex (&g, p->tailvex)->count++;

                instr = InterCodeGetInstByID (code, bitmap_first_set_bit (p->insns));
                p = inline_call (&g, instr, GetVex (&g, p->tailvex), GetVex (&g, i), stab);

            }
            p=GetVex (&g, i)->firstin;
        }
/*      printf("\n"); */
    }

    /* 删除未使用的函数。  */
    for(  curs=(control_flow_graph *) List_First(code->funcs)
       ;  curs!=NULL
       ;  curs = next
       )
    {
        next = (control_flow_graph *) List_Next ((void *)curs);
        if  (! GetVex (&g, InsertVex (&g, IRInstGetOperand (*(IRInst *)List_First((*curs)->entry_block_ptr->insns), 0)->var))->firstin &&
             ! IRInstGetOperand (*(IRInst *)List_First((*curs)->entry_block_ptr->insns), 0)->var->sdFnc.sdfEntryPt)
            free_cfg (code, *curs);
    }

    DestroyGraph (&g);
}

static BOOL
pure_function_internal (InterCode code, SymDef function, bitmap visited)
{
    control_flow_graph *F;
    IRInst *instr;
    basic_block* block ;
    int i;
    ssa_name name;

    bitmap_set_bit (visited, function->uid);

    if (!strcmp (stGetSymName (function), "memset"))
        return TRUE;

    for(  F=(control_flow_graph *)List_First(code->funcs)
       ;  F!=NULL
       ;  F = (control_flow_graph *)List_Next((void *)F)
       )
        if  (IRInstGetOperand(*(IRInst *)List_First ((*F)->entry_block_ptr->insns), 0)->var == function)
            break;

    if  (F==NULL)
        /* Implicit functions are not pure.  */
        return FALSE;
    
    for(  block=(basic_block *) List_First((*F)->basic_block_info)
       ;  block!=NULL
       ;  block = (basic_block *) List_Next((void *)block)
       )
    {
        for(  instr=(IRInst *) List_First((*block)->insns)
           ;  instr!=NULL
           ;  instr = (IRInst *) List_Next((void *)instr)
           )
        {
            if  ((*instr)->opcode == IRINST_OP_call &&
                 !bitmap_bit_p (visited, IRInstGetOperand (*instr, 1)->var->uid))
            {
                if  (!pure_function_internal (code, IRInstGetOperand (*instr, 1)->var, visited))
                    return FALSE;
            }
            for (i = 0; i < IRInstGetNumOperands (*instr); i++)
            {
                name = IRInstGetOperand (*instr, i);
                if (is_global_var(name->var) &&
                    !name->var->sdIsImplicit &&
                    name->var->sdSymKind == SYM_VAR &&
                    !name->var->sdVar.sdvConst)
                    return FALSE;
                if (!name->var->sdIsImplicit &&
                    name->var->sdSymKind == SYM_VAR &&
                    name->var->sdVar.sdvArgument &&
                    name->var->sdType->tdTypeKind > TYP_lastIntrins)
                    return FALSE;
            }
        }
    }

    return TRUE;
}

/* 判断函数引用是否为纯函数。如果函数是纯函数则返回非零，否则返回零。  */
BOOL
pure_function (InterCode code, SymDef function)
{
    bitmap visited = BITMAP_XMALLOC ();
    BOOL pure = pure_function_internal (code, function, visited);
    BITMAP_XFREE (visited);
    return pure;
}
