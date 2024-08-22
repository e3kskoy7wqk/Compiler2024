#include <stdlib.h>
#include <string.h>
#include "all.h"

typedef struct ArcBox
{
    int tailvex,headvex; /* 该弧的尾和头顶点的位置 */
}ArcBox; /* 弧结点 */
typedef struct
{
    struct avl_table *firstin,*firstout; /* 分别指向该顶点第一条入弧和出弧 */
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

static void
compute_global_live_sets (LIST blocks)
{
    BOOL change_occurred;
    basic_block* block;
    ArmInst instr;
    bitmap work = BITMAP_XMALLOC ();
    bitmap temp = BITMAP_XMALLOC ();

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

static int InsertVex(OLGraph *G, int v)
{  /* 初始条件: 有向图G存在,v和有向图G中顶点有相同特征 */
   /* 操作结果: 在有向图G中增添新顶点v(不增添与顶点相关的弧,留待InsertArc()去做) */
    struct pair *p;

    p = avl_find (G->map, &v);
    if  (p)
        return p->y;

    if ((*G). vexnum >= G->allocated) 
    {
        int new_size = (*G). vexnum * 2 + 1;
        (*G). xlist = (VexNode *) xrealloc ((*G). xlist, new_size * sizeof (VexNode));
        G->allocated = new_size;
    }

    memset ((*G). xlist + (*G). vexnum, 0, sizeof (VexNode));
    p = (struct pair *) xmalloc (sizeof (struct pair));
    p->x = v;
    p->y = (*G). vexnum;
    avl_insert (G->map, p);
    (*G). vexnum++;

    return p->y;
}

static void InsertArc(OLGraph *G,int i,int j)
{ /* 初始条件: 有向图G存在,v和w是G中两个顶点 */
   /* 操作结果: 在G中增添弧<v,w> */
    int *f;
    f = (int *) xmalloc (sizeof (int));
    *f = i;
    free (avl_replace ((*G). xlist[j].firstin, f));
    f = (int *) xmalloc (sizeof (int));
    *f = j;
    free (avl_replace ((*G). xlist[i].firstout, f));
    (*G). arcnum++; /* 弧数加1 */
}


static void CreateDG(OLGraph *G)
{ /* 采用十字链表存储表示,构造有向图G。算法7.3 */
    memset (G, 0, sizeof (*G));
    G->map = avl_create ((avl_comparison_func *) compare_pairs, NULL, NULL);
}

void DestroyGraph(OLGraph *G)
{   /* 初始条件: 有向图G存在 */
    /* 操作结果: 销毁有向图G */
    int j;

    for(j=0;j<(*G).vexnum;j++) /* 对所有顶点 */
    {
        avl_destroy ((*G).xlist[j].firstin, (avl_item_func *) free);
        avl_destroy ((*G).xlist[j].firstout, (avl_item_func *) free);
    }
    free (G->xlist);
    avl_destroy (G->map, (avl_item_func *) free);
}

static void
build_ifg_virtual (OLGraph *G, LIST blocks)
{
    basic_block* block;
    ArmInst instr;
    bitmap liveout;
    bitmap def;
    bitmap use;
    bitmap_iterator bi, bi2;
    bitmap_head temp_bitmap;
    unsigned i;
    unsigned k;
    int v;
    int w;

    def = BITMAP_XMALLOC ();
    use = BITMAP_XMALLOC ();
    for(  block=(basic_block*) List_Last(blocks)
       ;  block!=NULL
       ;  block = (basic_block*) List_Prev((void *)block)
       )
    {
        liveout = ((BblockArm32)(*block))->liveout;
        for(  instr=(ArmInst) List_Last(((BblockArm32)(*block))->code)
           ;  instr!=NULL
           ;  instr = (ArmInst) List_Prev((void *)instr)
           )
        {
            ArmInstOutput (instr, def);
            ArmInstInput (instr, use);
            bitmap_ior_into (liveout, def);

            for (bmp_iter_set_init (&bi, def, 0, &i);
                 bmp_iter_set (&bi, &i);
                 bmp_iter_next (&bi, &i))
            {
                v = InsertVex(G, i);
                for (bmp_iter_set_init (&bi2, liveout, 0, &k);
                     bmp_iter_set (&bi2, &k);
                     bmp_iter_next (&bi2, &k))
                {
                    w = InsertVex(G, k);
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
}

/* 寄存器分配的入口点。  */
void
regallocArm32 (control_flow_graph fn, struct avl_table *virtual_regs)
{
    OLGraph g;
    LIST blocks;

    blocks = List_Create ();
    pre_and_rev_post_order_compute (fn, NULL, blocks, TRUE, FALSE);

    compute_global_live_sets (blocks);

    CreateDG (&g);
    build_ifg_virtual (&g, blocks);

    DestroyGraph (&g);

    List_Destroy (&blocks);
}
