#include <stdlib.h>
#include <string.h>
#include "all.h"

void IRInstSetOperand(IRInst insn, int index, SymDef var)
{
    ssa_name Cursor = IRInstGetOperand (insn, index);
    Cursor->var = var;
}

int IRInstGetNumOperands(IRInst insn)
{
    if  (insn->opcode == IRINST_OP_phi ||
         insn->opcode == IRINST_OP_nop)
    {
        return List_Card(insn->operands);
    }

    switch (insn->opcode)
    {
#define OPDEF(OPCODE, OPNAME, name_zenglj, OPKIND, operand_count) \
    case IRINST_OP_ ## OPCODE: return operand_count;
#include "IROpcode.h"
#undef  OPDEF
    default:
        return 0;
        break;
    }
}

const char *IRInstGetOperatorSpelling(enum IRInstOperator Operator)
{
    switch (Operator)
    {
#define OPDEF(OPCODE, OPNAME, name_zenglj, OPKIND, operand_count) \
    case IRINST_OP_ ## OPCODE: return OPNAME;
#include "IROpcode.h"
#undef  OPDEF
    default:
        return NULL;
        break;
    }
}

int IRInstGetOpKind(enum IRInstOperator Operator)
{
    switch (Operator)
    {
#define OPDEF(OPCODE, OPNAME, name_zenglj, OPKIND, operand_count) \
    case IRINST_OP_ ## OPCODE: return OPKIND;
#include "IROpcode.h"
#undef  OPDEF
    default:
        return 0;
        break;
    }
}

static const char *IRInstGetOperatorSpelling_zenglj(enum IRInstOperator Operator)
{
    switch (Operator)
    {
#define OPDEF(OPCODE, OPNAME, name_zenglj, OPKIND, operand_count) \
    case IRINST_OP_ ## OPCODE: return name_zenglj;
#include "IROpcode.h"
#undef  OPDEF
    default:
        return NULL;
        break;
    }
}

ssa_name IRInstGetOperand(IRInst insn, int index)
{
    ssa_name Cursor;
    int length = List_Card(insn->operands);

    while (length <= index)
    {
        Cursor = (ssa_name)List_NewLast(insn->operands, sizeof (*Cursor));
        memset ( Cursor, 0, sizeof ( *Cursor ) );
        Cursor->inst = insn;
        length++;
    }
    for(  Cursor=(ssa_name)List_First(insn->operands)
        ;  Cursor!=NULL && index>0
        ;  Cursor = (ssa_name)List_Next((void *)Cursor), index--
        )
        ;
    return Cursor; 
}

void IRInstDump (IRInst insn, BOOL pretty, FILE *dump_file)
{
    int count, i;
    ssa_name name;

    fprintf (dump_file, "%5d:  (", insn->uid);

    switch (insn->opcode)
    {
#define OPDEF(OPCODE, OPNAME, name_zenglj, OPKIND, operand_count) \
    case IRINST_OP_ ## OPCODE: fprintf (dump_file, "%s", OPNAME); break;
#include "IROpcode.h"
#undef  OPDEF
    default:
        break;
    }

    count = IRInstGetNumOperands(insn);
    for (i = 0; i < count; i++)
    {
        name = IRInstGetOperand(insn, i);
        if  (name->var)
            fprintf (dump_file, ", %s%s%d%s", stGetSymName(name->var), (pretty) ? "<SUB>" : "_", name->version, (pretty) ? "</SUB>" : "");
        else
            fprintf (dump_file, ", ");
    }
    fprintf (dump_file, ")");
    fprintf (dump_file,"  Row: %d  Col: %d",insn->line, insn->column);
}

static void
dump_variable (SymDef var, FILE *file)
{
    if  (var->sdType->tdTypeKind != TYP_VOID &&
         (var->sdType->tdTypeKind > TYP_lastIntrins || !var->sdVar.sdvConst))
    {
        fprintf (file, "    declare ");
        stTypeName_zenglj (var->sdType, var, file);
        fprintf (file, "\n");
    }
}

void IRInstDump_zenglj (IRInst insn, SymTab stab, FILE *dump_file)
{
    switch (insn->opcode)
    {
    case IRINST_OP_load:
    case IRINST_OP_store:
    case IRINST_OP_move:
        fputs("    ", dump_file);
        DumpSymName_zenglj (IRInstGetOperand(insn, 0)->var, dump_file);
        fprintf (dump_file, " = ");
        DumpSymName_zenglj (IRInstGetOperand(insn, 1)->var, dump_file);
        fputs("\n", dump_file);
        break;

    case IRINST_OP_aload:
        fputs("    ", dump_file);
        DumpSymName_zenglj (IRInstGetOperand(insn, 3)->var, dump_file);
        fprintf (dump_file, " = add ");
        DumpSymName_zenglj (IRInstGetOperand(insn, 1)->var, dump_file);
        fprintf (dump_file, ", ");
        DumpSymName_zenglj (IRInstGetOperand(insn, 2)->var, dump_file);

        fprintf (dump_file, "\n    ");

        DumpSymName_zenglj (IRInstGetOperand(insn, 0)->var, dump_file);
        fprintf (dump_file, " = *");
        DumpSymName_zenglj (IRInstGetOperand(insn, 3)->var, dump_file);
        fputs("\n", dump_file);
        break;

    case IRINST_OP_astore:
        fputs("    ", dump_file);
        DumpSymName_zenglj (IRInstGetOperand(insn, 4)->var, dump_file);
        fprintf (dump_file, " = add ");
        DumpSymName_zenglj (IRInstGetOperand(insn, 0)->var, dump_file);
        fprintf (dump_file, ", ");
        DumpSymName_zenglj (IRInstGetOperand(insn, 1)->var, dump_file);

        fprintf (dump_file, "\n    ");

        fprintf (dump_file, "*");
        DumpSymName_zenglj (IRInstGetOperand(insn, 4)->var, dump_file);
        fprintf (dump_file, " = ");
        DumpSymName_zenglj (IRInstGetOperand(insn, 2)->var, dump_file);
        fputs("\n", dump_file);
        break;

    case IRINST_OP_add:
    case IRINST_OP_sub:
    case IRINST_OP_mul:
    case IRINST_OP_div:
    case IRINST_OP_rem:
        fputs("    ", dump_file);
        DumpSymName_zenglj (IRInstGetOperand(insn, 0)->var, dump_file);
        fprintf (dump_file, " = %s%s ", IRInstGetOperand(insn, 0)->var->sdType->tdTypeKind == TYP_FLOAT ? "f" : "", IRInstGetOperatorSpelling_zenglj(insn->opcode));
        DumpSymName_zenglj (IRInstGetOperand(insn, 1)->var, dump_file);
        fprintf (dump_file, ", ");
        DumpSymName_zenglj (IRInstGetOperand(insn, 2)->var, dump_file);
        fputs("\n", dump_file);
        break;

    case IRINST_OP_neg:
    case IRINST_OP_i2f:
    case IRINST_OP_f2i:
        fputs("    ", dump_file);
        DumpSymName_zenglj (IRInstGetOperand(insn, 0)->var, dump_file);
        fprintf (dump_file, " = %s ", IRInstGetOperatorSpelling_zenglj(insn->opcode));
        DumpSymName_zenglj (IRInstGetOperand(insn, 1)->var, dump_file);
        fputs("\n", dump_file);
        break;

    case IRINST_OP_ifeq:
    case IRINST_OP_ifne:
    case IRINST_OP_iflt:
    case IRINST_OP_ifge:
    case IRINST_OP_ifgt:
    case IRINST_OP_ifle:
        fputs("    ", dump_file);
        DumpSymName_zenglj (IRInstGetOperand(insn, 4)->var, dump_file);
        fprintf (dump_file, " = %c%s ", IRInstGetOperand(insn, 0)->var->sdType->tdTypeKind == TYP_FLOAT ? 'f' : 'i', IRInstGetOperatorSpelling_zenglj(insn->opcode));
        DumpSymName_zenglj (IRInstGetOperand(insn, 0)->var, dump_file);
        fprintf (dump_file, ", ");
        DumpSymName_zenglj (IRInstGetOperand(insn, 1)->var, dump_file);

        fprintf (dump_file, "\n    ");

        fprintf (dump_file, "bc ");
        DumpSymName_zenglj (IRInstGetOperand(insn, 4)->var, dump_file);
        fprintf (dump_file, ", label .L");
        DumpSymName_zenglj (IRInstGetOperand(insn, 2)->var, dump_file);
        fprintf (dump_file, ", label .L");
        DumpSymName_zenglj (IRInstGetOperand(insn, 3)->var, dump_file);
        fputs("\n", dump_file);
        break;

    case IRINST_OP_goto:
        if  (insn->bb->cfg->entry_block_ptr != insn->bb)
        {
            fputs("    ", dump_file);
            fprintf (dump_file, "br label .L");
            DumpSymName_zenglj (IRInstGetOperand(insn, 0)->var, dump_file);
            fputs("\n", dump_file);
        }
        break;

    case IRINST_OP_call:
        fprintf (dump_file, "    ");
        if  (IRInstGetOperand(insn, 0)->var->sdType->tdTypeKind != TYP_VOID)
        {
            DumpSymName_zenglj (IRInstGetOperand(insn, 0)->var, dump_file);
            fputs(" = ", dump_file);
        }
        fputs("call ", dump_file);
        stTypeName_zenglj (IRInstGetOperand(insn, 1)->var->sdType->tdFnc.tdfRett, NULL, dump_file);
        fputc(' ', dump_file);
        DumpSymName_zenglj (IRInstGetOperand(insn, 1)->var, dump_file);
        fputs("()\n", dump_file);
        break;

    case IRINST_OP_param:
        fprintf (dump_file, "    arg ");
        DumpSymName_zenglj (IRInstGetOperand(insn, 0)->var, dump_file);
        fputs("\n", dump_file);
        break;

    case IRINST_OP_fparam:
        fputs("    ", dump_file);
        DumpSymName_zenglj (IRInstGetOperand(insn, 0)->var, dump_file);
        fputs(" = ", dump_file);
        IRInstGetOperand(insn, 0)->var->sdVar.sdvLocal = FALSE;
        DumpSymName_zenglj (IRInstGetOperand(insn, 0)->var, dump_file);
        IRInstGetOperand(insn, 0)->var->sdVar.sdvLocal = TRUE;
        fputs("\n", dump_file);
        break;

    case IRINST_OP_entry:
        fprintf (dump_file, "define ");
        stTypeName_zenglj (IRInstGetOperand(insn, 0)->var->sdType, IRInstGetOperand(insn, 0)->var, dump_file);
        fputs(" {\n", dump_file);
        traverse_symtree (IRInstGetOperand(insn, 0)->var, (void (*) (SymDef, void *))dump_variable, dump_file);
        fputs("    entry\n", dump_file);
        break;

    case IRINST_OP_exit:
        fputs("    exit ", dump_file);
        if  (IRInstGetOperand(insn, 0)->var->sdType->tdTypeKind != TYP_VOID)
            DumpSymName_zenglj (IRInstGetOperand(insn, 0)->var, dump_file);
        fputs("\n}\n", dump_file);
        break;

    default:
        break;
    }
}

IRInst IRInstEmitInst(enum IRInstOperator opcode, int line, int column)
{
    IRInst insn;
    insn = (IRInst)xmalloc (sizeof (*insn ) );
    memset (insn, 0, sizeof (*insn));
    insn->operands = List_Create();
    insn->opcode = opcode;
    insn->line = line;
    insn->column = column;
    return insn;
}

void IRInstDelInst(IRInst insn)
{
    if ( insn)
    {
        List_Destroy (&insn->operands);
        free (insn);
    }
}

void IRInstResize(IRInst insn, int length)
{
    ssa_name Cursor;
    int cur_length = List_Card(insn->operands);

    while (length > cur_length)
    {
        Cursor = (ssa_name)List_NewLast(insn->operands, sizeof (*Cursor));
        memset ( Cursor, 0, sizeof ( *Cursor ) );
        Cursor->inst = insn;
        cur_length++;
    }
    while (length < cur_length)
    {
        List_DeleteLast (insn->operands);
        cur_length--;
    }
}

BOOL
IRInstIsOutput (IRInst insn, int opnum)
{
    switch (insn->opcode)
    {
    case IRINST_OP_nop:
    case IRINST_OP_ifeq:
    case IRINST_OP_ifne:
    case IRINST_OP_iflt:
    case IRINST_OP_ifge:
    case IRINST_OP_ifgt:
    case IRINST_OP_ifle:
    case IRINST_OP_goto:
    case IRINST_OP_entry:
    case IRINST_OP_exit:
    case IRINST_OP_begin_block:
    case IRINST_OP_end_block:
    case IRINST_OP_param:
        /* FIXME: There are probably missing cases here, double check.  */
        return FALSE;
    case IRINST_OP_addptr:
        return opnum <= 1;
    case IRINST_OP_load:
    case IRINST_OP_aload:
    case IRINST_OP_store:
    case IRINST_OP_astore:
    case IRINST_OP_move:
    case IRINST_OP_add:
    case IRINST_OP_sub:
    case IRINST_OP_mul:
    case IRINST_OP_div:
    case IRINST_OP_rem:
    case IRINST_OP_lsl:
    case IRINST_OP_lsr:
    case IRINST_OP_asr:
    case IRINST_OP_neg:
    case IRINST_OP_i2f:
    case IRINST_OP_f2i:
    case IRINST_OP_eq:
    case IRINST_OP_ne:
    case IRINST_OP_lt:
    case IRINST_OP_ge:
    case IRINST_OP_gt:
    case IRINST_OP_le:
    case IRINST_OP_not:
    case IRINST_OP_call:
    case IRINST_OP_fparam:
    case IRINST_OP_phi:
        return opnum == 0;
    }
    return FALSE;
}

int
cmp_ssa_name (ssa_name a, ssa_name b, void *no_backend)
{
    if  (a->var->sdType->tdTypeKind != b->var->sdType->tdTypeKind)
        return a->var->sdType->tdTypeKind - b->var->sdType->tdTypeKind;

    if  (a->var->sdType->tdTypeKind <= TYP_lastIntrins)
    {
        if      (a->var->sdVar.sdvConst != b->var->sdVar.sdvConst)
        {
            return  a->var->sdVar.sdvConst - b->var->sdVar.sdvConst;
        }
        else if (a->var->sdVar.sdvConst && b->var->sdVar.sdvConst)
        {
            return compare_constant (GetConstVal(a->var, 0), GetConstVal (b->var, 0), NULL);
        }
    }

    if  (a->var->uid != b->var->uid || 
         (!no_backend && a->var->sdType->tdTypeKind == TYP_ARRAY))
        return a->var->uid - b->var->uid;

    return a->version - b->version;

}

IRInst
IRInstCopy (IRInst orig)
{
    IRInst copy = IRInstEmitInst(orig->opcode, orig->line, orig->column);
    int i;

    if  (orig->opcode == IRINST_OP_begin_block || orig->opcode == IRINST_OP_end_block)
    {
        copy->opcode = IRINST_OP_nop;
    }
    else
    {
        for (i = 0; i < IRInstGetNumOperands(orig); i++)
        {
            IRInstSetOperand(copy, i, IRInstGetOperand(orig, i)->var);
            IRInstGetOperand(copy, i)->version = IRInstGetOperand(orig, i)->version;
        }
    }
    return copy;
}
