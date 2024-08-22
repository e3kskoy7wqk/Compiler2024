/* 强度削减.  */

#include <string.h>
#include "all.h"

/* Operations */
# ifndef ABS
#   define ABS(x)  ((x) < 0? (-(x)) : (x))
# endif

/* 给定无符号数X，返回2**Y <= X时的最大整数Y。
   如果X为0，返回-1。  */
static int
floor_log2 (unsigned int x)
{
    int t = 0;

    if (x == 0)
        return -1;

    if (x >= ((unsigned int) 1) << (t + 16))
        t += 16;
    if (x >= ((unsigned int) 1) << (t + 8))
        t += 8;
    if (x >= ((unsigned int) 1) << (t + 4))
        t += 4;
    if (x >= ((unsigned int) 1) << (t + 2))
        t += 2;
    if (x >= ((unsigned int) 1) << (t + 1))
        t += 1;

    return t;
}

/* 如果X是2的幂，则返回以2为底，考虑X无符号时X的对数。
   否则，返回-1。  */
static int
exact_log2 (unsigned int x)
{
    if (x != (x & -x))
        return -1;
    return floor_log2 (x);
}

static void
expand_mult (IRInst *instr, SymTab stab)
{
    struct ssa_name tmp = {0};
    BOOL negate = FALSE;
    int absval;
    int log = -1;
    IRInst tmp_insn;
    SymDef tmpSym;

    if  (IRInstGetOperand (*instr, 1)->var->sdType->tdTypeKind != TYP_FLOAT &&
         IRInstGetOperand (*instr, 1)->var->sdVar.sdvConst &&
         exact_log2 ((unsigned int) ABS (GetConstVal (IRInstGetOperand (*instr, 1)->var, 0)->cvValue.cvIval)) != -1)
    {
        /* 交换操作数。  */
        memcpy (&tmp, IRInstGetOperand (*instr, 1), sizeof (tmp));
        memcpy (IRInstGetOperand (*instr, 1), IRInstGetOperand (*instr, 2), sizeof (tmp));
        memcpy (IRInstGetOperand (*instr, 2), &tmp, sizeof (tmp));
    }

    if  (IRInstGetOperand (*instr, 2)->var->sdType->tdTypeKind != TYP_FLOAT &&
         IRInstGetOperand (*instr, 2)->var->sdVar.sdvConst)
    {
        absval = GetConstVal (IRInstGetOperand (*instr, 2)->var, 0)->cvValue.cvIval;
        if  (absval < 0)
            negate = TRUE, absval = - absval;

        log = exact_log2 ((unsigned int) absval);
        if  (log > 0 && negate)
        {
            tmpSym = stDeclareSym (stab, NULL, SYM_VAR);
            tmpSym->sdIsImplicit = TRUE;
            tmpSym->sdType = stAllocTypDef (TYP_INT);
            tmp_insn = IRInstEmitInst (IRINST_OP_lsl, (*instr)->line, (*instr)->column);
            IRInstSetOperand (tmp_insn, 0, tmpSym);
            IRInstSetOperand (tmp_insn, 1, IRInstGetOperand (*instr, 1)->var);
            IRInstGetOperand (tmp_insn, 1)->version = IRInstGetOperand (*instr, 1)->version;
            IRInstSetOperand (tmp_insn, 2, stCreateIconNode (stab, log));
            InterCodeInsertBefore ((*instr)->bb, instr, tmp_insn, TRUE, NULL);

            (*instr)->opcode = IRINST_OP_sub;
            IRInstSetOperand (*instr, 2, tmpSym);
            IRInstGetOperand (*instr, 2)->version = 0;
            IRInstSetOperand (*instr, 1, stCreateIconNode (stab, 0));
        }
        else if (log > 0)
        {
            IRInstSetOperand (*instr, 2, stCreateIconNode (stab, log));
            (*instr)->opcode = IRINST_OP_lsl;
        }
    }
}

static void
expand_divmod (IRInst *instr, SymTab stab)
{
    BOOL negate = FALSE;
    int absval;
    int log = -1;
    IRInst tmp_insn;
    SymDef tmpSym;

    if  (IRInstGetOperand (*instr, 2)->var->sdType->tdTypeKind != TYP_FLOAT &&
         IRInstGetOperand (*instr, 2)->var->sdVar.sdvConst)
    {
        absval = GetConstVal (IRInstGetOperand (*instr, 2)->var, 0)->cvValue.cvIval;
        if  (absval < 0)
            negate = TRUE, absval = - absval;

        log = exact_log2 ((unsigned int) absval);
        if  (log > 0)
        {
            tmpSym = stDeclareSym (stab, NULL, SYM_VAR);
            tmpSym->sdIsImplicit = TRUE;
            tmpSym->sdType = stAllocTypDef (TYP_INT);
            tmp_insn = IRInstEmitInst (IRINST_OP_asr, (*instr)->line, (*instr)->column);
            IRInstSetOperand (tmp_insn, 0, tmpSym);
            IRInstSetOperand (tmp_insn, 1, IRInstGetOperand (*instr, 1)->var);
            IRInstGetOperand (tmp_insn, 1)->version = IRInstGetOperand (*instr, 1)->version;
            IRInstSetOperand (tmp_insn, 2, stCreateIconNode (stab, 31));
            InterCodeInsertBefore ((*instr)->bb, instr, tmp_insn, TRUE, NULL);

            tmp_insn = IRInstEmitInst (IRINST_OP_lsr, (*instr)->line, (*instr)->column);
            IRInstSetOperand (tmp_insn, 1, tmpSym);
            IRInstSetOperand (tmp_insn, 2, stCreateIconNode (stab, 32 - log));
            tmpSym = stDeclareSym (stab, NULL, SYM_VAR);
            tmpSym->sdIsImplicit = TRUE;
            tmpSym->sdType = stAllocTypDef (TYP_INT);
            IRInstSetOperand (tmp_insn, 0, tmpSym);
            InterCodeInsertBefore ((*instr)->bb, instr, tmp_insn, TRUE, NULL);

            tmp_insn = IRInstEmitInst (IRINST_OP_add, (*instr)->line, (*instr)->column);
            IRInstSetOperand (tmp_insn, 1, IRInstGetOperand (*instr, 1)->var);
            IRInstGetOperand (tmp_insn, 1)->version = IRInstGetOperand (*instr, 1)->version;
            IRInstSetOperand (tmp_insn, 2, tmpSym);
            tmpSym = stDeclareSym (stab, NULL, SYM_VAR);
            tmpSym->sdIsImplicit = TRUE;
            tmpSym->sdType = stAllocTypDef (TYP_INT);
            IRInstSetOperand (tmp_insn, 0, tmpSym);
            InterCodeInsertBefore ((*instr)->bb, instr, tmp_insn, TRUE, NULL);

            if  (negate)
            {
                tmp_insn = IRInstEmitInst (IRINST_OP_asr, (*instr)->line, (*instr)->column);
                IRInstSetOperand (tmp_insn, 1, tmpSym);
                IRInstSetOperand (tmp_insn, 2, stCreateIconNode (stab, log));
                tmpSym = stDeclareSym (stab, NULL, SYM_VAR);
                tmpSym->sdIsImplicit = TRUE;
                tmpSym->sdType = stAllocTypDef (TYP_INT);
                IRInstSetOperand (tmp_insn, 0, tmpSym);
                InterCodeInsertBefore ((*instr)->bb, instr, tmp_insn, TRUE, NULL);

                (*instr)->opcode = IRINST_OP_sub;
                IRInstSetOperand (*instr, 1, stCreateIconNode (stab, 0));
                IRInstSetOperand (*instr, 2, tmpSym);
                IRInstGetOperand (*instr, 2)->version = 0;
            }
            else
            {
                (*instr)->opcode = IRINST_OP_asr;
                IRInstSetOperand (*instr, 1, tmpSym);
                IRInstGetOperand (*instr, 1)->version = 0;
                IRInstSetOperand (*instr, 2, stCreateIconNode (stab, log));
            }
        }
    }
}

void
StraightLineStrengthReduce (InterCode code, SymTab stab)
{
    control_flow_graph *F;
    basic_block *block;
    IRInst *next_insn;
    IRInst *instr;

    for(  F=(control_flow_graph *)List_First(code->funcs)
       ;  F!=NULL
       ;  F = (control_flow_graph *)List_Next((void *)F)
       )
    {
        for(  block=(basic_block *)List_First((*F)->basic_block_info)
           ;  block!=NULL
           ;  block = (basic_block *)List_Next((void *)block)
           )
        {
            for(  instr=(IRInst *) List_First ((*block)->insns)
               ;  instr!=NULL
               ;  instr = next_insn
               )
            {
                next_insn = (IRInst *) List_Next ((void *)instr);

                if  ((*instr)->opcode == IRINST_OP_mul)
                    expand_mult (instr, stab);
                if  ((*instr)->opcode == IRINST_OP_div)
                    expand_divmod (instr, stab);
            }
        }
    }
}
