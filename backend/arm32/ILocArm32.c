#include <string.h>
#include "all.h"
#include "CodeGeneratorArm32.h"

ArmInst
emit_ArmInst (basic_block bb, enum ArmInstOperator opcode)
{
    ArmInst insn;
    insn = (ArmInst) List_NewLast (((struct BblockArm32 *)bb->param)->code, sizeof (*insn ) );
    memset (insn, 0, sizeof (*insn));
    insn->lir.operands = List_Create ();
    insn->lir.bb = bb;
    insn->lir.opcode = opcode;
    return insn;
}

void
delete_ArmInst (ArmInst insn)
{
    LIR_Opr iter;

    if ( insn)
    {
        if  (insn->lir.opcode == ARMINST_OP_STRING)
        {
            for(  iter = (LIR_Opr)List_First(insn->lir.operands)
               ;  iter != NULL
               ;  iter = (LIR_Opr)List_Next ((void *)iter)
               )
               dyn_string_delete (iter->ds);
        }
        List_Destroy (&insn->lir.operands);
        List_Delete (insn);
    }
}

ArmInst
emit_ArmInst_before (ArmInst before, enum ArmInstOperator opcode)
{
    ArmInst prev;
    prev = (ArmInst)List_NewBefore (((struct BblockArm32 *)before->lir.bb->param)->code, before, sizeof (*before ) );
    memset (prev, 0, sizeof (*prev));
    prev->lir.operands = List_Create ();
    prev->lir.bb = before->lir.bb;
    prev->lir.opcode = opcode;
    return prev;
}

ArmInst
emit_ArmInst_after (ArmInst after, enum ArmInstOperator opcode)
{
    ArmInst next;
    next = (ArmInst)List_NewAfter (((struct BblockArm32 *)after->lir.bb->param)->code, after, sizeof (*after ) );
    memset (next, 0, sizeof (*next));
    next->lir.operands = List_Create ();
    next->lir.bb = after->lir.bb;
    next->lir.opcode = opcode;
    return next;
}

LIR_Opr
ArmInst_get_operand (ArmInst insn, int index)
{
    LIR_Opr Cursor;
    int length = List_Card(insn->lir.operands);

    while (length <= index)
    {
        Cursor = (LIR_Opr)List_NewLast(insn->lir.operands, sizeof (*Cursor));
        memset ( Cursor, 0, sizeof ( *Cursor ) );
        length++;
    }
    for(  Cursor=(LIR_Opr)List_First(insn->lir.operands)
       ;  Cursor!=NULL && index>0
       ;  Cursor = (LIR_Opr)List_Next((void *)Cursor), index--
       )
        ;
    return Cursor; 
}

vreg_t
ArmInst_get_as_Register (ArmInst insn, int index)
{
    LIR_Opr operand;
    operand = ArmInst_get_operand (insn, index);
    switch ((enum ArmInstOperator) insn->lir.opcode)
    {
    case ARMINST_OP_LDR_const:
    case ARMINST_OP_LDR_label:
    case ARMINST_OP_MOV_imm:
    case ARMINST_OP_BX:
    case ARMINST_OP_CMP_imm:
        if  (index == 0)
            return operand->vreg;
        break;

    case ARMINST_OP_MOV_reg:
    case ARMINST_OP_CLZ:
    case ARMINST_OP_ADD_reg:
    case ARMINST_OP_SDIV_reg:
    case ARMINST_OP_MUL_reg:
    case ARMINST_OP_SUB_reg:
    case ARMINST_OP_RSB_reg:
    case ARMINST_OP_CMP_reg:
    case ARMINST_OP_MLA:
    case ARMINST_OP_MLS:
    case ARMINST_OP_LDR_reg:
    case ARMINST_OP_STR_reg:
    case ARMINST_OP_LDR_reg_reg:
    case ARMINST_OP_STR_reg_reg:
    case ARMINST_OP_PUSH:
    case ARMINST_OP_POP:
    case ARMINST_OP_VADD:
    case ARMINST_OP_VDIV:
    case ARMINST_OP_VMUL:
    case ARMINST_OP_VSUB:
    case ARMINST_OP_VMOV:
    case ARMINST_OP_vcvt_floatingPointToSigned:
    case ARMINST_OP_vcvt_signedToFloatingPoint:
    case ARMINST_OP_VNEG:
    case ARMINST_OP_VCMP:
    case ARMINST_OP_VCMPz:
    case ARMINST_OP_VLDR_reg:
    case ARMINST_OP_VSTR_reg:
    case ARMINST_OP_VPUSH:
    case ARMINST_OP_VPOP:
        return operand->vreg;
        break;

    case ARMINST_OP_B:
    case ARMINST_OP_BL:
    case ARMINST_OP_STRING:
    case ARMINST_OP_LABEL:
    case ARMINST_OP_VMRS:
        break;

    case ARMINST_OP_ADD_imm:
    case ARMINST_OP_SUB_imm:
    case ARMINST_OP_RSB_imm:
    case ARMINST_OP_LSR_imm:
    case ARMINST_OP_LSL_imm:
    case ARMINST_OP_ASR_imm:
    case ARMINST_OP_LDR_imm:
    case ARMINST_OP_STR_imm:
    case ARMINST_OP_VLDR_imm:
    case ARMINST_OP_VSTR_imm:
    case ARMINST_OP_CMP_reg_LSL_imm:
    case ARMINST_OP_CMP_reg_LSR_imm:
    case ARMINST_OP_CMP_reg_ASR_imm:
    case ARMINST_OP_MOV_reg_LSL_imm:
    case ARMINST_OP_MOV_reg_LSR_imm:
    case ARMINST_OP_MOV_reg_ASR_imm:
        if  (index == 0 || index == 1)
            return operand->vreg;
        break;

    case ARMINST_OP_REGISTER:
        break;

    case ARMINST_OP_LDR_reg_LSL_imm:
    case ARMINST_OP_STR_reg_LSL_imm:
    case ARMINST_OP_ADD_reg_LSL_imm:
    case ARMINST_OP_ADD_reg_LSR_imm:
    case ARMINST_OP_ADD_reg_ASR_imm:
    case ARMINST_OP_SUB_reg_LSL_imm:
    case ARMINST_OP_SUB_reg_LSR_imm:
    case ARMINST_OP_SUB_reg_ASR_imm:
        if  (index == 0 || index == 1 || index == 2)
            return operand->vreg;
        break;
        /* FIXME: There are probably missing cases here, double check.  */
    }

    return NULL;
}

void
ArmInst_set_operand (ArmInst insn, int index, LIR_Opr operand)
{
    LIR_Opr Cursor = ArmInst_get_operand (insn, index);
    memcpy (Cursor, operand, sizeof (*operand));
}

const char *ArmInstGetOperatorSpelling(enum ArmInstOperator Operator)
{
    switch (Operator)
    {
#define OPDEF(OPNAME, OPCODE, name_string, type, operand_count) \
    case ARMINST_ ## OPNAME: return name_string;
#include "oparm32.h"
#undef  OPDEF
    default:
        return NULL;
        break;
    }
}

const char *ArmInstGetTypeString(enum ArmInstOperator Operator)
{
    switch (Operator)
    {
#define OPDEF(OPNAME, OPCODE, name_string, type, operand_count) \
    case ARMINST_ ## OPNAME: return type;
#include "oparm32.h"
#undef  OPDEF
    default:
        return NULL;
        break;
    }
}


int ArmInstGetNumOperands(ArmInst insn)
{
    if  (insn->lir.opcode == ARMINST_OP_PUSH ||
         insn->lir.opcode == ARMINST_OP_POP ||
         insn->lir.opcode == ARMINST_OP_VPOP ||
         insn->lir.opcode == ARMINST_OP_VPUSH)
    {
        return  List_Card (insn->lir.operands);
    }

    switch ((enum ArmInstOperator) insn->lir.opcode)
    {
#define OPDEF(OPNAME, OPCODE, name_string, type, operand_count) \
    case ARMINST_ ## OPNAME: return operand_count;
#include "oparm32.h"
#undef  OPDEF
    default:
        return 0;
        break;
    }
}

BOOL
ArmInstIsOutput (ArmInst inst, int index)
{
    switch ((enum ArmInstOperator) inst->lir.opcode)
    {
    case ARMINST_OP_LDR_const:
    case ARMINST_OP_LDR_label:
    case ARMINST_OP_MOV_imm:
    case ARMINST_OP_MOV_reg:
    case ARMINST_OP_CLZ:
    case ARMINST_OP_ADD_reg:
    case ARMINST_OP_SDIV_reg:
    case ARMINST_OP_MUL_reg:
    case ARMINST_OP_SUB_reg:
    case ARMINST_OP_RSB_reg:
    case ARMINST_OP_ADD_imm:
    case ARMINST_OP_SUB_imm:
    case ARMINST_OP_RSB_imm:
    case ARMINST_OP_LSR_imm:
    case ARMINST_OP_LSL_imm:
    case ARMINST_OP_ASR_imm:
    case ARMINST_OP_MLA:
    case ARMINST_OP_MLS:
    case ARMINST_OP_LDR_reg:
    case ARMINST_OP_LDR_reg_reg:
    case ARMINST_OP_LDR_imm:
    case ARMINST_OP_VADD:
    case ARMINST_OP_VDIV:
    case ARMINST_OP_VMUL:
    case ARMINST_OP_VSUB:
    case ARMINST_OP_VMOV:
    case ARMINST_OP_vcvt_signedToFloatingPoint:
    case ARMINST_OP_vcvt_floatingPointToSigned:
    case ARMINST_OP_VNEG:
    case ARMINST_OP_VLDR_reg:
    case ARMINST_OP_VLDR_imm:
    case ARMINST_OP_LDR_reg_LSL_imm:
    case ARMINST_OP_ADD_reg_LSL_imm:
    case ARMINST_OP_ADD_reg_LSR_imm:
    case ARMINST_OP_ADD_reg_ASR_imm:
    case ARMINST_OP_SUB_reg_LSL_imm:
    case ARMINST_OP_SUB_reg_LSR_imm:
    case ARMINST_OP_SUB_reg_ASR_imm:
    case ARMINST_OP_MOV_reg_LSL_imm:
    case ARMINST_OP_MOV_reg_LSR_imm:
    case ARMINST_OP_MOV_reg_ASR_imm:
        return index == 0;

    case ARMINST_OP_B:
    case ARMINST_OP_BL:
    case ARMINST_OP_BX:
    case ARMINST_OP_CMP_reg:
    case ARMINST_OP_CMP_imm:
    case ARMINST_OP_STR_reg:
    case ARMINST_OP_STR_reg_reg:
    case ARMINST_OP_STR_imm:
    case ARMINST_OP_LABEL:
    case ARMINST_OP_PUSH:
    case ARMINST_OP_STRING:
    case ARMINST_OP_VCMPz:
    case ARMINST_OP_VCMP:
    case ARMINST_OP_VMRS:
    case ARMINST_OP_VSTR_reg:
    case ARMINST_OP_VSTR_imm:
    case ARMINST_OP_VPUSH:
    case ARMINST_OP_STR_reg_LSL_imm:
    case ARMINST_OP_CMP_reg_LSL_imm:
    case ARMINST_OP_CMP_reg_LSR_imm:
    case ARMINST_OP_CMP_reg_ASR_imm:
        return FALSE;

    case ARMINST_OP_POP:
    case ARMINST_OP_VPOP:
        return TRUE;

    case ARMINST_OP_REGISTER:
        return FALSE;
        /* FIXME: There are probably missing cases here, double check.  */
    }
    return FALSE;
}

/* 获取指令定值的所有寄存器编号，使用位图接受返回值。  */
void
ArmInstOutput (ArmInst inst, bitmap bmp)
{
    int i;

    bitmap_clear (bmp);

    switch ((enum ArmInstOperator) inst->lir.opcode)
    {
    case ARMINST_OP_STRING:
    case ARMINST_OP_LABEL:
        break;
        /* FIXME: There are probably missing cases here, double check.  */
    default:
        bitmap_set_bit (bmp, PC_REGNUM);
        break;
    }

    switch ((enum ArmInstOperator) inst->lir.opcode)
    {
    case ARMINST_OP_BL:
        bitmap_set_bit (bmp, LR_REGNUM);
        bitmap_set_bit (bmp, IP_REGNUM);
        break;

    case ARMINST_OP_B:
        bitmap_set_bit (bmp, IP_REGNUM);
        break;
        /* FIXME: There are probably missing cases here, double check.  */
    default:
        break;
    }

    for (i = 0; i < ArmInstGetNumOperands(inst); i++)
        if  (ArmInstIsOutput (inst, i))
            bitmap_set_bit (bmp, ArmInst_get_operand (inst, i)->vreg->vregno);
}

BOOL
ArmInst_is_call (ArmInst inst)
{
    return inst->lir.opcode == ARMINST_OP_BL;
}

/* 获取指令使用的所有寄存器编号，使用位图接受返回值。  */
void
ArmInstInput (ArmInst inst, bitmap bmp)
{
    int i;

    bitmap_clear (bmp);

    switch ((enum ArmInstOperator) inst->lir.opcode)
    {
    case ARMINST_OP_STRING:
    case ARMINST_OP_LABEL:
        break;
      /* FIXME: There are probably missing cases here, double check.  */
    default:
        bitmap_set_bit (bmp, PC_REGNUM);
        break;
    }

    switch ((enum ArmInstOperator) inst->lir.opcode)
    {
    case ARMINST_OP_BL:
    case ARMINST_OP_B:
        bitmap_set_bit (bmp, IP_REGNUM);
        break;
    case ARMINST_OP_BX:
        bitmap_set_bit (bmp, R0_REGNUM);
        bitmap_set_bit (bmp, FIRST_VFP_REGNUM);
        break;
      /* FIXME: There are probably missing cases here, double check.  */
    default:
        break;
    }

    switch ((enum ArmInstOperator) inst->lir.opcode)
    {
    case ARMINST_OP_LDR_const:
    case ARMINST_OP_LDR_label:
    case ARMINST_OP_MOV_imm:
    case ARMINST_OP_B:
    case ARMINST_OP_BL:
    case ARMINST_OP_LABEL:
    case ARMINST_OP_POP:
    case ARMINST_OP_STRING:
    case ARMINST_OP_VMRS:
    case ARMINST_OP_VPOP:
        break;

    case ARMINST_OP_MOV_reg:
    case ARMINST_OP_CLZ:
    case ARMINST_OP_ADD_imm:
    case ARMINST_OP_SUB_imm:
    case ARMINST_OP_RSB_imm:
    case ARMINST_OP_LSR_imm:
    case ARMINST_OP_LSL_imm:
    case ARMINST_OP_ASR_imm:
    case ARMINST_OP_LDR_reg:
    case ARMINST_OP_LDR_imm:
    case ARMINST_OP_VMOV:
    case ARMINST_OP_vcvt_floatingPointToSigned:
    case ARMINST_OP_vcvt_signedToFloatingPoint:
    case ARMINST_OP_VNEG:
    case ARMINST_OP_VLDR_reg:
    case ARMINST_OP_VLDR_imm:
    case ARMINST_OP_MOV_reg_LSL_imm:
    case ARMINST_OP_MOV_reg_LSR_imm:
    case ARMINST_OP_MOV_reg_ASR_imm:
        bitmap_set_bit (bmp, ArmInst_get_operand (inst, 1)->vreg->vregno);
        break;

    case ARMINST_OP_BX:
    case ARMINST_OP_CMP_imm:
    case ARMINST_OP_VCMPz:
        bitmap_set_bit (bmp, ArmInst_get_operand (inst, 0)->vreg->vregno);
        break;

    case ARMINST_OP_ADD_reg:
    case ARMINST_OP_SDIV_reg:
    case ARMINST_OP_MUL_reg:
    case ARMINST_OP_SUB_reg:
    case ARMINST_OP_RSB_reg:
    case ARMINST_OP_LDR_reg_reg:
    case ARMINST_OP_VADD:
    case ARMINST_OP_VDIV:
    case ARMINST_OP_VMUL:
    case ARMINST_OP_VSUB:
    case ARMINST_OP_LDR_reg_LSL_imm:
    case ARMINST_OP_ADD_reg_LSL_imm:
    case ARMINST_OP_ADD_reg_LSR_imm:
    case ARMINST_OP_ADD_reg_ASR_imm:
    case ARMINST_OP_SUB_reg_LSL_imm:
    case ARMINST_OP_SUB_reg_LSR_imm:
    case ARMINST_OP_SUB_reg_ASR_imm:
        bitmap_set_bit (bmp, ArmInst_get_operand (inst, 1)->vreg->vregno);
        bitmap_set_bit (bmp, ArmInst_get_operand (inst, 2)->vreg->vregno);
        break;

    case ARMINST_OP_STR_reg_reg:
    case ARMINST_OP_STR_reg_LSL_imm:
        bitmap_set_bit (bmp, ArmInst_get_operand (inst, 0)->vreg->vregno);
        bitmap_set_bit (bmp, ArmInst_get_operand (inst, 1)->vreg->vregno);
        bitmap_set_bit (bmp, ArmInst_get_operand (inst, 2)->vreg->vregno);
        break;

    case ARMINST_OP_STR_reg:
    case ARMINST_OP_STR_imm:
    case ARMINST_OP_CMP_reg:
    case ARMINST_OP_VCMP:
    case ARMINST_OP_VSTR_reg:
    case ARMINST_OP_VSTR_imm:
    case ARMINST_OP_CMP_reg_LSL_imm:
    case ARMINST_OP_CMP_reg_LSR_imm:
    case ARMINST_OP_CMP_reg_ASR_imm:
        bitmap_set_bit (bmp, ArmInst_get_operand (inst, 0)->vreg->vregno);
        bitmap_set_bit (bmp, ArmInst_get_operand (inst, 1)->vreg->vregno);
        break;

    case ARMINST_OP_MLA:
    case ARMINST_OP_MLS:
        bitmap_set_bit (bmp, ArmInst_get_operand (inst, 1)->vreg->vregno);
        bitmap_set_bit (bmp, ArmInst_get_operand (inst, 2)->vreg->vregno);
        bitmap_set_bit (bmp, ArmInst_get_operand (inst, 3)->vreg->vregno);
        break;

    case ARMINST_OP_PUSH:
    case ARMINST_OP_VPUSH:
        for (i = 0; i < ArmInstGetNumOperands(inst); i++)
        {
            bitmap_set_bit (bmp, ArmInst_get_operand (inst, i)->vreg->vregno);
        }
        break;

    case ARMINST_OP_REGISTER:
        break;
        /* FIXME: There are probably missing cases here, double check.  */
    }
}

static BOOL
is_imm8m (int num)
{
    unsigned int new_num = (unsigned int) num;
    unsigned int overFlow;
    int i;

    for (i = 0; i < 16; i++) {

        if (new_num <= 0xff) {
            /* 有效表达式  */
            return TRUE;
        }

        /* 循环左移2位  */
        overFlow = new_num & 0xc0000000;
        new_num = (new_num << 2) | (overFlow >> 30);
    }

    return FALSE;
}

void
outputArmInst (ArmInst inst, FILE *file)
{
    int i;
    const char *comma = "";
    char *str = "";

    switch (inst->condition)
    {
    case ConditionAL: break;
    case ConditionEQ: str =  "eq"; break;
    case ConditionNE: str =  "ne"; break;
    case ConditionHS: str =  "cs"; break;
    case ConditionLO: str =  "cc"; break;
    case ConditionMI: str =  "mi"; break;
    case ConditionPL: str =  "pl"; break;
    case ConditionVS: str =  "vs"; break;
    case ConditionVC: str =  "vc"; break;
    case ConditionHI: str =  "hi"; break;
    case ConditionLS: str =  "ls"; break;
    case ConditionGE: str =  "ge"; break;
    case ConditionLT: str =  "lt"; break;
    case ConditionGT: str =  "gt"; break;
    case ConditionLE: str =  "le"; break;
    case ConditionInvalid: break;
    }

    if  (!strcmp (ArmInstGetOperatorSpelling (inst->lir.opcode), "<undef>"))
    {
        if  (ARMINST_OP_LABEL == inst->lir.opcode)
        {
            fprintf (file, ".L%d:", ArmInst_get_operand (inst, 0)->cval.cvValue.cvIval);
        }
        else if (ARMINST_OP_LDR_label == inst->lir.opcode)
        {
            fprintf (file, "\tmovw%s\t", str);
            output_reg (file, ArmInst_get_operand (inst, 0)->vreg);
            fprintf (file, ", #:lower16:%s\n", stGetSymName(ArmInst_get_operand (inst, 1)->sym));
            fprintf (file, "\tmovt%s\t", str);
            output_reg (file, ArmInst_get_operand (inst, 0)->vreg);
            fprintf (file, ", #:upper16:%s", stGetSymName(ArmInst_get_operand (inst, 1)->sym));
        }
        else if (ARMINST_OP_STRING == inst->lir.opcode)
        {
            fprintf (file, "%s", dyn_string_buf (ArmInst_get_operand (inst, 0)->ds));
        }
        else if (ARMINST_OP_LDR_const == inst->lir.opcode)
        {
            if      (is_imm8m(ArmInst_get_operand (inst, 1)->cval.cvValue.cvIval))
            {
                fprintf (file, "\tmov%s\t", str);
                output_reg (file, ArmInst_get_operand (inst, 0)->vreg);
                fprintf (file, ", 0x%x", ArmInst_get_operand (inst, 1)->cval.cvValue.cvIval);
            }
            else if (ArmInst_get_operand (inst, 1)->cval.cvValue.cvIval < 65536 && ArmInst_get_operand (inst, 1)->cval.cvValue.cvIval >= 0)
            {
                fprintf (file, "\tmovw%s\t", str);
                output_reg (file, ArmInst_get_operand (inst, 0)->vreg);
                fprintf (file, ", 0x%x", ArmInst_get_operand (inst, 1)->cval.cvValue.cvIval);
            }
            else
            {
                fprintf (file, "\tmovw%s\t", str);
                output_reg (file, ArmInst_get_operand (inst, 0)->vreg);
                fprintf (file, ", 0x%x\n", (int)(unsigned short)ArmInst_get_operand (inst, 1)->cval.cvValue.cvIval);
                fprintf (file, "\tmovt%s\t", str);
                output_reg (file, ArmInst_get_operand (inst, 0)->vreg);
                fprintf (file, ", 0x%x", (int)(unsigned short)((ArmInst_get_operand (inst, 1)->cval.cvValue.cvIval >> 16) & 0xffff));
            }
        }
        else if (ARMINST_OP_VMRS == inst->lir.opcode)
        {
            fprintf (file, "\tvmrs\tAPSR_nzcv, FPSCR");
        }
    }
    else
    {
        fprintf (file, "\t%s%s%s\t", ArmInstGetOperatorSpelling (inst->lir.opcode), str, ArmInstGetTypeString (inst->lir.opcode));

        switch ((enum ArmInstOperator) inst->lir.opcode)
        {
        case ARMINST_OP_MOV_imm:
            output_reg (file, ArmInst_get_operand (inst, 0)->vreg);
            fprintf (file, ", #%d", ArmInst_get_operand (inst, 1)->cval.cvValue.cvIval);
            break;

        case ARMINST_OP_MOV_reg:
        case ARMINST_OP_CLZ:
        case ARMINST_OP_VMOV:
        case ARMINST_OP_vcvt_signedToFloatingPoint:
        case ARMINST_OP_vcvt_floatingPointToSigned:
        case ARMINST_OP_VNEG:
            output_reg (file, ArmInst_get_operand (inst, 0)->vreg);
            fprintf (file, ", ");
            output_reg (file, ArmInst_get_operand (inst, 1)->vreg);
            break;

        case ARMINST_OP_B:
            fprintf (file, ".L%d", ArmInst_get_operand (inst, 0)->cval.cvValue.cvIval);
            break;

        case ARMINST_OP_BL:
            fprintf (file, "%s", stGetSymName(ArmInst_get_operand (inst, 0)->sym));
            break;

        case ARMINST_OP_BX:
            output_reg (file, ArmInst_get_operand (inst, 0)->vreg);
            break;

        case ARMINST_OP_ADD_reg:
        case ARMINST_OP_SDIV_reg:
        case ARMINST_OP_MUL_reg:
        case ARMINST_OP_SUB_reg:
        case ARMINST_OP_RSB_reg:
        case ARMINST_OP_CMP_reg:
        case ARMINST_OP_MLA:
        case ARMINST_OP_MLS:
        case ARMINST_OP_VADD:
        case ARMINST_OP_VDIV:
        case ARMINST_OP_VMUL:
        case ARMINST_OP_VSUB:
        case ARMINST_OP_VCMP:
            for (i = 0; i < ArmInstGetNumOperands(inst); i++)
            {
                fprintf (file, "%s", comma);
                output_reg (file, ArmInst_get_operand (inst, i)->vreg);
                comma = ", ";
            }
            break;

        case ARMINST_OP_ADD_imm:
        case ARMINST_OP_SUB_imm:
        case ARMINST_OP_RSB_imm:
        case ARMINST_OP_LSR_imm:
        case ARMINST_OP_LSL_imm:
        case ARMINST_OP_ASR_imm:
            output_reg (file, ArmInst_get_operand (inst, 0)->vreg);
            fprintf (file, ", ");
            output_reg (file, ArmInst_get_operand (inst, 1)->vreg);
            fprintf (file, ", #%d", ArmInst_get_operand (inst, 2)->cval.cvValue.cvIval);
            break;

        case ARMINST_OP_CMP_imm:
            output_reg (file, ArmInst_get_operand (inst, 0)->vreg);
            fprintf (file, ", #%d", ArmInst_get_operand (inst, 1)->cval.cvValue.cvIval);
            break;
    
        case ARMINST_OP_LDR_reg:
        case ARMINST_OP_STR_reg:
        case ARMINST_OP_VLDR_reg:
        case ARMINST_OP_VSTR_reg:
            output_reg (file, ArmInst_get_operand (inst, 0)->vreg);
            fprintf (file, ", [");
            output_reg (file, ArmInst_get_operand (inst, 1)->vreg);
            fprintf (file, "]");
            break;

        case ARMINST_OP_LDR_reg_reg:
        case ARMINST_OP_STR_reg_reg:
            output_reg (file, ArmInst_get_operand (inst, 0)->vreg);
            fprintf (file, ", [");
            output_reg (file, ArmInst_get_operand (inst, 1)->vreg);
            fprintf (file, ", ");
            output_reg (file, ArmInst_get_operand (inst, 2)->vreg);
            fprintf (file, "]");
            break;

        case ARMINST_OP_LDR_imm:
        case ARMINST_OP_STR_imm:
        case ARMINST_OP_VLDR_imm:
        case ARMINST_OP_VSTR_imm:
            output_reg (file, ArmInst_get_operand (inst, 0)->vreg);
            fprintf (file, ", [");
            output_reg (file, ArmInst_get_operand (inst, 1)->vreg);
            fprintf (file, ", #%d]", ArmInst_get_operand (inst, 2)->cval.cvValue.cvIval);
            break;

        case ARMINST_OP_REGISTER:
            break;

        case ARMINST_OP_VMRS:
            break;

        case ARMINST_OP_LDR_const:
        case ARMINST_OP_LABEL:
        case ARMINST_OP_LDR_label:
        case ARMINST_OP_STRING:
            fatal ("internal compiler error");
            break;

        case ARMINST_OP_PUSH:
        case ARMINST_OP_POP:
        case ARMINST_OP_VPUSH:
        case ARMINST_OP_VPOP:
            fprintf (file, " {");
            for (i = 0; i < ArmInstGetNumOperands(inst); i++)
            {
                fprintf (file, "%s", comma);
                output_reg (file, ArmInst_get_operand (inst, i)->vreg);
                comma = ", ";
            }
            fprintf (file, "}");
            break;

        case ARMINST_OP_VCMPz:
            output_reg (file, ArmInst_get_operand (inst, 0)->vreg);
            fprintf (file, ", #0");
            break;

        case ARMINST_OP_LDR_reg_LSL_imm:
        case ARMINST_OP_STR_reg_LSL_imm:
            output_reg (file, ArmInst_get_operand (inst, 0)->vreg);
            fprintf (file, ", [");
            output_reg (file, ArmInst_get_operand (inst, 1)->vreg);
            fprintf (file, ", ");
            output_reg (file, ArmInst_get_operand (inst, 2)->vreg);
            fprintf (file, ", lsl #%d]", ArmInst_get_operand (inst, 3)->cval.cvValue.cvIval);
            break;

        case ARMINST_OP_ADD_reg_LSL_imm:
        case ARMINST_OP_SUB_reg_LSL_imm:
            output_reg (file, ArmInst_get_operand (inst, 0)->vreg);
            fprintf (file, ", ");
            output_reg (file, ArmInst_get_operand (inst, 1)->vreg);
            fprintf (file, ", ");
            output_reg (file, ArmInst_get_operand (inst, 2)->vreg);
            fprintf (file, ", lsl #%d", ArmInst_get_operand (inst, 3)->cval.cvValue.cvIval);
            break;

        case ARMINST_OP_ADD_reg_LSR_imm:
        case ARMINST_OP_SUB_reg_LSR_imm:
            output_reg (file, ArmInst_get_operand (inst, 0)->vreg);
            fprintf (file, ", ");
            output_reg (file, ArmInst_get_operand (inst, 1)->vreg);
            fprintf (file, ", ");
            output_reg (file, ArmInst_get_operand (inst, 2)->vreg);
            fprintf (file, ", lsr #%d", ArmInst_get_operand (inst, 3)->cval.cvValue.cvIval);
            break;

        case ARMINST_OP_ADD_reg_ASR_imm:
        case ARMINST_OP_SUB_reg_ASR_imm:
            output_reg (file, ArmInst_get_operand (inst, 0)->vreg);
            fprintf (file, ", ");
            output_reg (file, ArmInst_get_operand (inst, 1)->vreg);
            fprintf (file, ", ");
            output_reg (file, ArmInst_get_operand (inst, 2)->vreg);
            fprintf (file, ", asr #%d", ArmInst_get_operand (inst, 3)->cval.cvValue.cvIval);
            break;

        case ARMINST_OP_CMP_reg_LSL_imm:
        case ARMINST_OP_MOV_reg_LSL_imm:
            output_reg (file, ArmInst_get_operand (inst, 0)->vreg);
            fprintf (file, ", ");
            output_reg (file, ArmInst_get_operand (inst, 1)->vreg);
            fprintf (file, ", lsl #%d", ArmInst_get_operand (inst, 2)->cval.cvValue.cvIval);
            break;

        case ARMINST_OP_CMP_reg_LSR_imm:
        case ARMINST_OP_MOV_reg_LSR_imm:
            output_reg (file, ArmInst_get_operand (inst, 0)->vreg);
            fprintf (file, ", ");
            output_reg (file, ArmInst_get_operand (inst, 1)->vreg);
            fprintf (file, ", lsr #%d", ArmInst_get_operand (inst, 2)->cval.cvValue.cvIval);
            break;

        case ARMINST_OP_CMP_reg_ASR_imm:
        case ARMINST_OP_MOV_reg_ASR_imm:
            output_reg (file, ArmInst_get_operand (inst, 0)->vreg);
            fprintf (file, ", ");
            output_reg (file, ArmInst_get_operand (inst, 1)->vreg);
            fprintf (file, ", asr #%d", ArmInst_get_operand (inst, 2)->cval.cvValue.cvIval);
            break;
        }
    }

    fprintf(file, "\t@ Sequence number: %d", inst->lir.uid);
    fprintf (file, "\n");
}

static BOOL
isNopCopy (ArmInst instr)
{
    BOOL retval;
    if  ((instr->lir. opcode == ARMINST_OP_MOV_reg ||
         instr->lir.opcode == ARMINST_OP_VMOV) &&
         ArmInst_get_as_Register (instr, 0)->hard_num == ArmInst_get_as_Register (instr, 1)->hard_num &&
         ArmInst_get_as_Register (instr, 0)->hard_num != -1)
        retval = TRUE;
    else
        retval = FALSE;
    return retval;
}

BOOL
ArmInst_is_move (ArmInst instr)
{
    return (instr->lir.opcode == ARMINST_OP_MOV_reg ||
            (instr->lir.opcode == ARMINST_OP_VMOV &&
            ArmInst_get_as_Register (instr, 1)->rclass == ArmInst_get_as_Register (instr, 0)->rclass));
}

void
outputArmCode (control_flow_graph fn, FILE *file)
{
    basic_block *bb;
    ArmInst insn;
    LIST blocks;

    blocks = List_Create ();
    pre_and_rev_post_order_compute (fn, NULL, blocks, TRUE, FALSE);

    for(  bb=(basic_block *)List_First(blocks)
       ;  bb!=NULL
       ;  bb = (basic_block *)List_Next((void *)bb)
       )
    {
        for(  insn=(ArmInst)List_First(((struct BblockArm32 *)(*bb)->param)->code)
           ;  insn!=NULL
           ;  insn = (ArmInst)List_Next((void *)insn)
           )
        {
            /* 避免生成冗余的goto指令。  */
            if  (! List_Next ((void *)insn) &&
                 insn->lir.opcode == ARMINST_OP_B &&
                 insn->condition == ConditionAL &&
                 List_Next((void *)bb) &&
                 (*(IRInst *) List_First ((*(basic_block *)List_Next((void *)bb))->insns))->uid == ArmInst_get_operand (insn, 0)->cval.cvValue.cvIval)
                continue;

            if  (! isNopCopy (insn))
                outputArmInst (insn, file);
        }
    }
    List_Destroy (&blocks);
}
