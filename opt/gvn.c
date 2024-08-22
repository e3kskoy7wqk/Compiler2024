/*
 *                      COMPILER 2.0
 *                      Copyright (C) MTC 2024
 *                      All Rights Reserved.
 */

/****************************************************************/
/*************************** INCLUDES ***************************/
/****************************************************************/

#include <math.h>
#include <stdlib.h>
#include <string.h>
#if ! defined (_WIN32)
#include <unistd.h>
#endif
#include "all.h"

/****************************************************************/
/*************************** CONSTANTS **************************/
/****************************************************************/

typedef struct
{
    struct ArcBox *firstin,*firstout; /* 分别指向该顶点第一条入弧和出弧 */
    int hash;
    unsigned Opcode;
    int refcnt;
    varpool_node subst;
    int trueval;
}VexNode; /* 顶点结点 */
typedef struct ArcBox
{  
    int tailvex,headvex; /* 该弧的尾和头顶点的位置 */
    struct ArcBox *hlink,*tlink; /* 分别为弧头相同和弧尾相同的弧的链域 */
}ArcBox; /* 弧结点 */
typedef struct
{
    VexNode *xlist; /* 表头向量(数组) */
    int allocated;
    int vexnum,arcnum; /* 有向图的当前顶点数和弧数 */
    struct avl_table *_entries;
    LIST identities;
}OLGraph;

typedef struct block_info_def
{
    basic_block adjvex;
    BOOL sorted;
    OLGraph g;
} *block_info;

struct variable_info
{
    bitmap uses;
    bitmap affected;
    int cnt;
    int v;
};

enum ValueLatticeElementTy {
    undef,
    one,
    zero,
    overdefined,
};

struct pair
{
    varpool_node var;
    int v;
};

struct Expression {
    unsigned Opcode;
    int HashVal;
    LIST Operands;
};


/****************************************************************/
/*************************** GLOBALS ****************************/
/****************************************************************/

/****************************************************************/
/*************************** PRIVATE FUNCTIONS ******************/
/****************************************************************/

static int
compare_pairs (struct pair *p1, struct pair *p2)
{
    return compare_varpool_node (p1->var, p2->var, (void *)TRUE);
}

static int compare( struct Expression *arg1, struct Expression *arg2 )
{
    int ret = 0 ;
    int * src;
    int * dst;

    for(  src=(int *) List_First(arg1->Operands), dst=(int *) List_First(arg2->Operands)
       ;  src!=NULL && dst!=NULL
       ;  src = (int *) List_Next((void *)src), dst = (int *) List_Next((void *)dst)
       )
    {
        ret = 0;
        if ( *src < *dst )
            ret = -1 ;
        else if ( *src > *dst )
            ret = 1 ;
        if  (ret)
            break;
    }

    if  (ret == 0)
    {
        if ( dst )
            ret = -1 ;
        else if ( src )
            ret = 1 ;
    }

    return( ret );
}

static int compare2( struct Expression *arg1, struct Expression *arg2 )
{
    if ( arg1->Opcode < arg2->Opcode )
        return -1 ;
    else if ( arg1->Opcode > arg2->Opcode )
        return 1 ;

    return compare (arg1, arg2);
}


static struct Expression *
createExpression (IRInst I, varpool_node_set set)
{
    struct Expression *E = (struct Expression *) xmalloc (sizeof (*E));
    varpool_node vnode;
    int i;
    int v;
    int count;
    IRInst *curs;

    E->Opcode = I->opcode;
    E->Operands = List_Create();
    if  (I->opcode != IRINST_OP_call)
    {
        for (i = 0; i < IRInstGetNumOperands (I); i ++)
        {
            if  (! IRInstIsOutput (I, i))
            {
                vnode = varpool_get_node (set, IRInstGetOperand (I, i));
                v = ((struct variable_info *) vnode->param)->v;
                *(int *) List_NewLast (E->Operands, sizeof (int)) = v;
            }
        }
    }
    else
    {
        count = GetConstVal (IRInstGetOperand (I, 2)->var, 0)->cvValue.cvIval;
        for(  curs=(IRInst *) InterCodeGetCursor (I->bb->cfg->code, I)
           ;  curs!=NULL && count > 0
           ;  curs = (IRInst *) List_Prev((void *)curs), count--
           )
            ;
        for(
           ;  *curs!=I
           ;  curs = (IRInst *) List_Next((void *)curs)
           )
        {
            vnode = varpool_get_node (set, IRInstGetOperand (*curs, 0));
            v = ((struct variable_info *) vnode->param)->v;
            *(int *) List_NewLast (E->Operands, sizeof (int)) = v;
        }
    }
    return (E);
}

static void
deleteExpression (struct Expression *E)
{
    List_Destroy (&E->Operands);
    free (E);
}

static int InsertVex(OLGraph *G)
{ /* 初始条件: 有向图G存在,v和有向图G中顶点有相同特征 */
   /* 操作结果: 在有向图G中增添新顶点v(不增添与顶点相关的弧,留待InsertArc()去做) */
    if ((*G). vexnum >= G->allocated) 
    {
        int new_size = (*G). vexnum * 2 + 1;
        (*G). xlist = (VexNode *) xrealloc ((*G). xlist, new_size * sizeof (VexNode));
        G->allocated = new_size;
    }
    (*G). xlist[(*G). vexnum].firstin=(*G). xlist[(*G). vexnum].firstout=NULL;
    (*G). xlist[(*G). vexnum].hash = (*G). vexnum;
    (*G). xlist[(*G). vexnum].refcnt = 0;
    (*G). xlist[(*G). vexnum].trueval = (*G). vexnum;
    (*G). vexnum++;
    return (*G). vexnum - 1;
}

static void InsertArc(OLGraph *G,int i,int j)
{  /* 初始条件: 有向图G存在,v和w是G中两个顶点 */
   /* 操作结果: 在G中增添弧<i,j> */
    ArcBox *p;
    p=(ArcBox *)xmalloc(sizeof(ArcBox)); /* 生成新结点 */
    p->tailvex=i; /* 给新结点赋值 */
    p->headvex=j;
    p->hlink=(*G). xlist[j].firstin; /* 插在入弧和出弧的链头 */
    p->tlink=(*G). xlist[i].firstout;
    (*G). xlist[j].firstin=(*G). xlist[i].firstout=p;
    (*G). arcnum++; /* 弧数加1 */
}

static void CreateDG(OLGraph *G, OLGraph* old)
{ /* 采用十字链表存储表示,构造有向图G。算法7.3 */
    int i;
    IRInst *curs;

    memset (G, 0, sizeof (*G));
    G->_entries = avl_create ((avl_comparison_func *) compare2, NULL, NULL);
    G->identities = List_Create ();
    if  (old)
    {
        for(  curs=List_Last(old->identities) 
           ;  curs!=NULL 
           ;  curs = List_Prev((void *)curs) 
           )
            *(IRInst *) List_NewFirst (G->identities, sizeof (IRInst)) = *curs;
        G->allocated = old->vexnum;
        G->vexnum = old->vexnum;
        G->xlist = (VexNode *) xmalloc (G->vexnum * sizeof (VexNode));
        memcpy (G->xlist, old->xlist, G->vexnum * sizeof (VexNode));
        for (i = G->vexnum - 1; i >= 0; i--)
            G->xlist[i].firstin = G->xlist[i].firstout = NULL;
    }
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
            free(q);
        }
    }
    free (G->xlist);
    avl_destroy (G->_entries, (avl_item_func *) deleteExpression);
    List_Destroy (&G->identities);
    (*G).arcnum=0;
    (*G).vexnum=0;
    (*G).allocated=0;
}

static VexNode* GetVex(OLGraph *G,int v)
 { /* 初始条件:有向图G存在,v是G中某个顶点的序号。操作结果:返回v的值 */
   return &G->xlist[v];
 }

static void Display(OLGraph *G, varpool_node_set set, const char *file)
 { /* 输出有向图G */
    FILE *fp;
    char *name;
    struct avl_traverser trav;
    varpool_node iter;
    int count = 0;
    int width;

/* printf("共%d个顶点,%d条弧:\n",G.vexnum,G.arcnum); */

    fp=fopen("psg.gv","w");
    if (NULL == fp)
    {
        fprintf (stderr, "error opening psg.gv\n");
        return;
    }

    fputs ("digraph \"\"\n {\n", fp);

#if 1
    {
        int i;
        ArcBox *p;

        fputs ("  subgraph cluster01\n", fp);
        fputs ("   {\n", fp);
        fputs ("    fontname=\"Helvetica,Arial,sans-serif\"\n", fp);
        fputs ("    node [fontname=\"Helvetica,Arial,sans-serif\"]\n", fp);
        fputs ("    edge [fontname=\"Helvetica,Arial,sans-serif\"]\n", fp);

        for(i=0;i<G->vexnum;i++)
        {
            if  (! G->xlist[i].firstout &&
                 ! G->xlist[i].firstin)
                continue;

/*          printf("顶点%s: 入度: ",G.xlist[i].data); */
            fprintf(fp, "  n%03d [ label =<<table border=\"0\" cellborder=\"0\" cellpadding=\"3\" bgcolor=\"white\">", i);
            fprintf(fp, "<tr><td align=\"left\">");
            if      (i == undef)
            {
                fprintf (fp, "undef");
            }
            else if (i == overdefined)
            {
                fprintf (fp, "overdefined");
            }
            else if (! G->xlist[i].firstout)
            {
                fprintf (fp, "<SUP>%d</SUP>", i);
            }
            else
            {
                fprintf (fp, "%s<SUP>%d</SUP>", IRInstGetOperatorSpelling (G->xlist[i].Opcode), i);
            }
            fprintf(fp, " </td></tr>");
            fprintf(fp, "</table>> ];\n");
    
/*          printf("出度: ");  */
            p=G->xlist[i].firstout;
            while(p)
            {
                fprintf(fp, "   n%03d -> n%03d ;\n", i, p->headvex);
                p=p->tlink;
            }
/*          printf("\n");  */
        }
        fputs ("   }\n\n", fp);
    }
#endif

    fputs ("  subgraph not_a_cluster {\n", fp);
    fputs ("  aHtmlTable [\n", fp);
    fputs ("   shape=plaintext\n", fp);
    fputs ("   label=<\n\n", fp);
    fputs ("     <table border='1' cellborder='0'>\n", fp);

    for(  iter=(varpool_node) avl_t_first (&trav, set->nodes)
       ;  iter!=NULL
       ;  iter = (varpool_node) avl_t_next (&trav)
       )
        if  (iter->var->sdSymKind == SYM_VAR && iter->param)
            count++;

    width = (int) ceil (sqrt (count * 1.0 / 2));

    for(  iter=(varpool_node) avl_t_first (&trav, set->nodes), count = 0
       ;  iter!=NULL
       ;  iter = (varpool_node) avl_t_next (&trav)
       )
    {
        if  (iter->var->sdSymKind == SYM_VAR && iter->param)
        {
            if  (count % width == 0)
                fputs ("       <tr>", fp);
            count++;
            fprintf (fp, "<td>%s<SUB>%d</SUB> : %d    </td>", stGetSymName(iter->var), iter->version, ((struct variable_info *)iter->param)->v);
            if  (count % width == 0)
                fputs ("</tr>\n", fp);
        }
    }
    if  (count % width != 0)
        fputs ("</tr>\n", fp);

    fputs ("     </table>\n\n", fp);
    fputs ("  >];\n\n", fp);
    fputs ("   }\n", fp);

    fputs ("}\n", fp);
    fclose (fp);

    name = (char *) xmalloc (strlen(file) + 64);
    sprintf( name, "dot -Tpdf psg.gv > \"%s\"", file );
    if (system(name) != 0) {
        fprintf( stderr, "Error executing %s\n", name );
        free (name);
        return;
    }

    free (name);
#if defined (_WIN32)
    _unlink("psg.gv");
#else
    unlink("psg.gv");
#endif
}

static BOOL
is_relevant (IRInst insn)
{
    switch (insn->opcode)
    {
    case IRINST_OP_nop: case IRINST_OP_load: case IRINST_OP_aload:
    case IRINST_OP_store: case IRINST_OP_astore: case IRINST_OP_move:
    case IRINST_OP_add: case IRINST_OP_addptr: case IRINST_OP_sub:
    case IRINST_OP_mul: case IRINST_OP_div: case IRINST_OP_rem:
    case IRINST_OP_neg: case IRINST_OP_i2f: case IRINST_OP_f2i:
    case IRINST_OP_eq: case IRINST_OP_ne: case IRINST_OP_lt:
    case IRINST_OP_ge: case IRINST_OP_gt: case IRINST_OP_le:
    case IRINST_OP_not: case IRINST_OP_param: case IRINST_OP_entry:
    case IRINST_OP_exit: case IRINST_OP_phi: case IRINST_OP_begin_block:
    case IRINST_OP_end_block: case IRINST_OP_lsl: case IRINST_OP_lsr:
    case IRINST_OP_asr:
        return FALSE;

    case IRINST_OP_ifeq: case IRINST_OP_ifne: case IRINST_OP_iflt:
    case IRINST_OP_ifge: case IRINST_OP_ifgt: case IRINST_OP_ifle:
    case IRINST_OP_goto: case IRINST_OP_fparam:
        return TRUE;

    case IRINST_OP_call:
        return TRUE;
    }
    return FALSE;
}

static int
find (OLGraph *G, struct Expression* entry)
{
    struct Expression* prev_entry;
    int *curs;

    prev_entry = (struct Expression* ) avl_find (G->_entries, entry);
    if  (prev_entry)
    {
        entry = prev_entry;
    }
    else
    {
        prev_entry = entry;
        entry = (struct Expression *) xmalloc (sizeof (*entry));
        entry->Opcode = prev_entry->Opcode;
        entry->Operands = List_Create ();
        entry->HashVal = InsertVex (G);
        GetVex (G, entry->HashVal)->Opcode = entry->Opcode;
        for(  curs=(int *) List_First(prev_entry->Operands)
           ;  curs!=NULL
           ;  curs = (int *) List_Next((void *)curs)
           )
        {
            *(int *) List_NewLast (entry->Operands, sizeof (int)) = *curs;
            InsertArc (G, entry->HashVal, *curs);
        }
        avl_insert (G->_entries, entry);
    }
    return entry->HashVal;
}

static int
find_insert (OLGraph *G, IRInst x, varpool_node_set set)
{
    struct ssa_name tmp = {0};
    int retval;
    varpool_node a1;
    varpool_node a2;
    struct Expression* entry;
    int *curs;

    /* 交换运算。  */
    if  ((x->opcode == IRINST_OP_add ||
         x->opcode == IRINST_OP_mul) &&
         ((struct variable_info *) varpool_get_node (set, IRInstGetOperand (x, 1)))->v < ((struct variable_info *) varpool_get_node (set, IRInstGetOperand (x, 2)))->v)
    {
        memcpy (&tmp, IRInstGetOperand (x, 1), sizeof (tmp));
        memcpy (IRInstGetOperand (x, 1), IRInstGetOperand (x, 2), sizeof (tmp));
        memcpy (IRInstGetOperand (x, 2), &tmp, sizeof (tmp));
    }

    /* 处理代数恒等式。  */
    switch (x->opcode)
    {
    case IRINST_OP_load:
    case IRINST_OP_store:
    case IRINST_OP_move:
        a1 = varpool_get_node (set, IRInstGetOperand (x, 1));
        retval = ((struct variable_info *)a1->param)->v;
        break;

    case IRINST_OP_add:
        do
        {
            a2 = varpool_get_node (set, IRInstGetOperand (x, 2));
            a1 = varpool_get_node (set, IRInstGetOperand (x, 1));

            if  (((struct variable_info *)a2->param)->v == zero)
            {
                retval = ((struct variable_info *)a1->param)->v;
                break;
            }

            goto doit;
        }
        while (FALSE);
        break;

    case IRINST_OP_sub:
        do
        {
            a2 = varpool_get_node (set, IRInstGetOperand (x, 2));
            a1 = varpool_get_node (set, IRInstGetOperand (x, 1));

            if  (((struct variable_info *)a2->param)->v == zero)
            {
                retval = ((struct variable_info *)a1->param)->v;
                break;
            }

            if  (((struct variable_info *)a1->param)->v == ((struct variable_info *)a2->param)->v &&
                 a1->var->sdType->tdTypeKind == TYP_INT)
            {
                retval = zero;
                break;
            }

            goto doit;
        }
        while (FALSE);
        break;

    case IRINST_OP_mul:
        do
        {
            a2 = varpool_get_node (set, IRInstGetOperand (x, 2));
            a1 = varpool_get_node (set, IRInstGetOperand (x, 1));

            if  (((struct variable_info *)a2->param)->v == one)
            {
                retval = ((struct variable_info *)a1->param)->v;
                break;
            }

            if  (((struct variable_info *)a2->param)->v == zero)
            {
                retval = zero;
                break;
            }

            goto doit;
        }
        while (FALSE);
        break;

    case IRINST_OP_div:
        do
        {
            a2 = varpool_get_node (set, IRInstGetOperand (x, 2));
            a1 = varpool_get_node (set, IRInstGetOperand (x, 1));

            if  (((struct variable_info *)a2->param)->v == one)
            {
                retval = ((struct variable_info *)a1->param)->v;
                break;
            }

            goto doit;
        }
        while (FALSE);
        break;

    default:
        goto doit;
    }

    return retval;

doit:

    entry = createExpression (x, set);
    for(  curs=(int *) List_First(entry->Operands)
       ;  curs!=NULL
       ;  curs = (int *) List_Next((void *)curs)
       )
        if  (*curs == overdefined)
            break;
    if  (curs != NULL)
        retval = overdefined;
    else
        retval = find (G, entry);

    deleteExpression (entry);

    return retval;
}

static void
BuildDUChains (LIST blocks, varpool_node_set set)
{
    basic_block *block;
    IRInst *Cursor;
    int count;
    int i1, i2;
    edge *ei;
    varpool_node src;
    varpool_node dest;

    for(  block=(basic_block *)List_First(blocks)
        ;  block!=NULL
        ;  block = (basic_block *)List_Next((void *)block)
        )
    {
        for(  Cursor=(IRInst *)List_First((*block)->insns)
            ;  Cursor!=NULL
            ;  Cursor = (IRInst *)List_Next((void *)Cursor)
            )
        {
            if  (! is_relevant (*Cursor) &&
                 (*Cursor)->opcode != IRINST_OP_phi)
            {
                count = IRInstGetNumOperands(*Cursor);
                for (i1 = 0; i1 < count; ++i1)
                {
                    if  (! IRInstIsOutput(*Cursor, i1))
                    {
                        src = varpool_get_node (set, IRInstGetOperand (*Cursor, i1));
                        for (i2 = 0; i2 < count; ++i2)
                        {
                            if  (IRInstIsOutput(*Cursor, i2))
                            {
                                dest = varpool_get_node (set, IRInstGetOperand (*Cursor, i2));
                                bitmap_set_bit (((struct variable_info *)src->param)->uses, dest->uid);
                            }
                        }
                    }
                }
            }
        }
    }

    for(  block=(basic_block *)List_First(blocks)
        ;  block!=NULL
        ;  block = (basic_block *)List_Next((void *)block)
        )
    {
        for(  Cursor=(IRInst *)List_First((*block)->insns)
            ;  Cursor!=NULL
            ;  Cursor = (IRInst *)List_Next((void *)Cursor)
            )
        {
            if  ((*Cursor)->opcode == IRINST_OP_phi)
            {
                count = IRInstGetNumOperands(*Cursor);
                dest = varpool_get_node (set, IRInstGetOperand (*Cursor, 0));
                for (i1 = 1; i1 < count; ++i1)
                {
                    src = varpool_get_node (set, IRInstGetOperand (*Cursor, i1));
                    bitmap_set_bit (((struct variable_info *)src->param)->affected, (*block)->index);
                    for(  ei=(edge *)List_First((*block)->preds), i2 = 1
                       ;  ei!=NULL && i2 != i1
                       ;  ei = (edge *)List_Next((void *)ei), ++i2
                       )
                        ;
                    if  (((block_info)(*ei)->dest->param)->adjvex == (*ei)->src)
                        bitmap_set_bit (((struct variable_info *)src->param)->uses, dest->uid);
                }
            }
        }
    }
}

static void
SetCounter (varpool_node node, varpool_node_set set, InterCode code)
{
    bitmap_iterator bi1;
    unsigned i1;
    varpool_node node2;

    ((struct variable_info *)node->param)->cnt = ((struct variable_info *)node->param)->cnt + 1;

    /* Visit v’s successors at the first time  */
    if  (((struct variable_info *)node->param)->cnt == 1)
    {
        for (bmp_iter_set_init (&bi1, ((struct variable_info *)node->param)->uses, 0, &i1);
             bmp_iter_set (&bi1, &i1);
             bmp_iter_next (&bi1, &i1))
        {
            node2 = varpool_node_set_find(set, i1);
            if  (InterCodeGetInstByID (code, bitmap_first_set_bit (node2->_defines))->opcode != IRINST_OP_phi)
                SetCounter (node2, set, code);
        }
    }
}

static void
UpdateVN (struct avl_table *newupdates, control_flow_graph cfun, varpool_node_set set, bitmap changed, OLGraph *G)
{
    struct avl_traverser trav;
    struct pair *iter;
    bitmap wl = BITMAP_XMALLOC ();
    varpool_node node, node2;
    bitmap_iterator bi1;
    unsigned i1;
    int v;
    BOOL bFound;
    IRInst instr;

    for(  node = (varpool_node) avl_t_first (&trav, set->nodes)
       ;  node != NULL
       ;  node = (varpool_node) avl_t_next (&trav)
       )
        if  (node->param)
            ((struct variable_info *)node->param)->cnt = 0;

    /* Count dependence numbers of affected variables  */
    for(  iter = (struct pair *) avl_t_first (&trav, newupdates)
       ;  iter != NULL
       ;  iter = (struct pair *) avl_t_next (&trav)
       )
        SetCounter (iter->var, set, cfun->code);

    /* Update MD for variables in newupdates  */
    for(  iter = (struct pair *) avl_t_first (&trav, newupdates)
       ;  iter != NULL
       ;  iter = (struct pair *) avl_t_next (&trav)
       )
    {
        ((struct variable_info *)iter->var->param)->cnt = ((struct variable_info *)iter->var->param)->cnt - 1;
        ((struct variable_info *)iter->var->param)->v = iter->v;
        bitmap_set_bit (wl, iter->var->uid);
    }

    /* Update MD for other affected variables  */
    while   (! bitmap_empty_p(wl))
    {
        bFound = FALSE;
        for (bmp_iter_set_init (&bi1, wl, 0, &i1);
             bmp_iter_set (&bi1, &i1);
             bmp_iter_next (&bi1, &i1))
        {
            node = varpool_node_set_find (set, i1);
            if  (! ((struct variable_info *)node->param)->cnt)
            {
                bitmap_clear_bit (wl, node->uid);
                bFound = TRUE;
                break;
            }
        }
        if  (!bFound)
        {
            fatal ("internal compiler error");
            node = varpool_node_set_find (set, bitmap_first_set_bit (wl));
            bitmap_clear_bit (wl, node->uid);
        }
        bitmap_ior_into (changed, ((struct variable_info *)node->param)->affected);
        for (bmp_iter_set_init (&bi1, ((struct variable_info *)node->param)->uses, 0, &i1);
             bmp_iter_set (&bi1, &i1);
             bmp_iter_next (&bi1, &i1))
        {
            node2 = varpool_node_set_find (set, i1);
            if  (InterCodeGetInstByID (cfun->code, bitmap_first_set_bit (node2->_defines))->opcode != IRINST_OP_phi)
            {
                ((struct variable_info *)node2->param)->cnt = ((struct variable_info *)node2->param)->cnt - 1;
                if  (! ((struct variable_info *)node2->param)->cnt)
                {
                    /* MD(x) := vn(rhs(def(x)))  */
                    instr = InterCodeGetInstByID (cfun->code, bitmap_first_set_bit (node2->_defines));
                    v = find_insert (G, instr, set);
                    ((struct variable_info *)node2->param)->v = v;
                    bitmap_set_bit (wl, node2->uid);
                }
            }
        }
    }

    BITMAP_XFREE (wl);
}

static IRInst *GetNode(LIST insns, int index)
{
    IRInst *instr;

    for(  instr=(IRInst *) List_First(insns)
       ;  instr!=NULL&&index>0
       ;  instr = (IRInst *) List_Next((void *)instr),--index
       )
        ;
    return instr;
}

static void bubble_sort(basic_block block, varpool_node_set set)
{   /* 将a中整数序列重新排列成自小至大有序的整数序列(起泡排序) */
    BOOL change;
    IRInst tmp_insn;
    int i,j;
    int n;
    IRInst *insn1;
    IRInst *insn2;

    for(  insn1=(IRInst *) List_First (block->insns), n = 0
       ;  insn1!=NULL
       ;  insn1 = (IRInst *) List_Next ((void *)insn1)
       )
    {
        if  ((*insn1)->opcode == IRINST_OP_phi)
        {
            n++;
        }
    }
    for(i=n-1,change=TRUE;i>1&&change;--i)
    {
        change=FALSE;
        for(j=0;j<i;++j)
        {
            insn1 = GetNode (block->insns, j);
            insn2 = GetNode (block->insns, j + 1);
            if(((struct variable_info *)varpool_get_node (set, IRInstGetOperand (*insn1, 0))->param)->v > ((struct variable_info *)varpool_get_node (set, IRInstGetOperand (*insn2, 0))->param)->v)
            {
                tmp_insn = *insn1;
                InterCodeRemoveInst (block->cfg->code, tmp_insn, set);
                InterCodeInsertAfter (block, insn2, tmp_insn, FALSE, set);
                change=TRUE;
            }
        }
    }
}

static void
DetectNewUpdates (bitmap changed, control_flow_graph cfun, varpool_node_set set, struct avl_table *newupdates, OLGraph *G, int *count_ptr)
{
    bitmap_iterator bi1;
    unsigned i1;
    basic_block block;

    for (bmp_iter_set_init (&bi1, changed, 0, &i1);
         bmp_iter_set (&bi1, &i1);
         bmp_iter_next (&bi1, &i1))
    {
        block = lookup_block_by_id(cfun, i1);
        if  (! ((block_info)block->param)->sorted)
        {
            bubble_sort (block, set);
            ((block_info)block->param)->sorted = TRUE;
        }
    }
}

static void
classify_record (OLGraph *G, IRInst instr)
{
    if  (instr->opcode == IRINST_OP_add ||
         instr->opcode == IRINST_OP_sub)
        *(IRInst *) List_NewLast (G->identities, sizeof (IRInst)) = instr;
}

static int
verify_expr (OLGraph *G, varpool_node_set set, IRInst instr, int v)
{
    IRInst *curs;
    int op1, op2;
    varpool_node vnode = NULL;

    if  (instr->opcode == IRINST_OP_add)
    {
        op1 = GetVex (G, ((struct variable_info *) varpool_get_node (set, IRInstGetOperand (instr, 1))->param)->v)->trueval;
        op2 = GetVex (G, ((struct variable_info *) varpool_get_node (set, IRInstGetOperand (instr, 2))->param)->v)->trueval;
        for(  curs=(IRInst *) List_Last(G->identities)
           ;  curs!=NULL 
           ;  curs = (IRInst *) List_Prev((void *)curs)
           )
        {
            if  ((*curs)->opcode == IRINST_OP_sub &&
#if 0
                 /* Do nothing special.  */
#else
                 IRInstGetOperand (*curs, 2)->var->sdType->tdTypeKind != TYP_FLOAT &&
#endif
                 GetVex (G, ((struct variable_info *) varpool_get_node (set, IRInstGetOperand (*curs, 2))->param)->v)->trueval == op1 &&
                 GetVex (G, ((struct variable_info *) varpool_get_node (set, IRInstGetOperand (*curs, 0))->param)->v)->trueval == op2)
            {
                vnode = varpool_get_node (set, IRInstGetOperand (*curs, 1));
                GetVex (G, v)->trueval = GetVex (G, ((struct variable_info *) vnode->param)->v)->trueval;
                break;
            }
            if  ((*curs)->opcode == IRINST_OP_sub &&
#if 0
                 /* Do nothing special.  */
#else
                 IRInstGetOperand (*curs, 2)->var->sdType->tdTypeKind != TYP_FLOAT &&
#endif
                 GetVex (G, ((struct variable_info *) varpool_get_node (set, IRInstGetOperand (*curs, 2))->param)->v)->trueval == op2 &&
                 GetVex (G, ((struct variable_info *) varpool_get_node (set, IRInstGetOperand (*curs, 0))->param)->v)->trueval == op1)
            {
                vnode = varpool_get_node (set, IRInstGetOperand (*curs, 1));
                GetVex (G, v)->trueval = GetVex (G, ((struct variable_info *) vnode->param)->v)->trueval;
                break;
            }
            if  ((*curs)->opcode == IRINST_OP_add &&
#if 0
                 /* Do nothing special.  */
#else
                 IRInstGetOperand (*curs, 2)->var->sdType->tdTypeKind != TYP_FLOAT &&
#endif
                 GetVex (G, ((struct variable_info *) varpool_get_node (set, IRInstGetOperand (*curs, 1))->param)->v)->trueval == op2 &&
                 GetVex (G, ((struct variable_info *) varpool_get_node (set, IRInstGetOperand (*curs, 2))->param)->v)->trueval == op1)
            {
                vnode = varpool_get_node (set, IRInstGetOperand (*curs, 0));
                GetVex (G, v)->trueval = GetVex (G, ((struct variable_info *) vnode->param)->v)->trueval;
                break;
            }
        }
    }
    else if (instr->opcode == IRINST_OP_sub)
    {
        op1 = GetVex (G, ((struct variable_info *) varpool_get_node (set, IRInstGetOperand (instr, 1))->param)->v)->trueval;
        op2 = GetVex (G, ((struct variable_info *) varpool_get_node (set, IRInstGetOperand (instr, 2))->param)->v)->trueval;
        for(  curs=(IRInst *) List_Last(G->identities)
           ;  curs!=NULL 
           ;  curs = (IRInst *) List_Prev((void *)curs)
           )
        {
            if  ((*curs)->opcode == IRINST_OP_add &&
#if 0
                 /* Do nothing special.  */
#else
                 IRInstGetOperand (*curs, 2)->var->sdType->tdTypeKind != TYP_FLOAT &&
#endif
                 GetVex (G, ((struct variable_info *) varpool_get_node (set, IRInstGetOperand (*curs, 2))->param)->v)->trueval == op2 &&
                 GetVex (G, ((struct variable_info *) varpool_get_node (set, IRInstGetOperand (*curs, 0))->param)->v)->trueval == op1)
            {
                vnode = varpool_get_node (set, IRInstGetOperand (*curs, 1));
                GetVex (G, v)->trueval = GetVex (G, ((struct variable_info *) vnode->param)->v)->trueval;
                break;
            }
            if  ((*curs)->opcode == IRINST_OP_add &&
#if 0
                 /* Do nothing special.  */
#else
                 IRInstGetOperand (*curs, 2)->var->sdType->tdTypeKind != TYP_FLOAT &&
#endif
                 GetVex (G, ((struct variable_info *) varpool_get_node (set, IRInstGetOperand (*curs, 1))->param)->v)->trueval == op2 &&
                 GetVex (G, ((struct variable_info *) varpool_get_node (set, IRInstGetOperand (*curs, 0))->param)->v)->trueval == op1)
            {
                vnode = varpool_get_node (set, IRInstGetOperand (*curs, 2));
                GetVex (G, v)->trueval = GetVex (G, ((struct variable_info *) vnode->param)->v)->trueval;
                break;
            }
            if  ((*curs)->opcode == IRINST_OP_sub &&
#if 0
                 /* Do nothing special.  */
#else
                 IRInstGetOperand (*curs, 2)->var->sdType->tdTypeKind != TYP_FLOAT &&
#endif
                 GetVex (G, ((struct variable_info *) varpool_get_node (set, IRInstGetOperand (*curs, 1))->param)->v)->trueval == op1 &&
                 GetVex (G, ((struct variable_info *) varpool_get_node (set, IRInstGetOperand (*curs, 0))->param)->v)->trueval == op2)
            {
                vnode = varpool_get_node (set, IRInstGetOperand (*curs, 2));
                GetVex (G, v)->trueval = GetVex (G, ((struct variable_info *) vnode->param)->v)->trueval;
                break;
            }
        }
    }

    if  (GetVex (G, v)->subst == NULL &&
         vnode != NULL)
    {
        GetVex (G, v)->subst = vnode;
        GetVex (G, v)->refcnt++;
    }

    return GetVex (G, v)->trueval;
}

static void
eliminateInstructions (basic_block block, varpool_node_set set, OLGraph *G)
{
    IRInst *instr;
    IRInst *next_insn;
    IRInst tmp_insn;
    int count;
    int i;
    int x;
    OLGraph *_current_map = &((block_info)block->param)->g;
    varpool_node output; 

    for(  instr=(IRInst *) List_First (block->insns)
       ;  instr!=NULL
       ;  instr = next_insn
       )
    {
        next_insn = (IRInst *) List_Next ((void *)instr);

        for (i = 0, count = IRInstGetNumOperands (*instr); i < IRInstGetNumOperands (*instr); i++)
            if  (! IRInstIsOutput (*instr, i))
                count--;

        if  (count == 1 &&
             ! is_relevant (*instr) &&
             (*instr)->opcode != IRINST_OP_store &&
             (*instr)->opcode != IRINST_OP_phi)
        {
            x = find_insert (G, *instr, set);
            if  (x >= _current_map->vexnum)
                fatal ("internal compiler error");

            output = NULL;
            for (i = 0; i < IRInstGetNumOperands (*instr); i++)
            {
                if  (IRInstIsOutput (*instr, i))
                {
                    output = varpool_get_node (set, IRInstGetOperand (*instr, i));
                    break;
                }
            }

            if  (x == overdefined ||
                 output == NULL ||
                 output->var->sdSymKind != SYM_VAR ||
                 output->var->sdType->tdTypeKind > TYP_lastIntrins)
                continue;

            x = verify_expr (_current_map, set, *instr, x);

            if  (! GetVex (_current_map, x)->refcnt)
            {
                GetVex (_current_map, x)->subst = output;
                GetVex (_current_map, x)->refcnt++;
                classify_record (_current_map, *instr);
            }
            else if (GetVex (_current_map, x)->refcnt > 0 &&
                     !(((*instr)->opcode == IRINST_OP_load || (*instr)->opcode == IRINST_OP_store) &&
                     IRInstGetOperand (*instr, 1)->var->sdVar.sdvConst))
            {
                tmp_insn = IRInstEmitInst (output->var->sdVar.sdvLocal ? IRINST_OP_store : IRINST_OP_move, (*instr)->line, (*instr)->column);
                IRInstSetOperand (tmp_insn, 0, output->var);
                IRInstGetOperand (tmp_insn, 0)->version = output->version;
                IRInstSetOperand (tmp_insn, 1, GetVex(_current_map, x)->subst->var);
                IRInstGetOperand (tmp_insn, 1)->version = GetVex(_current_map, x)->subst->version;
                InterCodeInsertAfter (block, instr, tmp_insn, TRUE, set);
                tmp_insn = *instr;
                InterCodeRemoveInst (block->cfg->code, tmp_insn, set);
                IRInstDelInst (tmp_insn);
                GetVex (_current_map, x)->refcnt++;
            }
        }
    }
}


/****************************************************************/
/*************************** PUBLIC FUNCTIONS *******************/
/****************************************************************/


void
GlobalValueNumbering (InterCode code, SymTab stab)
{
    control_flow_graph *F;
    LIST blocks;
    basic_block *block;
    varpool_node_set set;
    varpool_node vnode;
    struct avl_traverser trav;
    OLGraph g;
    struct avl_table *newupdates;
    struct pair *node;
    bitmap changed;
    struct avl_table *leaves;
    int count;
    int num_vars;
    IRInst *instr;
    basic_block dominator;
    struct ssa_name name = {0};

    for(  F=(control_flow_graph *)List_First(code->funcs)
       ;  F!=NULL
       ;  F = (control_flow_graph *)List_Next((void *)F)
       )
    {
        set = varpool_node_set_new (*F, TRUE);
        blocks = List_Create ();
        CreateDG (&g, NULL);
        newupdates = avl_create ((avl_comparison_func *) compare_pairs, NULL, NULL);
        changed = BITMAP_XMALLOC ();
        leaves = avl_create ((avl_comparison_func *) compare, NULL, NULL);

        /* undef  */
        InsertVex (&g);
        /* one  */
        name.var = stCreateIconNode (stab, 1);
        varpool_node_set_add (set, &name);
        InsertVex (&g);
        /* zero  */
        name.var = stCreateIconNode (stab, 0);
        varpool_node_set_add (set, &name);
        InsertVex (&g);
        /* overdefined  */
        InsertVex (&g);

        pre_and_rev_post_order_compute (*F, NULL, blocks, TRUE, FALSE);

        for(  block=(basic_block *)List_First(blocks)
           ;  block!=NULL
           ;  block = (basic_block *)List_Next((void *)block)
           )
        {
            (*block)->param = xmalloc (sizeof (struct block_info_def));
            memset ((*block)->param, 0, sizeof (struct block_info_def));
        }

        for(  vnode = (varpool_node) avl_t_first (&trav, set->nodes)
           ;  vnode != NULL
           ;  vnode = (varpool_node) avl_t_next (&trav)
           )
        {
            vnode->param = xmalloc (sizeof (struct variable_info));
            memset (vnode->param, 0, sizeof (struct variable_info));
            ((struct variable_info *)vnode->param)->uses = BITMAP_XMALLOC ();
            ((struct variable_info *)vnode->param)->affected = BITMAP_XMALLOC ();
            ((struct variable_info *)vnode->param)->v = undef;
        }

        compute_dominators (*F, FALSE);

        for(  block=(basic_block *)List_First(blocks)
           ;  block!=NULL
           ;  block = (basic_block *)List_Next((void *)block)
           )
            ((block_info) (*block)->param)->adjvex = get_immediate_dominator (*F, FALSE, *block);

        BuildDUChains (blocks, set);

        for(  vnode = (varpool_node) avl_t_first (&trav, set->nodes), num_vars = 0
           ;  vnode != NULL
           ;  vnode = (varpool_node) avl_t_next (&trav)
           )
        {
            node = NULL;
            if  (! bitmap_empty_p (vnode->_defines) &&
                 (is_relevant (InterCodeGetInstByID ((*F)->code, bitmap_first_set_bit (vnode->_defines))) ||
                 InterCodeGetInstByID ((*F)->code, bitmap_first_set_bit (vnode->_defines))->opcode == IRINST_OP_phi))
            {
                node = (struct pair *) xmalloc (sizeof (struct pair));
                node->var = vnode;
                node->v = InsertVex(&g);
                free (avl_replace (newupdates, node));
            }
            if  (vnode->var->sdSymKind == SYM_VAR &&
                 bitmap_empty_p (vnode->_defines) &&
                 !vnode->var->sdVar.sdvConst &&
                 (!vnode->var->sdVar.sdvLocal || vnode->var->sdType->tdTypeKind > TYP_lastIntrins) &&
                 !is_global_var (vnode->var) &&
                 node == NULL)
            {
                node = (struct pair *) xmalloc (sizeof (struct pair));
                node->var = vnode;
                node->v = InsertVex(&g);
                free (avl_replace (newupdates, node));
            }
            if  (vnode->var->sdSymKind == SYM_VAR &&
                 vnode->var->sdVar.sdvConst &&
                 node == NULL)
            {
                /* 操作数是0或1，可用于代数恒等式。  */
                if  (vnode->var->sdType->tdTypeKind == TYP_INT &&
                     GetConstVal(vnode->var, 0)->cvValue.cvIval == 0)
                {
                    node = (struct pair *) xmalloc (sizeof (struct pair));
                    node->var = vnode;
                    node->v = zero;
                    free (avl_replace (newupdates, node));
                }
                else if (vnode->var->sdType->tdTypeKind == TYP_INT &&
                         GetConstVal(vnode->var, 0)->cvValue.cvIval == 1)
                {
                    node = (struct pair *) xmalloc (sizeof (struct pair));
                    node->var = vnode;
                    node->v = one;
                    free (avl_replace (newupdates, node));
                }
            }
            if  (vnode->var->sdSymKind == SYM_VAR &&
                 (vnode->var->sdVar.sdvArgument ||
                 vnode->var->sdVar.sdvConst ||
                 is_global_var (vnode->var)) &&
                 node == NULL)
            {
                node = (struct pair *) xmalloc (sizeof (struct pair));
                node->var = vnode;
                node->v = InsertVex (&g);
                free (avl_replace (newupdates, node));
            }
            if  (vnode->var->sdSymKind == SYM_VAR &&
                 !vnode->var->sdVar.sdvConst)
            {
                num_vars++;
            }
        }

        for(  block=(basic_block *)List_First(blocks), count = 0
           ;  block!=NULL
           ;  block = (basic_block *)List_Next((void *)block)
           )
        {
            for(  instr=(IRInst *) List_First ((*block)->insns)
               ;  instr!=NULL
               ;  instr = (IRInst *) List_Next ((void *)instr)
               )
            {
                count = count + num_vars + 1;
            }
        }

        while   (avl_count (newupdates))
        {
            UpdateVN (newupdates, *F, set, changed, &g);
            avl_destroy (newupdates, (avl_item_func *) free);
            newupdates = avl_create((avl_comparison_func *) compare_pairs, NULL, NULL);
            DetectNewUpdates (changed, *F, set, newupdates, &g, &count);
            bitmap_clear (changed);
        }

#if !defined(NDEBUG)

#if 0
        if (1)
        {
            /* 转储VNDAG。  */
            char *lpTmpFile = NULL;
            lpTmpFile = (char *)xmalloc (strlen(comp->cmpConfig.input_file_name)+strlen(stGetSymName(IRInstGetOperand(*(IRInst *)List_First((*F)->entry_block_ptr->insns), 0)->var))+64);
            strcpy (lpTmpFile, comp->cmpConfig.input_file_name);
            strcat (lpTmpFile, "-");
            strcat (lpTmpFile, stGetSymName(IRInstGetOperand(*(IRInst *)List_First((*F)->entry_block_ptr->insns), 0)->var));
            strcat (lpTmpFile, ".vndag.pdf");
            Display (&g, set, lpTmpFile);
            free (lpTmpFile);
        }
#endif

#endif

        CreateDG (&((block_info)(*F)->entry_block_ptr->param)->g, &g);
        for(  block=(basic_block *)List_First(blocks)
           ;  block!=NULL
           ;  block = (basic_block *)List_Next((void *)block)
           )
        {
            if  (*block == (*F)->entry_block_ptr)
                continue;

            dominator = ((block_info)(*block)->param)->adjvex;
            CreateDG (&((block_info)(*block)->param)->g, &((block_info)dominator->param)->g);
            eliminateInstructions (*block, set, &g);
        }

        for(  vnode = (varpool_node) avl_t_first (&trav, set->nodes)
           ;  vnode != NULL
           ;  vnode = (varpool_node) avl_t_next (&trav)
           )
        {
            if  (vnode->param)
            {
                BITMAP_XFREE (((struct variable_info *)vnode->param)->uses);
                BITMAP_XFREE (((struct variable_info *)vnode->param)->affected);
                free (vnode->param);
                vnode->param = NULL;
            }
        }

        for(  block=(basic_block *)List_First(blocks)
           ;  block!=NULL
           ;  block = (basic_block *)List_Next((void *)block)
           )
        {
            DestroyGraph (&((block_info)(*block)->param)->g);
            free ((*block)->param);
            (*block)->param = NULL;
        }

        avl_destroy (leaves, (avl_item_func *) deleteExpression);
        BITMAP_XFREE (changed);
        avl_destroy (newupdates,  (avl_item_func *) free);
        DestroyGraph (&g);
        List_Destroy (&blocks);
        free_varpool_node_set (set);
    }
}
