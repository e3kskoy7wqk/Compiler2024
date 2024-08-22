/****************************************************/
/* File: IRCode.c                                   */
/* 中间IR指令序列管理类                             */
/****************************************************/

#include <stdlib.h>
#include <string.h>
#include "all.h"

struct addr_pair
{
  IRInst object;
  IRInst *curs_nobb;
  IRInst *curs;
};

static int
cmp_addr_pair (struct addr_pair *a, struct addr_pair *b, void *p)
{
    if (a->object < b->object) {
        return - 1;

    } else if (a->object > b->object) {
        return 1;

    } else {
        return 0;
    }
}

static int
compare_ids (IRInst a, IRInst b, void *p)
{
    if (a->uid < b->uid)
        return -1;
    else if (a->uid > b->uid)
        return 1;
    return 0;
}

InterCode InterCodeNew ()
{
    InterCode code;

    code = (InterCode)xmalloc(sizeof(*code));

    memset (code, 0, sizeof(*code));
    code->code = List_Create ();
    code->funcs = List_Create ();
    code->id_map = avl_create((avl_comparison_func *)compare_ids, NULL, NULL);
    code->addr_map = avl_create((avl_comparison_func *)cmp_addr_pair, NULL, NULL);
    code->counter = 100;

    return  code;
}

void InterCodeDelete ( InterCode code )
{
    void *Cursor;

    if ( code)
    {
        for(  Cursor=List_Last(code->funcs)
            ;  Cursor!=NULL
            ;  Cursor = List_Last(code->funcs)
            )
            free_cfg(code, *(control_flow_graph *)Cursor);
        for(  Cursor=List_Last(code->code)
            ;  Cursor!=NULL
            ;  Cursor = List_Prev((void *)Cursor)
            )
            IRInstDelInst(*(IRInst *)Cursor);
        List_Destroy(&code->code);
        List_Destroy(&code->funcs);
        avl_destroy(code->addr_map, (avl_item_func *)free);
        avl_destroy(code->id_map, (avl_item_func *)NULL);
        free (code);
    }
}

void InterCodeAddInst_nobb (InterCode code, IRInst insn, BOOL fNewID)
{
    IRInst *Cursor;
    struct addr_pair *pair;

    if ( fNewID )
    {
        insn->uid = code->counter++;
    }
    Cursor = (IRInst *)List_NewLast(code->code, sizeof (IRInst));
    *Cursor = insn;
    pair = (struct addr_pair *) xmalloc (sizeof (*pair));
    memset (pair, '\0', sizeof (*pair));
    pair->object = insn;
    pair->curs_nobb = Cursor;
    avl_insert(code->addr_map, pair);
    avl_insert(code->id_map, insn);
}

IRInst *InterCodeAddExistingInst (basic_block bb, IRInst insn)
{
    struct addr_pair *pair;
    pair = (struct addr_pair *)avl_find(bb->cfg->code->addr_map, &insn);
    if  (pair)
    {
        pair->curs = (IRInst *)List_NewLast(bb->insns, sizeof (IRInst));
        *pair->curs = insn;
        insn->bb = bb;
    }
    return ( pair ? pair->curs : NULL );
}

IRInst *InterCodeAddInst (basic_block bb, IRInst insn, BOOL fNewID)
{
    InterCodeAddInst_nobb (bb->cfg->code, insn, fNewID);
    return InterCodeAddExistingInst (bb, insn);
}

void InterCodeRemoveInst_nobb (InterCode code, IRInst insn)
{
    struct addr_pair *pair;
    if ( insn)
    {
        avl_delete(code->id_map, insn);
        pair = (struct addr_pair *)avl_delete(code->addr_map, &insn);
        if  (pair && pair->curs_nobb)
        {
            List_Delete(pair->curs_nobb);
            pair->curs_nobb = NULL;
        }
        free (pair);
    }
}

void
update_destinations (basic_block bb, int orig_insn, int new_insn, varpool_node_set set)
{
    edge *ei;
    IRInst insn;
    ssa_name name;
    int constVal;

    for(  ei=(edge *)List_First(bb->preds)
       ;  ei!=NULL
       ;  ei = (edge *)List_Next((void *)ei)
       )
    {
        insn = *(IRInst *)List_Last((*ei)->src->insns);
        switch (insn->opcode)
        {
        case IRINST_OP_goto:
            name = IRInstGetOperand (insn, 0);
            constVal = GetConstVal (name->var, 0)->cvValue.cvIval;
            if  (constVal == orig_insn)
            {
                name->var = stCreateIconNode(name->var->stab, new_insn);
                if  (set)
                    varpool_node_set_add(set, name);
            }
            break;

        case IRINST_OP_ifeq:
        case IRINST_OP_ifne:
        case IRINST_OP_ifle:
        case IRINST_OP_iflt:
        case IRINST_OP_ifge:
        case IRINST_OP_ifgt:
            name = IRInstGetOperand (insn, 2);
            constVal = GetConstVal (name->var, 0)->cvValue.cvIval;
            if  (constVal == orig_insn)
            {
                name->var = stCreateIconNode(name->var->stab, new_insn);
                if  (set)
                    varpool_node_set_add(set, name);
            }
            name = IRInstGetOperand (insn, 3);
            constVal = GetConstVal (name->var, 0)->cvValue.cvIval;
            if  (constVal == orig_insn)
            {
                name->var = stCreateIconNode(name->var->stab, new_insn);
                if  (set)
                    varpool_node_set_add(set, name);
            }
            break;

        default:
            fatal ("internal compiler error");
            break;
        }
    }
}

void InterCodeRemoveInst (InterCode code, IRInst insn, varpool_node_set set)
{
    struct addr_pair *pair;
    if ( insn)
    {
        pair = (struct addr_pair *)avl_find(code->addr_map, &insn);
        if  (pair && pair->curs)
        {
            if  (! List_Prev (pair->curs))
            {
                /* 如果删除的是基本块的第一条指令，则需要更新其前驱分支语句的目标指令号。  */
                update_destinations ((*pair->curs)->bb, (*pair->curs)->uid, (*(IRInst *)List_Next(pair->curs))->uid, set);
            }
            List_Delete(pair->curs);
            pair->curs = NULL;
        }
        InterCodeRemoveInst_nobb (code, insn);
    }
}

void InterCodeDump (InterCode code, FILE *dump_file)
{
    IRInst *Cursor;
    for(  Cursor=(IRInst *)List_First(code->code)
        ;  Cursor!=NULL
        ;  Cursor = (IRInst *)List_Next((void *)Cursor)
        )
    {
        IRInstDump (*Cursor, FALSE, dump_file);
        fprintf (dump_file, "\n");
    }
}

IRInst InterCodeGetInstByID (InterCode code, int id)
{
    struct IRInst buf;
    buf.uid = id;
    return (IRInst)avl_find(code->id_map, &buf);
}

IRInst *InterCodeGetCursor (InterCode code, IRInst inst)
{
    struct addr_pair *pair;
    pair = (struct addr_pair *)avl_find (code->addr_map, &inst);
    return pair ? pair->curs : NULL;
}

IRInst *InterCodeInsertBefore (basic_block bb, IRInst *Curs, IRInst inst, BOOL fNewID, varpool_node_set set)
{
    struct addr_pair *pair;
    int orig_insn;

    orig_insn = (*(IRInst *)List_First (bb->insns))->uid;
    InterCodeAddInst_nobb (bb->cfg->code, inst, fNewID);
    pair = (struct addr_pair *)avl_find(bb->cfg->code->addr_map, &inst);
    pair->curs = (IRInst *)List_NewBefore (bb->insns, Curs, sizeof (IRInst));
    *pair->curs = inst;
    inst->bb = bb;
    if  ((*(IRInst *)List_First (bb->insns))->uid == inst->uid)
    {
        /* 如果插入为基本块的第一条指令，则需要更新其前驱分支语句的目标指令号。  */
        update_destinations (bb, orig_insn, inst->uid, set);
    }
    return pair->curs;
}

IRInst *InterCodeInsertAfter (basic_block bb, IRInst *Curs, IRInst inst, BOOL fNewID, varpool_node_set set)
{
    struct addr_pair *pair;
    int orig_insn;

    orig_insn = (*(IRInst *)List_First (bb->insns))->uid;
    InterCodeAddInst_nobb (bb->cfg->code, inst, fNewID);
    pair = (struct addr_pair *)avl_find(bb->cfg->code->addr_map, &inst);
    pair->curs = (IRInst *)List_NewAfter (bb->insns, Curs, sizeof (IRInst));
    *pair->curs = inst;
    inst->bb = bb;
    if  ((*(IRInst *)List_First (bb->insns))->uid == inst->uid)
    {
        /* 如果插入为基本块的第一条指令，则需要更新其前驱分支语句的目标指令号。  */
        update_destinations (bb, orig_insn, inst->uid, set);
    }
    return pair->curs;
}
