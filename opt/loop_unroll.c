#include <stdlib.h>
#include "all.h"

#define THRESHOLD 1000
#define MAX_ITERATIONS 50

#ifndef min
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif

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

/* 给定一个头结点为h，回边为si->h的循环L，可以按以下方式展开:
   1) 复制结点，构建一个头结点为h'，回边为si'->h'的循环L'；
   2) 将循环L中所有从si->h的回边改为si->h'；
   3) 将循环L'中所有从si'->h'的回边改为si'->h。  */
static void
LoopUnroll (control_flow_graph fn, struct loop *ls, int max_iterations)
{
    basic_block* curs;
    basic_block block, block2;
    struct avl_table *block_map;
    struct pair *p;
    struct loop **loop;
    edge *ei, *ei2;
    LIST blocks;
    LIST new_blocks;
    basic_block header;
    int its;
    int count;

    flow_loops_find (fn);
    for(  loop=(struct loop **)List_Last(fn->loops->larray)
       ;  loop!=NULL
       ;  loop = (struct loop **) List_Prev((void *)loop)
       )
        if ((*loop)->header == ls->header)
            break;

    blocks = get_loop_body (*loop);

    for(  curs=(basic_block *) List_First(blocks), count = 1
       ;  curs!=NULL
       ;  curs = (basic_block *) List_Next((void *)curs)
       )
        count += List_Card ((*curs)->insns);
    max_iterations = min (max_iterations, THRESHOLD / count);

    for (its = 0;
         its < max_iterations;
         its++)
    {
        /* 复制基本块。  */
        new_blocks = List_Create ();
        block_map = avl_create ((avl_comparison_func *) compare, NULL, NULL);
        for(  curs=(basic_block *) List_First(blocks)
           ;  curs!=NULL
           ;  curs = (basic_block *) List_Next((void *)curs)
           )
        {
            block = alloc_block ();
            link_block (fn, block);
            copy_block (block, *curs);
            p = (struct pair *) xmalloc (sizeof (struct pair));
            p->x = (*curs)->index;
            p->y = block->index;
            avl_insert (block_map, p);
            *(basic_block *) List_NewLast (new_blocks, sizeof (basic_block)) = block;
        }

        header = lookup_block_by_id (fn, ((struct pair *) avl_find(block_map, &(*(basic_block *) List_First(blocks))->index))->y);

        /* 连接新创建的基本块。  */
        for(  curs=(basic_block *) List_First(blocks)
           ;  curs!=NULL
           ;  curs = (basic_block *) List_Next((void *)curs)
           )
        {
            block = lookup_block_by_id (fn, ((struct pair *) avl_find(block_map, &(*curs)->index))->y);
            for(  ei=(edge *) List_First((*curs)->succs)
               ;  ei!=NULL
               ;  ei = (edge *) List_Next((void *)ei)
               )
            {
                p = (struct pair *) avl_find(block_map, &(*ei)->dest->index);
                block2 = (p == NULL ? (*ei)->dest : lookup_block_by_id (fn, p->y));
                make_edge (block, block2);
            }
        }

        for(  curs=(basic_block *) List_First(blocks)
           ;  curs!=NULL
           ;  curs = (basic_block *) List_Next((void *)curs)
           )
        {
            for(  ei=(edge *) List_First((*curs)->succs)
               ;  ei!=NULL
               ;  ei = ei2
               )
            {
                ei2 = (edge *) List_Next((void *)ei);
                if  ((*ei)->dest == ls->header)
                {
                    make_edge ((*ei)->src, header);
                    remove_edge (*ei);
                }
            }
        }

        for(  curs=(basic_block *) List_First(blocks)
           ;  curs!=NULL
           ;  curs = (basic_block *) List_Next((void *)curs)
           )
        {
            block = lookup_block_by_id (fn, ((struct pair *) avl_find(block_map, &(*curs)->index))->y);
            for(  ei=(edge *) List_First(block->succs)
               ;  ei!=NULL
               ;  ei = ei2
               )
            {
                ei2 = (edge *) List_Next((void *)ei);
                if  ((*ei)->dest == header)
                {
                    make_edge ((*ei)->src, ls->header);
                    remove_edge (*ei);
                }
            }
        }

        /* 更新跳转的目标指令。  */
        for(  curs=(basic_block *) List_First(blocks)
           ;  curs!=NULL
           ;  curs = (basic_block *) List_Next((void *)curs)
           )
        {
            block = lookup_block_by_id (fn, ((struct pair *) avl_find(block_map, &(*curs)->index))->y);
            update_destinations (block, (*(IRInst *)List_First((*curs)->insns))->uid, (*(IRInst *)List_First(block->insns))->uid, NULL);
        }
        update_destinations (header, (*(IRInst *)List_First(ls->header->insns))->uid, (*(IRInst *)List_First(header->insns))->uid, NULL);

        avl_destroy (block_map, (avl_item_func *) free);
        List_Destroy (&blocks);
        blocks = new_blocks;
        new_blocks = NULL;
    }

    List_Destroy (&blocks);
    flow_loops_free (&fn->loops);
}

void
unroll_loops (InterCode code, SymTab stab)
{
    control_flow_graph *F;
    struct loop **loop;
    struct loops *current_loops;

    for(  F=(control_flow_graph *)List_First(code->funcs)
       ;  F!=NULL
       ;  F = (control_flow_graph *)List_Next((void *)F)
       )
    {
        flow_loops_find (*F);
        current_loops = (*F)->loops;
        (*F)->loops =NULL;
/*      flow_loops_dump (current_loops, stdout); */
        for(  loop=(struct loop **)List_Last(current_loops->larray)
           ;  loop!=NULL
           ;  loop = (struct loop **) List_Prev((void *)loop)
           )
        {
            if ((*loop)->header == (*F)->entry_block_ptr)
                continue;
            LoopUnroll (*F, *loop, 15);
        }
        flow_loops_free (&current_loops);
    }
}
