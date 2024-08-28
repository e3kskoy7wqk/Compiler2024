/* Graph coloring register allocator.  */

#include <stdlib.h>
#include <string.h>
#include "all.h"

/* # define SIMPLE  */

/* Classification of a given node (i.e. what state it's in).  */
enum ra_node_type
{
    INITIAL = 0,
    SIMPLIFY,
# if !defined(SIMPLE)
    FREEZE,
# endif /* SIMPLE */
    SPILL,
    SELECT,
# if !defined(SIMPLE)
    COALESCED,
# endif /* SIMPLE */
    LAST_NODE_TYPE
};

typedef struct
{
    bitmap firstin,firstout; /* 分别指向该顶点第一条入弧和出弧 */
    int v;
    struct Interval* interval;
    int hard_num;
    int spill_slot;
    int num_conflicts;
    int weight;

# if !defined(SIMPLE)
    bitmap moves;
    int alias;
# endif /* SIMPLE */
}IfNode; /* 顶点结点 */
typedef struct
{
    IfNode *xlist; /* 表头向量(数组) */
    int allocated;
    int vexnum,arcnum; /* 有向图的当前顶点数和弧数 */

    struct avl_table *map;
    struct avl_table *insn_map;

    bitmap web_lists[(int) LAST_NODE_TYPE];

    LIST stack;

    bitmap spilledRegs;

# if !defined(SIMPLE)
    bitmap mv_worklist;
    bitmap mv_active;
    bitmap mv_coalesced;
    bitmap mv_constrained;
# endif /* SIMPLE */
}PhaseIFG;

struct pair
{
    int x;
    int y;
};

struct reg_class_desc
{
    unsigned next_avail, max_num;
    unsigned used_num, max_used;
    bitmap available;
    char cl_char;
};

struct Range
{
    int _from; /* from (inclusive) */
    int _to; /* to (exclusive) */
    struct Range* _next; /* linear list of Ranges */
    struct Interval* interval;
};

struct Interval
{
    /* Unique id of the node.  */
    int uid;

    /* sorted list of Ranges */
    struct Range* _first;
    int vreg;
    int weight;
};


static struct Interval* create_interval(int vreg)
{
    struct Interval* interval = (struct Interval *) xmalloc (sizeof (struct Interval));
    static int counter = 0;
    memset (interval, '\0', sizeof (*interval));
    interval->vreg = vreg;
    interval->uid = counter++;
    return interval;
}

static void delete_interval(struct Interval* interval)
{
    struct Range * p,*r;

    if  (interval)
    {
        p=interval->_first;
        while(p)
        {
            r=p;
            p=p->_next;
            free(r);
        }

        free(interval);
    }
}

static void
compute_global_live_sets (LIST blocks)
{
    BOOL change_occurred;
    basic_block* block;
    ArmInst instr;
    bitmap work = BITMAP_XMALLOC ();
    bitmap temp = BITMAP_XMALLOC ();

    for(  block=(basic_block *)List_Last(blocks)
       ;  block!=NULL
       ;  block = (basic_block *)List_Prev((void *)block)
       )
    {
        bitmap_clear (((BblockArm32)(*block)->param)->livein);
        bitmap_clear (((BblockArm32)(*block)->param)->livein);
    }

    do
      {
        change_occurred = FALSE;
        for(  block=(basic_block *)List_Last(blocks)
           ;  block!=NULL
           ;  block = (basic_block *)List_Prev((void *)block)
           )
        {
            edge *ei;
            /* 后继的livein的并集（如果没有，则为空）。  */
            BOOL first = TRUE;
            for(  ei=(edge *)List_First((*block)->succs)
               ;  ei!=NULL
               ;  ei = (edge *)List_Next((void *)ei)
               )
            {
                basic_block succ =  ((*ei)->dest);
                if (first)
                {
                    bitmap_copy (work, ((BblockArm32)succ->param)->livein);
                    first = FALSE;
                }
                else
                    bitmap_ior_into (work, ((BblockArm32)succ->param)->livein);
            }
            if (first)
                bitmap_clear (work);
  
            bitmap_copy (((BblockArm32)(*block)->param)->liveout, work);

            /* 在反向指令遍历中删除定义，包括使用。  */
            for(  instr=(ArmInst)List_Last(((BblockArm32)(*block)->param)->code)
               ;  instr!=NULL
               ;  instr = (ArmInst)List_Prev((void *)instr)
               )
            {
                unsigned regno;
                bitmap_iterator bi;
                ArmInstOutput (instr, temp);
                for (bmp_iter_set_init (&bi, temp, 0, &regno);
                     bmp_iter_set (&bi, &regno);
                     bmp_iter_next (&bi, &regno))
                    bitmap_clear_bit (work, regno);
                ArmInstInput (instr, temp);
                for (bmp_iter_set_init (&bi, temp, 0, &regno);
                     bmp_iter_set (&bi, &regno);
                     bmp_iter_next (&bi, &regno))
                    bitmap_set_bit (work, regno);
            }

            /* 记录这是否改变了什么。  */
            if (bitmap_ior_into (((BblockArm32)(*block)->param)->livein, work))
                change_occurred = TRUE;
        }
      }
    while (change_occurred);

    BITMAP_XFREE (work);
    BITMAP_XFREE (temp);
}

static int compare_pairs( struct pair *arg1, struct pair *arg2 )
{
    int ret = 0 ;

    if ( arg1->x < arg2->x )
        ret = -1 ;
    else if ( arg1->x > arg2->x )
        ret = 1 ;

    return( ret );
}

static int
compare_insns (ArmInst first, ArmInst second)
{
    int ret = 0 ;

    if ( first->uid < second->uid )
        ret = -1 ;
    else if ( first->uid > second->uid )
        ret = 1 ;

    return( ret );
}

static void add_range(struct Interval* it, int from, int to)
{
    if (it->_first && it->_first->_from <= to)
    {
        /* 连接相交范围  */
        it->_first->_from = min(from, it->_first->_from);
        it->_first->_to  = max(to,   it->_first->_to);
    }
    else
    {
        /* 插入新范围  */
        struct Range* next = it->_first;
        it->_first = (struct Range *) xmalloc (sizeof (struct Range));
        it->_first->_from = from;
        it->_first->_to = to;
        it->_first->_next = next;
        it->_first->interval = it;
    }
}

static void add_def(struct Interval* interval, int def_pos)
{
    if (interval != NULL)
    {
        struct Range* r = interval->_first;
        if (r != NULL && r->_from <= def_pos)
        {
            /* 更新起始点(当一个范围第一次被创建用于某个用途时，它的起始点是
               当前块的开始，直到遇到一个def)。  */
            r->_from = def_pos;
        }
        else
        {
            /* 死值  */
            add_range(interval, def_pos, def_pos + 1);
        }

    }
}

static void add_use(struct Interval* interval, int from, int to)
{
    add_range(interval, from, to);
}

static int intersects_at(const struct Range* r1, const struct Range* r2)
{
    if (!r1)      return -1;
    if (!r2)      return -1;
    do {
        if (r1->_from < r2->_from) {
            if (r1->_to <= r2->_from) {
                r1 = r1->_next;
                if (r1 == NULL)
                    return -1;
            } else {
                return r2->_from;
            }
        } else if (r2->_from < r1->_from) {
            if (r2->_to <= r1->_from) {
                r2 = r2->_next;
                if (r2 == NULL)
                    return -1;
            } else {
                return r1->_from;
            }
        } else { // r1->from() == r2->from()
            if (r1->_from == r1->_to) {
                r1 = r1->_next;
                if (r1 == NULL)
                    return -1;
            } else if (r2->_from == r2->_to) {
                r2 = r2->_next;
                if (r2 == NULL)
                    return -1;
            } else {
                return r1->_from;
            }
        }
    } while (TRUE);
}

static BOOL intersects(const struct Range* r1, const struct Range* r2)
{
    return intersects_at(r1, r2) != -1; 
}

static int InsertVex(PhaseIFG *G, int v)
{  /* 初始条件: 有向图G存在,v和有向图G中顶点有相同特征 */
   /* 操作结果: 在有向图G中增添新顶点v(不增添与顶点相关的弧,留待InsertArc()去做) */
    struct pair *p;

    p = avl_find (G->map, &v);
    if  (p)
        return p->y;

    if ((*G). vexnum >= G->allocated) 
    {
        int new_size = (*G). vexnum * 2 + 1;
        (*G). xlist = (IfNode *) xrealloc ((*G). xlist, new_size * sizeof (IfNode));
        G->allocated = new_size;
    }

    memset ((*G). xlist + (*G). vexnum, 0, sizeof (IfNode));
    (*G).xlist[(*G). vexnum].firstin = BITMAP_XMALLOC ();
    (*G).xlist[(*G). vexnum].firstout = BITMAP_XMALLOC ();
    (*G).xlist[(*G). vexnum].v = v;
    (*G).xlist[(*G). vexnum].interval = create_interval (v);
    (*G).xlist[(*G). vexnum].hard_num = v < FIRST_VIRTUAL_REGISTER ? v : -1;
    (*G).xlist[(*G). vexnum].spill_slot = -1;

# if !defined(SIMPLE)
    (*G).xlist[(*G). vexnum].moves = BITMAP_XMALLOC ();
    (*G).xlist[(*G). vexnum].alias = (*G). vexnum;
# endif /* SIMPLE */

    p = (struct pair *) xmalloc (sizeof (struct pair));
    p->x = v;
    p->y = (*G). vexnum;
    avl_insert (G->map, p);
    (*G). vexnum++;

    return p->y;
}

static void DeleteVex(PhaseIFG *G,int k)
{  /* 初始条件: 有向图G存在,v是G中某个顶点 */
   /* 操作结果: 删除G中顶点v及其相关的弧 */
    /* 以下删除顶点v的出弧 */
    bitmap_clear ((*G). xlist[k].firstout);
    bitmap_clear ((*G). xlist[k].firstin);
    delete_interval ((*G).xlist[k].interval);
    (*G).xlist[k].interval = create_interval ((*G). xlist[k].v);

# if !defined(SIMPLE)
    bitmap_clear ((*G). xlist[k].moves);
# endif /* SIMPLE */
}

static void InsertArc(PhaseIFG *G,int i,int j)
{ /* 初始条件: 有向图G存在,v和w是G中两个顶点 */
   /* 操作结果: 在G中增添弧<v,w> */
    if  (!bitmap_bit_p ((*G). xlist[j].firstin, i) &&
         i != j)
    {
        bitmap_set_bit((*G). xlist[j].firstin, i);
        bitmap_set_bit((*G). xlist[i].firstout, j);
        bitmap_set_bit((*G). xlist[j].firstout, i);
        bitmap_set_bit((*G). xlist[i].firstin, j);
        (*G). arcnum++; /* 弧数加1 */
        (*G). xlist[i].num_conflicts++;
        (*G). xlist[j].num_conflicts++;
    }
}

static void CreateDG(PhaseIFG *G)
{ /* 采用十字链表存储表示,构造有向图G。算法7.3 */
    int i;
    
    memset (G, 0, sizeof (*G));
    G->map = avl_create ((avl_comparison_func *) compare_pairs, NULL, NULL);
    G->insn_map = avl_create ((avl_comparison_func *) compare_insns, NULL, NULL);
    G->stack = List_Create ();
    G->spilledRegs = BITMAP_XMALLOC ();

    for (i = 0; i < LAST_NODE_TYPE; i++)
        G->web_lists[i] = BITMAP_XMALLOC ();

# if !defined(SIMPLE)
    G->mv_active = BITMAP_XMALLOC ();
    G->mv_worklist = BITMAP_XMALLOC ();
    G->mv_coalesced = BITMAP_XMALLOC ();
    G->mv_constrained = BITMAP_XMALLOC ();
# endif /* SIMPLE */
}

void DestroyGraph(PhaseIFG *G)
{   /* 初始条件: 有向图G存在 */
    /* 操作结果: 销毁有向图G */
    int j;

    for(j=0;j<(*G).vexnum;j++) /* 对所有顶点 */
    {
        BITMAP_XFREE ((*G).xlist[j].firstin);
        BITMAP_XFREE ((*G).xlist[j].firstout);
        delete_interval ((*G).xlist[j].interval);

# if !defined(SIMPLE)
        BITMAP_XFREE ((*G).xlist[j].moves);
# endif /* SIMPLE */
    }
    free (G->xlist);
    avl_destroy (G->map, (avl_item_func *) free);
    avl_destroy (G->insn_map, NULL);
    List_Destroy (&G->stack);
    BITMAP_XFREE (G->spilledRegs);

    for (j = 0; j < LAST_NODE_TYPE; j++)
        BITMAP_XFREE (G->web_lists[j]);

# if !defined(SIMPLE)
    BITMAP_XFREE (G->mv_active);
    BITMAP_XFREE (G->mv_worklist);
    BITMAP_XFREE (G->mv_coalesced);
    BITMAP_XFREE (G->mv_constrained);
# endif /* SIMPLE */

    memset (G, 0, sizeof (*G));
}

static IfNode* GetVex(PhaseIFG *G,int v)
{   /* 初始条件:有向图G存在,v是G中某个顶点的序号。操作结果:返回v的值 */
    return G->xlist + v;
}

static void
d_dump (PhaseIFG *g, bitmap bmp)
{
    const char *comma = "";
    unsigned i;
    bitmap_iterator bi;

    for (bmp_iter_set_init (&bi, bmp, 0, &i);
         bmp_iter_set (&bi, &i);
         bmp_iter_next (&bi, &i))
    {
        fprintf (stdout, "%s%d", comma, GetVex (g, i)->v);
        comma = ", ";
    }
    fputs ("\n", stdout);
}

static void Display(PhaseIFG *G)
{ /* 输出有向图G */
    int k;
    printf("共%d个顶点,%d条弧:\n",G->vexnum,G->arcnum);
    for(k=0;k<G->vexnum;k++)
    {
        printf("顶点%d(%d): 入度: ",k, G->xlist[k].v);
        d_dump (G, (*G). xlist[k].firstin);
    }
}

static ArmInst
handle_method_arguments (ArmInst curs)
{
    ArmInst first = NULL;
    for(  
       ;  curs!=NULL
       ;  curs = (ArmInst) List_Prev((void *)curs)
       )
    {
        first = curs;
        if  (curs->opcode == ARMINST_OP_STRING &&
             !strcmp (dyn_string_buf (ArmInstGetOperand (curs, 0)->ds), "\t@ Prepare arguments for call."))
            return curs;
    }
    return first;
}

/* 构建(或重建)带有冲突的完整干涉图。  */
static void
build_i_graph (PhaseIFG *G, LIST blocks)
{
    basic_block* block;
    ArmInst instr;
    bitmap liveout;
    bitmap def;
    bitmap use;
    bitmap_iterator bi, bi2;
    bitmap_head temp_bitmap;
    unsigned i, k;
    int v, w;
    ArmInst curs;

    def = BITMAP_XMALLOC ();
    use = BITMAP_XMALLOC ();
    liveout = BITMAP_XMALLOC ();
    for(  block=(basic_block*) List_Last(blocks)
       ;  block!=NULL
       ;  block = (basic_block*) List_Prev((void *)block)
       )
    {
        bitmap_copy (liveout, ((BblockArm32)(*block)->param)->liveout);
        for(  instr=(ArmInst) List_Last(((BblockArm32)(*block)->param)->code)
           ;  instr!=NULL
           ;  instr = (ArmInst) List_Prev((void *)instr)
           )
        {
            if  (ArmInst_is_call (instr))
            {
                /* 保护调用者保存的寄存器。  */
                for(  curs=(ArmInst) handle_method_arguments (instr)
                   ;  curs!=instr
                   ;  curs = (ArmInst) List_Next((void *)curs)
                   )
                {
                    ArmInstOutput (curs, def);
                    for (i = 0; i < (unsigned) num_caller_save_registers; i++)
                    {
                        v = InsertVex (G, caller_save_registers[i]);
                        for (bmp_iter_set_init (&bi2, liveout, 0, &k);
                             bmp_iter_set (&bi2, &k);
                             bmp_iter_next (&bi2, &k))
                        {
                            w = InsertVex (G, k);
                            if (w != v)
                                InsertArc (G, w, v);
                        }
                        for (bmp_iter_set_init (&bi2, def, 0, &k);
                             bmp_iter_set (&bi2, &k);
                             bmp_iter_next (&bi2, &k))
                        {
                            w = InsertVex (G, k);
                            if (w != v)
                                InsertArc (G, w, v);
                        }
                    }
                }
            }

            ArmInstOutput (instr, def);
            ArmInstInput (instr, use);
            bitmap_ior_into (liveout, def);

# if !defined(SIMPLE)
            if  (ArmInst_is_move (instr))
            {
                bitmap_and_compl_into (liveout, use);
                for (bmp_iter_set_init (&bi, def, 0, &i);
                     bmp_iter_set (&bi, &i);
                     bmp_iter_next (&bi, &i))
                {
                    v = InsertVex (G, i);
                    bitmap_set_bit (GetVex (G, v)->moves, instr->uid);
                }
                for (bmp_iter_set_init (&bi, use, 0, &i);
                     bmp_iter_set (&bi, &i);
                     bmp_iter_next (&bi, &i))
                {
                    v = InsertVex (G, i);
                    bitmap_set_bit (GetVex (G, v)->moves, instr->uid);
                }
                bitmap_set_bit (G->mv_worklist, instr->uid);
            }
# endif /* SIMPLE */

            for (bmp_iter_set_init (&bi, def, 0, &i);
                 bmp_iter_set (&bi, &i);
                 bmp_iter_next (&bi, &i))
            {
                v = InsertVex(G, i);
                for (bmp_iter_set_init (&bi2, liveout, 0, &k);
                     bmp_iter_set (&bi2, &k);
                     bmp_iter_next (&bi2, &k))
                {
                    w = InsertVex (G, k);
                    if (w != v)
                        InsertArc (G, w, v);
                }
            }
            temp_bitmap.first = temp_bitmap.current = 0;
            bitmap_ior_and_compl (&temp_bitmap, use, liveout, def);
            bitmap_copy (liveout, &temp_bitmap);
            bitmap_clear (&temp_bitmap);
        }
    }

    BITMAP_XFREE (def);
    BITMAP_XFREE (use);
    BITMAP_XFREE (liveout);
}

static void
number_instructions (LIST blocks, PhaseIFG *g)
{
    int op_id = 0;
    basic_block* block;
    ArmInst op;

    avl_destroy (g->insn_map, (avl_item_func *) NULL);
    g->insn_map = avl_create ((avl_comparison_func *) compare_insns, NULL, NULL);
    for(  block=(basic_block *)List_First(blocks)
       ;  block!=NULL
       ;  block = (basic_block *)List_Next((void *)block)
       )
    {
        for(  op=(ArmInst)List_First(((BblockArm32)(*block)->param)->code)
           ;  op!=NULL
           ;  op = (ArmInst)List_Next((void *)op)
           )
        {
            op->uid = op_id++;
            avl_insert (g->insn_map, op);
        }
    }
}

static int calc_to (struct Interval* cur)
{
    struct Range* r = cur->_first;
    while (r->_next != NULL)
    {
        r = r->_next;
    }
    return r->_to;
}

static void
build_intervals (PhaseIFG *g, LIST blocks)
{
    basic_block* block;
    ArmInst op;
    unsigned bit;
    bitmap_iterator bi;
    int u;
    int start;
    int k;
    int block_from;
    int block_to;
    bitmap temp = BITMAP_XMALLOC ();

    for(  block=(basic_block *)List_Last(blocks)
       ;  block!=NULL
       ;  block = (basic_block *)List_Prev((void *)block)
       )
    {
        block_to = ((ArmInst)List_First (((BblockArm32)(*block)->param)->code))->uid;
        block_from = ((ArmInst)List_Last (((BblockArm32)(*block)->param)->code))->uid;
        for (bmp_iter_set_init (&bi, ((BblockArm32)(*block)->param)->liveout, 0, &bit);
             bmp_iter_set (&bi, &bit);
             bmp_iter_next (&bi, &bit))
        {
            u = InsertVex (g, bit);
            add_use (GetVex(g, u)->interval, block_from, block_to + 1);
        }

        for(  op=(ArmInst)List_Last(((BblockArm32)(*block)->param)->code)
           ;  op!=NULL
           ;  op = (ArmInst)List_Prev((void *)op)
           )
        {
            /* 如果指令销毁调用者保存的寄存器，则为每个寄存器添加一个临时范围。  */
            if (ArmInst_is_call (op))
            {
                start = handle_method_arguments (op)->uid;
                for (k = 0; k < num_caller_save_registers; k++)
                {
                    u = InsertVex (g, caller_save_registers[k]);
                    add_use (GetVex(g, u)->interval, start, op->uid + 1);
                }
            }

            ArmInstOutput (op, temp);
            for (bmp_iter_set_init (&bi, temp, 0, &bit);
                 bmp_iter_set (&bi, &bit);
                 bmp_iter_next (&bi, &bit))
            {
                u = InsertVex (g, bit);
                add_def (GetVex(g, u)->interval, op->uid);
            }
            ArmInstInput (op, temp);
            for (bmp_iter_set_init (&bi, temp, 0, &bit);
                 bmp_iter_set (&bi, &bit);
                 bmp_iter_next (&bi, &bit))
            {
                u = InsertVex (g, bit);
                add_use (GetVex (g, u)->interval, block_from, op->uid);
            }
        }
    }

    BITMAP_XFREE (temp);
}

static void
adjacent (PhaseIFG *g, int v, bitmap bmp)
{
    bitmap_copy (bmp, GetVex (g, v)->firstin);
    bitmap_and_compl_into (bmp, g->web_lists[SELECT]);
# if !defined(SIMPLE)
    bitmap_and_compl_into (bmp, g->web_lists[COALESCED]);
# endif /* SIMPLE */
}

# if !defined(SIMPLE)
static void
node_moves (PhaseIFG *g, int v, bitmap bmp)
{
    bitmap_ior (bmp, g->mv_active, g->mv_worklist);
    bitmap_and_into (bmp, GetVex (g, v)->moves);
}

static BOOL
move_related (PhaseIFG *g, int v)
{
    bitmap bmp;
    BOOL retval;

    bmp = BITMAP_XMALLOC ();
    node_moves (g, v, bmp);
    retval = !bitmap_empty_p (bmp);
    BITMAP_XFREE (bmp);
    return retval;
}

static void
enable_moves (PhaseIFG *g, bitmap bmp)
{
    bitmap_iterator bi1, bi2;
    unsigned i1, i2;
    bitmap temp_bitmap;

    temp_bitmap = BITMAP_XMALLOC ();
    for (bmp_iter_set_init (&bi1, bmp, 0, &i1);
         bmp_iter_set (&bi1, &i1);
         bmp_iter_next (&bi1, &i1))
    {
        node_moves (g, i1, temp_bitmap);
        for (bmp_iter_set_init (&bi2, temp_bitmap, 0, &i2);
             bmp_iter_set (&bi2, &i2);
             bmp_iter_next (&bi2, &i2))
        {
            if  (bitmap_bit_p (g->mv_active, i2))
            {
                bitmap_clear_bit (g->mv_active, i2);
                bitmap_set_bit (g->mv_worklist, i2);
            }
        }
    }
    BITMAP_XFREE (temp_bitmap);
}
# endif /* SIMPLE */

/* 将节点的度减1。  */
static void
decrement_degree (PhaseIFG *g, int v, struct reg_class_desc *classes, struct avl_table *virtual_regs)
{
    int cl = gen_vregArm32(virtual_regs, GetVex (g, v)->v, ALL_REGS)->rclass;
    int before = GetVex (g, v)->num_conflicts;
    GetVex (g, v)->num_conflicts -= 1;
    if  (bitmap_count_bits (classes[cl].available) == before)
    {
# if !defined(SIMPLE)
        bitmap temp_bitmap;
        temp_bitmap = BITMAP_XMALLOC ();
        adjacent (g, v, temp_bitmap);
        bitmap_set_bit (temp_bitmap, v);
        enable_moves (g, temp_bitmap);
        BITMAP_XFREE (temp_bitmap);
# endif /* SIMPLE */
        bitmap_clear_bit (g->web_lists[SPILL], v);
# if !defined(SIMPLE)
        if  (move_related (g, v))
            bitmap_set_bit (g->web_lists[FREEZE], v);
        else if (GetVex (g, v)->v >= FIRST_VIRTUAL_REGISTER)
# endif /* SIMPLE */
        {
            bitmap_set_bit (g->web_lists[SIMPLIFY], v);
        }
    }
}

/* 简化简化工作列表上的节点。  */
static void
simplify(PhaseIFG *g, struct reg_class_desc *classes, struct avl_table *virtual_regs)
{
    int f;
    bitmap temp_bitmap;
    unsigned i;
    bitmap_iterator bi;

    f = bitmap_first_set_bit (g->web_lists[SIMPLIFY]);
    bitmap_clear_bit (g->web_lists[SIMPLIFY], f);
    *(int *) List_NewFirst (g->stack, sizeof (int))= f;
    bitmap_set_bit (g->web_lists[SELECT], f);

    temp_bitmap = BITMAP_XMALLOC ();
    adjacent (g, f, temp_bitmap);

    for (bmp_iter_set_init (&bi, temp_bitmap, 0, &i);
         bmp_iter_set (&bi, &i);
         bmp_iter_next (&bi, &i))
        decrement_degree (g, i, classes, virtual_regs);

    BITMAP_XFREE (temp_bitmap);
}

/* 如果结点被合并了，返回它的别名。否则，返回传入的内容。  */
static int
alias (PhaseIFG *g, int v)
{
    int retval;
# if !defined(SIMPLE)
    if  (bitmap_bit_p (g->web_lists[COALESCED], v))
        retval = alias (g, GetVex (g, v)->alias);
    else
# endif /* SIMPLE */
    {
        retval = v;
    }
    return retval;
}

# if !defined(SIMPLE)
/* 从冻结工作列表中添加一个结点到简化工作列表。  */
static void
add_worklist (PhaseIFG *G, int v, struct reg_class_desc *classes, struct avl_table *virtual_regs)
{
    int cl;
    cl =gen_vregArm32 (virtual_regs, GetVex (G, v)->v, ALL_REGS)->rclass;
    if  (GetVex (G, v)->v >= FIRST_VIRTUAL_REGISTER &&
         !move_related (G, v) &&
         GetVex (G, v)->num_conflicts < bitmap_count_bits (classes[cl].available))
    {
        bitmap_clear_bit (G->web_lists[FREEZE], v);
        bitmap_set_bit (G->web_lists[SIMPLIFY], v);
    }
}

/* 预着色节点合并启发式算法。  */
static BOOL
ok (PhaseIFG *G, int target, int source,struct reg_class_desc *classes, struct avl_table *virtual_regs)
{
    int cl;

    cl =gen_vregArm32 (virtual_regs, GetVex (G, target)->v, ALL_REGS)->rclass;
    if  (GetVex (G, target)->num_conflicts < bitmap_count_bits (classes[cl].available) ||
         GetVex (G, target)->v < FIRST_VIRTUAL_REGISTER ||
         bitmap_bit_p (GetVex (G, target)->firstout, source))
        return TRUE;
    return FALSE;
}

/* 预着色节点合并启发式算法。  */
static BOOL
gate (PhaseIFG *G, int v,int w,struct reg_class_desc *classes, struct avl_table *virtual_regs)
{
    bitmap temp_bitmap;
    unsigned i;
    bitmap_iterator bi;
    BOOL bResult = TRUE;

    temp_bitmap = BITMAP_XMALLOC ();
    adjacent (G, v, temp_bitmap);
    for (bmp_iter_set_init (&bi, temp_bitmap, 0, &i);
         bmp_iter_set (&bi, &i);
         bmp_iter_next (&bi, &i))
    {
        if  (!ok (G, i, w, classes, virtual_regs))
        {
            bResult = FALSE;
            break;
        }
    }
    BITMAP_XFREE (temp_bitmap);

    return bResult;
}

/* 非预着色节点合并启发式算法。  */
static BOOL
conservative (PhaseIFG *G, int v,int w,struct reg_class_desc *classes, struct avl_table *virtual_regs)
{
    bitmap temp, temp2;
    int count;
    unsigned i;
    bitmap_iterator bi;
    int cl;
    int avail;

    temp = BITMAP_XMALLOC ();
    temp2 = BITMAP_XMALLOC ();

    adjacent (G, v, temp);
    adjacent (G, w, temp2);
    bitmap_ior_into (temp, temp2);
    BITMAP_XFREE (temp2);

    cl =gen_vregArm32 (virtual_regs, GetVex (G, v)->v, ALL_REGS)->rclass;
    avail = bitmap_count_bits (classes[cl].available);

    for (bmp_iter_set_init (&bi, temp, 0, &i), count = 0;
         bmp_iter_set (&bi, &i);
         bmp_iter_next (&bi, &i))
        if  (GetVex (G, i)->num_conflicts >= avail)
            count = count + 1;

    BITMAP_XFREE (temp);

    return count < avail;
}

/* 实际上是合并两个可以合并的节点。  */
static void
combine (PhaseIFG *G, int v,int w,struct reg_class_desc *classes, struct avl_table *virtual_regs)
{
    bitmap temp_bitmap;
    unsigned i;
    bitmap_iterator bi;
    int cl;

    if  (bitmap_bit_p (G->web_lists[FREEZE], w))
        bitmap_clear_bit (G->web_lists[FREEZE], w);
    else
        bitmap_clear_bit (G->web_lists[SPILL], w);
    bitmap_set_bit (G->web_lists[COALESCED], w);
    GetVex (G, w)->alias = v;
    bitmap_ior_into (GetVex (G, v)->moves, GetVex (G, w)->moves);

    temp_bitmap = BITMAP_XMALLOC ();
    bitmap_set_bit (temp_bitmap, w);
    enable_moves (G, temp_bitmap);

    adjacent (G, w, temp_bitmap);
    for (bmp_iter_set_init (&bi, temp_bitmap, 0, &i);
         bmp_iter_set (&bi, &i);
         bmp_iter_next (&bi, &i))
    {
        InsertArc (G, i, v);
        decrement_degree (G, i, classes, virtual_regs);
    }
    BITMAP_XFREE (temp_bitmap);

    cl =gen_vregArm32 (virtual_regs, GetVex (G, v)->v, ALL_REGS)->rclass;
    if  (GetVex (G, v)->num_conflicts >= bitmap_count_bits (classes[cl].available) &&
         bitmap_bit_p (G->web_lists[FREEZE], v))
    {
        bitmap_clear_bit (G->web_lists[FREEZE], v);
        bitmap_set_bit (G->web_lists[SPILL], v);
    }
}

/* 尝试合并移动工作列表上的第一个结点。  */
static void
coalesce (PhaseIFG *g,struct reg_class_desc *classes, struct avl_table *virtual_regs)
{
    int mv_num, sregno, dregno;
    ArmInst mv;
    int src, dst;
    int temp;

    mv_num = bitmap_first_set_bit (g->mv_worklist);
    mv = (ArmInst) avl_find (g->insn_map, &mv_num);
    dregno = ArmInstGetOperandAsReg (mv, 0)->vregno;
    sregno = ArmInstGetOperandAsReg (mv, 1)->vregno;

    dst = alias (g, InsertVex (g, dregno));
    src = alias (g, InsertVex (g, sregno));

    if  (GetVex (g, src)->v < FIRST_VIRTUAL_REGISTER)
    {
        temp = src;
        src = dst;
        dst = temp;
    }

    bitmap_clear_bit (g->mv_worklist, mv_num);

    if      (src == dst)
    {
        bitmap_set_bit (g->mv_coalesced, mv_num);
        add_worklist (g, dst, classes, virtual_regs);
    }
    else if (GetVex (g, src)->v < FIRST_VIRTUAL_REGISTER ||
             bitmap_bit_p (GetVex (g, src)->firstout, dst))
    {
        bitmap_set_bit (g->mv_constrained, mv_num);
        add_worklist (g, dst, classes, virtual_regs);
        add_worklist (g, src, classes, virtual_regs);
    }
    else if ((GetVex (g, dst)->v < FIRST_VIRTUAL_REGISTER &&
             gate (g, src, dst, classes, virtual_regs)) ||
             (GetVex (g, dst)->v >= FIRST_VIRTUAL_REGISTER &&
             conservative (g, src, dst, classes, virtual_regs)))
    {
        bitmap_set_bit (g->mv_coalesced, mv_num);
        combine (g, dst, src, classes, virtual_regs);
        add_worklist (g, dst, classes, virtual_regs);
    }
    else
    {
        bitmap_set_bit (g->mv_active, mv_num);
    }
}

/* 冻结与v相关的复制。用于迭代合并。  */
static void
freeze_moves (PhaseIFG *G, int v, struct reg_class_desc *classes, struct avl_table *virtual_regs)
{
    bitmap temp, temp2;
    unsigned mv_num;
    bitmap_iterator bi;
    int sregno, dregno;
    ArmInst mv;
    int src;
    int w;
    int cl;

    temp = BITMAP_XMALLOC ();
    temp2 = BITMAP_XMALLOC ();

    node_moves (G, v, temp);
    for (bmp_iter_set_init (&bi, temp, 0, &mv_num);
         bmp_iter_set (&bi, &mv_num);
         bmp_iter_next (&bi, &mv_num))
    {
        mv = (ArmInst) avl_find (G->insn_map, &mv_num);
        sregno = ArmInstGetOperandAsReg (mv, 1)->vregno;
        dregno = ArmInstGetOperandAsReg (mv, 0)->vregno;

        src = alias (G, InsertVex (G, sregno));
        if  (alias (G, v) == src)
            w = alias (G, InsertVex (G, dregno));
        else
            w = src;
        bitmap_clear_bit (G->mv_active, mv_num);
        node_moves (G, w, temp2);
        cl =gen_vregArm32 (virtual_regs, GetVex (G, w)->v, ALL_REGS)->rclass;
        if  (bitmap_empty_p (temp2) &&
             GetVex (G, w)->num_conflicts < bitmap_count_bits (classes[cl].available))
        {
            bitmap_clear_bit (G->web_lists[FREEZE], w);
            bitmap_set_bit (G->web_lists[SIMPLIFY], w);
        }
    }

    BITMAP_XFREE (temp2);
    BITMAP_XFREE (temp);
}

/* 冻结冻结工作列表上的第一个结点(仅用于迭代合并)。  */
static void
freeze (PhaseIFG *g, struct reg_class_desc *classes, struct avl_table *virtual_regs)
{
    int f;

    f = bitmap_first_set_bit (g->web_lists[FREEZE]);
    bitmap_clear_bit (g->web_lists[FREEZE], f);
    bitmap_set_bit (g->web_lists[SIMPLIFY], f);
    freeze_moves (g, f, classes, virtual_regs);
}
# endif /* SIMPLE */

/* 选择最便宜的可能溢出的结点(我们在需要的时候才会真正溢出)。  */
static void
select_spill (PhaseIFG *g, struct reg_class_desc *classes, struct avl_table *virtual_regs)
{
    unsigned i;
    bitmap_iterator bi;
    int a = bitmap_first_set_bit (g->web_lists[SPILL]);

    for (bmp_iter_set_init (&bi, g->web_lists[SPILL], 0, &i);
         bmp_iter_set (&bi, &i);
         bmp_iter_next (&bi, &i))
        if  (calc_to (GetVex (g, a)->interval) - GetVex (g, a)->interval->_first->_from < calc_to (GetVex (g, i)->interval) - GetVex (g, i)->interval->_first->_from)
            a = i;

    bitmap_clear_bit (g->web_lists[SPILL], a);
    bitmap_set_bit (g->web_lists[SIMPLIFY], a);
# if !defined(SIMPLE)
    freeze_moves (g, a, classes, virtual_regs);
# endif /* SIMPLE */
}

/* 创建一个类型为type的溢出符号。  */
static int
assign_spill_slot (control_flow_graph cfun, enum reg_classArm32 type)
{
    int ret = cfun->spill_size + cfun->arg_size + cfun->local_size;
    cfun->spill_size = cfun->spill_size + UNITS_PER_WORD;
    return ret;
}

/* 将颜色分配给select栈上的所有节点。并更新合并后的网络的颜色。  */
static void
assign_colors (control_flow_graph cfun, PhaseIFG *g, struct avl_table *virtual_regs, struct reg_class_desc *classes)
{
    int v;
    bitmap temp_bitmap;
    int cl;
    unsigned i;
    bitmap_iterator bi;

    temp_bitmap = BITMAP_XMALLOC ();
    while (!List_IsEmpty (g->stack))
    {
        v = *(int *) List_First (g->stack);
        List_DeleteFirst (g->stack);
        bitmap_clear_bit (g->web_lists[SELECT], v);

        cl = gen_vregArm32(virtual_regs, GetVex(g, v)->v, ALL_REGS)->rclass;
        bitmap_copy (temp_bitmap, classes[cl].available);

        for (bmp_iter_set_init (&bi, GetVex(g, v)->firstout, 0, &i);
             bmp_iter_set (&bi, &i);
             bmp_iter_next (&bi, &i))
        {
            if  (GetVex (g, alias (g, i))->hard_num != -1)
            {
                bitmap_clear_bit (temp_bitmap, GetVex (g, alias (g, i))->hard_num);
            }
        }

        if  (GetVex (g, v)->hard_num == -1 &&
             bitmap_empty_p (temp_bitmap))
        {
            if  (GetVex (g, v)->weight == 0x7FFFFFFF)
                fatal ("attempt to spill already spilled interval!");
            bitmap_set_bit (g->spilledRegs, v);
            GetVex (g, v)->spill_slot = assign_spill_slot (cfun, cl);
        }
        else if (GetVex (g, v)->hard_num == -1)
        {
            GetVex (g, v)->hard_num = bitmap_first_set_bit (temp_bitmap);
        }
    }
    BITMAP_XFREE (temp_bitmap);

# if !defined(SIMPLE)
    for (bmp_iter_set_init (&bi, g->web_lists[COALESCED], 0, &i);
         bmp_iter_set (&bi, &i);
         bmp_iter_next (&bi, &i))
    {
        GetVex (g, i)->hard_num = GetVex (g, alias (g, i))->hard_num;
    }
# endif /* SIMPLE */
}

static void
assign_reg_num (PhaseIFG *g, LIST blocks, struct avl_table *virtual_regs)
{
    int i;
    for (i = 0; i < g->vexnum; i++)
    {
        gen_vregArm32 (virtual_regs, GetVex(g, i)->v, NO_REGS)->hard_num = GetVex (g, i)->hard_num;
        if  (GetVex (g, i)->spill_slot != -1)
            gen_vregArm32 (virtual_regs, GetVex(g, i)->v, NO_REGS)->spill_slot = GetVex (g, i)->spill_slot;
    }
}

static void addIntervalsForSpills(PhaseIFG *g, struct avl_table *virtual_regs, LIST blocks)
{
    int start;
    int end;
    basic_block* block;
    ArmInst op, next_op;
    int i;
    union Arm_Operand operand;
    int v;
    int w;

    for(  block=(basic_block *)List_Last(blocks)
       ;  block!=NULL
       ;  block = (basic_block *)List_Prev((void *)block)
       )
    {
        for(  op=(ArmInst)List_Last(((BblockArm32)(*block)->param)->code)
           ;  op!=NULL
           ;  op = next_op
           )
        {
            next_op  = (ArmInst)List_Prev((void *)op);
            for (i = 0; i < ArmInstGetNumOperands (op); ++i)
            {
                if  (ArmInstGetOperandAsReg (op, i) &&
                     bitmap_bit_p (g->spilledRegs, InsertVex (g, ArmInstGetOperandAsReg (op, i)->vregno)))
                {
                    w = InsertVex (g, ArmInstGetOperandAsReg (op, i)->vregno);

                    start = op->uid - !ArmInstIsOutput (op, i);
                    end = 1 + op->uid - !ArmInstIsOutput (op, i);

                    operand.vreg = gen_vregArm32 (virtual_regs, -1, ArmInstGetOperandAsReg (op, i)->rclass);
                    register_descriptorArm32 (operand.vreg, gen_vregArm32(virtual_regs, ArmInstGetOperandAsReg (op, i)->vregno, ALL_REGS), NULL, NULL, FALSE);
    
                    v = InsertVex (g, operand.vreg->vregno);
                    add_use (GetVex (g, v)->interval, start, end);
                    add_def (GetVex (g, v)->interval, start);
                    ArmInstSetOperand (op, i, operand);
                    GetVex (g, v)->weight = 0x7FFFFFFF;
                    if  (ArmInstIsOutput (op, i))
                        spill_outArm32 (op, operand.vreg, GetVex (g, w)->spill_slot, virtual_regs);
                    else
                        spill_inArm32 (op, operand.vreg, GetVex (g, w)->spill_slot, virtual_regs);
                }
            }
        }
    }

    bitmap_clear(g->spilledRegs);
}

/* 构建我们要处理的工作列表。  */
static void
build_worklists (PhaseIFG *g, struct avl_table *virtual_regs, struct reg_class_desc *classes)
{
    unsigned i;
    bitmap_iterator bi;
    int cl;

    for (bmp_iter_set_init (&bi, g->web_lists[INITIAL], 0, &i);
         bmp_iter_set (&bi, &i);
         bmp_iter_next (&bi, &i))
    {
        cl =gen_vregArm32 (virtual_regs, GetVex (g, i)->v, ALL_REGS)->rclass;
        if  (bitmap_count_bits (GetVex (g, i)->firstin) >= bitmap_count_bits (classes[cl].available))
            bitmap_set_bit (g->web_lists[SPILL], i);
# if !defined(SIMPLE)
        else if (move_related (g, i))
            bitmap_set_bit (g->web_lists[FREEZE], i);
# endif /* SIMPLE */
        else
            bitmap_set_bit (g->web_lists[SIMPLIFY], i);
    }
}

/* 寄存器分配的入口点。  */
void
regallocArm32 (control_flow_graph fn, struct avl_table *virtual_regs)
{
    PhaseIFG g;
    LIST blocks;
    struct reg_class_desc classes[LIM_REG_CLASSES];
    int i;

    memset (classes, 0, sizeof (classes));
    for (i = 0; i < LIM_REG_CLASSES; i++)
    {
        classes[i].available = BITMAP_XMALLOC ();
    }

    classes[GENERAL_REGS].next_avail = 0;
    classes[GENERAL_REGS].max_num = LAST_ARM_REGNUM + 1;
    for (i = classes[GENERAL_REGS].next_avail; i < classes[GENERAL_REGS].max_num; i++)
    {
        bitmap_set_bit (classes[GENERAL_REGS].available, i);
    }
    bitmap_clear_bit (classes[GENERAL_REGS].available, SPILL_REG);

    classes[VFP_REGS].next_avail = FIRST_VFP_REGNUM;
    classes[VFP_REGS].max_num = LAST_VFP_REGNUM + 1;
    for (i = classes[VFP_REGS].next_avail; i < classes[VFP_REGS].max_num; i++)
    {
        bitmap_set_bit (classes[VFP_REGS].available, i);
    }
    bitmap_clear_bit (classes[VFP_REGS].available, SPILL_REG);

    CreateDG (&g);

    blocks = List_Create ();
    pre_and_rev_post_order_compute (fn, NULL, blocks, TRUE, FALSE);

 repeat:
    number_instructions (blocks, &g);

    compute_global_live_sets (blocks);
    build_intervals (&g, blocks);

    build_i_graph (&g, blocks);
/*  Display (&g);*/

    for (i = 0; i < g.vexnum; i++)
    {
        if  (GetVex (&g, i)->hard_num == -1)
        {
            bitmap_set_bit (g.web_lists[INITIAL], i);
        }
    }

    build_worklists (&g, virtual_regs, classes);

    do
    {
        if  (! bitmap_empty_p (g.web_lists[SIMPLIFY]))
            simplify (&g, classes, virtual_regs);
# if !defined(SIMPLE)
        else if  (! bitmap_empty_p (g.mv_worklist))
            coalesce (&g, classes, virtual_regs);
        else if  (! bitmap_empty_p (g.web_lists[FREEZE]))
            freeze (&g, classes, virtual_regs);
# endif /* SIMPLE */
        else if  (! bitmap_empty_p (g.web_lists[SPILL]))
            select_spill (&g, classes, virtual_regs);
    }
    while (! bitmap_empty_p (g.web_lists[SIMPLIFY]) ||
# if !defined(SIMPLE)
           ! bitmap_empty_p (g.mv_worklist) ||
           ! bitmap_empty_p (g.web_lists[FREEZE]) ||
# endif /* SIMPLE */
           ! bitmap_empty_p (g.web_lists[SPILL]));

/*  Display (&g); */
    assign_colors (fn, &g, virtual_regs, classes);

    if  (!bitmap_empty_p (g.spilledRegs))
    {
        addIntervalsForSpills (&g, virtual_regs, blocks);
        for (i = 0; i < g.vexnum; i++)
        {
            DeleteVex (&g, i);
            GetVex (&g, i)->hard_num = GetVex (&g, i)->v < FIRST_VIRTUAL_REGISTER ? GetVex (&g, i)->v : -1;
            GetVex (&g, i)->spill_slot = -1;
            GetVex (&g, i)->num_conflicts = 0;
# if !defined(SIMPLE)
            GetVex (&g, i)->alias = i;
# endif /* SIMPLE */
        }
        g.arcnum = 0;
        bitmap_clear (g.web_lists[INITIAL]);
# if !defined(SIMPLE)
        bitmap_clear (g.web_lists[COALESCED]);
        bitmap_clear (g.mv_coalesced);
        bitmap_clear (g.mv_active);
        bitmap_clear (g.mv_worklist);
        bitmap_clear (g.mv_constrained);
# endif /* SIMPLE */

        goto repeat;
    }

    assign_reg_num (&g, blocks, virtual_regs);

    DestroyGraph (&g);
    List_Destroy (&blocks);
    for (i = 0; i < LIM_REG_CLASSES; i++)
    {
        BITMAP_XFREE (classes[i].available);
    }
}

