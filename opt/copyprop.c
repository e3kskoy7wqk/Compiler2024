#include <stdlib.h>
#include <string.h>
#include "all.h"

typedef struct block_info_def
{
    bitmap Executable;
    basic_block adjvex;
} *block_info;

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
                    bitmap_ior (&tmp, vnode->_uses, vnode->_defines);
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
                    bitmap_ior (&tmp, vnode->_uses, vnode->_defines);
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
