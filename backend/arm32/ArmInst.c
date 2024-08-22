#include <string.h>
#include "all.h"

ArmInst emitArmInst (basic_block bb, enum ArmInstOperator opcode)
{
    ArmInst insn;
    insn = (ArmInst)List_NewLast (((BblockArm32)bb->param)->code, sizeof (*insn ) );
    memset (insn, 0, sizeof (*insn));
    insn->operands = List_Create ();
    insn->bb = bb;
    insn->opcode = opcode;
    return insn;
}

void deleteArmInst(ArmInst Cursor)
{
    Arm_Operand iter;

    if ( Cursor)
    {
        if  (Cursor->opcode == ARMINST_OP_STRING)
        {
            for(  iter = (Arm_Operand)List_First(Cursor->operands)
               ;  iter != NULL
               ;  iter = (Arm_Operand)List_Next ((void *)iter)
               )
               dyn_string_delete (iter->ds);
        }
        List_Destroy (&Cursor->operands);
        List_Delete (Cursor);
    }
}

ArmInst ArmInstInsertBefore (ArmInst insn, enum ArmInstOperator opcode)
{
    ArmInst new_insn;
    new_insn = (ArmInst)List_NewBefore (((BblockArm32)insn->bb->param)->code, insn, sizeof (*insn ) );
    memset (new_insn, 0, sizeof (*new_insn));
    new_insn->operands = List_Create ();
    new_insn->bb = insn->bb;
    new_insn->opcode = opcode;
    return new_insn;
}

ArmInst ArmInstInsertAfter (ArmInst insn, enum ArmInstOperator opcode)
{
    ArmInst new_insn;
    new_insn = (ArmInst)List_NewAfter (((BblockArm32)insn->bb->param)->code, insn, sizeof (*insn ) );
    memset (new_insn, 0, sizeof (*new_insn));
    new_insn->operands = List_Create ();
    new_insn->bb = insn->bb;
    new_insn->opcode = opcode;
    return new_insn;
}

Arm_Operand ArmInstGetOperand(ArmInst insn, int index)
{
    Arm_Operand Cursor;
    int length = List_Card(insn->operands);

    while (length <= index)
    {
        Cursor = (Arm_Operand)List_NewLast(insn->operands, sizeof (*Cursor));
        memset ( Cursor, 0, sizeof ( *Cursor ) );
        length++;
    }
    for(  Cursor=(Arm_Operand)List_First(insn->operands)
        ;  Cursor!=NULL && index>0
        ;  Cursor = (Arm_Operand)List_Next((void *)Cursor), index--
        )
        ;
    return Cursor; 
}

vreg_tArm32 ArmInstGetOperandAsReg(ArmInst inst, int index)
{
    Arm_Operand operand;
    operand = ArmInstGetOperand (inst, index);
    switch (inst->opcode)
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

void ArmInstSetOperand(ArmInst insn, int index, union Arm_Operand operand)
{
    Arm_Operand Cursor = ArmInstGetOperand (insn, index);
    *Cursor = operand;
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
    if  (insn->opcode == ARMINST_OP_PUSH ||
         insn->opcode == ARMINST_OP_POP ||
         insn->opcode == ARMINST_OP_VPOP ||
         insn->opcode == ARMINST_OP_VPUSH)
    {
        return  List_Card (insn->operands);
    }

    switch (insn->opcode)
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
    switch (inst->opcode)
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

    switch (inst->opcode)
    {
    case ARMINST_OP_STRING:
    case ARMINST_OP_LABEL:
        break;
      /* FIXME: There are probably missing cases here, double check.  */
    default:
        bitmap_set_bit (bmp, PC_REGNUM);
        break;
    }

    switch (inst->opcode)
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
            bitmap_set_bit (bmp, ArmInstGetOperand (inst, i)->vreg->vregno);
}

BOOL
ArmInst_is_call (ArmInst inst)
{
    return inst->opcode == ARMINST_OP_BL;
}

/* 获取指令使用的所有寄存器编号，使用位图接受返回值。  */
void
ArmInstInput (ArmInst inst, bitmap bmp)
{
    int i;

    bitmap_clear (bmp);

    switch (inst->opcode)
    {
    case ARMINST_OP_STRING:
    case ARMINST_OP_LABEL:
        break;
      /* FIXME: There are probably missing cases here, double check.  */
    default:
        bitmap_set_bit (bmp, PC_REGNUM);
        break;
    }

    switch (inst->opcode)
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

    switch (inst->opcode)
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
        bitmap_set_bit (bmp, ArmInstGetOperand (inst, 1)->vreg->vregno);
        break;

    case ARMINST_OP_BX:
    case ARMINST_OP_CMP_imm:
    case ARMINST_OP_VCMPz:
        bitmap_set_bit (bmp, ArmInstGetOperand (inst, 0)->vreg->vregno);
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
        bitmap_set_bit (bmp, ArmInstGetOperand (inst, 1)->vreg->vregno);
        bitmap_set_bit (bmp, ArmInstGetOperand (inst, 2)->vreg->vregno);
        break;

    case ARMINST_OP_STR_reg_reg:
    case ARMINST_OP_STR_reg_LSL_imm:
        bitmap_set_bit (bmp, ArmInstGetOperand (inst, 0)->vreg->vregno);
        bitmap_set_bit (bmp, ArmInstGetOperand (inst, 1)->vreg->vregno);
        bitmap_set_bit (bmp, ArmInstGetOperand (inst, 2)->vreg->vregno);
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
        bitmap_set_bit (bmp, ArmInstGetOperand (inst, 0)->vreg->vregno);
        bitmap_set_bit (bmp, ArmInstGetOperand (inst, 1)->vreg->vregno);
        break;

    case ARMINST_OP_MLA:
    case ARMINST_OP_MLS:
        bitmap_set_bit (bmp, ArmInstGetOperand (inst, 1)->vreg->vregno);
        bitmap_set_bit (bmp, ArmInstGetOperand (inst, 2)->vreg->vregno);
        bitmap_set_bit (bmp, ArmInstGetOperand (inst, 3)->vreg->vregno);
        break;

    case ARMINST_OP_PUSH:
    case ARMINST_OP_VPUSH:
        for (i = 0; i < ArmInstGetNumOperands(inst); i++)
        {
            bitmap_set_bit (bmp, ArmInstGetOperand (inst, i)->vreg->vregno);
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

    if  (!strcmp (ArmInstGetOperatorSpelling (inst->opcode), "<undef>"))
    {
        if  (ARMINST_OP_LABEL == inst->opcode)
        {
            fprintf (file, ".L%d:", ArmInstGetOperand (inst, 0)->cval.cvValue.cvIval);
        }
        else if (ARMINST_OP_LDR_label == inst->opcode)
        {
            fprintf (file, "\tmovw%s\t", str);
            output_reg (file, ArmInstGetOperand (inst, 0)->vreg);
            fprintf (file, ", #:lower16:%s\n", stGetSymName(ArmInstGetOperand (inst, 1)->sym));
            fprintf (file, "\tmovt%s\t", str);
            output_reg (file, ArmInstGetOperand (inst, 0)->vreg);
            fprintf (file, ", #:upper16:%s", stGetSymName(ArmInstGetOperand (inst, 1)->sym));
        }
        else if (ARMINST_OP_STRING == inst->opcode)
        {
            fprintf (file, "%s", dyn_string_buf (ArmInstGetOperand (inst, 0)->ds));
        }
        else if (ARMINST_OP_LDR_const == inst->opcode)
        {
            if      (is_imm8m(ArmInstGetOperand (inst, 1)->cval.cvValue.cvIval))
            {
                fprintf (file, "\tmov%s\t", str);
                output_reg (file, ArmInstGetOperand (inst, 0)->vreg);
                fprintf (file, ", 0x%x", ArmInstGetOperand (inst, 1)->cval.cvValue.cvIval);
            }
            else if (ArmInstGetOperand (inst, 1)->cval.cvValue.cvIval < 65536 && ArmInstGetOperand (inst, 1)->cval.cvValue.cvIval >= 0)
            {
                fprintf (file, "\tmovw%s\t", str);
                output_reg (file, ArmInstGetOperand (inst, 0)->vreg);
                fprintf (file, ", 0x%x", ArmInstGetOperand (inst, 1)->cval.cvValue.cvIval);
            }
            else
            {
                fprintf (file, "\tmovw%s\t", str);
                output_reg (file, ArmInstGetOperand (inst, 0)->vreg);
                fprintf (file, ", 0x%x\n", (int)(unsigned short)ArmInstGetOperand (inst, 1)->cval.cvValue.cvIval);
                fprintf (file, "\tmovt%s\t", str);
                output_reg (file, ArmInstGetOperand (inst, 0)->vreg);
                fprintf (file, ", 0x%x", (int)(unsigned short)((ArmInstGetOperand (inst, 1)->cval.cvValue.cvIval >> 16) & 0xffff));
            }
        }
        else if (ARMINST_OP_VMRS == inst->opcode)
        {
            fprintf (file, "\tvmrs\tAPSR_nzcv, FPSCR");
        }
    }
    else
    {
        fprintf (file, "\t%s%s%s\t", ArmInstGetOperatorSpelling (inst->opcode), str, ArmInstGetTypeString (inst->opcode));

        switch (inst->opcode)
        {
        case ARMINST_OP_MOV_imm:
            output_reg (file, ArmInstGetOperand (inst, 0)->vreg);
            fprintf (file, ", #%d", ArmInstGetOperand (inst, 1)->cval.cvValue.cvIval);
            break;

        case ARMINST_OP_MOV_reg:
        case ARMINST_OP_CLZ:
        case ARMINST_OP_VMOV:
        case ARMINST_OP_vcvt_signedToFloatingPoint:
        case ARMINST_OP_vcvt_floatingPointToSigned:
        case ARMINST_OP_VNEG:
            output_reg (file, ArmInstGetOperand (inst, 0)->vreg);
            fprintf (file, ", ");
            output_reg (file, ArmInstGetOperand (inst, 1)->vreg);
            break;

        case ARMINST_OP_B:
            fprintf (file, ".L%d", ArmInstGetOperand (inst, 0)->cval.cvValue.cvIval);
            break;

        case ARMINST_OP_BL:
            fprintf (file, "%s", stGetSymName(ArmInstGetOperand (inst, 0)->sym));
            break;

        case ARMINST_OP_BX:
            output_reg (file, ArmInstGetOperand (inst, 0)->vreg);
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
                output_reg (file, ArmInstGetOperand (inst, i)->vreg);
                comma = ", ";
            }
            break;

        case ARMINST_OP_ADD_imm:
        case ARMINST_OP_SUB_imm:
        case ARMINST_OP_RSB_imm:
        case ARMINST_OP_LSR_imm:
        case ARMINST_OP_LSL_imm:
        case ARMINST_OP_ASR_imm:
            output_reg (file, ArmInstGetOperand (inst, 0)->vreg);
            fprintf (file, ", ");
            output_reg (file, ArmInstGetOperand (inst, 1)->vreg);
            fprintf (file, ", #%d", ArmInstGetOperand (inst, 2)->cval.cvValue.cvIval);
            break;

        case ARMINST_OP_CMP_imm:
            output_reg (file, ArmInstGetOperand (inst, 0)->vreg);
            fprintf (file, ", #%d", ArmInstGetOperand (inst, 1)->cval.cvValue.cvIval);
            break;
    
        case ARMINST_OP_LDR_reg:
        case ARMINST_OP_STR_reg:
        case ARMINST_OP_VLDR_reg:
        case ARMINST_OP_VSTR_reg:
            output_reg (file, ArmInstGetOperand (inst, 0)->vreg);
            fprintf (file, ", [");
            output_reg (file, ArmInstGetOperand (inst, 1)->vreg);
            fprintf (file, "]");
            break;

        case ARMINST_OP_LDR_reg_reg:
        case ARMINST_OP_STR_reg_reg:
            output_reg (file, ArmInstGetOperand (inst, 0)->vreg);
            fprintf (file, ", [");
            output_reg (file, ArmInstGetOperand (inst, 1)->vreg);
            fprintf (file, ", ");
            output_reg (file, ArmInstGetOperand (inst, 2)->vreg);
            fprintf (file, "]");
            break;

        case ARMINST_OP_LDR_imm:
        case ARMINST_OP_STR_imm:
        case ARMINST_OP_VLDR_imm:
        case ARMINST_OP_VSTR_imm:
            output_reg (file, ArmInstGetOperand (inst, 0)->vreg);
            fprintf (file, ", [");
            output_reg (file, ArmInstGetOperand (inst, 1)->vreg);
            fprintf (file, ", #%d]", ArmInstGetOperand (inst, 2)->cval.cvValue.cvIval);
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
                output_reg (file, ArmInstGetOperand (inst, i)->vreg);
                comma = ", ";
            }
            fprintf (file, "}");
            break;

        case ARMINST_OP_VCMPz:
            output_reg (file, ArmInstGetOperand (inst, 0)->vreg);
            fprintf (file, ", #0");
            break;

        case ARMINST_OP_LDR_reg_LSL_imm:
        case ARMINST_OP_STR_reg_LSL_imm:
            output_reg (file, ArmInstGetOperand (inst, 0)->vreg);
            fprintf (file, ", [");
            output_reg (file, ArmInstGetOperand (inst, 1)->vreg);
            fprintf (file, ", ");
            output_reg (file, ArmInstGetOperand (inst, 2)->vreg);
            fprintf (file, ", lsl #%d]", ArmInstGetOperand (inst, 3)->cval.cvValue.cvIval);
            break;

        case ARMINST_OP_ADD_reg_LSL_imm:
        case ARMINST_OP_SUB_reg_LSL_imm:
            output_reg (file, ArmInstGetOperand (inst, 0)->vreg);
            fprintf (file, ", ");
            output_reg (file, ArmInstGetOperand (inst, 1)->vreg);
            fprintf (file, ", ");
            output_reg (file, ArmInstGetOperand (inst, 2)->vreg);
            fprintf (file, ", lsl #%d", ArmInstGetOperand (inst, 3)->cval.cvValue.cvIval);
            break;

        case ARMINST_OP_ADD_reg_LSR_imm:
        case ARMINST_OP_SUB_reg_LSR_imm:
            output_reg (file, ArmInstGetOperand (inst, 0)->vreg);
            fprintf (file, ", ");
            output_reg (file, ArmInstGetOperand (inst, 1)->vreg);
            fprintf (file, ", ");
            output_reg (file, ArmInstGetOperand (inst, 2)->vreg);
            fprintf (file, ", lsr #%d", ArmInstGetOperand (inst, 3)->cval.cvValue.cvIval);
            break;

        case ARMINST_OP_ADD_reg_ASR_imm:
        case ARMINST_OP_SUB_reg_ASR_imm:
            output_reg (file, ArmInstGetOperand (inst, 0)->vreg);
            fprintf (file, ", ");
            output_reg (file, ArmInstGetOperand (inst, 1)->vreg);
            fprintf (file, ", ");
            output_reg (file, ArmInstGetOperand (inst, 2)->vreg);
            fprintf (file, ", asr #%d", ArmInstGetOperand (inst, 3)->cval.cvValue.cvIval);
            break;

        case ARMINST_OP_CMP_reg_LSL_imm:
        case ARMINST_OP_MOV_reg_LSL_imm:
            output_reg (file, ArmInstGetOperand (inst, 0)->vreg);
            fprintf (file, ", ");
            output_reg (file, ArmInstGetOperand (inst, 1)->vreg);
            fprintf (file, ", lsl #%d", ArmInstGetOperand (inst, 2)->cval.cvValue.cvIval);
            break;

        case ARMINST_OP_CMP_reg_LSR_imm:
        case ARMINST_OP_MOV_reg_LSR_imm:
            output_reg (file, ArmInstGetOperand (inst, 0)->vreg);
            fprintf (file, ", ");
            output_reg (file, ArmInstGetOperand (inst, 1)->vreg);
            fprintf (file, ", lsr #%d", ArmInstGetOperand (inst, 2)->cval.cvValue.cvIval);
            break;

        case ARMINST_OP_CMP_reg_ASR_imm:
        case ARMINST_OP_MOV_reg_ASR_imm:
            output_reg (file, ArmInstGetOperand (inst, 0)->vreg);
            fprintf (file, ", ");
            output_reg (file, ArmInstGetOperand (inst, 1)->vreg);
            fprintf (file, ", asr #%d", ArmInstGetOperand (inst, 2)->cval.cvValue.cvIval);
            break;
        }
    }

    fprintf(file, "\t@ Sequence number: %d",
        inst->uid);
    fprintf (file, "\n");
}

static BOOL
isNopCopy (ArmInst instr)
{
    BOOL retval;
    if  ((instr->opcode == ARMINST_OP_MOV_reg ||
         instr->opcode == ARMINST_OP_VMOV) &&
         ArmInstGetOperandAsReg(instr, 0)->hard_num == ArmInstGetOperandAsReg(instr, 1)->hard_num &&
         ArmInstGetOperandAsReg(instr, 0)->hard_num != -1)
    {
        retval = TRUE;
    }
    else
    {
        retval = FALSE;
    }
    return retval;
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
        for(  insn=(ArmInst)List_First(((BblockArm32)(*bb)->param)->code)
           ;  insn!=NULL
           ;  insn = (ArmInst)List_Next((void *)insn)
           )
        {
            /* 避免生成冗余的goto指令。  */
            if  (! List_Next ((void *)insn) &&
                 insn->opcode == ARMINST_OP_B &&
                 insn->condition == ConditionAL &&
                 List_Next((void *)bb) &&
                 (*(IRInst *) List_First ((*(basic_block *)List_Next((void *)bb))->insns))->uid == ArmInstGetOperand (insn, 0)->cval.cvValue.cvIval)
                continue;

            if  (! isNopCopy (insn))
                outputArmInst (insn, file);
        }
    }
    List_Destroy (&blocks);
}
