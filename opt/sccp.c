#include <string.h>
#include <stdlib.h>
#include "all.h"

enum ValueLatticeElementTy {
    /* 这条指令没有已知的值  */
    undef,

    /* 这条指令有一个常量值  */
    constant,

    /* 这条指令的值未知  */
    overdefined,
};

struct variable_info
{
    enum ValueLatticeElementTy LatticeValue;
    struct constVal ConstantVal;
};

typedef struct block_info_def
{
    bitmap Executable;
} *block_info;

static int
compare (const void *t1, const void *t2, void *unused)
{
    return ((ConstVal)t1)->index - ((ConstVal)t2)->index;
}

static ConstVal
set_value (int index, varpool_node vd)
{
    ConstVal v;
    struct constVal buf;

    buf.index = index;
    v = (ConstVal) avl_find ((struct avl_table *) vd->var->param, &buf);
    if  (!v)
    {
        v = (ConstVal) xmalloc (sizeof (*v));
        memset (v, 0, sizeof (*v));
        v->index = index;
        avl_insert ((struct avl_table *) vd->var->param, v);
    }
    return v;
}

static BOOL sccp_evaluate (IRInst I, varpool_node_set set)
{
    /* state changed  */
    BOOL value_changed = FALSE;
    varpool_node dst = NULL;
    varpool_node src0, src1, src2;
    struct constVal ConstantVal;

    if  (IRInstIsOutput(I, 0))
        dst = varpool_get_node(set, IRInstGetOperand (I, 0));

    switch (I->opcode)
    {
    case IRINST_OP_nop:
    case IRINST_OP_param:
    case IRINST_OP_entry:
    case IRINST_OP_exit:
    case IRINST_OP_begin_block:
    case IRINST_OP_end_block:
        /* nothing to do */
        break;

    case IRINST_OP_load:
    case IRINST_OP_store:
    case IRINST_OP_move:
        src0 = varpool_get_node (set, IRInstGetOperand (I, 1));
        if      (((struct variable_info *)src0->param)->LatticeValue == overdefined &&
                 ((struct variable_info *)dst->param)->LatticeValue != overdefined)
        {
            value_changed = TRUE;
            ((struct variable_info *)dst->param)->LatticeValue = overdefined;
        }
        else if (((struct variable_info *)src0->param)->LatticeValue == constant &&
                 (((struct variable_info *)dst->param)->LatticeValue < constant ||
                 compare_constant(&((struct variable_info *)dst->param)->ConstantVal, &((struct variable_info *)src0->param)->ConstantVal, NULL)))
        {
            value_changed = TRUE;
            ((struct variable_info *)dst->param)->LatticeValue = constant;
            ((struct variable_info *)dst->param)->ConstantVal = ((struct variable_info *)src0->param)->ConstantVal;
        }
        else if  (((struct variable_info *)dst->param)->LatticeValue == constant &&
                 compare_constant(&((struct variable_info *)dst->param)->ConstantVal, &((struct variable_info *)src0->param)->ConstantVal, NULL))
        {
            value_changed = TRUE;
            ((struct variable_info *)dst->param)->LatticeValue = overdefined;
        }
        break;

    case IRINST_OP_astore:
        src0 = varpool_get_node (set, IRInstGetOperand (I, 1));
        src1 = varpool_get_node (set, IRInstGetOperand (I, 2));
        src2 = varpool_get_node (set, IRInstGetOperand (I, 3));
        if  ((((struct variable_info *)src0->param)->LatticeValue == overdefined ||
             ((struct variable_info *)src1->param)->LatticeValue == overdefined ||
             ((struct variable_info *)src2->param)->LatticeValue == overdefined) &&
             ((struct variable_info *)dst->param)->LatticeValue != overdefined)
        {
            ((struct variable_info *)dst->param)->LatticeValue = overdefined;
            value_changed = TRUE;
        }
        else if (((struct variable_info *)src0->param)->LatticeValue == constant &&
                 ((struct variable_info *)src1->param)->LatticeValue == constant &&
                 ((struct variable_info *)src2->param)->LatticeValue != overdefined)
        {
            ConstVal v;
            v = set_value (((struct variable_info *)src0->param)->ConstantVal.cvValue.cvIval / type_size(stGetBaseType(dst->var->sdType)), dst);
            v->cvVtyp = stGetBaseType (dst->var->sdType)->tdTypeKind;
            v->cvValue = ((struct variable_info *)src1->param)->ConstantVal.cvValue;
        }
        break;

    case IRINST_OP_aload:
        src0 = varpool_get_node (set, IRInstGetOperand(I, 1));
        src1 = varpool_get_node (set, IRInstGetOperand(I, 2));
        if  ((((struct variable_info *)src0->param)->LatticeValue == overdefined ||
             ((struct variable_info *)src1->param)->LatticeValue == overdefined) &&
             ((struct variable_info *)dst->param)->LatticeValue != overdefined)
        {
            ((struct variable_info *)dst->param)->LatticeValue = overdefined;
            value_changed = TRUE;
        }
        else if (((struct variable_info *)src0->param)->LatticeValue != overdefined &&
                 ((struct variable_info *)src1->param)->LatticeValue == constant)
        {
            ConstVal v;
            v = set_value (((struct variable_info *)src1->param)->ConstantVal.cvValue.cvIval / type_size(stGetBaseType(src0->var->sdType)), src0);
            if  (v->cvVtyp == TYP_UNDEF)
            {
                memset (&ConstantVal, 0, sizeof (ConstantVal));
                ConstantVal.cvVtyp = stGetBaseType(src0->var->sdType)->tdTypeKind;
            }
            else
            {
                memcpy (&ConstantVal, v, sizeof (ConstantVal));
            }
            if  (((struct variable_info *)dst->param)->LatticeValue == constant &&
                 compare_constant(&((struct variable_info *)dst->param)->ConstantVal, &ConstantVal, NULL))
            {
                value_changed = TRUE;
                ((struct variable_info *)dst->param)->LatticeValue = overdefined;
            }
            else if (((struct variable_info *)dst->param)->LatticeValue != constant ||
                     compare_constant(&((struct variable_info *)dst->param)->ConstantVal, &ConstantVal, NULL))
            {
                value_changed = TRUE;
                ((struct variable_info *)dst->param)->LatticeValue = constant;
                ((struct variable_info *)dst->param)->ConstantVal = ConstantVal;
            }
        }
        break;

    case IRINST_OP_addptr:
        {
            IRInst *curs;
            int i;
            BOOL is_memset = FALSE;
            
            src0 = varpool_get_node (set, IRInstGetOperand(I, 1));
            for(  curs=InterCodeGetCursor (I->bb->cfg->code, InterCodeGetInstByID(I->bb->cfg->code, bitmap_first_set_bit (dst->use_chain)))
               ;  curs!=NULL && (*curs)->opcode != IRINST_OP_call
               ;  curs = (IRInst *)List_Next((void *)curs)
               )
               ;
            if (curs!=NULL && ! strcmp (stGetSymName(IRInstGetOperand(*curs, 1)->var), "memset"))
            {
                for(  i = 0
                   ;  curs!=NULL && i < 2
                   ;  curs = (IRInst *)List_Prev((void *)curs), ++i
                   )
                    ;
                if  (! GetConstVal(IRInstGetOperand(*curs, 0)->var, 0)->cvValue.cvIval)
                    is_memset = TRUE;
            }
            if  (((struct variable_info *)src0->param)->LatticeValue != (is_memset ? undef : overdefined) ||
                 ((struct variable_info *)dst->param)->LatticeValue != (is_memset ? undef : overdefined))
            {
                value_changed = TRUE;
                ((struct variable_info *)src0->param)->LatticeValue = (is_memset ? undef : overdefined);
                ((struct variable_info *)dst->param)->LatticeValue = (is_memset ? undef : overdefined);
            }
        }
        break;

    case IRINST_OP_add:
    case IRINST_OP_sub:
    case IRINST_OP_mul:
    case IRINST_OP_div:
    case IRINST_OP_rem:
    case IRINST_OP_lsl:
    case IRINST_OP_lsr:
    case IRINST_OP_asr:
    case IRINST_OP_eq:
    case IRINST_OP_ne:
    case IRINST_OP_lt:
    case IRINST_OP_ge:
    case IRINST_OP_gt:
    case IRINST_OP_le:
        src0 = varpool_get_node (set, IRInstGetOperand(I, 1));
        src1 = varpool_get_node (set, IRInstGetOperand(I, 2));
        if  ((src0->var->sdType->tdTypeKind > TYP_lastIntrins ||
             src1->var->sdType->tdTypeKind > TYP_lastIntrins) &&
             ((struct variable_info *)dst->param)->LatticeValue != overdefined)
        {
            ((struct variable_info *)dst->param)->LatticeValue = overdefined;
            if  (src0->var->sdType->tdTypeKind > TYP_lastIntrins)
                ((struct variable_info *)src0->param)->LatticeValue = overdefined;
            if  (src1->var->sdType->tdTypeKind > TYP_lastIntrins)
                ((struct variable_info *)src1->param)->LatticeValue = overdefined;
            value_changed = TRUE;
        }
        else if ((((struct variable_info *)src0->param)->LatticeValue == overdefined ||
                 ((struct variable_info *)src1->param)->LatticeValue == overdefined) &&
                 ((struct variable_info *)dst->param)->LatticeValue != overdefined)
        {
            ((struct variable_info *)dst->param)->LatticeValue = overdefined;
            value_changed = TRUE;
        }
        else if (((struct variable_info *)src0->param)->LatticeValue == constant &&
                 ((struct variable_info *)src1->param)->LatticeValue == constant)
        {
            switch (I->opcode)
            {
            default:        fatal ("internal compiler error");
            case IRINST_OP_add:
                ConstantVal.cvVtyp = ((struct variable_info *)src0->param)->ConstantVal.cvVtyp;
                if  (src0->var->sdType->tdTypeKind == TYP_FLOAT)
                    ConstantVal.cvValue.cvFval = ((struct variable_info *)src0->param)->ConstantVal.cvValue.cvFval + ((struct variable_info *)src1->param)->ConstantVal.cvValue.cvFval;
                else
                    ConstantVal.cvValue.cvIval = ((struct variable_info *)src0->param)->ConstantVal.cvValue.cvIval + ((struct variable_info *)src1->param)->ConstantVal.cvValue.cvIval;
                break;
            case IRINST_OP_sub:
                ConstantVal.cvVtyp = ((struct variable_info *)src0->param)->ConstantVal.cvVtyp;
                if  (src0->var->sdType->tdTypeKind == TYP_FLOAT)
                    ConstantVal.cvValue.cvFval = ((struct variable_info *)src0->param)->ConstantVal.cvValue.cvFval - ((struct variable_info *)src1->param)->ConstantVal.cvValue.cvFval;
                else
                    ConstantVal.cvValue.cvIval = ((struct variable_info *)src0->param)->ConstantVal.cvValue.cvIval - ((struct variable_info *)src1->param)->ConstantVal.cvValue.cvIval;
                break;
            case IRINST_OP_mul:
                ConstantVal.cvVtyp = ((struct variable_info *)src0->param)->ConstantVal.cvVtyp;
                if  (src0->var->sdType->tdTypeKind == TYP_FLOAT)
                    ConstantVal.cvValue.cvFval = ((struct variable_info *)src0->param)->ConstantVal.cvValue.cvFval * ((struct variable_info *)src1->param)->ConstantVal.cvValue.cvFval;
                else
                    ConstantVal.cvValue.cvIval = ((struct variable_info *)src0->param)->ConstantVal.cvValue.cvIval * ((struct variable_info *)src1->param)->ConstantVal.cvValue.cvIval;
                break;
            case IRINST_OP_div:
                ConstantVal.cvVtyp = ((struct variable_info *)src0->param)->ConstantVal.cvVtyp;
                if  (src0->var->sdType->tdTypeKind == TYP_FLOAT)
                    ConstantVal.cvValue.cvFval = ((struct variable_info *)src0->param)->ConstantVal.cvValue.cvFval / ((struct variable_info *)src1->param)->ConstantVal.cvValue.cvFval;
                else
                {
                    if (((struct variable_info *)src1->param)->ConstantVal.cvValue.cvIval == 0) goto EXPR_DIV_BY_ZERO;
                    ConstantVal.cvValue.cvIval = ((struct variable_info *)src0->param)->ConstantVal.cvValue.cvIval / ((struct variable_info *)src1->param)->ConstantVal.cvValue.cvIval;
                }
                break;
            case IRINST_OP_rem:
                if (((struct variable_info *)src1->param)->ConstantVal.cvValue.cvIval == 0) goto EXPR_DIV_BY_ZERO;
                ConstantVal.cvVtyp = ((struct variable_info *)src0->param)->ConstantVal.cvVtyp;
                ConstantVal.cvValue.cvIval = ((struct variable_info *)src0->param)->ConstantVal.cvValue.cvIval % ((struct variable_info *)src1->param)->ConstantVal.cvValue.cvIval;
                break;
            case IRINST_OP_lsl:
                ConstantVal.cvVtyp = ((struct variable_info *)src0->param)->ConstantVal.cvVtyp;
                ConstantVal.cvValue.cvIval = ((struct variable_info *)src0->param)->ConstantVal.cvValue.cvIval << ((struct variable_info *)src1->param)->ConstantVal.cvValue.cvIval;
                break;
            case IRINST_OP_lsr:
                ConstantVal.cvVtyp = ((struct variable_info *)src0->param)->ConstantVal.cvVtyp;
                ConstantVal.cvValue.cvIval = ((unsigned) ((struct variable_info *)src0->param)->ConstantVal.cvValue.cvIval) >> ((struct variable_info *)src1->param)->ConstantVal.cvValue.cvIval;
                break;
            case IRINST_OP_asr:
                ConstantVal.cvVtyp = ((struct variable_info *)src0->param)->ConstantVal.cvVtyp;
                ConstantVal.cvValue.cvIval = ((signed) ((struct variable_info *)src0->param)->ConstantVal.cvValue.cvIval) >> ((struct variable_info *)src1->param)->ConstantVal.cvValue.cvIval;
                break;
            case IRINST_OP_eq:
                ConstantVal.cvVtyp = TYP_INT;
                if  (src0->var->sdType->tdTypeKind == TYP_FLOAT)
                    ConstantVal.cvValue.cvIval = ((struct variable_info *)src0->param)->ConstantVal.cvValue.cvFval == ((struct variable_info *)src1->param)->ConstantVal.cvValue.cvFval;
                else
                    ConstantVal.cvValue.cvIval = ((struct variable_info *)src0->param)->ConstantVal.cvValue.cvIval == ((struct variable_info *)src1->param)->ConstantVal.cvValue.cvIval;
                break;
            case IRINST_OP_ne:
                ConstantVal.cvVtyp = TYP_INT;
                if  (src0->var->sdType->tdTypeKind == TYP_FLOAT)
                    ConstantVal.cvValue.cvIval = ((struct variable_info *)src0->param)->ConstantVal.cvValue.cvFval != ((struct variable_info *)src1->param)->ConstantVal.cvValue.cvFval;
                else
                    ConstantVal.cvValue.cvIval = ((struct variable_info *)src0->param)->ConstantVal.cvValue.cvIval != ((struct variable_info *)src1->param)->ConstantVal.cvValue.cvIval;
                break;
            case IRINST_OP_lt:
                ConstantVal.cvVtyp = TYP_INT;
                if  (src0->var->sdType->tdTypeKind == TYP_FLOAT)
                    ConstantVal.cvValue.cvIval = ((struct variable_info *)src0->param)->ConstantVal.cvValue.cvFval < ((struct variable_info *)src1->param)->ConstantVal.cvValue.cvFval;
                else
                    ConstantVal.cvValue.cvIval = ((struct variable_info *)src0->param)->ConstantVal.cvValue.cvIval < ((struct variable_info *)src1->param)->ConstantVal.cvValue.cvIval;
                break;
            case IRINST_OP_ge:
                ConstantVal.cvVtyp = TYP_INT;
                if  (src0->var->sdType->tdTypeKind == TYP_FLOAT)
                    ConstantVal.cvValue.cvIval = ((struct variable_info *)src0->param)->ConstantVal.cvValue.cvFval >= ((struct variable_info *)src1->param)->ConstantVal.cvValue.cvFval;
                else
                    ConstantVal.cvValue.cvIval = ((struct variable_info *)src0->param)->ConstantVal.cvValue.cvIval >= ((struct variable_info *)src1->param)->ConstantVal.cvValue.cvIval;
                break;
            case IRINST_OP_gt:
                ConstantVal.cvVtyp = TYP_INT;
                if  (src0->var->sdType->tdTypeKind == TYP_FLOAT)
                    ConstantVal.cvValue.cvIval = ((struct variable_info *)src0->param)->ConstantVal.cvValue.cvFval > ((struct variable_info *)src1->param)->ConstantVal.cvValue.cvFval;
                else
                    ConstantVal.cvValue.cvIval = ((struct variable_info *)src0->param)->ConstantVal.cvValue.cvIval > ((struct variable_info *)src1->param)->ConstantVal.cvValue.cvIval;
                break;
            case IRINST_OP_le:
                ConstantVal.cvVtyp = TYP_INT;
                if  (src0->var->sdType->tdTypeKind == TYP_FLOAT)
                    ConstantVal.cvValue.cvIval = ((struct variable_info *)src0->param)->ConstantVal.cvValue.cvFval <= ((struct variable_info *)src1->param)->ConstantVal.cvValue.cvFval;
                else
                    ConstantVal.cvValue.cvIval = ((struct variable_info *)src0->param)->ConstantVal.cvValue.cvIval <= ((struct variable_info *)src1->param)->ConstantVal.cvValue.cvIval;
                break;
            }
            if  (((struct variable_info *)dst->param)->LatticeValue == constant &&
                 compare_constant(&((struct variable_info *)dst->param)->ConstantVal, &ConstantVal, NULL))
            {
                value_changed = TRUE;
                ((struct variable_info *)dst->param)->LatticeValue = overdefined;
            }
            else if (((struct variable_info *)dst->param)->LatticeValue < constant ||
                     compare_constant(&((struct variable_info *)dst->param)->ConstantVal, &ConstantVal, NULL))
            {
                value_changed = TRUE;
                ((struct variable_info *)dst->param)->LatticeValue = constant;
                ((struct variable_info *)dst->param)->ConstantVal = ConstantVal;
            }
        }
        break;

EXPR_DIV_BY_ZERO:
        if (((struct variable_info *)dst->param)->LatticeValue != overdefined)
        {
            ((struct variable_info *)dst->param)->LatticeValue = overdefined;
            value_changed = TRUE;
        }
        break;

    case IRINST_OP_not:
    case IRINST_OP_neg:
    case IRINST_OP_i2f:
    case IRINST_OP_f2i:
        src0 = varpool_get_node (set, IRInstGetOperand(I, 1));
        if      (((struct variable_info *)src0->param)->LatticeValue == overdefined &&
         ((struct variable_info *)dst->param)->LatticeValue != overdefined)
        {
            value_changed = TRUE;
            ((struct variable_info *)dst->param)->LatticeValue = overdefined;
        }
        else if (((struct variable_info *)src0->param)->LatticeValue == constant)
        {
            switch (I->opcode)
            {
            case IRINST_OP_not:
                ConstantVal.cvVtyp = TYP_INT;
                if  (src0->var->sdType->tdTypeKind == TYP_FLOAT)
                    ConstantVal.cvValue.cvIval = !((struct variable_info *)src0->param)->ConstantVal.cvValue.cvFval;
                else
                    ConstantVal.cvValue.cvIval = !((struct variable_info *)src0->param)->ConstantVal.cvValue.cvIval;
                break;
            case IRINST_OP_neg:
                ConstantVal.cvVtyp = src0->var->sdType->tdTypeKind;
                if  (src0->var->sdType->tdTypeKind == TYP_FLOAT)
                    ConstantVal.cvValue.cvFval = -((struct variable_info *)src0->param)->ConstantVal.cvValue.cvFval;
                else
                    ConstantVal.cvValue.cvIval = -((struct variable_info *)src0->param)->ConstantVal.cvValue.cvIval;
                break;
            case IRINST_OP_i2f:
                ConstantVal.cvVtyp = TYP_FLOAT;
                ConstantVal.cvValue.cvFval = (float) ((struct variable_info *)src0->param)->ConstantVal.cvValue.cvIval;
                break;
            case IRINST_OP_f2i:
                ConstantVal.cvVtyp = TYP_INT;
                ConstantVal.cvValue.cvIval = (int) ((struct variable_info *)src0->param)->ConstantVal.cvValue.cvFval;
                break;
            default:
                fatal ("internal compiler error");
                break;
            }

            if  (((struct variable_info *)dst->param)->LatticeValue == constant &&
                 compare_constant(&((struct variable_info *)dst->param)->ConstantVal, &ConstantVal, NULL))
            {
                value_changed = TRUE;
                ((struct variable_info *)dst->param)->LatticeValue = overdefined;
            }
            else if (((struct variable_info *)dst->param)->LatticeValue < constant ||
                     compare_constant(&((struct variable_info *)dst->param)->ConstantVal, &ConstantVal, NULL))
            {
                value_changed = TRUE;
                ((struct variable_info *)dst->param)->LatticeValue = constant;
                ((struct variable_info *)dst->param)->ConstantVal = ConstantVal;
            }
        }
        break;

    case IRINST_OP_ifeq:
    case IRINST_OP_ifne:
    case IRINST_OP_iflt:
    case IRINST_OP_ifge:
    case IRINST_OP_ifgt:
    case IRINST_OP_ifle:
    case IRINST_OP_goto:
        fatal ("internal compiler error");
        break;

    case IRINST_OP_fparam:
    case IRINST_OP_call:
        if  (((struct variable_info *)dst->param)->LatticeValue != overdefined)
        {
            value_changed = TRUE;
            ((struct variable_info *)dst->param)->LatticeValue = overdefined;
        }
        break;

    case IRINST_OP_phi:
        do
        {
            int i;
            edge *ei;

            for(  ei=(edge *)List_First (I->bb->preds), i = 1
               ;  ei!=NULL
               ;  ei = (edge *)List_Next((void *)ei), ++i
               )
            {
                src0 = varpool_get_node (set, IRInstGetOperand(I, i));
                if  (((struct variable_info *)src0->param)->LatticeValue != undef &&
                     bitmap_bit_p (((block_info)(*ei)->src->param)->Executable, I->bb->index))
                    break;
            }
            if  (ei==NULL)
            {
                if  (dst->var->sdType->tdTypeKind > TYP_lastIntrins &&
                     ((struct variable_info *)dst->param)->LatticeValue != overdefined)
                {
                    value_changed = TRUE;
                    ((struct variable_info *)dst->param)->LatticeValue = overdefined;
                }
                break;
            }

            for(  ei=(edge *)List_First (I->bb->preds), i = 1
               ;  ei!=NULL
               ;  ei = (edge *)List_Next((void *)ei), ++i
               )
            {
                src0 = varpool_get_node (set, IRInstGetOperand (I, i));
                if  (((struct variable_info *)src0->param)->LatticeValue == overdefined &&
                     bitmap_bit_p (((block_info)(*ei)->src->param)->Executable, I->bb->index))
                    break;
            }
            if  (ei!=NULL)
            {
                if  (((struct variable_info *)dst->param)->LatticeValue != overdefined)
                {
                    value_changed = TRUE;
                    ((struct variable_info *)dst->param)->LatticeValue = overdefined;
                }
                break;
            }

            for(  ei=(edge *)List_First (I->bb->preds), i = 1
               ;  ei!=NULL
               ;  ei = (edge *)List_Next((void *)ei), ++i
               )
            {
                src0 = varpool_get_node (set, IRInstGetOperand(I, i));
                if  (((struct variable_info *)src0->param)->LatticeValue == constant &&
                     bitmap_bit_p (((block_info)(*ei)->src->param)->Executable, I->bb->index))
                {
                    ConstantVal = ((struct variable_info *)src0->param)->ConstantVal;
                }
            }
            if  (src0->var->sdType->tdTypeKind > TYP_lastIntrins &&
                 ((struct variable_info *)dst->param)->LatticeValue != overdefined)
            {
                value_changed = TRUE;
                ((struct variable_info *)dst->param)->LatticeValue = overdefined;
                break;
            }
            for(  ei=(edge *)List_First (I->bb->preds), i = 1
               ;  ei!=NULL
               ;  ei = (edge *)List_Next((void *)ei), ++i
               )
            {
                src0 = varpool_get_node (set, IRInstGetOperand(I, i));
                if  (((struct variable_info *)src0->param)->LatticeValue == constant &&
                     compare_constant (&((struct variable_info *)src0->param)->ConstantVal, &ConstantVal, NULL) &&
                     bitmap_bit_p (((block_info)(*ei)->src->param)->Executable, I->bb->index))
                {
                    break;
                }
            }
            if  (ei==NULL)
            {
                if  (((struct variable_info *)dst->param)->LatticeValue < constant)
                {
                    value_changed = TRUE;
                    ((struct variable_info *)dst->param)->LatticeValue = constant;
                    ((struct variable_info *)dst->param)->ConstantVal = ConstantVal;
                }
                break;
            }
            else if  (((struct variable_info *)dst->param)->LatticeValue != overdefined)
            {
                value_changed = TRUE;
                ((struct variable_info *)dst->param)->LatticeValue = overdefined;
            }
        }
        while (FALSE);
        break;
    }

    return value_changed;
}

static void updateInstrUses (varpool_node vnode, bitmap InstWorkList)
{
    bitmap_iterator bi;
    unsigned i;

    for (bmp_iter_set_init (&bi, vnode->use_chain, 0, &i);
         bmp_iter_set (&bi, &i);
         bmp_iter_next (&bi, &i))

        bitmap_set_bit(InstWorkList, i);
}

static void getFeasibleSuccessors (IRInst I, varpool_node_set set, bitmap succ_feasible)
{
    IRInst target_insn;
    varpool_node src0, src1;
    struct constVal ConstantVal;

    bitmap_clear (succ_feasible);

    switch (I->opcode)
    {
    case IRINST_OP_ifeq:
    case IRINST_OP_ifne:
    case IRINST_OP_iflt:
    case IRINST_OP_ifge:
    case IRINST_OP_ifgt:
    case IRINST_OP_ifle:
        src0 = varpool_get_node (set, IRInstGetOperand(I, 0));
        src1 = varpool_get_node (set, IRInstGetOperand(I, 1));
        if  (((struct variable_info *)src0->param)->LatticeValue == overdefined ||
             ((struct variable_info *)src1->param)->LatticeValue == overdefined)
        {
            target_insn = InterCodeGetInstByID(I->bb->cfg->code, GetConstVal(IRInstGetOperand(I, 2)->var, 0)->cvValue.cvIval);
            bitmap_set_bit (succ_feasible, target_insn->bb->index);
            target_insn = InterCodeGetInstByID(I->bb->cfg->code, GetConstVal(IRInstGetOperand(I, 3)->var, 0)->cvValue.cvIval);
            bitmap_set_bit (succ_feasible, target_insn->bb->index);
        }
        else if (((struct variable_info *)src0->param)->LatticeValue == constant &&
                 ((struct variable_info *)src1->param)->LatticeValue == constant)
        {
            switch (I->opcode)
            {
            default:        fatal ("internal compiler error");
            case IRINST_OP_ifeq:
                ConstantVal.cvVtyp = TYP_INT;
                if  (src0->var->sdType->tdTypeKind == TYP_FLOAT)
                    ConstantVal.cvValue.cvIval = ((struct variable_info *)src0->param)->ConstantVal.cvValue.cvFval == ((struct variable_info *)src1->param)->ConstantVal.cvValue.cvFval;
                else
                    ConstantVal.cvValue.cvIval = ((struct variable_info *)src0->param)->ConstantVal.cvValue.cvIval == ((struct variable_info *)src1->param)->ConstantVal.cvValue.cvIval;
                break;
            case IRINST_OP_ifne:
                ConstantVal.cvVtyp = TYP_INT;
                if  (src0->var->sdType->tdTypeKind == TYP_FLOAT)
                    ConstantVal.cvValue.cvIval = ((struct variable_info *)src0->param)->ConstantVal.cvValue.cvFval != ((struct variable_info *)src1->param)->ConstantVal.cvValue.cvFval;
                else
                    ConstantVal.cvValue.cvIval = ((struct variable_info *)src0->param)->ConstantVal.cvValue.cvIval != ((struct variable_info *)src1->param)->ConstantVal.cvValue.cvIval;
                break;
            case IRINST_OP_iflt:
                ConstantVal.cvVtyp = TYP_INT;
                if  (src0->var->sdType->tdTypeKind == TYP_FLOAT)
                    ConstantVal.cvValue.cvIval = ((struct variable_info *)src0->param)->ConstantVal.cvValue.cvFval < ((struct variable_info *)src1->param)->ConstantVal.cvValue.cvFval;
                else
                    ConstantVal.cvValue.cvIval = ((struct variable_info *)src0->param)->ConstantVal.cvValue.cvIval < ((struct variable_info *)src1->param)->ConstantVal.cvValue.cvIval;
                break;
            case IRINST_OP_ifge:
                ConstantVal.cvVtyp = TYP_INT;
                if  (src0->var->sdType->tdTypeKind == TYP_FLOAT)
                    ConstantVal.cvValue.cvIval = ((struct variable_info *)src0->param)->ConstantVal.cvValue.cvFval >= ((struct variable_info *)src1->param)->ConstantVal.cvValue.cvFval;
                else
                    ConstantVal.cvValue.cvIval = ((struct variable_info *)src0->param)->ConstantVal.cvValue.cvIval >= ((struct variable_info *)src1->param)->ConstantVal.cvValue.cvIval;
                break;
            case IRINST_OP_ifgt:
                ConstantVal.cvVtyp = TYP_INT;
                if  (src0->var->sdType->tdTypeKind == TYP_FLOAT)
                    ConstantVal.cvValue.cvIval = ((struct variable_info *)src0->param)->ConstantVal.cvValue.cvFval > ((struct variable_info *)src1->param)->ConstantVal.cvValue.cvFval;
                else
                    ConstantVal.cvValue.cvIval = ((struct variable_info *)src0->param)->ConstantVal.cvValue.cvIval > ((struct variable_info *)src1->param)->ConstantVal.cvValue.cvIval;
                break;
            case IRINST_OP_ifle:
                ConstantVal.cvVtyp = TYP_INT;
                if  (src0->var->sdType->tdTypeKind == TYP_FLOAT)
                    ConstantVal.cvValue.cvIval = ((struct variable_info *)src0->param)->ConstantVal.cvValue.cvFval <= ((struct variable_info *)src1->param)->ConstantVal.cvValue.cvFval;
                else
                    ConstantVal.cvValue.cvIval = ((struct variable_info *)src0->param)->ConstantVal.cvValue.cvIval <= ((struct variable_info *)src1->param)->ConstantVal.cvValue.cvIval;
                break;
            }
            target_insn = InterCodeGetInstByID(I->bb->cfg->code, GetConstVal(IRInstGetOperand(I, ConstantVal.cvValue.cvIval ? 2 : 3)->var, 0)->cvValue.cvIval);
            bitmap_set_bit (succ_feasible, target_insn->bb->index);
        }
        break;

    case IRINST_OP_goto:
        target_insn = InterCodeGetInstByID(I->bb->cfg->code, GetConstVal(IRInstGetOperand(I, 0)->var, 0)->cvValue.cvIval);
        bitmap_set_bit (succ_feasible, target_insn->bb->index);
        break;

    default:
        fatal ("internal compiler error");
        break;
    }
}

static void sccp_instruction (IRInst I, varpool_node_set set, bitmap InstWorkList, bitmap BBWorkList)
{
    bitmap succ_feasible;
    bitmap_iterator bi;
    unsigned int i;
    int k;
    basic_block target_block;

    switch (I->opcode)
    {
    case IRINST_OP_ifeq:
    case IRINST_OP_ifne:
    case IRINST_OP_iflt:
    case IRINST_OP_ifge:
    case IRINST_OP_ifgt:
    case IRINST_OP_ifle:
    case IRINST_OP_goto:
        succ_feasible = BITMAP_XMALLOC ();
        getFeasibleSuccessors (I, set, succ_feasible);
        for (bmp_iter_set_init (&bi, succ_feasible, 0, &i);
             bmp_iter_set (&bi, &i);
             bmp_iter_next (&bi, &i))
        {
            target_block = lookup_block_by_id(I->bb->cfg, i);
            if  (! bitmap_bit_p (((block_info)(I->bb)->param)->Executable, target_block->index))
            {
                bitmap_set_bit (((block_info)(I->bb)->param)->Executable, target_block->index);
                bitmap_set_bit (BBWorkList, target_block->index);
            }
        }
        BITMAP_XFREE (succ_feasible);
        break;

    default:
        if  (sccp_evaluate (I, set))
            for (k = 0; k < IRInstGetNumOperands(I); k++)
                if (IRInstIsOutput(I, k))
                    updateInstrUses (varpool_get_node(set, IRInstGetOperand(I, k)), InstWorkList);
        break;
    }
}

static void sccp_block (basic_block BB, bitmap Visited, varpool_node_set set, bitmap InstWorkList, bitmap BBWorkList)
{
    IRInst *instr;

    for(  instr=(IRInst *)List_First(BB->insns)
       ;  instr!=NULL
       ;  instr = (IRInst *)List_Next((void *)instr)
       )
        if  ((*instr)->opcode == IRINST_OP_phi)
            sccp_instruction (*instr, set, InstWorkList, BBWorkList);

    if  (! bitmap_bit_p (Visited, BB->index))
    {
        bitmap_set_bit (Visited, BB->index);
        for(  instr=(IRInst *)List_First(BB->insns)
           ;  instr!=NULL
           ;  instr = (IRInst *)List_Next((void *)instr)
           )
            if  ((*instr)->opcode != IRINST_OP_phi)
                sccp_instruction (*instr, set, InstWorkList, BBWorkList);
    }
}

static void
init_variable (varpool_node variable)
{
    ConstVal iter;
    struct avl_traverser trav;

    if  (! variable->param)
    {
        variable->param = xmalloc (sizeof (struct variable_info));
        memset (variable->param, 0, sizeof (struct variable_info));
        ((struct variable_info *)variable->param)->LatticeValue = undef;

        if  (variable->var->sdSymKind == SYM_VAR &&
             variable->var->sdType->tdTypeKind == TYP_ARRAY &&
             !variable->var->param)
        {
            variable->var->param = avl_create ((avl_comparison_func *) compare, NULL, NULL);
            for(  iter = (ConstVal)avl_t_first (&trav, variable->var->sdVar.sdvCnsVal)
               ;  iter != NULL
               ;  iter = (ConstVal)avl_t_next (&trav)
               )
            {
                set_value (iter->index, variable)->cvVtyp = iter->cvVtyp;
                set_value (iter->index, variable)->cvValue = iter->cvValue;
            }
        }

        if  (variable->var->sdSymKind == SYM_VAR &&
             variable->var->sdVar.sdvConst)
        {
            ((struct variable_info *)variable->param)->LatticeValue = constant;
            if  (variable->var->sdType->tdTypeKind <= TYP_lastIntrins)
            {
                ((struct variable_info *)variable->param)->ConstantVal = *GetConstVal(variable->var, 0);
                ((struct variable_info *)variable->param)->ConstantVal.cvVtyp = variable->var->sdType->tdTypeKind;
            }
        }

        /* 未初始化的变量。  */
        if  (variable->var->sdSymKind == SYM_VAR &&
             !variable->var->sdVar.sdvConst &&
             bitmap_empty_p(variable->_defines))
            ((struct variable_info *)variable->param)->LatticeValue = overdefined;
    }
}

static void SCCPSolver (control_flow_graph F, SymTab stab)
{
    bitmap BBWorkList;
    bitmap InstWorkList;
    bitmap Visited;
    varpool_node_set set;
    struct avl_traverser trav;
    varpool_node vnode;
    basic_block *bb;
    IRInst *insn;
    IRInst *next_insn;
    IRInst temp;
    IRInst new_insn;
    IRInst jmp;
    ssa_name name;
    int i, k;
    edge *ei;
    edge *next_edge;

    BBWorkList = BITMAP_XMALLOC ();
    InstWorkList = BITMAP_XMALLOC ();
    Visited = BITMAP_XMALLOC ();
    set = varpool_node_set_new(F, TRUE);

    for(  bb=(basic_block *)List_First(F->basic_block_info)
       ;  bb!=NULL
       ;  bb = (basic_block *)List_Next((void *)bb)
       )
    {
        (*bb)->param = xmalloc (sizeof (struct block_info_def));
        memset ((*bb)->param, 0, sizeof (struct block_info_def));
        ((block_info)(*bb)->param)->Executable = BITMAP_XMALLOC ();
    }

    for(  vnode = (varpool_node)avl_t_first (&trav, set->nodes)
       ;  vnode != NULL
       ;  vnode = (varpool_node)avl_t_next (&trav)
       )
        init_variable (vnode);
    bitmap_set_bit (BBWorkList, F->entry_block_ptr->index);

    while (!bitmap_empty_p (BBWorkList) || !bitmap_empty_p(InstWorkList))
    {
        /* Process the instruction work list.  */
        while (!bitmap_empty_p(InstWorkList))
        {
            IRInst I = InterCodeGetInstByID(F->code, bitmap_first_set_bit(InstWorkList));
            bitmap_clear_bit(InstWorkList, I->uid);
            sccp_instruction (I, set, InstWorkList, BBWorkList);
        }

        /* Process the basic block work list.  */
        while (!bitmap_empty_p (BBWorkList))
        {
            basic_block BB = lookup_block_by_id (F, bitmap_first_set_bit (BBWorkList));
            bitmap_clear_bit (BBWorkList, BB->index);
            sccp_block (BB, Visited, set, InstWorkList, BBWorkList);
        }
    }

    /* 更新指令。  */
    for(  bb=(basic_block *)List_First(F->basic_block_info)
       ;  bb!=NULL
       ;  bb = (basic_block *)List_Next((void *)bb)
       )
    {
        for(  insn=(IRInst *)List_First((*bb)->insns)
           ;  insn!=NULL
           ;  insn = (IRInst *)List_Next((void *)insn)
           )
        {
            for (i = 0; i < IRInstGetNumOperands (*insn); ++i)
            {
                vnode = varpool_get_node (set, IRInstGetOperand (*insn, i));
                if  (! IRInstIsOutput(*insn, i) &&
                     vnode->var->sdType->tdTypeKind <= TYP_lastIntrins &&
/*                   ! is_global_var (vnode->var) && */
                     ((struct variable_info *)vnode->param)->LatticeValue == constant)
                {
                    IRInstSetOperand (*insn, i, ((struct variable_info *)vnode->param)->ConstantVal.cvVtyp == TYP_FLOAT ? stCreateFconNode(stab, ((struct variable_info *)vnode->param)->ConstantVal.cvValue.cvFval) : stCreateIconNode(stab, ((struct variable_info *)vnode->param)->ConstantVal.cvValue.cvIval));
                    /* 更新变量池，因为我们增加了新操作数。  */
                    init_variable (varpool_node_set_add (set, IRInstGetOperand (*insn, i)));
                }
            }
        }
    }

    /* 删除指令，须在更新指令之后进行，因为如果不这么做，我们就需要更新变量池。  */
    for(  bb=(basic_block *)List_First(F->basic_block_info)
       ;  bb!=NULL
       ;  bb = (basic_block *)List_Next((void *)bb)
       )
    {
        for(  insn=(IRInst *)List_First((*bb)->insns)
           ;  insn!=NULL
           ;  insn = next_insn
           )
        {
            next_insn = (IRInst *)List_Next((void *)insn);
            if  (IRInstIsOutput(*insn, 0) &&
                 IRInstGetOperand(*insn, 0)->var->sdType->tdTypeKind <= TYP_lastIntrins &&
                 ! is_global_var (IRInstGetOperand(*insn, 0)->var))
            {
                vnode = varpool_get_node (set, IRInstGetOperand (*insn, 0));
                if  (((struct variable_info *)vnode->param)->LatticeValue == constant)
                {
                    temp = *insn;
                    InterCodeRemoveInst(F->code, *insn, NULL);
                    IRInstDelInst(temp);
                }
            }
        }
    }

    /* 删除不可达基本块。  */
    for(  bb=(basic_block *)List_First(F->basic_block_info)
       ;  bb!=NULL
       ;  bb = (basic_block *)List_Next((void *)bb)
       )
    {
        for(  ei=(edge *)List_First((*bb)->preds), i = 1
           ;  ei!=NULL
           ;  ei = next_edge, ++i
           )
        {
            next_edge = (edge *)List_Next((void *)ei);
            if  (! bitmap_bit_p (((block_info)(*ei)->src->param)->Executable, (*bb)->index))
            {
                /* 删除phi指令的参数。  */
                for(  insn=(IRInst *)List_First((*bb)->insns)
                   ;  insn!=NULL
                   ;  insn = next_insn
                   )
                {
                    next_insn = (IRInst *)List_Next((void *)insn);
                    if  ((*insn)->opcode == IRINST_OP_phi)
                    {
                        new_insn = IRInstEmitInst(IRINST_OP_phi, (*insn)->line, (*insn)->column);
                        for (k = 0; k < IRInstGetNumOperands(*insn); ++k)
                        {
                            if  (k != i)
                            {
                                name = IRInstGetOperand(*insn, k);
                                IRInstSetOperand(new_insn, ((k > i) ? k - 1 : k), name->var);
                                IRInstGetOperand(new_insn, ((k > i) ? k - 1 : k))->version = name->version;
                            }
                        }
                        InterCodeInsertAfter (*bb, insn, new_insn, TRUE, NULL);
                        temp = *insn;
                        InterCodeRemoveInst(F->code, *insn, NULL);
                        IRInstDelInst(temp);
                    }
                }
                jmp = *(IRInst *)List_Last ((*ei)->src->insns);
                switch (jmp->opcode)
                {
                case IRINST_OP_ifeq:
                case IRINST_OP_ifne:
                case IRINST_OP_iflt:
                case IRINST_OP_ifge:
                case IRINST_OP_ifgt:
                case IRINST_OP_ifle:
                    new_insn = IRInstEmitInst(IRINST_OP_goto, jmp->line, jmp->column);
                    name = IRInstGetOperand (jmp, 2);
                    if  (InterCodeGetInstByID((*bb)->cfg->code, GetConstVal(name->var, 0)->cvValue.cvIval)->bb == *bb)
                        IRInstSetOperand(new_insn, 0, stCreateIconNode(stab, GetConstVal(IRInstGetOperand (jmp, 3)->var, 0)->cvValue.cvIval));
                    else
                        IRInstSetOperand(new_insn, 0, stCreateIconNode(stab, GetConstVal(name->var, 0)->cvValue.cvIval));
                    InterCodeInsertAfter ((*ei)->src, (IRInst *)List_Last ((*ei)->src->insns), new_insn, TRUE, NULL);
                    InterCodeRemoveInst(F->code, jmp, NULL);
                    IRInstDelInst(jmp);
                    break;
                case IRINST_OP_goto:
                    break;
                default:
                    fatal ("internal compiler error");
                    break;
                }
                remove_edge(*ei);
                --i;
            }
        }
    }

    for(  bb=(basic_block *)List_First(F->basic_block_info)
       ;  bb!=NULL
       ;  bb = (basic_block *)List_Next((void *)bb)
       )
    {
        BITMAP_XFREE (((block_info)(*bb)->param)->Executable);
        free ((*bb)->param);
        (*bb)->param = NULL;
    }

    for(  vnode = (varpool_node)avl_t_first (&trav, set->nodes)
       ;  vnode != NULL
       ;  vnode = (varpool_node)avl_t_next (&trav)
       )
    {
        if  (vnode->var->sdSymKind == SYM_VAR &&
             vnode->var->sdType->tdTypeKind == TYP_ARRAY &&
             vnode->var->param)
        {
            avl_destroy ((struct avl_table *) vnode->var->param, (avl_item_func *) free);
            vnode->var->param = NULL;
        }
        free (vnode->param);
    }

    BITMAP_XFREE (BBWorkList);
    BITMAP_XFREE (InstWorkList);
    BITMAP_XFREE (Visited);
    free_varpool_node_set (set);

    /* 如果exit块不可达，则将其和entry块连接。  */
    if  (List_IsEmpty (F->exit_block_ptr->preds))
    {
        remove_edge (*(edge *)List_First(F->entry_block_ptr->succs));
        make_edge (F->entry_block_ptr, F->exit_block_ptr);
        IRInstSetOperand (*(IRInst *)List_Last (F->entry_block_ptr->insns), 0, stCreateIconNode (stab, (*(IRInst *)List_First(F->exit_block_ptr->insns))->uid));
    }

    /* 死基本块删除。  */
    cleanup_cfg (F, stab);
}

void SparseCondConstProp(InterCode code, SymTab stab)
{
    control_flow_graph *F;

    for(  F=(control_flow_graph *)List_First(code->funcs)
       ;  F!=NULL
       ;  F = (control_flow_graph *)List_Next((void *)F)
       )
    {
        SCCPSolver (*F, stab);
    }

}
