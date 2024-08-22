#include <stdlib.h>
#include "all.h"

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

void
GlobalVariableLocalization (InterCode code, SymTab stab)
{
    control_flow_graph *F;
    basic_block* block, target_block;
    IRInst *instr, *next, tmp_insn;
    varpool_node_set set;
    varpool_node vnode;
    struct avl_traverser trav;
    SymDef sym, tmpSym, *curs;
    struct pair *p;
    struct avl_table *map;
    int i;
    bitmap temp_bitmap;
    ssa_name name;
    control_flow_graph   entry_point = NULL;
    bitmap_iterator bi;
    unsigned k;
    LIST globals;

    /* Step 1 :
       ------------------------------------------------------------------------
       将只在入口函数中出现的全局变量改成局部变量。
       ------------------------------------------------------------------------
    */

    temp_bitmap = BITMAP_XMALLOC ();
    for(  F=(control_flow_graph *)List_First(code->funcs)
       ;  F!=NULL
       ;  F = (control_flow_graph *)List_Next((void *)F)
       )
    {
        if  (IRInstGetOperand (*(IRInst *) List_First ((*F)->entry_block_ptr->insns), 0)->var->sdFnc.sdfEntryPt)
        {
            entry_point = *F;
            for(  block=(basic_block*) List_First((*F)->basic_block_info)
               ;  block!=NULL
               ;  block = (basic_block*) List_Next((void *)block)
               )
            {
                for(  instr=(IRInst *) List_First((*block)->insns)
                   ;  instr!=NULL
                   ;  instr = (IRInst *) List_Next((void *)instr)
                   )
                {
                    for (i = IRInstGetNumOperands (*instr) - 1; i >= 0; i--)
                    {
                        name = IRInstGetOperand (*instr, i);
                        if  (name->var->sdSymKind == SYM_VAR &&
                             is_global_var (name->var) &&
                             name->var->sdType->tdTypeKind <= TYP_lastIntrins &&
                             !name->var->sdVar.sdvConst)
                        {
                            bitmap_set_bit (temp_bitmap, name->var->uid);
                        }
                    }
                }
            }
        }
    }
    for(  F=(control_flow_graph *)List_First(code->funcs)
       ;  F!=NULL
       ;  F = (control_flow_graph *)List_Next((void *)F)
       )
    {
        if  (!IRInstGetOperand (*(IRInst *) List_First ((*F)->entry_block_ptr->insns), 0)->var->sdFnc.sdfEntryPt)
        {
            for(  block=(basic_block*) List_First((*F)->basic_block_info)
               ;  block!=NULL
               ;  block = (basic_block*) List_Next((void *)block)
               )
            {
                for(  instr=(IRInst *) List_First((*block)->insns)
                   ;  instr!=NULL
                   ;  instr = (IRInst *) List_Next((void *)instr)
                   )
                {
                    for (i = IRInstGetNumOperands (*instr) - 1; i >= 0; i--)
                    {
                        name = IRInstGetOperand (*instr, i);
                        if  (name->var->sdSymKind == SYM_VAR &&
                             is_global_var (name->var) &&
                             name->var->sdType->tdTypeKind <= TYP_lastIntrins &&
                             !name->var->sdVar.sdvConst)
                        {
                            bitmap_clear_bit (temp_bitmap, name->var->uid);
                        }
                    }
                }
            }
        }
    }

    map = avl_create ((avl_comparison_func *) compare_pairs, NULL, NULL);
    target_block = (*(edge *) List_First (entry_point->entry_block_ptr->succs))->dest;
    for (bmp_iter_set_init (&bi, temp_bitmap, 0, &k);
         bmp_iter_set (&bi, &k);
         bmp_iter_next (&bi, &k))
    {
        sym = stGetSymByID (stab, k);
        tmpSym = stDeclareSym (stab, NULL, SYM_VAR);
        tmpSym->sdIsImplicit = TRUE;
        tmpSym->sdVar.sdvLocal = TRUE;
        tmpSym->sdType = CopyType (sym->sdType);
        p = (struct pair *) xmalloc (sizeof (struct pair));
        p->x = sym->uid;
        p->y = tmpSym->uid;
        free (avl_replace (map, p));

        tmp_insn = IRInstEmitInst (IRINST_OP_store, (*(IRInst *) List_First(target_block->insns))->line, (*(IRInst *) List_First(target_block->insns))->column);
        IRInstSetOperand (tmp_insn, 0, tmpSym);
        if  (sym->sdType->tdTypeKind == TYP_FLOAT)
            IRInstSetOperand (tmp_insn, 1, stCreateFconNode (stab, GetConstVal (sym, 0) ? GetConstVal (sym, 0)->cvValue.cvFval : 0.0f));
        else
            IRInstSetOperand (tmp_insn, 1, stCreateIconNode (stab, GetConstVal (sym, 0) ? GetConstVal (sym, 0)->cvValue.cvIval : 0));
        InterCodeInsertAfter (target_block, NULL, tmp_insn, TRUE, NULL);
    }

    for(  block=(basic_block*) List_First(entry_point->basic_block_info)
       ;  block!=NULL
       ;  block = (basic_block*) List_Next((void *)block)
       )
    {
        for(  instr=(IRInst *) List_First((*block)->insns)
           ;  instr!=NULL
           ;  instr = (IRInst *) List_Next((void *)instr)
           )
        {
            for (i = IRInstGetNumOperands (*instr) - 1; i >= 0; i--)
            {
                name = IRInstGetOperand (*instr, i);
                p = avl_find (map, &name->var->uid);
                if  (p)
                {
                    IRInstSetOperand (*instr, i, stGetSymByID (stab, p->y));
                }
            }
        }
    }

    for (bmp_iter_set_init (&bi, temp_bitmap, 0, &k);
         bmp_iter_set (&bi, &k);
         bmp_iter_next (&bi, &k))
    {
        sym = stGetSymByID (stab, k);
        stRemoveSym (sym);
    }

    avl_destroy (map, (avl_item_func *) free);
    BITMAP_XFREE (temp_bitmap);

    /* Step 2 :
       ------------------------------------------------------------------------
       将局部变量改成临时变量。
       ------------------------------------------------------------------------
    */

    for(  F=(control_flow_graph *)List_First(code->funcs)
       ;  F!=NULL
       ;  F = (control_flow_graph *)List_Next((void *)F)
       )
    {
        set = varpool_node_set_new (*F, TRUE);
        map = avl_create ((avl_comparison_func *) compare_pairs, NULL, NULL);

        for(  vnode = (varpool_node)avl_t_first (&trav, set->nodes)
           ;  vnode != NULL
           ;  vnode = (varpool_node)avl_t_next (&trav)
           )
        {
            if  (vnode->var->sdSymKind == SYM_VAR &&
                 vnode->var->sdVar.sdvLocal &&
                 ! vnode->var->sdVar.sdvConst &&
                 vnode->var->sdType->tdTypeKind <= TYP_lastIntrins)
            {
                tmpSym = stDeclareSym (stab, NULL, SYM_VAR);
                tmpSym->sdIsImplicit = TRUE;
                tmpSym->sdType = CopyType (vnode->var->sdType);
                p = (struct pair *) xmalloc (sizeof (struct pair));
                p->x = vnode->uid;
                p->y = tmpSym->uid;
                free (avl_replace (map, p));
            }
        }
        for(  block=(basic_block*) List_First((*F)->basic_block_info)
           ;  block!=NULL
           ;  block = (basic_block*) List_Next((void *)block)
           )
        {
            for(  instr=(IRInst *) List_First((*block)->insns)
               ;  instr!=NULL
               ;  instr = (IRInst *) List_Next((void *)instr)
               )
            {
                if      ((*instr)->opcode == IRINST_OP_load)
                {
                    vnode = varpool_get_node (set, IRInstGetOperand (*instr, 1));
                    p = avl_find (map, &vnode->uid);
                    if  (p)
                    {
                        IRInstSetOperand (*instr, 1, stGetSymByID (stab, p->y));
                        IRInstGetOperand (*instr, 1)->version = 0;
                        (*instr)->opcode = IRINST_OP_move;
                    }
                }
                else if ((*instr)->opcode == IRINST_OP_store)
                {
                    vnode = varpool_get_node (set, IRInstGetOperand (*instr, 0));
                    p = avl_find (map, &vnode->uid);
                    if  (p)
                    {
                        IRInstSetOperand (*instr, 0, stGetSymByID (stab, p->y));
                        IRInstGetOperand (*instr, 0)->version = 0;
                        if  (IRInstGetOperand (*instr, 1)->var->sdVar.sdvConst)
                            (*instr)->opcode = IRINST_OP_load;
                        else
                            (*instr)->opcode = IRINST_OP_move;
                    }
                }
                else
                {
                    for (i = IRInstGetNumOperands (*instr) - 1; i >= 0; i--)
                    {
                        vnode = varpool_get_node (set, IRInstGetOperand (*instr, i));
                        p = avl_find (map, &vnode->uid);
                        if  (p)
                        {
                            IRInstSetOperand (*instr, i, stGetSymByID (stab, p->y));
                            IRInstGetOperand (*instr, i)->version = 0;
                        }
                    }
                }
            }
        }

        avl_destroy (map, (avl_item_func *) free);
        free_varpool_node_set (set);
    }

    /* Step 3 :
       ------------------------------------------------------------------------
       删除只定值不使用的全局变量。
       ------------------------------------------------------------------------
    */

    temp_bitmap = BITMAP_XMALLOC ();
    for(  F=(control_flow_graph *)List_First(code->funcs)
       ;  F!=NULL
       ;  F = (control_flow_graph *)List_Next((void *)F)
       )
    {
        for(  block=(basic_block*) List_First((*F)->basic_block_info)
           ;  block!=NULL
           ;  block = (basic_block*) List_Next((void *)block)
           )
        {
            for(  instr=(IRInst *) List_First((*block)->insns)
               ;  instr!=NULL
               ;  instr = (IRInst *) List_Next((void *)instr)
               )
            {
                for (i = IRInstGetNumOperands (*instr) - 1; i >= 0; i--)
                {
                    name = IRInstGetOperand (*instr, i);
                    if  (IRInstIsOutput (*instr, i) &&
                         name->var->sdSymKind == SYM_VAR &&
                         is_global_var (name->var) &&
                         !name->var->sdVar.sdvConst)
                    {
                        bitmap_set_bit (temp_bitmap, name->var->uid);
                    }
                }
            }
        }
    }
    for(  F=(control_flow_graph *)List_First(code->funcs)
       ;  F!=NULL
       ;  F = (control_flow_graph *)List_Next((void *)F)
       )
    {
        for(  block=(basic_block*) List_First((*F)->basic_block_info)
           ;  block!=NULL
           ;  block = (basic_block*) List_Next((void *)block)
           )
        {
            for(  instr=(IRInst *) List_First((*block)->insns)
               ;  instr!=NULL
               ;  instr = (IRInst *) List_Next((void *)instr)
               )
            {
                for (i = IRInstGetNumOperands (*instr) - 1; i >= 0; i--)
                {
                    name = IRInstGetOperand (*instr, i);
                    if  ((*instr)->opcode != IRINST_OP_astore &&
                         !IRInstIsOutput (*instr, i) &&
                         name->var->sdSymKind == SYM_VAR &&
                         is_global_var (name->var) &&
                         !name->var->sdVar.sdvConst)
                    {
                        bitmap_clear_bit (temp_bitmap, name->var->uid);
                    }
                }
            }
        }
    }
    for(  F=(control_flow_graph *)List_First(code->funcs)
       ;  F!=NULL
       ;  F = (control_flow_graph *)List_Next((void *)F)
       )
    {
        for(  block=(basic_block*) List_First((*F)->basic_block_info)
           ;  block!=NULL
           ;  block = (basic_block*) List_Next((void *)block)
           )
        {
            for(  instr=(IRInst *) List_First((*block)->insns)
               ;  instr!=NULL
               ;  instr = next
               )
            {
                next = (IRInst *) List_Next((void *)instr);
                for (i = IRInstGetNumOperands (*instr) - 1; i >= 0; i--)
                {
                    name = IRInstGetOperand (*instr, i);
                    if  (bitmap_bit_p (temp_bitmap, name->var->uid))
                    {
                        tmp_insn = *instr;
                        InterCodeRemoveInst (code, tmp_insn, NULL);
                        IRInstDelInst (tmp_insn);
                        break;
                    }
                }
            }
        }
    }
    for (bmp_iter_set_init (&bi, temp_bitmap, 0, &k);
         bmp_iter_set (&bi, &k);
         bmp_iter_next (&bi, &k))
    {
        sym = stGetSymByID (stab, k);
        stRemoveSym (sym);
    }

    BITMAP_XFREE (temp_bitmap);

    /* Step 4 :
       ------------------------------------------------------------------------
       删除未引用的全局变量。
       ------------------------------------------------------------------------
    */

    temp_bitmap = BITMAP_XMALLOC ();
    globals = get_globals (stab);
    for(  curs=(SymDef *) List_First(globals)
       ;  curs!=NULL
       ;  curs = (SymDef *) List_Next((void *)curs)
       )
        if  ((*curs)->sdSymKind == SYM_VAR &&
             is_global_var (*curs) &&
             !(*curs)->sdVar.sdvConst)
            bitmap_set_bit (temp_bitmap, (*curs)->uid);
    for(  F=(control_flow_graph *)List_First(code->funcs)
       ;  F!=NULL
       ;  F = (control_flow_graph *)List_Next((void *)F)
       )
    {
        for(  block=(basic_block*) List_First((*F)->basic_block_info)
           ;  block!=NULL
           ;  block = (basic_block*) List_Next((void *)block)
           )
        {
            for(  instr=(IRInst *) List_First((*block)->insns)
               ;  instr!=NULL
               ;  instr = (IRInst *) List_Next((void *)instr)
               )
            {
                for (i = IRInstGetNumOperands (*instr) - 1; i >= 0; i--)
                {
                    name = IRInstGetOperand (*instr, i);
                    if  (name->var->sdSymKind == SYM_VAR &&
                         is_global_var (name->var) &&
                         !name->var->sdVar.sdvConst)
                    {
                        bitmap_clear_bit (temp_bitmap, name->var->uid);
                    }
                }
            }
        }
    }
    for (bmp_iter_set_init (&bi, temp_bitmap, 0, &k);
         bmp_iter_set (&bi, &k);
         bmp_iter_next (&bi, &k))
    {
        sym = stGetSymByID (stab, k);
        stRemoveSym (sym);
    }
    List_Destroy (&globals);
    BITMAP_XFREE (temp_bitmap);
}
