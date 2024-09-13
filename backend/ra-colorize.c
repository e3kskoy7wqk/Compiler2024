/* Graph coloring register allocator.  */

#include <stdlib.h>
#include <string.h>
#include "all.h"

/* # define SIMPLE
*/

#ifndef min
#define min(a,b) ((a) <= (b)? (a):(b))
#endif
#ifndef max
#define max(a,b) ((a) >= (b)? (a):(b))
#endif

/* the infamous mp_int structure */
typedef struct  {
    int used, alloc;
    int sign;
    unsigned short *dp;
} mp_int;

/* 对给定节点的分类(即它处于什么状态)。  */
enum ra_node_type
{
    INITIAL = 0,
    PRECOLORED,
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
    int reg_class;

    /* Cost of spilling.  */
    mp_int spill_cost;
    unsigned int orig_spill_cost;

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
    bitmap mv_worklist, mv_coalesced, mv_constrained;
    bitmap mv_frozen, mv_active;
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
    bitmap available;
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

static int
compare_insns (LIR_Op first, LIR_Op second)
{
    int ret = 0 ;

    if ( first->uid < second->uid )
        ret = -1 ;
    else if ( first->uid > second->uid )
        ret = 1 ;

    return( ret );
}

#define MP_MASK          ((((unsigned short)1)<<((unsigned short)MP_DIGIT_BIT))-((unsigned short)1))
#define MP_DIGIT_BIT 15
#define MP_SIZEOF_BITS(type)    ((size_t)8 * sizeof(type))

/* 根据给定的长度初始化一个mp_init对象 */
static void mp_init_size(mp_int *a, int size)
{
    /* 分配所需的内存 */
    a->dp = (unsigned short *) xcalloc((size_t)size, sizeof(unsigned short));

    /* 将used设置为0，分配的数字设置为默认精度，符号设置为正数 */
    a->used  = 0;
    a->alloc = size;
    a->sign  = 0;
}

static void mp_set_u32(mp_int * a, unsigned int b)
{
    int i = 0;
    while (b != 0u) {
        a->dp[i++] = ((unsigned short)b & MP_MASK);
        if (MP_SIZEOF_BITS(unsigned int) <= MP_DIGIT_BIT) { break; }
        b >>= ((MP_SIZEOF_BITS(unsigned int) <= MP_DIGIT_BIT) ? 0 : MP_DIGIT_BIT);
    }   
    a->used = i;
    a->sign = 0;
    if (a->alloc - a->used > 0) {
        memset(a->dp + a->used, 0, (size_t)(a->alloc - a->used) * sizeof(unsigned short));
    }
}

/* clear one (frees)  */
static void mp_clear(mp_int *a)
{
    /* 只有在a之前没有被释放时才执行操作 */
    if (a->dp != NULL) {
        /* 释放内存 */
        free(a->dp);

        /* 重置成员，使调试更容易 */
        a->dp    = NULL;
        a->alloc = a->used = 0;
        a->sign  = 0;
    }
}

/* 按需增长 */
static void mp_grow(mp_int *a, int size)
{
    /* 如果分配大小较小，则分配更多的内存 */
    if (a->alloc < size) {
        unsigned short *dp;

        /* 重新分配数组a->dp
         *
         * 我们将返回值存储在一个临时变量中，以防操作失
         * 败——我们不想覆盖a的dp成员。
         */
        dp = (unsigned short *) xrealloc(a->dp,
                                     (size_t)size * sizeof(unsigned short));

        /* 重新分配成功，设置a->dp */
        a->dp = dp;

        /* 多余位数为零 */
        if (size - a->alloc > 0)
        {
            memset(a->dp + a->alloc, 0, (size_t)(size - a->alloc) * sizeof(unsigned short));
        }
        a->alloc = size;
    }
}

/* 削减未使用数字
 *
 * 这用于确保修剪前导零，并且“used”的前导数字通常会非常快地
 * 非零。如果没有更多的前导数字，也可以修复符号
 */
static void mp_clamp(mp_int *a)
{
    /* 当最高有效数字为零时减少used。
     */
    while ((a->used > 0) && (a->dp[a->used - 1] == 0u))
    {
        --(a->used);
    }

    /* 如果为零，则重置标志位 */
    if (a->used == 0)
    {
        a->sign = 0;
    }
}

static void mp_add(const mp_int *a, const mp_int *b, mp_int *c)
{
    int oldused, min, max, i;
    unsigned short u;

    c->sign = a->sign;

    /* 计算大小，我们让|a| <= |b|这意味着我们需要对它们进行排序。
     * “x”将指向数字最多的输入
     */
    if (a->used < b->used)
    {
        const mp_int * _c = a; a = b; b = _c;
    }

    min = b->used;
    max = a->used;

    /* 初始化结果 */
    mp_grow(c, max + 1);

    /* 获取旧的使用过的数字计数并设置新的 */
    oldused = c->used;
    c->used = max + 1;

    /* 进位归零 */
    u = 0;
    for (i = 0; i < min; i++)
    {
        /* 计算一位数的和，T[i] = A[i] + B[i] + U */
        c->dp[i] = a->dp[i] + b->dp[i] + u;

        /* U = T[i]的进位 */
        u = c->dp[i] >> (unsigned short)MP_DIGIT_BIT;

        /* 去掉T[i]的进位 */
        c->dp[i] &= MP_MASK;
    }

    /* 现在复制较高位的数字（如果有的话），即在A+B中，如果A或B有更多位数，则将这些位数添加进去。
     */
    if (min != max)
    {
        for (; i < max; i++)
        {
            /* T[i] = A[i] + U */
            c->dp[i] = a->dp[i] + u;

            /* U = T[i]的进位 */
            u = c->dp[i] >> (unsigned short)MP_DIGIT_BIT;

            /* 去掉T[i]的进位 */
            c->dp[i] &= MP_MASK;
        }
    }

    /* 添加进位 */
    c->dp[i] = u;

    /* clear digits above oldused */
    if (oldused - c->used > 0)
    {
        memset(c->dp + c->used, 0, (size_t)(oldused - c->used) * sizeof(unsigned short));
    }

    mp_clamp(c);
}

static void mp_mul(const mp_int *a, const mp_int *b, mp_int *c)
{
    int digs = a->used + b->used + 1;
    mp_int  t;
    int     pa, ix;

    mp_init_size(&t, digs);
    t.used = digs;

    /* 直接计算乘积的位数 */
    pa = a->used;
    for (ix = 0; ix < pa; ix++)
    {
        int iy, pb;
        unsigned short u = 0;

        /* 将我们自己限制在输出位数上 */
        pb = min(b->used, digs - ix);

        /* 计算输出的列并传播进位 */
        for (iy = 0; iy < pb; iy++)
        {
            /* 计算mp_word列 */
            unsigned int r = (unsigned int)t.dp[ix + iy] +
                        ((unsigned int)a->dp[ix] * (unsigned int)b->dp[iy]) +
                        (unsigned int)u;

            /* 新列是结果的较低部分 */
            t.dp[ix + iy] = (unsigned short)(r & (unsigned int)MP_MASK);

            /* 从结果中获取进位字 */
            u       = (unsigned short)(r >> (unsigned int)MP_DIGIT_BIT);
        }
        /* 如果进位小于digs，则设置进位 */
        if ((ix + iy) < digs)
        {
           t.dp[ix + pb] = u;
        }
    }

    mp_clamp(&t);
    do { mp_int _c = t; t = *c; *c = _c; } while (0);

    mp_clear(&t);
}

/* 比较两个int*/
static int mp_cmp(const mp_int *a, const mp_int *b)
{
    int n;

    /* 根据非零数字的个数进行比较 */
    if (a->used != b->used)
    {
        return a->used > b->used ? 1 : -1;
    }

    /* 基于数字的比较  */
    for (n = a->used; n --> 0;)
    {
        if (a->dp[n] != b->dp[n])
        {
            return a->dp[n] > b->dp[n] ? 1 : -1;
        }
    }

    return 0;
}

static int LocateVex(PhaseIFG *G,int u)
{ /* 返回顶点u在有向图G中的位置(序号),如不存在则返回-1 */
    struct pair *p;
    p = avl_find (G->map, &u);
    if  (p)
        return p->y;
    return -1;
}

static int InsertVex(PhaseIFG *G, int v, int reg_class, struct Backend* backend)
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
    (*G).xlist[(*G). vexnum].reg_class = reg_class;
    (*G).xlist[(*G). vexnum].hard_num = backend->is_virtual_register (v) ? -1 : v;
    (*G).xlist[(*G). vexnum].spill_slot = -1;
    mp_init_size (&(*G).xlist[(*G). vexnum].spill_cost, 1);

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
    G->mv_frozen = BITMAP_XMALLOC ();
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
        mp_clear (&(*G).xlist[j].spill_cost);

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
    BITMAP_XFREE (G->mv_frozen);
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

void
compute_global_live_sets (LIST blocks, struct Backend* backend)
{
    BOOL change_occurred;
    basic_block* block;
    LIR_Op instr;
    bitmap work = BITMAP_XMALLOC ();
    bitmap temp = BITMAP_XMALLOC ();

    for(  block=(basic_block *)List_Last(blocks)
       ;  block!=NULL
       ;  block = (basic_block *)List_Prev((void *)block)
       )
    {
        bitmap_clear (backend->get_live_in (*block));
        bitmap_clear (backend->get_live_out (*block));
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
                    bitmap_copy (work, backend->get_live_in (succ));
                    first = FALSE;
                }
                else
                    bitmap_ior_into (work, backend->get_live_in (succ));
            }
            if (first)
                bitmap_clear (work);
  
            bitmap_copy (backend->get_live_out (*block), work);

            /* 在反向指令遍历中删除定义，包括使用。  */
            for(  instr=(LIR_Op)List_Last(backend->get_code (*block))
               ;  instr!=NULL
               ;  instr = (LIR_Op)List_Prev((void *)instr)
               )
            {
                unsigned regno;
                bitmap_iterator bi;
                backend->output_regs (instr, temp);
                for (bmp_iter_set_init (&bi, temp, 0, &regno);
                     bmp_iter_set (&bi, &regno);
                     bmp_iter_next (&bi, &regno))
                    bitmap_clear_bit (work, regno);
                backend->input_regs (instr, temp);
                for (bmp_iter_set_init (&bi, temp, 0, &regno);
                     bmp_iter_set (&bi, &regno);
                     bmp_iter_next (&bi, &regno))
                    bitmap_set_bit (work, regno);
            }

            /* 记录这是否改变了什么。  */
            if (bitmap_ior_into (backend->get_live_in (*block), work))
                change_occurred = TRUE;
        }
      }
    while (change_occurred);

    BITMAP_XFREE (work);
    BITMAP_XFREE (temp);
}

/* 构建(或重建)带有冲突的完整干涉图。  */
static void
build_i_graph (PhaseIFG *G, LIST blocks, struct Backend* backend)
{
    basic_block* block;
    LIR_Op instr;
    bitmap liveout;
    bitmap def;
    bitmap use;
    bitmap_iterator bi, bi2;
    bitmap_head temp_bitmap;
    unsigned i, k;
    int v, w;
    LIR_Op curs;

    def = BITMAP_XMALLOC ();
    use = BITMAP_XMALLOC ();
    liveout = BITMAP_XMALLOC ();
    for(  block=(basic_block*) List_Last(blocks)
       ;  block!=NULL
       ;  block = (basic_block*) List_Prev((void *)block)
       )
    {
        bitmap_copy (liveout, backend->get_live_out (*block));
        for(  instr=(LIR_Op) List_Last(backend->get_code (*block))
           ;  instr!=NULL
           ;  instr = (LIR_Op) List_Prev((void *)instr)
           )
        {
            if  (backend->is_call (instr))
            {
                /* 保护调用者保存的寄存器。  */
                for(  curs=(LIR_Op) backend->handle_method_arguments (instr)
                   ;  curs!=instr
                   ;  curs = (LIR_Op) List_Next((void *)curs)
                   )
                {
                    backend->output_regs (curs, def);
                    for (i = 0; i < (unsigned) backend->num_caller_save_registers; i++)
                    {
                        v = LocateVex (G, backend->caller_save_registers[i]);
                        for (bmp_iter_set_init (&bi2, liveout, 0, &k);
                             bmp_iter_set (&bi2, &k);
                             bmp_iter_next (&bi2, &k))
                        {
                            w = LocateVex (G, k);
                            if (w != v)
                                InsertArc (G, w, v);
                        }
                        for (bmp_iter_set_init (&bi2, def, 0, &k);
                             bmp_iter_set (&bi2, &k);
                             bmp_iter_next (&bi2, &k))
                        {
                            w = LocateVex (G, k);
                            if (w != v)
                                InsertArc (G, w, v);
                        }
                    }
                }
            }

            backend->output_regs (instr, def);
            backend->input_regs (instr, use);
            bitmap_ior_into (liveout, def);

# if !defined(SIMPLE)
            if  (backend->is_move (instr))
            {
                bitmap_and_compl_into (liveout, use);
                for (bmp_iter_set_init (&bi, def, 0, &i);
                     bmp_iter_set (&bi, &i);
                     bmp_iter_next (&bi, &i))
                {
                    v = LocateVex (G, i);
                    bitmap_set_bit (GetVex (G, v)->moves, instr->uid);
                }
                for (bmp_iter_set_init (&bi, use, 0, &i);
                     bmp_iter_set (&bi, &i);
                     bmp_iter_next (&bi, &i))
                {
                    v = LocateVex (G, i);
                    bitmap_set_bit (GetVex (G, v)->moves, instr->uid);
                }
                bitmap_set_bit (G->mv_worklist, instr->uid);
            }
# endif /* SIMPLE */

            for (bmp_iter_set_init (&bi, def, 0, &i);
                 bmp_iter_set (&bi, &i);
                 bmp_iter_next (&bi, &i))
            {
                v = LocateVex(G, i);
                for (bmp_iter_set_init (&bi2, liveout, 0, &k);
                     bmp_iter_set (&bi2, &k);
                     bmp_iter_next (&bi2, &k))
                {
                    w = LocateVex (G, k);
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
number_instructions (LIST blocks, PhaseIFG *g, struct Backend* backend)
{
    int op_id = 0;
    basic_block* block;
    LIR_Op op;

    avl_destroy (g->insn_map, (avl_item_func *) NULL);
    g->insn_map = avl_create ((avl_comparison_func *) compare_insns, NULL, NULL);
    for(  block=(basic_block *)List_First(blocks)
       ;  block!=NULL
       ;  block = (basic_block *)List_Next((void *)block)
       )
    {
        for(  op=(LIR_Op)List_First(backend->get_code (*block))
           ;  op!=NULL
           ;  op = (LIR_Op)List_Next((void *)op)
           )
        {
            op->uid = op_id++;
            avl_insert (g->insn_map, op);
        }
    }
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
decrement_degree (PhaseIFG *g, int v, struct reg_class_desc *classes)
{
    int cl = GetVex (g, v)->reg_class;
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
        else if (! bitmap_bit_p (g->web_lists[PRECOLORED], v))
# endif /* SIMPLE */
        {
            bitmap_set_bit (g->web_lists[SIMPLIFY], v);
        }
    }
}

/* 简化简化工作列表上的节点。  */
static void
simplify(PhaseIFG *g, struct reg_class_desc *classes)
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
        decrement_degree (g, i, classes);

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
add_worklist (PhaseIFG *G, int v, struct reg_class_desc *classes)
{
    int cl;
    cl = GetVex (G, v)->reg_class;
    if  (!bitmap_bit_p (G->web_lists[PRECOLORED], v) &&
         !move_related (G, v) &&
         GetVex (G, v)->num_conflicts < (int) bitmap_count_bits (classes[cl].available))
    {
        bitmap_clear_bit (G->web_lists[FREEZE], v);
        bitmap_set_bit (G->web_lists[SIMPLIFY], v);
    }
}

/* 预着色节点合并启发式算法。  */
static BOOL
ok (PhaseIFG *G, int target,int source,struct reg_class_desc *classes)
{
    bitmap temp_bitmap;
    unsigned i;
    bitmap_iterator bi;
    BOOL bResult = TRUE;
    int cl;

    temp_bitmap = BITMAP_XMALLOC ();
    adjacent (G, target, temp_bitmap);
    for (bmp_iter_set_init (&bi, temp_bitmap, 0, &i);
         bmp_iter_set (&bi, &i);
         bmp_iter_next (&bi, &i))
    {
        cl = GetVex (G, i)->reg_class;
        if  (GetVex (G, i)->num_conflicts < (int) bitmap_count_bits (classes[cl].available) ||
             bitmap_bit_p (G->web_lists[PRECOLORED], i) ||
             bitmap_bit_p (GetVex (G, i)->firstout, source))
            continue;
        bResult = FALSE;
        break;
    }
    BITMAP_XFREE (temp_bitmap);

    return bResult;
}

/* 非预着色节点合并启发式算法。  */
static BOOL
conservative (PhaseIFG *G, int v,int w,struct reg_class_desc *classes)
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

    cl = GetVex (G, v)->reg_class;
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
combine (PhaseIFG *G, int v,int w,struct reg_class_desc *classes)
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
        decrement_degree (G, i, classes);
    }
    BITMAP_XFREE (temp_bitmap);

    cl = GetVex (G, v)->reg_class;
    if  (GetVex (G, v)->num_conflicts >= (int) bitmap_count_bits (classes[cl].available) &&
         bitmap_bit_p (G->web_lists[FREEZE], v))
    {
        bitmap_clear_bit (G->web_lists[FREEZE], v);
        bitmap_set_bit (G->web_lists[SPILL], v);
    }
}

/* 尝试合并移动工作列表上的第一个结点。  */
static void
coalesce (PhaseIFG *g,struct reg_class_desc *classes, struct Backend* backend)
{
    int mv_num, sregno, dregno;
    LIR_Op mv;
    int src, dst;
    int temp;

    mv_num = bitmap_first_set_bit (g->mv_worklist);
    mv = (LIR_Op) avl_find (g->insn_map, &mv_num);
    dregno = backend->as_register (mv, 0)->vregno;
    sregno = backend->as_register (mv, 1)->vregno;

    dst = alias (g, LocateVex (g, dregno));
    src = alias (g, LocateVex (g, sregno));

    if  (bitmap_bit_p (g->web_lists[PRECOLORED], src))
    {
        temp = src;
        src = dst;
        dst = temp;
    }

    bitmap_clear_bit (g->mv_worklist, mv_num);

    if      (src == dst)
    {
        bitmap_set_bit (g->mv_coalesced, mv_num);
        add_worklist (g, dst, classes);
    }
    else if (bitmap_bit_p (g->web_lists[PRECOLORED], src) ||
             bitmap_bit_p (GetVex (g, src)->firstout, dst))
    {
        bitmap_set_bit (g->mv_constrained, mv_num);
        add_worklist (g, dst, classes);
        add_worklist (g, src, classes);
    }
    else if ((bitmap_bit_p (g->web_lists[PRECOLORED], dst) &&
             ok (g, src, dst, classes)) ||
             (!bitmap_bit_p (g->web_lists[PRECOLORED], dst) &&
             conservative (g, src, dst, classes)))
    {
        bitmap_set_bit (g->mv_coalesced, mv_num);
        combine (g, dst, src, classes);
        add_worklist (g, dst, classes);
    }
    else
    {
        bitmap_set_bit (g->mv_active, mv_num);
    }
}

/* 冻结与v相关的复制。用于迭代合并。  */
static void
freeze_moves (PhaseIFG *G, int v, struct reg_class_desc *classes, struct Backend* backend)
{
    bitmap temp, temp2;
    unsigned mv_num;
    bitmap_iterator bi;
    int sregno, dregno;
    LIR_Op mv;
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
        mv = (LIR_Op) avl_find (G->insn_map, &mv_num);
        sregno = backend->as_register (mv, 1)->vregno;
        dregno = backend->as_register (mv, 0)->vregno;

        src = alias (G, LocateVex (G, sregno));
        if  (alias (G, v) == src)
            w = alias (G, LocateVex (G, dregno));
        else
            w = src;
        bitmap_clear_bit (G->mv_active, mv_num);
        bitmap_set_bit (G->mv_frozen, mv_num);
        node_moves (G, w, temp2);
        cl = GetVex (G, w)->reg_class;
        if  (bitmap_empty_p (temp2) &&
             GetVex (G, w)->num_conflicts < (int) bitmap_count_bits (classes[cl].available))
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
freeze (PhaseIFG *g, struct reg_class_desc *classes, struct Backend* backend)
{
    int f;

    f = bitmap_first_set_bit (g->web_lists[FREEZE]);
    bitmap_clear_bit (g->web_lists[FREEZE], f);
    bitmap_set_bit (g->web_lists[SIMPLIFY], f);
    freeze_moves (g, f, classes, backend);
}
# endif /* SIMPLE */

/* 选择最便宜的可能溢出的结点(我们在需要的时候才会真正溢出)。  */
static void
select_spill (PhaseIFG *g, struct reg_class_desc *classes, struct Backend* backend)
{
    unsigned i;
    bitmap_iterator bi;
    int selected_reg = bitmap_first_set_bit (g->web_lists[SPILL]);

    for (bmp_iter_set_init (&bi, g->web_lists[SPILL], 0, &i);
         bmp_iter_set (&bi, &i);
         bmp_iter_next (&bi, &i))
    {
        if  (GetVex (g, i)->orig_spill_cost != 0x7FFFFFFF)
        {
            selected_reg = i;
            break;
        }
    }

    for (bmp_iter_set_init (&bi, g->web_lists[SPILL], 0, &i);
         bmp_iter_set (&bi, &i);
         bmp_iter_next (&bi, &i))
        if  (GetVex (g, i)->orig_spill_cost != 0x7FFFFFFF &&
             mp_cmp (&GetVex (g, i)->spill_cost, &GetVex (g, selected_reg)->spill_cost) < 0)
            selected_reg = i;

    bitmap_clear_bit (g->web_lists[SPILL], selected_reg);
    bitmap_set_bit (g->web_lists[SIMPLIFY], selected_reg);
# if !defined(SIMPLE)
    freeze_moves (g, selected_reg, classes, backend);
# endif /* SIMPLE */
}

/* 将颜色分配给select栈上的所有节点。并更新合并后的网络的颜色。  */
static void
assign_colors (control_flow_graph cfun, PhaseIFG *g, struct reg_class_desc *classes, struct Backend* backend)
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

        cl = GetVex(g, v)->reg_class;
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
            if  (GetVex (g, v)->orig_spill_cost == 0x7FFFFFFF)
                fatal ("attempt to spill already spilled interval!");
            bitmap_set_bit (g->spilledRegs, v);
            GetVex (g, v)->spill_slot = backend->assign_spill_slot (cfun, cl);
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
assign_reg_num (PhaseIFG *g, LIST blocks, struct avl_table *virtual_regs, struct Backend* backend)
{
    int i;
    for (i = 0; i < g->vexnum; i++)
    {
        backend->gen_vreg (virtual_regs, GetVex(g, i)->v, 0)->hard_num = GetVex (g, i)->hard_num;
        if  (GetVex (g, i)->spill_slot != -1)
            backend->gen_vreg (virtual_regs, GetVex(g, i)->v, 0)->spill_slot = GetVex (g, i)->spill_slot;
    }
}

static void insert_spill_code(PhaseIFG *g, struct avl_table *virtual_regs, LIST blocks, struct Backend* backend)
{
    basic_block* block;
    LIR_Op op, next_op;
    int i;
    union LIR_Opr operand;
    int v;
    int w;

    for(  block=(basic_block *)List_Last(blocks)
       ;  block!=NULL
       ;  block = (basic_block *)List_Prev((void *)block)
       )
    {
        for(  op=(LIR_Op)List_Last(backend->get_code (*block))
           ;  op!=NULL
           ;  op = next_op
           )
        {
            next_op  = (LIR_Op)List_Prev((void *)op);
            for (i = 0; i < backend->operand_count (op); ++i)
            {
                if  (backend->as_register (op, i) &&
                     bitmap_bit_p (g->spilledRegs, LocateVex (g, backend->as_register (op, i)->vregno)))
                {
                    w = LocateVex (g, backend->as_register (op, i)->vregno);

                    operand.vreg = backend->gen_vreg (virtual_regs, -1, backend->as_register (op, i)->rclass);
                    backend->regdesc (operand.vreg, backend->gen_vreg (virtual_regs, backend->as_register (op, i)->vregno, 0), NULL, NULL, FALSE);
    
                    v = InsertVex (g, operand.vreg->vregno, operand.vreg->rclass, backend);
                    backend->set_op (op, i, &operand);
                    GetVex (g, v)->orig_spill_cost = 0x7FFFFFFF;
                    if  (backend->op_output_p (op, i))
                        backend->spill_out (op, operand.vreg, GetVex (g, w)->spill_slot, virtual_regs);
                    else
                        backend->spill_in (op, operand.vreg, GetVex (g, w)->spill_slot, virtual_regs);
                }
            }
        }
    }

    bitmap_clear(g->spilledRegs);
}

/* 构建我们要处理的工作列表。  */
static void
build_worklists (PhaseIFG *g, struct avl_table *virtual_regs, struct reg_class_desc *classes, struct Backend* backend)
{
    unsigned i;
    bitmap_iterator bi;
    int cl;

    for (bmp_iter_set_init (&bi, g->web_lists[INITIAL], 0, &i);
         bmp_iter_set (&bi, &i);
         bmp_iter_next (&bi, &i))
    {
        cl =backend->gen_vreg (virtual_regs, GetVex (g, i)->v, 0)->rclass;
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

static void
estimate_spill_costs (control_flow_graph fn, PhaseIFG *g, struct Backend* backend)
{
    basic_block* block;
    LIR_Op op;
    int i;
    vreg_t vreg;
    mp_int frequency;
    mp_int temp;
    mp_int ten;

    /* 初始化逐出代价。  */
    for (i = 0; i < g->vexnum; i++)
    {
        mp_clear (&GetVex (g, i)->spill_cost);
        mp_init_size (&GetVex (g, i)->spill_cost, 1);
        mp_set_u32 (&GetVex (g, i)->spill_cost, 0);
    }

    mp_init_size (&ten, 1);
    mp_set_u32 (&ten, 10);

    /* 计算逐出代价。  */
    for(  block=(basic_block*) List_Last(fn->basic_block_info)
       ;  block!=NULL
       ;  block = (basic_block*) List_Prev((void *)block)
       )
    {
        for (i = loop_depth (*block), mp_init_size (&frequency, 1), mp_set_u32 (&frequency, 1);
            i > 0;
            i--)
        {
            mp_init_size (&temp, 1);
            mp_mul (&frequency, &ten, &temp);
            do { mp_int _c = temp; temp = frequency; frequency = _c; } while (0);
            mp_clear (&temp);
        }

        for(  op=(LIR_Op) List_Last(backend->get_code (*block))
           ;  op!=NULL
           ;  op = (LIR_Op) List_Prev((void *)op)
           )
        {
            for (i = 0; i < backend->operand_count (op); i++)
            {
                vreg = backend->as_register (op, i);
                if  (vreg)
                {
                    mp_init_size (&temp, 1);
                    mp_add(&GetVex (g, LocateVex (g, vreg->vregno))->spill_cost, &frequency, &temp);
                    do { mp_int _c = temp; temp = GetVex (g, LocateVex (g, vreg->vregno))->spill_cost; GetVex (g, LocateVex (g, vreg->vregno))->spill_cost = _c; } while (0);
                    mp_clear (&temp);
                }
            }
        }

        mp_clear (&frequency);
    }
    mp_clear (&ten);
}

/* 寄存器分配的入口点。  */
void
ra_colorize_graph (control_flow_graph fn, struct avl_table *virtual_regs, struct Backend* backend)
{
    PhaseIFG g;
    LIST blocks;
    struct reg_class_desc *classes;
    int i;
    vreg_t iter;
    struct avl_traverser trav;

    classes = (struct reg_class_desc *) xmalloc (sizeof (struct reg_class_desc) * backend->class_count);
    memset (classes, 0, sizeof (struct reg_class_desc) * backend->class_count);
    for (i = 0; i < backend->class_count; i++)
    {
        classes[i].available = BITMAP_XMALLOC ();
        bitmap_copy (classes[i].available, backend->classes[i].available);
        classes[i].next_avail = backend->classes[i].next_avail;
        classes[i].max_num = backend->classes[i].max_num;
    }

    flow_loops_find (fn);
    CreateDG (&g);

    for(  iter = (vreg_t)avl_t_first (&trav, virtual_regs)
       ;  iter != NULL
       ;  iter = (vreg_t)avl_t_next (&trav)
       )
        InsertVex (&g, iter->vregno, iter->rclass, backend);

    blocks = List_Create ();
    pre_and_rev_post_order_compute (fn, NULL, blocks, TRUE, FALSE);

 repeat:
    number_instructions (blocks, &g, backend);

    compute_global_live_sets (blocks, backend);

    build_i_graph (&g, blocks, backend);
/*  Display (&g);*/
    estimate_spill_costs (fn, &g, backend);

    bitmap_clear (g.web_lists[PRECOLORED]);
    for (i = 0; i < g.vexnum; i++)
    {
        if  (GetVex (&g, i)->hard_num == -1)
            bitmap_set_bit (g.web_lists[INITIAL], i);
        else
            bitmap_set_bit (g.web_lists[PRECOLORED], i);
    }

    build_worklists (&g, virtual_regs, classes, backend);
    bitmap_clear (g.web_lists[INITIAL]);

    do
    {
        if  (! bitmap_empty_p (g.web_lists[SIMPLIFY]))
            simplify (&g, classes);
# if !defined(SIMPLE)
        else if  (! bitmap_empty_p (g.mv_worklist))
            coalesce (&g, classes, backend);
        else if  (! bitmap_empty_p (g.web_lists[FREEZE]))
            freeze (&g, classes, backend);
# endif /* SIMPLE */
        else if  (! bitmap_empty_p (g.web_lists[SPILL]))
            select_spill (&g, classes, backend);
    }
    while (! bitmap_empty_p (g.web_lists[SIMPLIFY]) ||
# if !defined(SIMPLE)
           ! bitmap_empty_p (g.mv_worklist) ||
           ! bitmap_empty_p (g.web_lists[FREEZE]) ||
# endif /* SIMPLE */
           ! bitmap_empty_p (g.web_lists[SPILL]));

/*  Display (&g); */
    assign_colors (fn, &g, classes, backend);

    if  (!bitmap_empty_p (g.spilledRegs))
    {
        insert_spill_code (&g, virtual_regs, blocks, backend);
        for (i = 0; i < g.vexnum; i++)
        {
            DeleteVex (&g, i);
            GetVex (&g, i)->hard_num = backend->is_virtual_register (GetVex (&g, i)->v) ? -1 : GetVex (&g, i)->v;
            GetVex (&g, i)->spill_slot = -1;
            GetVex (&g, i)->num_conflicts = 0;
# if !defined(SIMPLE)
            GetVex (&g, i)->alias = i;
# endif /* SIMPLE */
        }
        g.arcnum = 0;
# if !defined(SIMPLE)
        bitmap_clear (g.web_lists[COALESCED]);
        bitmap_clear (g.mv_coalesced);
        bitmap_clear (g.mv_active);
        bitmap_clear (g.mv_worklist);
        bitmap_clear (g.mv_constrained);
        bitmap_clear (g.mv_frozen);
# endif /* SIMPLE */

        goto repeat;
    }

    assign_reg_num (&g, blocks, virtual_regs, backend);

    flow_loops_free (&fn->loops);
    DestroyGraph (&g);
    List_Destroy (&blocks);
    for (i = 0; i < backend->class_count; i++)
    {
        BITMAP_XFREE (classes[i].available);
    }
    free (classes);
}

