#include <stdlib.h>
#include <string.h>
#if ! defined (_WIN32)
#include <unistd.h>
#endif
#include "all.h"

/* Standard register usage.  */

/* Register allocation in ARM Procedure Call Standard
   (S - saved over call).

        r0           *        argument word/integer result
        r1-r3                argument word

        r4-r8             S        register variable
        r9             S        (rfp) register variable (real frame pointer)

        r10             F S        (sl) stack limit (used by -mapcs-stack-check)
        r11            F S        (fp) argument pointer
        r12                (ip) temp workspace
        r13             F S        (sp) lower end of current stack frame
        r14                (lr) link address/workspace
        r15           F        (pc) program counter

   *: See TARGET_CONDITIONAL_REGISTER_USAGE  */

/*        s0-s15                VFP scratch (aka d0-d7).
        s16-s31              S        VFP variable (aka d8-d15).
        vfpcc                Not a real register.  Represents the VFP condition
                        code flags.  */

/* The stack backtrace structure is as follows:
  fp points to here:  |  save code pointer  |      [fp]
                      |  return link value  |      [fp, #-4]
                      |  return sp value    |      [fp, #-8]
                      |  return fp value    |      [fp, #-12]
                     [|  saved r10 value    |]
                     [|  saved r9 value     |]
                     [|  saved r8 value     |]
                     [|  saved r7 value     |]
                     [|  saved r6 value     |]
                     [|  saved r5 value     |]
                     [|  saved r4 value     |]
                     [|  saved r3 value     |]
                     [|  saved r2 value     |]
                     [|  saved r1 value     |]
                     [|  saved r0 value     |]
  r0-r3 are not normally saved in a C function.  */

/* ARCompact stack frames look like:

           Before call                     After call
  high  +-----------------------+       +-----------------------+
  mem   |  reg parm save area   |       | reg parm save area    |
        |  only created for     |       | only created for      |
        |  variable arg fns     |       | variable arg fns      |
    AP  +-----------------------+       +-----------------------+
        |  return addr register |       | return addr register  |
        |  (if required)        |       | (if required)         |
        +-----------------------+       +-----------------------+
        |                       |       |                       |
        |  reg save area        |       | reg save area         |
        |                       |       |                       |
        +-----------------------+       +-----------------------+
        |  frame pointer        |       | frame pointer         |
        |  (if required)        |       | (if required)         |
    FP  +-----------------------+       +-----------------------+
        |                       |       |                       |
        |  local/temp variables |       | local/temp variables  |
        |                       |       |                       |
        +-----------------------+       +-----------------------+
        |                       |       |                       |
        |  arguments on stack   |       | arguments on stack    |
        |                       |       |                       |
    SP  +-----------------------+       +-----------------------+
                                        | reg parm save area    |
                                        | only created for      |
                                        | variable arg fns      |
                                    AP  +-----------------------+
                                        | return addr register  |
                                        | (if required)         |
                                        +-----------------------+
                                        |                       |
                                        | reg save area         |
                                        |                       |
                                        +-----------------------+
                                        | frame pointer         |
                                        | (if required)         |
                                    FP  +-----------------------+
                                        |                       |
                                        | local/temp variables  |
                                        |                       |
                                        +-----------------------+
                                        |                       |
                                        | arguments on stack    |
  low                                   |                       |
  mem                               SP  +-----------------------+

Notes:
1) The "reg parm save area" does not exist for non variable argument fns.
   The "reg parm save area" can be eliminated completely if we created our
   own va-arc.h, but that has tradeoffs as well (so it's not done).  */

typedef struct
{
    varpool_node_set set;               /* 中间代码操作数的集合。  */
    control_flow_graph func;            /* 控制流图。  */
    struct avl_table *virtual_regs;     /* 虚拟寄存器的集合。  */
    struct dwarf_data *ddata;           /* 调式信息。  */
    SymTab stab;                        /* 符号表。  */
    struct avl_table *reg_map;          /* 变量到底层AST结点的映射。  */
    struct avl_table *node_map;         /* 树结点的索引。  */
    LIST tree_nodes;                    /* 所有底层AST的结点集合，可用于保证不发生内存泄漏。  */
    LIST trees;                         /* 待指令选择的底层AST的集合，contains TREEINFO。  */
    LIST _constants;                    /* 基本块内常量池，contains ConstantPoolCache  */
} INTERNAL_DATA, * PINTERNAL_DATA;

typedef struct tagTREEINFO
{
    NODEPTR_TYPE p;
    int goalnt;
    basic_block block;
}
TREEINFO, *PTREEINFO;

typedef struct reg_mapping
{
    varpool_node vnode;
    NODEPTR_TYPE p;
}
reg_mapping;

struct ConstantPoolCache
{
    int val;
    enum var_types type;
    NODEPTR_TYPE p;
};

/* Note: 此枚举需要与arm32.brg中的值保持一致。  */
enum { imm12 = 1, simm8 = 2, CNSTI4 =  4, INDIRI4 = 7, 
       ADD=9, SDIV=10, MUL=11, SUB=12, JUMPV=19, LABELV=20,
       LDR=21, STR=24, REGISTER=28, RSB=29, CMP=30, imm16=31,
       CLZ=38, LSR=39, MOV=40, BL=41, LABEL=42, BX=43, PUSH=44, POP=45,
       STRING=46, VMOV=47, VMUL=48, VADD=49, VSUB=50, VDIV=51,
       vcvt_signedToFloatingPoint=52, vcvt_floatingPointToSigned=53,
       VNEG=54, VCMP=55, VCMPz=56, VMRS=57, VSTR=58, VLDR=59,
       imm10=60, VPUSH=61, VPOP=62, LSL=63, ASR=64, imm5=65 };

/* iburg产生的程序不会释放动态分配的内存，这样会造成内存泄漏。为了解决这一
   问题，我们定义一个内存池，让iburg产生的程序从内存池中分配内存，当一
   切结束后我们自己释放这些内存。  */
LIST mempool;

/* Local function prototypes.  */
static int compare( reg_mapping *arg1, reg_mapping *arg2 )
{
    int ret = 0 ;

    if ( arg1->vnode->uid < arg2->vnode->uid )
        ret = -1 ;
    else if ( arg1->vnode->uid > arg2->vnode->uid )
        ret = 1 ;

    return( ret );
}
static int
compare_tree (NODEPTR_TYPE t1, NODEPTR_TYPE t2)
{
    int ret = 0 ;

    if ( t1->operand.vreg->vregno < t2->operand.vreg->vregno )
        ret = -1 ;
    else if ( t1->operand.vreg->vregno > t2->operand.vreg->vregno )
        ret = 1 ;

    return( ret );
}

static NODEPTR_TYPE tree(LIST tree_nodes, int op, NODEPTR_TYPE l, NODEPTR_TYPE r)
{
    static int count = 0;
    NODEPTR_TYPE p = (NODEPTR_TYPE) List_NewLast (tree_nodes, sizeof (*p));

    memset (p, 0, sizeof(*p));
    p->op = op;
    p->kids[0] = l; p->kids[1] = r;
    if  (l)
        l->ref_counter++;
    if  (r)
        r->ref_counter++;
    p->id = ++count;
    return p;
}

static NODEPTR_TYPE copy_tree(LIST tree_nodes, NODEPTR_TYPE tp)
{
    NODEPTR_TYPE p;
    if  (!tp)
        return NULL;
    p = tree (tree_nodes, tp->op, 
              copy_tree(tree_nodes, tp->kids[0]), 
              copy_tree(tree_nodes, tp->kids[1]));
    memcpy (&p->operand, &tp->operand, sizeof (p->operand));
    return p;
}

static void dumpCover(NODEPTR_TYPE p, int goalnt, int indent)
{
    int eruleno = burmArm32_rule(STATE_LABEL(p), goalnt);
    short *nts = burmArm32_nts[eruleno];
    NODEPTR_TYPE kids[100];
    int i;

    burmArm32_kids(p, eruleno, kids);
    for (i = 0; nts[i]; i++)
        dumpCover(kids[i], nts[i], indent + 1);
    for (i = 0; i < indent; i++)
        fprintf(stderr, " ");
    fprintf(stderr, "%s\n", burmArm32_string[eruleno]);
}

/* 指令选择。  */
static void burmArm32_select(basic_block bb, NODEPTR_TYPE p, int goalnt, struct avl_table *virtual_regs)
{
    int eruleno = burmArm32_rule(STATE_LABEL(p), goalnt);
    short *nts = burmArm32_nts[eruleno];
    NODEPTR_TYPE kids[100];
    int i;
    int counter = 0;
    ArmInst insn;
    union Arm_Operand operand;

    burmArm32_kids(p, eruleno, kids);
    for (i = 0; nts[i]; i++)
        burmArm32_select(bb, kids[i], nts[i], virtual_regs);

    insn = emitArmInst(bb, (enum ArmInstOperator)eruleno);
    insn->condition = p->condition;
    switch ((enum ArmInstOperator)eruleno)
    {
    case ARMINST_OP_LDR_const:
    case ARMINST_OP_LDR_label:
    case ARMINST_OP_MOV_reg:
    case ARMINST_OP_MOV_imm:
    case ARMINST_OP_CLZ:
    case ARMINST_OP_VMOV:
    case ARMINST_OP_vcvt_signedToFloatingPoint:
    case ARMINST_OP_vcvt_floatingPointToSigned:
    case ARMINST_OP_VNEG:
        ArmInstSetOperand (insn, 0, p->operand);
        ArmInstSetOperand (insn, 1, LEFT_CHILD(p)->operand);
        break;

    case ARMINST_OP_BL:
    case ARMINST_OP_B:
    case ARMINST_OP_BX:
    case ARMINST_OP_VCMPz:
        ArmInstSetOperand (insn, 0, LEFT_CHILD(p)->operand);
        break;

    case ARMINST_OP_ADD_reg:
    case ARMINST_OP_ADD_imm:
    case ARMINST_OP_SDIV_reg:
    case ARMINST_OP_MUL_reg:
    case ARMINST_OP_SUB_reg:
    case ARMINST_OP_SUB_imm:
    case ARMINST_OP_RSB_reg:
    case ARMINST_OP_RSB_imm:
    case ARMINST_OP_LSR_imm:
    case ARMINST_OP_LSL_imm:
    case ARMINST_OP_ASR_imm:
    case ARMINST_OP_VADD:
    case ARMINST_OP_VMUL:
    case ARMINST_OP_VDIV:
    case ARMINST_OP_VSUB:
        ArmInstSetOperand (insn, 0, p->operand);
        ArmInstSetOperand (insn, 1, LEFT_CHILD(p)->operand);
        ArmInstSetOperand (insn, 2, RIGHT_CHILD(p)->operand);
        break;

    case ARMINST_OP_CMP_reg:
    case ARMINST_OP_CMP_imm:
    case ARMINST_OP_VCMP:
        ArmInstSetOperand (insn, 0, LEFT_CHILD(p)->operand);
        ArmInstSetOperand (insn, 1, RIGHT_CHILD(p)->operand);
        break;

    case ARMINST_OP_MLA:
        ArmInstSetOperand (insn, 0, p->operand);
        ArmInstSetOperand (insn, 1, LEFT_CHILD(LEFT_CHILD(p))->operand);
        ArmInstSetOperand (insn, 2, RIGHT_CHILD(LEFT_CHILD(p))->operand);
        ArmInstSetOperand (insn, 3, RIGHT_CHILD(p)->operand);
        break;

    case ARMINST_OP_MLS:
        ArmInstSetOperand (insn, 0, p->operand);
        ArmInstSetOperand (insn, 3, LEFT_CHILD(p)->operand);
        ArmInstSetOperand (insn, 1, LEFT_CHILD(RIGHT_CHILD(p))->operand);
        ArmInstSetOperand (insn, 2, RIGHT_CHILD(RIGHT_CHILD(p))->operand);
        break;

    case ARMINST_OP_LDR_reg:
    case ARMINST_OP_VLDR_reg:
        ArmInstSetOperand (insn, 0, p->operand);
        ArmInstSetOperand (insn, 1, LEFT_CHILD(LEFT_CHILD(p))->operand);
        break;

    case ARMINST_OP_LDR_reg_reg:
    case ARMINST_OP_LDR_imm:
    case ARMINST_OP_VLDR_imm:
        ArmInstSetOperand (insn, 0, p->operand);
        ArmInstSetOperand (insn, 1, LEFT_CHILD(LEFT_CHILD(LEFT_CHILD(p)))->operand);
        ArmInstSetOperand (insn, 2, RIGHT_CHILD(LEFT_CHILD(LEFT_CHILD(p)))->operand);
        break;

    case ARMINST_OP_STR_reg:
    case ARMINST_OP_VSTR_reg:
        ArmInstSetOperand (insn, 0, LEFT_CHILD(p)->operand);
        ArmInstSetOperand (insn, 1, LEFT_CHILD(RIGHT_CHILD(p))->operand);
        break;

    case ARMINST_OP_STR_reg_reg:
    case ARMINST_OP_STR_imm:
    case ARMINST_OP_VSTR_imm:
        ArmInstSetOperand (insn, 0, LEFT_CHILD(p)->operand);
        ArmInstSetOperand (insn, 1, LEFT_CHILD(LEFT_CHILD(RIGHT_CHILD(p)))->operand);
        ArmInstSetOperand (insn, 2, RIGHT_CHILD(LEFT_CHILD(RIGHT_CHILD(p)))->operand);
        break;

    case ARMINST_OP_REGISTER:
        deleteArmInst(insn);
        break;

    case ARMINST_OP_LABEL:
    case ARMINST_OP_STRING:
        ArmInstSetOperand (insn, 0, p->operand);
        break;

    case ARMINST_OP_PUSH:
    case ARMINST_OP_POP:
        for (i = 0; i < num_callee_save_registers; i++)
        {
            if  (!IS_VFP_REGNUM (callee_save_registers[i]))
            {
                operand.vreg = gen_vregArm32(virtual_regs, callee_save_registers[i], NO_REGS);
                ArmInstSetOperand (insn, counter++, operand);
            }
        }
        break;

    case ARMINST_OP_VPUSH:
    case ARMINST_OP_VPOP:
        for (i = 0; i < num_callee_save_registers; i++)
        {
            if  (IS_VFP_REGNUM (callee_save_registers[i]))
            {
                operand.vreg = gen_vregArm32(virtual_regs, callee_save_registers[i], NO_REGS);
                ArmInstSetOperand (insn, counter++, operand);
            }
        }
        break;

    case ARMINST_OP_VMRS:
        break;

    case ARMINST_OP_LDR_reg_LSL_imm:
        ArmInstSetOperand (insn, 0, p->operand);
        ArmInstSetOperand (insn, 1, LEFT_CHILD(LEFT_CHILD(LEFT_CHILD(p)))->operand);
        ArmInstSetOperand (insn, 2, LEFT_CHILD(RIGHT_CHILD(LEFT_CHILD(LEFT_CHILD(p))))->operand);
        ArmInstSetOperand (insn, 3, RIGHT_CHILD(RIGHT_CHILD(LEFT_CHILD(LEFT_CHILD(p))))->operand);
        break;

    case ARMINST_OP_STR_reg_LSL_imm:
        ArmInstSetOperand (insn, 0, LEFT_CHILD (p)->operand);
        ArmInstSetOperand (insn, 1, LEFT_CHILD (LEFT_CHILD (RIGHT_CHILD (p)))->operand);
        ArmInstSetOperand (insn, 2, LEFT_CHILD (RIGHT_CHILD (LEFT_CHILD (RIGHT_CHILD (p))))->operand);
        ArmInstSetOperand (insn, 3, RIGHT_CHILD (RIGHT_CHILD (LEFT_CHILD (RIGHT_CHILD (p))))->operand);
        break;

    case ARMINST_OP_ADD_reg_LSL_imm:
    case ARMINST_OP_ADD_reg_LSR_imm:
    case ARMINST_OP_ADD_reg_ASR_imm:
    case ARMINST_OP_SUB_reg_LSL_imm:
    case ARMINST_OP_SUB_reg_LSR_imm:
    case ARMINST_OP_SUB_reg_ASR_imm:
        ArmInstSetOperand (insn, 0, p->operand);
        ArmInstSetOperand (insn, 1, LEFT_CHILD(p)->operand);
        ArmInstSetOperand (insn, 2, LEFT_CHILD (RIGHT_CHILD(p))->operand);
        ArmInstSetOperand (insn, 3, RIGHT_CHILD (RIGHT_CHILD(p))->operand);
        break;

    case ARMINST_OP_CMP_reg_LSL_imm:
    case ARMINST_OP_CMP_reg_LSR_imm:
    case ARMINST_OP_CMP_reg_ASR_imm:
        ArmInstSetOperand (insn, 0, LEFT_CHILD (p)->operand);
        ArmInstSetOperand (insn, 1, LEFT_CHILD (RIGHT_CHILD (p))->operand);
        ArmInstSetOperand (insn, 2, RIGHT_CHILD (RIGHT_CHILD (p))->operand);
        break;

    case ARMINST_OP_MOV_reg_LSL_imm:
    case ARMINST_OP_MOV_reg_LSR_imm:
    case ARMINST_OP_MOV_reg_ASR_imm:
        ArmInstSetOperand (insn, 0, p->operand);
        ArmInstSetOperand (insn, 1, LEFT_CHILD (LEFT_CHILD (p))->operand);
        ArmInstSetOperand (insn, 2, RIGHT_CHILD (LEFT_CHILD (p))->operand);
        break;
    }
}

static void commit (basic_block block, NODEPTR_TYPE p, int goalnt, PINTERNAL_DATA pmydata)
{
    PTREEINFO pti = (PTREEINFO) List_NewLast (pmydata->trees, sizeof (TREEINFO));
    pti->goalnt = goalnt;
    pti->p = p;
    pti->block = block;
}

static BOOL isDisp(int num)
{
    return num < 4096 && num > -4096;
}

static BOOL is_simm8 (int num)
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

static BOOL is_imm16(int num)
{
    return num < 65536 && num >= 0;
}

static BOOL is_imm10(int num)
{
    return num <= 1020 && num >= -1020 && ((num & 3) == 0);
}

/* 在 INSN 之前添加从溢出符号加载的指令，溢出符号存储在 SPILL_REG 中。
   我们假设不再处于 SSA，因此返回的寄存器没有设置其定值。  */
void
spill_inArm32 (ArmInst insn, vreg_tArm32 spill_reg, int slot, struct avl_table *virtual_regs)
{
    vreg_tArm32 tmpreg;
    union Arm_Operand operand;
    ArmInst new_insn;
    struct base_offset *constVal;

    constVal = get_locationArm32 (spill_reg);
    if  (constVal != NULL &&
         ! constVal->is_addr &&
         constVal->base->var->sdSymKind == SYM_VAR &&
         constVal->base->var->sdType->tdTypeKind == TYP_INT &&
         constVal->base->var->sdVar.sdvConst)
    {
        /* 加载常量的值。  */
        new_insn = ArmInstInsertBefore (insn, ARMINST_OP_LDR_const);
        operand.vreg = spill_reg;
        ArmInstSetOperand (new_insn, 0, operand);
        operand.cval.cvValue.cvIval = GetConstVal (constVal->base->var, 0)->cvValue.cvIval;
        ArmInstSetOperand (new_insn, 1, operand);
    }
    else if (constVal != NULL &&
             constVal->is_addr &&
             constVal->base->var->sdSymKind == SYM_VAR &&
             is_global_var (constVal->base->var) &&
             (!constVal->base->var->sdVar.sdvConst || constVal->base->var->sdType->tdTypeKind > TYP_lastIntrins))
    {
        /* 加载全局变量地址。  */
        new_insn = ArmInstInsertBefore (insn, ARMINST_OP_LDR_label);
        operand.vreg = spill_reg;
        ArmInstSetOperand (new_insn, 0, operand);
        operand.sym = constVal->base->var;
        ArmInstSetOperand (new_insn, 1, operand);
    }
    else if ((spill_reg->rclass==VFP_REGS ? is_imm10 : isDisp) (slot))
    {
        new_insn = ArmInstInsertBefore (insn, spill_reg->rclass==VFP_REGS ? ARMINST_OP_VLDR_imm : ARMINST_OP_LDR_imm);
        operand.vreg = spill_reg;
        ArmInstSetOperand (new_insn, 0, operand);
        operand.vreg = gen_vregArm32 (virtual_regs, SP_REGNUM, GENERAL_REGS);
        ArmInstSetOperand (new_insn, 1, operand);
        operand.cval.cvValue.cvIval = slot;
        ArmInstSetOperand (new_insn, 2, operand);
    }
    else if (spill_reg->rclass == VFP_REGS)
    {
        tmpreg = gen_vregArm32 (virtual_regs, SPILL_REG, GENERAL_REGS);
        new_insn = ArmInstInsertBefore (insn, ARMINST_OP_LDR_const);
        operand.vreg = tmpreg;
        ArmInstSetOperand (new_insn, 0, operand);
        operand.cval.cvValue.cvIval = slot;
        ArmInstSetOperand (new_insn, 1, operand);

        new_insn = ArmInstInsertBefore (insn, ARMINST_OP_ADD_reg);
        operand.vreg = tmpreg;
        ArmInstSetOperand (new_insn, 0, operand);
        operand.vreg = tmpreg;
        ArmInstSetOperand (new_insn, 1, operand);
        operand.vreg = gen_vregArm32 (virtual_regs, SP_REGNUM, GENERAL_REGS);
        ArmInstSetOperand (new_insn, 2, operand);

        new_insn = ArmInstInsertBefore (insn, ARMINST_OP_VLDR_reg);
        operand.vreg = spill_reg;
        ArmInstSetOperand (new_insn, 0, operand);
        operand.vreg = tmpreg;
        ArmInstSetOperand (new_insn, 1, operand);
    }
    else
    {
        tmpreg = gen_vregArm32 (virtual_regs, SPILL_REG, GENERAL_REGS);
        new_insn = ArmInstInsertBefore (insn, ARMINST_OP_LDR_const);
        operand.vreg = tmpreg;
        ArmInstSetOperand (new_insn, 0, operand);
        operand.cval.cvValue.cvIval = slot;
        ArmInstSetOperand (new_insn, 1, operand);

        new_insn = ArmInstInsertBefore (insn, ARMINST_OP_LDR_reg_reg);
        operand.vreg = spill_reg;
        ArmInstSetOperand (new_insn, 0, operand);
        operand.vreg = gen_vregArm32 (virtual_regs, SP_REGNUM, GENERAL_REGS);
        ArmInstSetOperand (new_insn, 1, operand);
        operand.vreg = tmpreg;
        ArmInstSetOperand (new_insn, 2, operand);
    }
}

/* Append after INSN a store to spill symbol of SPILL_REG.  Return the register
   from which we stored.  If this required another register to convert to a B1
   type, return it in *PTMP2, otherwise store NULL into it.  We assume we are
   out of SSA so the returned register does not have its use updated.  */
void
spill_outArm32 (ArmInst insn, vreg_tArm32 spill_reg, int slot, struct avl_table *virtual_regs)
{
    vreg_tArm32 tmpreg;
    union Arm_Operand operand;
    ArmInst new_insn;
    struct base_offset *constVal;

    constVal = get_locationArm32 (spill_reg);
    if  (constVal != NULL &&
         ! constVal->is_addr &&
         constVal->base->var->sdSymKind == SYM_VAR &&
         constVal->base->var->sdType->tdTypeKind == TYP_INT &&
         constVal->base->var->sdVar.sdvConst)
    {
        /* 常量不需要逐出。  */
    }
    else if (constVal != NULL &&
             constVal->is_addr &&
             constVal->base->var->sdSymKind == SYM_VAR &&
             is_global_var (constVal->base->var) &&
             (!constVal->base->var->sdVar.sdvConst || constVal->base->var->sdType->tdTypeKind > TYP_lastIntrins))
    {
        /* 全局变量地址不需要逐出。  */
    }
    else if ((spill_reg->rclass==VFP_REGS ? is_imm10 : isDisp) (slot))
    {
        new_insn = ArmInstInsertAfter (insn, spill_reg->rclass==VFP_REGS ? ARMINST_OP_VSTR_imm : ARMINST_OP_STR_imm);
        operand.vreg = spill_reg;
        ArmInstSetOperand (new_insn, 0, operand);
        operand.vreg = gen_vregArm32 (virtual_regs, SP_REGNUM, GENERAL_REGS);
        ArmInstSetOperand (new_insn, 1, operand);
        operand.cval.cvValue.cvIval = slot;
        ArmInstSetOperand (new_insn, 2, operand);
    }
    else if (spill_reg->rclass == VFP_REGS)
    {
        tmpreg = gen_vregArm32 (virtual_regs, SPILL_REG, GENERAL_REGS);
        new_insn = ArmInstInsertAfter (insn, ARMINST_OP_LDR_const);
        operand.vreg = tmpreg;
        ArmInstSetOperand (new_insn, 0, operand);
        operand.cval.cvValue.cvIval = slot;
        ArmInstSetOperand (new_insn, 1, operand);

        new_insn = ArmInstInsertAfter (new_insn, ARMINST_OP_ADD_reg);
        operand.vreg = tmpreg;
        ArmInstSetOperand (new_insn, 0, operand);
        operand.vreg = tmpreg;
        ArmInstSetOperand (new_insn, 1, operand);
        operand.vreg = gen_vregArm32 (virtual_regs, SP_REGNUM, GENERAL_REGS);
        ArmInstSetOperand (new_insn, 2, operand);

        new_insn = ArmInstInsertAfter (new_insn, ARMINST_OP_VSTR_reg);
        operand.vreg = spill_reg;
        ArmInstSetOperand (new_insn, 0, operand);
        operand.vreg = tmpreg;
        ArmInstSetOperand (new_insn, 1, operand);
    }
    else
    {
        tmpreg = gen_vregArm32 (virtual_regs, SPILL_REG, GENERAL_REGS);
        new_insn = ArmInstInsertAfter (insn, ARMINST_OP_LDR_const);
        operand.vreg = tmpreg;
        ArmInstSetOperand (new_insn, 0, operand);
        operand.cval.cvValue.cvIval = slot;
        ArmInstSetOperand (new_insn, 1, operand);

        new_insn = ArmInstInsertAfter (new_insn, ARMINST_OP_STR_reg_reg);
        operand.vreg = spill_reg;
        ArmInstSetOperand (new_insn, 0, operand);
        operand.vreg = gen_vregArm32 (virtual_regs, SP_REGNUM, GENERAL_REGS);
        ArmInstSetOperand (new_insn, 1, operand);
        operand.vreg = tmpreg;
        ArmInstSetOperand (new_insn, 2, operand);
    }
}

void update_frame_layoutArm32 (control_flow_graph func, struct avl_table *virtual_regs)
{
    basic_block *bb;
    ArmInst insn;
    int i;
    int counter;
    union Arm_Operand operand;
    int stack_size;
    bitmap temp = BITMAP_XMALLOC ();
    bitmap_iterator bi;
    unsigned regno;
    ArmInst push_insn = NULL;
    ArmInst pop_insn = NULL;
    ArmInst vpush_insn = NULL;
    ArmInst vpop_insn = NULL;
    int first = FIRST_VIRTUAL_REGISTER;
    int last = 0;

    /* 统计指令中使用了哪些寄存器。  */
    for(  bb=(basic_block *) List_First(func->basic_block_info)
       ;  bb!=NULL
       ;  bb = (basic_block *) List_Next((void *)bb)
       )
    {
        for(  insn=(ArmInst) List_First(((BblockArm32)(*bb)->param)->code)
           ;  insn!=NULL
           ;  insn = (ArmInst) List_Next((void *)insn)
           )
        {
            if  (insn->opcode == ARMINST_OP_PUSH ||
                 insn->opcode == ARMINST_OP_POP ||
                 insn->opcode == ARMINST_OP_VPUSH ||
                 insn->opcode == ARMINST_OP_VPOP)
                continue;
            ArmInstOutput (insn, temp);
            for (bmp_iter_set_init (&bi, temp, 0, &regno);
                 bmp_iter_set (&bi, &regno);
                 bmp_iter_next (&bi, &regno))
                bitmap_set_bit (((struct BfunctionArm32 *)func->param)->callee_saved_reg, gen_vregArm32(virtual_regs, regno, NO_REGS)->hard_num);
        }
    }

    /* 计算callee_saved_reg_size的值。  */
    for (i = 0; i < num_callee_save_registers; ++i)
        if  (bitmap_bit_p (((struct BfunctionArm32 *)func->param)->callee_saved_reg, callee_save_registers[i]))
            func->callee_saved_reg_size += UNITS_PER_WORD;

    /* 找到push、pop、vpush、vpop指令的位置。  */
    for(  insn=(ArmInst) List_First(((BblockArm32)(func->entry_block_ptr)->param)->code)
       ;  insn!=NULL
       ;  insn = (ArmInst) List_Next((void *)insn)
       )
    {
        if  (insn->opcode == ARMINST_OP_PUSH)
        {
            push_insn = ArmInstInsertBefore (insn, ARMINST_OP_PUSH);
            deleteArmInst (insn);
            insn = (ArmInst)List_Next((void *)push_insn);
            vpush_insn = ArmInstInsertBefore (insn, ARMINST_OP_VPUSH);
            deleteArmInst (insn);
            break;
        }
    }
    for(  insn=(ArmInst) List_First(((BblockArm32)(func->exit_block_ptr)->param)->code)
       ;  insn!=NULL 
       ;  insn = (ArmInst) List_Next((void *)insn)
       )
    {
        if  (insn->opcode == ARMINST_OP_POP)
        {
            pop_insn = ArmInstInsertBefore (insn, ARMINST_OP_POP);
            deleteArmInst (insn);
            insn = (ArmInst)List_Prev((void *)pop_insn);
            vpop_insn = ArmInstInsertBefore (insn, ARMINST_OP_VPOP);
            deleteArmInst (insn);
            break;
        }
    }

    /* 计算栈指针移动的距离。  */
    stack_size = func->arg_size + func->local_size + func->spill_size;

    /* 要求栈指针8字节对齐。  */
    if (((stack_size + func->callee_saved_reg_size) & 7) != 0)
    {
        func->pad_size += UNITS_PER_WORD;
        stack_size += UNITS_PER_WORD;
    }

    /* 如果栈大小超过simm8范围，则会使用SPILL_REG加载栈移动的距离，因而需要将其记录。  */
    if  (!is_simm8 (stack_size) &&
         !bitmap_bit_p (((struct BfunctionArm32 *)func->param)->callee_saved_reg, SPILL_REG))
    {
        func->callee_saved_reg_size += UNITS_PER_WORD;
        bitmap_set_bit (((struct BfunctionArm32 *)func->param)->callee_saved_reg, SPILL_REG);
    }

    /* 要求栈指针8字节对齐。  */
    if (((stack_size + func->callee_saved_reg_size) & 3) != 0)
    {
        stack_size += func->pad_size ? -UNITS_PER_WORD : UNITS_PER_WORD;
        func->pad_size = func->pad_size ? -UNITS_PER_WORD : UNITS_PER_WORD;
    }

    /* 更新帧指针偏移量。  */
    for(  insn=(ArmInst) List_First(((BblockArm32)(func->entry_block_ptr)->param)->code)
       ;  insn!=NULL
       ;  insn = (ArmInst) List_Next((void *)insn)
       )
    {
        if  (insn->opcode == ARMINST_OP_STRING &&
             !strcmp (dyn_string_buf (ArmInstGetOperand (insn, 0)->ds), "\t@ Set up the frame pointer."))
        {
            operand.cval.cvValue.cvIval = func->callee_saved_reg_size;
            ArmInstSetOperand ((ArmInst) List_Next((void *)insn), 2, operand);
            break;
        }
    }

    /* 更新移动栈指针的指令。  */
    for(  insn=(ArmInst) List_First(((BblockArm32)(func->entry_block_ptr)->param)->code)
       ;  insn!=NULL
       ;  insn = (ArmInst) List_Next((void *)insn)
       )
    {
        if  (insn->opcode == ARMINST_OP_STRING &&
             !strcmp (dyn_string_buf (ArmInstGetOperand (insn, 0)->ds), "\t@ Decrement the stack pointer."))
        {
            insn = (ArmInst) List_Next((void *)insn);
            if  (is_simm8 (stack_size))
            {
                operand.cval.cvValue.cvIval = stack_size;
                ArmInstSetOperand (insn, 2, operand);
            }
            else
            {
                operand.vreg = gen_vregArm32 (virtual_regs, SPILL_REG, GENERAL_REGS);
                insn->opcode = ARMINST_OP_SUB_reg;
                ArmInstSetOperand (insn, 2, operand);
                insn = ArmInstInsertBefore (insn, ARMINST_OP_LDR_const);
                ArmInstSetOperand (insn, 0, operand);
                operand.cval.cvValue.cvIval = stack_size;
                ArmInstSetOperand (insn, 1, operand);
            }
            break;
        }
    }
    for(  insn=(ArmInst) List_First(((BblockArm32)(func->exit_block_ptr)->param)->code)
       ;  insn!=NULL
       ;  insn = (ArmInst) List_Next((void *)insn)
       )
    {
        if  (insn->opcode == ARMINST_OP_STRING &&
             !strcmp (dyn_string_buf (ArmInstGetOperand (insn, 0)->ds), "\t@ Recover the stack pointer."))
        {
            insn = (ArmInst) List_Next((void *)insn);
            if  (is_simm8 (stack_size))
            {
                operand.cval.cvValue.cvIval = stack_size;
                ArmInstSetOperand (insn, 2, operand);
            }
            else
            {
                operand.vreg = gen_vregArm32 (virtual_regs, SPILL_REG, GENERAL_REGS);
                insn->opcode = ARMINST_OP_ADD_reg;
                ArmInstSetOperand (insn, 2, operand);
                insn = ArmInstInsertBefore (insn, ARMINST_OP_LDR_const);
                ArmInstSetOperand (insn, 0, operand);
                operand.cval.cvValue.cvIval = stack_size;
                ArmInstSetOperand (insn, 1, operand);
            }
            break;
        }
    }

    /* 设置要压栈、出栈的寄存器。  */
    for (i = 0, counter = 0; i < num_callee_save_registers; ++i)
    {
        if  (bitmap_bit_p (((struct BfunctionArm32 *)func->param)->callee_saved_reg, callee_save_registers[i]) &&
             !IS_VFP_REGNUM(callee_save_registers[i]))
        {
            counter++;
        }
    }
    if  (0 == counter)
    {
        /* 若没有callee_saved_reg，则删除压栈、出栈指令。  */
        deleteArmInst (push_insn);
        deleteArmInst (pop_insn);
    }
    else
    {
        for (i = 0, counter = 0; i < num_callee_save_registers; ++i)
        {
            if  (bitmap_bit_p (((struct BfunctionArm32 *)func->param)->callee_saved_reg, callee_save_registers[i]) &&
                 !IS_VFP_REGNUM(callee_save_registers[i]))
            {
                operand.vreg = gen_vregArm32 (virtual_regs, callee_save_registers[i], GENERAL_REGS);
                ArmInstSetOperand (push_insn, counter, operand);
                ArmInstSetOperand (pop_insn, counter, operand);
                counter++;
            }
        }
    }

    for (i = 0, counter = 0; i < num_callee_save_registers; ++i)
    {
        if  (bitmap_bit_p (((struct BfunctionArm32 *)func->param)->callee_saved_reg, callee_save_registers[i]) &&
             IS_VFP_REGNUM(callee_save_registers[i]))
        {
            counter++;
            first = min (first, callee_save_registers[i]);
            last = max (last, callee_save_registers[i]);
        }
    }
    if  (0 == counter)
    {
        /* 若没有callee_saved_reg，则删除压栈、出栈指令。  */
        deleteArmInst (vpush_insn);
        deleteArmInst (vpop_insn);
    }
    else
    {
        for (i = first; i <= last; ++i)
        {
            operand.vreg = gen_vregArm32 (virtual_regs, i, VFP_REGS);
            ArmInstSetOperand (vpush_insn, i - first, operand);
            ArmInstSetOperand (vpop_insn, i - first, operand);
            counter++;
        }
    }

    BITMAP_XFREE (temp);
}

BOOL isCopyArm32 (ArmInst instr, int *Def, int *Src)
{
    int retval = instr->opcode == ARMINST_OP_MOV_reg;
    if  (retval)
    {
        *Def = ArmInstGetOperand(instr, 0)->vreg->vregno;
        *Src = ArmInstGetOperand(instr, 1)->vreg->vregno;
    }
    return retval;
}

/* Output a marker (i.e. a label) for the point in the generated code where
   the real body of the function begins (after parameters have been moved to
   their home locations).  */
static void
dwarfout_begin_function (IRInst inst, PINTERNAL_DATA pmydata)
{
    char label[30];
    dw_fde_ref fde;
    NODEPTR_TYPE p;
    dw_fde_ref old_ptr;

    sprintf (label, ".L_b%u:", inst->bb->cfg->funcdef_number);
    p = tree(pmydata->tree_nodes, STRING, NULL, NULL);
    p->operand.ds = dyn_string_new (3);
    dyn_string_insert_cstr (p->operand.ds, 0, label);

    commit (inst->bb, p, 1, pmydata);

  /* Expand the fde table if necessary.  */
  if (pmydata->ddata->fde_table_in_use == pmydata->ddata->fde_table_allocated)
    {
      pmydata->ddata->fde_table_allocated += 256;
      old_ptr = pmydata->ddata->fde_table;
      pmydata->ddata->fde_table = (dw_fde_ref) List_NewLast (pmydata->ddata->mem,
               pmydata->ddata->fde_table_allocated * sizeof (dw_fde_node));
      memcpy (pmydata->ddata->fde_table, old_ptr, pmydata->ddata->fde_table_in_use * sizeof (dw_fde_node));
      List_Delete(old_ptr);
    }

  /* Record the FDE associated with this function.  */
  pmydata->ddata->current_funcdef_fde = pmydata->ddata->fde_table_in_use;

  /* Add the new FDE at the end of the fde_table.  */
  fde = &pmydata->ddata->fde_table[pmydata->ddata->fde_table_in_use++];
  fde->dw_fde_begin = (char *) List_NewLast(pmydata->ddata->mem, (unsigned int) strlen(stGetSymName(IRInstGetOperand(inst, 0)->var)) + 1);
  strcpy(fde->dw_fde_begin,stGetSymName(IRInstGetOperand(inst, 0)->var));
  fde->dw_fde_end_prolog = (char *) List_NewLast(pmydata->ddata->mem, (unsigned int) strlen(label) + 1);
  strcpy(fde->dw_fde_end_prolog,label);
  fde->dw_fde_begin_epilogue = NULL;
  fde->dw_fde_end = NULL;
  fde->dw_fde_cfi = NULL;
}

/* Output a marker (i.e. a label) for the point in the generated code where
   the real body of the function ends (just before the epilogue code).  */
static void
dwarfout_end_function (IRInst inst, PINTERNAL_DATA pmydata)
{
    dw_fde_ref fde;
    char label[30];
    NODEPTR_TYPE p;

    sprintf (label, ".L_b%u_e:", inst->bb->cfg->funcdef_number);
    p = tree(pmydata->tree_nodes, STRING, NULL, NULL);
    p->operand.ds = dyn_string_new (3);
    dyn_string_insert_cstr (p->operand.ds, 0, label);

    commit (inst->bb, p, 1, pmydata);

    /* Record the ending code location in the FDE.  */
    fde = &pmydata->ddata->fde_table[pmydata->ddata->fde_table_in_use - 1];
    fde->dw_fde_begin_epilogue = (char *) List_NewLast(pmydata->ddata->mem, (unsigned int) strlen(label) + 1);
    strcpy(fde->dw_fde_begin_epilogue,label);
}

/* Output a label to mark the beginning of a source code line entry
   and record information relating to this source line, in
   'line_info_table' for later output of the .debug_line section.  */
static void
dwarfout_line (IRInst inst, PINTERNAL_DATA pmydata)
{
    char label[30];
    register dw_line_info_ref line_info;
    NODEPTR_TYPE p;
    dw_line_info_ref old_ptr;

    sprintf (label, ".L_LC%u:\t@ ", pmydata->ddata->line_info_table_in_use);
    p = tree(pmydata->tree_nodes, STRING, NULL, NULL);
    p->operand.ds = dyn_string_new (3);
    dyn_string_insert_cstr (p->operand.ds, 0, label);
    dyn_string_append_cstr (p->operand.ds, pmydata->ddata->main_input_filename);
    sprintf (label, ":%d:%d, index: %d", inst->line, inst->column, inst->uid);
    dyn_string_append_cstr (p->operand.ds, label);

    commit (inst->bb, p, 1, pmydata);

      /* expand the line info table if necessary */
      if (pmydata->ddata->line_info_table_in_use == pmydata->ddata->line_info_table_allocated)
        {
          pmydata->ddata->line_info_table_allocated += 1024;
          old_ptr = pmydata->ddata->line_info_table;
          pmydata->ddata->line_info_table
            = (dw_line_info_ref)
            List_NewLast (pmydata->ddata->mem,
                   pmydata->ddata->line_info_table_allocated * sizeof (dw_line_info_entry));
          memcpy (pmydata->ddata->line_info_table, old_ptr, pmydata->ddata->line_info_table_in_use * sizeof (dw_line_info_entry));
          List_Delete(old_ptr);
        }
      /* add the new entry at the end of the line_info_table.  */
      line_info = &pmydata->ddata->line_info_table[pmydata->ddata->line_info_table_in_use++];
      line_info->dw_file_num = 1;
      line_info->dw_line_num = inst->line;
      line_info->column_num = inst->column;
}

static NODEPTR_TYPE
getreg (varpool_node vnode, struct avl_table *reg_map)
{
    reg_mapping *p = (reg_mapping *) avl_find (reg_map, &vnode);
    return p ? p->p : NULL;
}

static void
setreg (varpool_node vnode, NODEPTR_TYPE p, PINTERNAL_DATA pmydata)
{
    reg_mapping *ptr = (reg_mapping *) xmalloc (sizeof (reg_mapping));
    ptr->p = p;
    ptr->vnode = vnode;
    free (avl_replace (pmydata->reg_map, ptr));
    avl_insert(pmydata->node_map, p);
}

static NODEPTR_TYPE
load_immediate (basic_block block, vreg_tArm32 dest_reg, int num, enum var_types type, PINTERNAL_DATA pmydata)
{
    NODEPTR_TYPE p;
    struct ConstantPoolCache* cp;

    if  (!dest_reg)
    {
        for(  cp=(struct ConstantPoolCache*) List_Last(pmydata->_constants)
           ;  cp!=NULL
           ;  cp = (struct ConstantPoolCache*) List_Prev((void *)cp)
           )
            if  (cp->type == type &&
                 cp->val == num)
                break;

        if  (cp == NULL)
        {
            p = tree (pmydata->tree_nodes, CNSTI4, NULL, NULL);
            p->operand.cval.cvValue.cvIval = num;
            p = tree (pmydata->tree_nodes, LDR, p, NULL);

            p->operand.vreg = gen_vregArm32 (pmydata->virtual_regs, -1, GENERAL_REGS);

            if  (type == TYP_FLOAT)
            {
                p = tree (pmydata->tree_nodes, VMOV, p, NULL);
                p->operand.vreg = gen_vregArm32 (pmydata->virtual_regs, -1, VFP_REGS);
            }

            commit (block, p, burmArm32_reg_NT, pmydata);
            dest_reg = p->operand.vreg;
            p = tree (pmydata->tree_nodes, REGISTER, NULL, NULL);
            p->operand.vreg = dest_reg;

            cp = (struct ConstantPoolCache *) List_NewLast (pmydata->_constants, sizeof (struct ConstantPoolCache));
            cp->p = p;
            cp->type = type;
            cp->val = num;
        }
        else
        {
            p = cp->p;
        }
    }
    else
    {
        p = tree (pmydata->tree_nodes, CNSTI4, NULL, NULL);
        p->operand.cval.cvValue.cvIval = num;
        p = tree (pmydata->tree_nodes, LDR, p, NULL);

        p->operand.vreg = dest_reg;

        if  (type == TYP_FLOAT)
        {
            p->operand.vreg = gen_vregArm32 (pmydata->virtual_regs, -1, GENERAL_REGS);
            p = tree (pmydata->tree_nodes, VMOV, p, NULL);
            p->operand.vreg = dest_reg;
        }
    }

    return p;
}

static NODEPTR_TYPE load_label (varpool_node vnode, basic_block block, PINTERNAL_DATA pmydata)
{
    vreg_tArm32 vreg;
    NODEPTR_TYPE p;
    NODEPTR_TYPE label;

    /* 操作数的地址是否在寄存器？  */
    vreg = get_registerArm32 (vnode, NULL, GENERAL_REGS,TRUE);
    if  (vreg != NULL)
    {
        /* 操作数的地址已经在寄存器，无需加载。  */
        p = tree (pmydata->tree_nodes, REGISTER, NULL, NULL);
        p->operand.vreg = vreg;
    }
    else
    {
        /* 操作数的地址不在寄存器，需要装载。  */
        label = tree (pmydata->tree_nodes, LABELV, NULL, NULL);
        label->operand.sym = vnode->var;
        p = tree (pmydata->tree_nodes, LDR, label, NULL);
        p->operand.vreg = vreg = gen_vregArm32 (pmydata->virtual_regs, -1, GENERAL_REGS);
        commit (block, p, burmArm32_reg_NT, pmydata);

        /* 设置地址描述符、寄存器描述符。  */
        p = tree (pmydata->tree_nodes, REGISTER, NULL, NULL);
        p->operand.vreg = vreg;
        register_descriptorArm32 (p->operand.vreg, NULL, vnode, NULL, TRUE);
        location_descriptorArm32 (vnode, NULL, p->operand.vreg, FALSE, TRUE);
    }

    return p;
}

static NODEPTR_TYPE load_base(basic_block block, vreg_tArm32 dest_reg, enum var_types tdTypeKind, NODEPTR_TYPE base_reg_no, int disp, PINTERNAL_DATA pmydata)
{
    NODEPTR_TYPE conv;
    NODEPTR_TYPE ind;
    NODEPTR_TYPE p;

    /* ldr  r3, [fp, #-8].  */

    if  ((tdTypeKind==TYP_FLOAT ? is_imm10 : isDisp) (disp))
    {
        conv = tree (pmydata->tree_nodes, tdTypeKind==TYP_FLOAT ? imm10 : imm12, NULL, NULL);
        conv->operand.cval.cvVtyp = TYP_INT;
        conv->operand.cval.cvValue.cvIval = disp;
    }
    else
    {
        conv = load_immediate (block, NULL, disp, TYP_INT, pmydata);
    }

    p = tree (pmydata->tree_nodes, ADD, base_reg_no, conv);
    p->operand.vreg = gen_vregArm32 (pmydata->virtual_regs, -1, GENERAL_REGS);
    ind = tree (pmydata->tree_nodes, INDIRI4, p, NULL);
    p = tree (pmydata->tree_nodes, tdTypeKind==TYP_FLOAT ? VLDR : LDR, ind, NULL);

    p->operand.vreg = dest_reg ? dest_reg : gen_vregArm32 (pmydata->virtual_regs, -1, tdTypeKind==TYP_FLOAT ? VFP_REGS : GENERAL_REGS);

    return p;
}

static NODEPTR_TYPE store_base(basic_block block, NODEPTR_TYPE src_reg, NODEPTR_TYPE base_reg_no, int disp, PINTERNAL_DATA pmydata)
{
    NODEPTR_TYPE conv;
    NODEPTR_TYPE ind;
    NODEPTR_TYPE p;

    /* ldr  r3, [fp, #-8].  */

    if  ((src_reg->operand.vreg->rclass == VFP_REGS ? is_imm10 : isDisp) (disp))
    {
        conv = tree (pmydata->tree_nodes, src_reg->operand.vreg->rclass == VFP_REGS ? imm10 : imm12, NULL, NULL);
        conv->operand.cval.cvVtyp = TYP_INT;
        conv->operand.cval.cvValue.cvIval = disp;
    }
    else
    {
        conv = load_immediate (block, NULL, disp, TYP_INT, pmydata);
    }

    p = tree (pmydata->tree_nodes, ADD, base_reg_no, conv);
    p->operand.vreg = gen_vregArm32 (pmydata->virtual_regs, -1, GENERAL_REGS);
    ind = tree (pmydata->tree_nodes, INDIRI4, p, NULL);
    p = tree (pmydata->tree_nodes, src_reg->operand.vreg->rclass == VFP_REGS ? VSTR : STR, src_reg, ind);

    return p;
}

static NODEPTR_TYPE
GetTreeNode (varpool_node vnode, PINTERNAL_DATA pmydata)
{
    NODEPTR_TYPE p;
    vreg_tArm32 vreg;

    p = getreg (vnode, pmydata->reg_map);
    if  (!p)
    {
        vreg = gen_vregArm32 (pmydata->virtual_regs, -1, vnode->var->sdType->tdTypeKind == TYP_FLOAT ? VFP_REGS : GENERAL_REGS);
        register_descriptorArm32 (vreg, NULL, vnode, NULL, FALSE);
        location_descriptorArm32 (vnode, NULL, vreg, FALSE, FALSE);
        p = tree (pmydata->tree_nodes, REGISTER, NULL, NULL);
        p->operand.vreg = vreg;
        setreg (vnode, p, pmydata);
    }

    return p;
}

/* 获取数组首地址+偏移量。  */
static NODEPTR_TYPE
getAddress (vreg_tArm32 dest_reg, varpool_node base, varpool_node addend, basic_block block, PINTERNAL_DATA pmydata)
{
    vreg_tArm32 vreg;
    NODEPTR_TYPE p;
    NODEPTR_TYPE conv;
    NODEPTR_TYPE tmp;
    BOOL calculated = FALSE;
    int offset;

    vreg = get_registerArm32 (base, NULL, GENERAL_REGS, TRUE);
    if  (vreg)
    {
        /* 数组的地址之前加载过。  */
        p = tree (pmydata->tree_nodes, REGISTER, NULL, NULL);
        p->operand.vreg = vreg;
    }
    else if  (is_global_var(base->var))
    {
        /* 数组是全局变量。  */
        p = load_label (base, block, pmydata);
    }
    else
    {
        /* 数组是局部变量。  */
        p = tree (pmydata->tree_nodes, REGISTER, NULL, NULL);
        p->operand.vreg = gen_vregArm32 (pmydata->virtual_regs, SP_REGNUM, GENERAL_REGS);

        if  (addend && addend->var->sdVar.sdvConst)
        {
            offset = base->sdvOffset + GetConstVal (addend->var, 0)->cvValue.cvIval;
            calculated = TRUE;
        }
        else
        {
            offset = base->sdvOffset;
        }

        if  (is_simm8 (offset))
        {
            conv = tree (pmydata->tree_nodes, simm8, NULL, NULL);
            conv->operand.cval.cvVtyp = TYP_INT;
            conv->operand.cval.cvValue.cvIval = offset;
        }
        else
        {
            conv = load_immediate (block, NULL, offset, TYP_INT, pmydata);
        }

        p = tree (pmydata->tree_nodes, ADD, p, conv);
        vreg = ((!dest_reg || (!calculated && addend)) ? gen_vregArm32 (pmydata->virtual_regs, -1, GENERAL_REGS) : dest_reg);
        p->operand.vreg = vreg;
        if  (!calculated)
        {
            commit (block, p, burmArm32_reg_NT, pmydata);
            p = tree (pmydata->tree_nodes, REGISTER, NULL, NULL);
            p->operand.vreg = vreg;
            register_descriptorArm32 (p->operand.vreg, NULL, base, NULL, TRUE);
            location_descriptorArm32 (base, NULL, p->operand.vreg, FALSE, TRUE);
        }
    }

    if  (!calculated && addend)
    {
        if  (addend->var->sdVar.sdvConst && is_simm8 (GetConstVal (addend->var, 0)->cvValue.cvIval))
        {
            tmp = tree (pmydata->tree_nodes, simm8, NULL, NULL);
            tmp->operand.cval.cvVtyp = TYP_INT;
            tmp->operand.cval.cvValue.cvIval = GetConstVal(addend->var, 0)->cvValue.cvIval;
        }
        else
        {
            tmp = addend->var->sdVar.sdvConst ? load_immediate(block, NULL, GetConstVal(addend->var, 0)->cvValue.cvIval, addend->var->sdType->tdTypeKind, pmydata)
                                                         : getreg (addend, pmydata->reg_map);
            if  (!tmp)
            {
                tmp = GetTreeNode (addend, pmydata);
            }
        }

        p = tree (pmydata->tree_nodes, ADD, p, tmp);
        p->operand.vreg = dest_reg ? dest_reg : gen_vregArm32 (pmydata->virtual_regs, -1, GENERAL_REGS);
    }

    return p;
}

static NODEPTR_TYPE
handler (varpool_node_set set, IRInst inst, NODEPTR_TYPE p, vreg_tArm32 vreg, PINTERNAL_DATA pmydata)
{
    NODEPTR_TYPE old_node;
    BOOL delay = TRUE;
    int i;
    varpool_node vnode;
    bitmap_iterator bi;
    unsigned instr;

    for (i = IRInstGetNumOperands (inst) - 1; i >= 0; i--)
    {
        vnode = varpool_get_node (set, IRInstGetOperand (inst, i));
        if  (IRInstIsOutput (inst, i) &&
             /* 不止一处使用。  */
             (bitmap_count_bits (vnode->use_chain) > 1 ||
             /* 多次定值。  */
             bitmap_count_bits (vnode->_defines) > 1 ||
             /* 在另一个基本块使用。  */
             (!bitmap_empty_p (vnode->use_chain) &&
             InterCodeGetInstByID (inst->bb->cfg->code, bitmap_first_set_bit (vnode->use_chain))->bb != inst->bb)))
        {
            delay = FALSE;
            break;
        }

        if  (! IRInstIsOutput (inst, i) &&
             bitmap_count_bits (vnode->_defines) > 1 &&
             vnode->var->sdSymKind == SYM_VAR &&
             ! vnode->var->sdVar.sdvConst &&
             vnode->var->sdType->tdTypeKind <= TYP_lastIntrins)
        {
            /* 对操作数的定值在基本块内当前指令之后。  */
            for (bmp_iter_set_init (&bi, vnode->_defines, 0, &instr);
                 bmp_iter_set (&bi, &instr);
                 bmp_iter_next (&bi, &instr))
            {
                if  (InterCodeGetInstByID (inst->bb->cfg->code, instr)->bb == inst->bb)
                {
                    delay = FALSE;
                    break;
                }
            }
        }

        /* 使用未定值的变量。  */
        if  (bitmap_count_bits (vnode->use_chain) &&
             bitmap_empty_p (vnode->_defines) &&
             vnode->var->sdSymKind == SYM_VAR &&
             ! vnode->var->sdVar.sdvConst &&
             vnode->var->sdType->tdTypeKind <= TYP_lastIntrins)
            warning (comp->cmpConfig.input_file_name, inst->line, "%s is used uninitialized in this function", stGetSymName (vnode->var));

        /* 定值不支配使用。  */
        if  (vnode->var->sdSymKind == SYM_VAR &&
             ! vnode->var->sdVar.sdvConst &&
             vnode->var->sdType->tdTypeKind <= TYP_lastIntrins &&
             ! IRInstIsOutput (inst, i) &&
             bitmap_count_bits (vnode->_defines) == 1 &&
             ! bitmap_bit_p (inst->bb->dom[0], InterCodeGetInstByID (inst->bb->cfg->code, bitmap_first_set_bit (vnode->_defines))->bb->index))
            warning (comp->cmpConfig.input_file_name, inst->line, "%s may be used uninitialized in this function", stGetSymName (vnode->var));
/*          fatal ("%s may be used uninitialized in this function", stGetSymName (vnode->var));*/
    }

    if  (delay)
    {
        old_node = avl_find (pmydata->node_map, p);
        if  (old_node)
        {
            LEFT_CHILD(old_node) = LEFT_CHILD(p);
            RIGHT_CHILD(old_node) = RIGHT_CHILD(p);
            old_node->op = p->op;
            LEFT_CHILD(p) = RIGHT_CHILD(p) = NULL;
            p = old_node;
        }
    }
    else
    {
        if  (pmydata->ddata)
            dwarfout_line (inst, pmydata);
        commit (inst->bb, p, burmArm32_reg_NT, pmydata);
        p = tree (pmydata->tree_nodes, REGISTER, NULL, NULL);
        p->operand.vreg = vreg;
    }

    return p;
}

static BOOL translate_nop (IRInst inst, PINTERNAL_DATA pmydata) 
{
    if  (pmydata->ddata)
        dwarfout_line (inst, pmydata);
    
    return TRUE;
}
static BOOL translate_load (IRInst inst, PINTERNAL_DATA pmydata) 
{
    vreg_tArm32 src_reg, dest_reg;
    varpool_node src, dest;
    NODEPTR_TYPE p = NULL;

    if  (pmydata->ddata)
        dwarfout_line (inst, pmydata);

    /* 获得目标操作数所在寄存器。  */
    dest = varpool_get_node (pmydata->set, IRInstGetOperand (inst, 0));
    dest_reg = get_registerArm32(dest, NULL, dest->var->sdType->tdTypeKind == TYP_FLOAT ? VFP_REGS : GENERAL_REGS, FALSE);
    if  (! dest_reg)
    {
        dest_reg = gen_vregArm32 (pmydata->virtual_regs, -1, dest->var->sdType->tdTypeKind == TYP_FLOAT ? VFP_REGS : GENERAL_REGS);
        register_descriptorArm32 (dest_reg, NULL, dest, NULL, FALSE);
        location_descriptorArm32 (dest, NULL, dest_reg, FALSE, FALSE);
    }

    /* 获得源操作数所在寄存器。  */
    src = varpool_get_node (pmydata->set, IRInstGetOperand (inst, 1));
    if  (!src->var->sdVar.sdvConst)
    {
        p = GetTreeNode (src, pmydata);
        src_reg = p->operand.vreg;
    }

    if  (p &&
         ! is_global_var (src->var) &&
         ! src->var->sdVar.sdvConst)
    {
        p = tree (pmydata->tree_nodes, src->var->sdType->tdTypeKind == TYP_FLOAT ? VMOV : MOV, p, NULL);
        p->operand.vreg = dest_reg;
    }
    else if (src->var->sdVar.sdvConst)
    {
        /* 要装载的是立即数。  */
        p = load_immediate (inst->bb, dest_reg, GetConstVal(src->var, 0)->cvValue.cvIval, dest->var->sdType->tdTypeKind, pmydata);
    }
    else if (is_global_var (src->var))
    {
        /* 获取全局变量的地址。  */
        p = getAddress (NULL, src, NULL, inst->bb, pmydata);

        /* 加载全局变量。  */
        p = tree (pmydata->tree_nodes, INDIRI4, p, NULL);
        p = tree (pmydata->tree_nodes, src->var->sdType->tdTypeKind==TYP_FLOAT ? VLDR : LDR, p, NULL);
        p->operand.vreg = src_reg;
        p = tree (pmydata->tree_nodes, dest->var->sdType->tdTypeKind == TYP_FLOAT ? VMOV : MOV, p, NULL);
        p->operand.vreg = dest_reg;
    }
    else
        fatal("internal compiler error");

    /* 需要立刻提交指令选择，因为它并不是静态单赋值形式的。  */
    commit (inst->bb, p, burmArm32_reg_NT, pmydata);
    p = tree (pmydata->tree_nodes, REGISTER, NULL, NULL);
    p->operand.vreg = dest_reg;
    setreg (dest, p, pmydata);
    return TRUE;
}
static BOOL translate_aload (IRInst inst, PINTERNAL_DATA pmydata) 
{
    varpool_node base;
    varpool_node offset;
    vreg_tArm32 dest_reg;
    varpool_node dest;
    NODEPTR_TYPE addr;
    NODEPTR_TYPE p;

    if  (pmydata->ddata)
        dwarfout_line (inst, pmydata);

    base = varpool_get_node (pmydata->set, IRInstGetOperand(inst, 1));
    offset = varpool_get_node (pmydata->set, IRInstGetOperand(inst, 2));

    dest = varpool_get_node (pmydata->set, IRInstGetOperand (inst, 0));
    dest_reg = get_registerArm32(dest, NULL, dest->var->sdType->tdTypeKind == TYP_FLOAT ? VFP_REGS : GENERAL_REGS, FALSE);
    if  (! dest_reg)
    {
        dest_reg = gen_vregArm32 (pmydata->virtual_regs, -1, dest->var->sdType->tdTypeKind == TYP_FLOAT ? VFP_REGS : GENERAL_REGS);
        register_descriptorArm32 (dest_reg, NULL, dest, NULL, FALSE);
        location_descriptorArm32 (dest, NULL, dest_reg, FALSE, FALSE);
    }

    if  (!offset->var->sdVar.sdvConst)
    {
        /* 偏移量是变量。  */

        /* ldr  r1, [sp, #44]
           add  r0, sp, #4
           ldr  r1, [r0, r1, lsl #2].  */

        /* 获取数组首地址。  */
        addr = getAddress (NULL, base, NULL, inst->bb, pmydata);
        /* 产生加载指令。  */
        p = GetTreeNode (offset, pmydata);
        p = tree (pmydata->tree_nodes, ADD, addr, p);
        /* 为ADD指令的结果分配临时寄存器，因为树模式匹配有可能为ADD单独产生一条指令。  */
        p->operand.vreg = gen_vregArm32 (pmydata->virtual_regs, -1, GENERAL_REGS);
        p = tree (pmydata->tree_nodes, INDIRI4, p, NULL);
        p = tree (pmydata->tree_nodes, dest->var->sdType->tdTypeKind==TYP_FLOAT ? VLDR : LDR, p, NULL);
        p->operand.vreg = dest_reg;
    }
    else if (base->var->sdVar.sdvArgument || is_global_var (base->var))
    {
        /* 数组是函数参数或全局变量。  */
        addr = getAddress (NULL, base, NULL, inst->bb, pmydata);
        p = load_base (inst->bb, dest_reg, stGetBaseType(base->var->sdType)->tdTypeKind, addr, GetConstVal(offset->var, 0)->cvValue.cvIval, pmydata);
    }
    else
    {
        /* 数组是局部变量。  */

        /* ldr  r3, [fp, #-32].  */
        p = tree (pmydata->tree_nodes, REGISTER, NULL, NULL);
        p->operand.vreg = gen_vregArm32(pmydata->virtual_regs, SP_REGNUM, GENERAL_REGS);
        p = load_base (inst->bb, dest_reg, stGetBaseType(base->var->sdType)->tdTypeKind, p, base->sdvOffset + GetConstVal(offset->var, 0)->cvValue.cvIval, pmydata);
    }

    /* 需要立刻提交指令选择，因为它并不是静态单赋值形式的。  */
    commit (inst->bb, p, burmArm32_reg_NT, pmydata);
    p = tree (pmydata->tree_nodes, REGISTER, NULL, NULL);
    p->operand.vreg = dest_reg;
    setreg (dest, p, pmydata);

    return TRUE;
}
static BOOL translate_store (IRInst inst, PINTERNAL_DATA pmydata) 
{
    vreg_tArm32 dest_reg;
    varpool_node src, dest;
    NODEPTR_TYPE p, ind;

    if  (pmydata->ddata)
        dwarfout_line (inst, pmydata);

    dest = varpool_get_node (pmydata->set, IRInstGetOperand (inst, 0));
    dest_reg = get_registerArm32 (dest, NULL, dest->var->sdType->tdTypeKind == TYP_FLOAT ? VFP_REGS : GENERAL_REGS, FALSE);
    if  (!dest_reg)
    {
        /* 如果目标操作数不在寄存器，为它新建一个。  */
        dest_reg = gen_vregArm32 (pmydata->virtual_regs, -1, dest->var->sdType->tdTypeKind == TYP_FLOAT ? VFP_REGS : GENERAL_REGS);
        register_descriptorArm32 (dest_reg, NULL, dest, NULL, FALSE);
        location_descriptorArm32 (dest, NULL, dest_reg, FALSE, FALSE);
    }
    src = varpool_get_node (pmydata->set, IRInstGetOperand (inst, 1));

    if  (src->var->sdVar.sdvConst)
    {
        /* 要存储的是立即数。  */
        p = load_immediate (inst->bb, dest_reg, GetConstVal(src->var, 0)->cvValue.cvIval, dest->var->sdType->tdTypeKind, pmydata);
        if  (is_global_var (dest->var))
        {
            ind = tree (pmydata->tree_nodes, INDIRI4, getAddress (NULL, dest, NULL, inst->bb, pmydata), NULL);
            p = tree (pmydata->tree_nodes, dest->var->sdType->tdTypeKind == TYP_FLOAT ? VSTR : STR, p, ind);
        }
    }
    else
    {
        p = GetTreeNode (src, pmydata);
        if  (is_global_var (dest->var))
        {
            ind = tree (pmydata->tree_nodes, INDIRI4, getAddress (NULL, dest, NULL, inst->bb, pmydata), NULL);
            p = tree (pmydata->tree_nodes, dest->var->sdType->tdTypeKind == TYP_FLOAT ? VSTR : STR, p, ind);
        }
        else
        {
            p = tree (pmydata->tree_nodes, dest->var->sdType->tdTypeKind == TYP_FLOAT ? VMOV : MOV, p, NULL);
            p->operand.vreg = dest_reg;
        }
    }

    if  (is_global_var (dest->var))
    {
        commit (inst->bb, p, 1, pmydata);
    }
    else
    {
        commit (inst->bb, p, burmArm32_reg_NT, pmydata);
        p = tree (pmydata->tree_nodes, REGISTER, NULL, NULL);
        p->operand.vreg = dest_reg;
        setreg (dest, p, pmydata);
    }

    return TRUE;
}
static BOOL translate_astore (IRInst inst, PINTERNAL_DATA pmydata) 
{
    varpool_node base;
    varpool_node offset;
    NODEPTR_TYPE p;
    varpool_node src;
    NODEPTR_TYPE addr;
    NODEPTR_TYPE tmp;

    if  (pmydata->ddata)
        dwarfout_line (inst, pmydata);

    base = varpool_get_node (pmydata->set, IRInstGetOperand (inst, 0));
    offset = varpool_get_node (pmydata->set, IRInstGetOperand (inst, 1));
    src = varpool_get_node (pmydata->set, IRInstGetOperand (inst, 2));
    p = src->var->sdVar.sdvConst ? load_immediate (inst->bb, NULL, GetConstVal(src->var, 0)->cvValue.cvIval, src->var->sdType->tdTypeKind, pmydata)
                                                 : getreg (src, pmydata->reg_map);

    if  (!p)
        p = GetTreeNode (src, pmydata);

    if  (!offset->var->sdVar.sdvConst)
    {
        /* 偏移量是变量。  */

        /* 获取数组首地址。  */
        addr = getAddress (NULL, base, NULL, inst->bb, pmydata);
        tmp = GetTreeNode (offset, pmydata);
        /* 产生存储指令。  */
        tmp = tree (pmydata->tree_nodes, ADD, addr, tmp);
        /* 为ADD指令的结果分配临时寄存器，因为树模式匹配有可能为ADD单独产生一条指令。  */
        tmp->operand.vreg = gen_vregArm32 (pmydata->virtual_regs, -1, GENERAL_REGS);
        tmp = tree (pmydata->tree_nodes, INDIRI4, tmp, NULL);
        tmp = tree (pmydata->tree_nodes, src->var->sdType->tdTypeKind == TYP_FLOAT ? VSTR : STR, p, tmp);
    }
    else if  (base->var->sdVar.sdvArgument || is_global_var (base->var))
    {
        /* 数组是函数参数或全局变量。  */
        addr = getAddress (NULL, base, NULL, inst->bb, pmydata);
        tmp = store_base (inst->bb, p, addr, GetConstVal(offset->var, 0)->cvValue.cvIval, pmydata);
    }
    else
    {
        /* 数组是局部变量。  */

        /* ldr  r3, [fp, #-32].  */
        tmp = tree (pmydata->tree_nodes, REGISTER, NULL, NULL);
        tmp->operand.vreg = gen_vregArm32 (pmydata->virtual_regs, SP_REGNUM, GENERAL_REGS);
        tmp = store_base (inst->bb, p, tmp, base->sdvOffset + GetConstVal(offset->var, 0)->cvValue.cvIval, pmydata);
    }

    /* 设置操作数的寄存器、地址描述符。  */
    register_descriptorArm32 (p->operand.vreg, NULL, base, offset, FALSE);
    location_descriptorArm32 (base, NULL, NULL, TRUE, FALSE);
    location_descriptorArm32 (base, offset, p->operand.vreg, FALSE, FALSE);
    commit (inst->bb, tmp, 1, pmydata);

    return TRUE;
}
static BOOL translate_move (IRInst inst, PINTERNAL_DATA pmydata) 
{
    varpool_node src, dest;
    NODEPTR_TYPE p;
    vreg_tArm32 dest_reg;

    dest = varpool_get_node (pmydata->set, IRInstGetOperand (inst, 0));
    src = varpool_get_node (pmydata->set, IRInstGetOperand (inst, 1));
    p = src->var->sdVar.sdvConst ? load_immediate(inst->bb, NULL, GetConstVal(src->var, 0)->cvValue.cvIval, src->var->sdType->tdTypeKind, pmydata)
                                                 : getreg (src, pmydata->reg_map);
    if  (!p)
        p = GetTreeNode (src, pmydata);

    p = tree (pmydata->tree_nodes, dest->var->sdType->tdTypeKind == TYP_FLOAT ? VMOV : MOV, p, NULL);
    dest_reg = get_registerArm32 (dest, NULL, dest->var->sdType->tdTypeKind == TYP_FLOAT ? VFP_REGS : GENERAL_REGS, FALSE);
    if  (!dest_reg)
    {
        /* 如果目标操作数不在寄存器，为它新建一个。  */
        dest_reg = gen_vregArm32 (pmydata->virtual_regs, -1, dest->var->sdType->tdTypeKind == TYP_FLOAT ? VFP_REGS : GENERAL_REGS);
        register_descriptorArm32 (dest_reg, NULL, dest, NULL, FALSE);
        location_descriptorArm32 (dest, NULL, dest_reg, FALSE, FALSE);
    }
    p->operand.vreg = dest_reg;

    p = handler (pmydata->set, inst, p, dest_reg, pmydata);
    setreg (dest, p, pmydata);

    return TRUE;
}
static BOOL translate_addop (IRInst inst, int op, PINTERNAL_DATA pmydata) 
{
    NODEPTR_TYPE left, right, p;
    varpool_node arg1, arg2, dest, temp;
    vreg_tArm32 dest_reg;

    dest = varpool_get_node (pmydata->set, IRInstGetOperand(inst, 0));
    arg1 = varpool_get_node (pmydata->set, IRInstGetOperand(inst, 1));
    arg2 = varpool_get_node (pmydata->set, IRInstGetOperand(inst, 2));

    if  (arg1->var->sdVar.sdvConst &&
         (op == ADD || op == MUL) &&
         arg2->var->sdType->tdTypeKind != TYP_FLOAT)
    {
        /* 若操作数1是立即数，交换二操作数。  */
        temp = arg1;
        arg1 = arg2;
        arg2 = temp;
    }

    left = arg1->var->sdVar.sdvConst ? load_immediate(inst->bb, NULL, GetConstVal(arg1->var, 0)->cvValue.cvIval, arg1->var->sdType->tdTypeKind, pmydata)
                                                 : getreg (arg1, pmydata->reg_map);
    if  (!left)
        left = GetTreeNode (arg1, pmydata);
    if  (!arg2->var->sdVar.sdvConst ||
         arg2->var->sdType->tdTypeKind == TYP_FLOAT ||
         !is_simm8(GetConstVal(arg2->var, 0)->cvValue.cvIval) ||
         op == MUL || op == SDIV)
    {
        right = arg2->var->sdVar.sdvConst ? load_immediate(inst->bb, NULL, GetConstVal(arg2->var, 0)->cvValue.cvIval, arg2->var->sdType->tdTypeKind, pmydata)
                                                    : getreg (arg2, pmydata->reg_map);
        if  (!right)
            right = GetTreeNode (arg2, pmydata);
    }
    else if (op == LSL || op == ASR || op == LSR)
    {
        right = tree (pmydata->tree_nodes, imm5, NULL, NULL);
        right->operand.cval = *GetConstVal (arg2->var, 0);
    }
    else
    {
        right = tree (pmydata->tree_nodes, simm8, NULL, NULL);
        right->operand.cval = *GetConstVal (arg2->var, 0);
    }

    p = tree (pmydata->tree_nodes, op, left, right);
    dest_reg = get_registerArm32 (dest, NULL, dest->var->sdType->tdTypeKind == TYP_FLOAT ? VFP_REGS : GENERAL_REGS, FALSE);
    if  (!dest_reg)
    {
        /* 如果目标操作数不在寄存器，为它新建一个。  */
        dest_reg = gen_vregArm32 (pmydata->virtual_regs, -1, dest->var->sdType->tdTypeKind == TYP_FLOAT ? VFP_REGS : GENERAL_REGS);
        register_descriptorArm32 (dest_reg, NULL, dest, NULL, FALSE);
        location_descriptorArm32 (dest, NULL, dest_reg, FALSE, FALSE);
    }
    p->operand.vreg = dest_reg;

    p = handler (pmydata->set, inst, p, dest_reg, pmydata);
    setreg (dest, p, pmydata);

    return TRUE;
}
static BOOL translate_add (IRInst inst, PINTERNAL_DATA pmydata) 
{
    return translate_addop (inst, IRInstGetOperand(inst, 1)->var->sdType->tdTypeKind == TYP_FLOAT ? VADD : ADD, pmydata);
}
static BOOL translate_addptr (IRInst inst, PINTERNAL_DATA pmydata) 
{
    varpool_node arg1, arg2, dest;
    NODEPTR_TYPE p;
    vreg_tArm32 dest_reg;

    arg1 = varpool_get_node (pmydata->set, IRInstGetOperand (inst, 1));
    arg2 = varpool_get_node (pmydata->set, IRInstGetOperand (inst, 2));
    dest = varpool_get_node (pmydata->set, IRInstGetOperand (inst, 0));
    dest_reg = get_registerArm32 (dest, NULL, dest->var->sdType->tdTypeKind == TYP_FLOAT ? VFP_REGS : GENERAL_REGS, FALSE);
    if  (!dest_reg)
    {
        /* 如果目标操作数不在寄存器，为它新建一个。  */
        dest_reg = gen_vregArm32 (pmydata->virtual_regs, -1, dest->var->sdType->tdTypeKind == TYP_FLOAT ? VFP_REGS : GENERAL_REGS);
        register_descriptorArm32 (dest_reg, NULL, dest, NULL, FALSE);
        location_descriptorArm32 (dest, NULL, dest_reg, FALSE, FALSE);
    }

    p = getAddress (dest_reg, arg1, arg2, inst->bb, pmydata);

    p = handler (pmydata->set, inst, p, dest_reg, pmydata);
    setreg (dest, p, pmydata);

    return TRUE;
}
static BOOL translate_sub (IRInst inst, PINTERNAL_DATA pmydata) 
{
    return translate_addop (inst, IRInstGetOperand(inst, 1)->var->sdType->tdTypeKind == TYP_FLOAT ? VSUB : SUB, pmydata);
}
static BOOL translate_mul (IRInst inst, PINTERNAL_DATA pmydata) 
{
    return translate_addop (inst, IRInstGetOperand(inst, 1)->var->sdType->tdTypeKind == TYP_FLOAT ? VMUL : MUL, pmydata);
}
static BOOL translate_div (IRInst inst, PINTERNAL_DATA pmydata) 
{
    return translate_addop (inst, IRInstGetOperand(inst, 1)->var->sdType->tdTypeKind == TYP_FLOAT ? VDIV : SDIV, pmydata);
}
static BOOL translate_rem (IRInst inst, PINTERNAL_DATA pmydata) 
{
    NODEPTR_TYPE left, right, p;
    varpool_node dest, arg1, arg2;
    vreg_tArm32 dest_reg;

    dest = varpool_get_node (pmydata->set, IRInstGetOperand(inst, 0));
    arg1 = varpool_get_node (pmydata->set, IRInstGetOperand(inst, 1));
    arg2 = varpool_get_node (pmydata->set, IRInstGetOperand(inst, 2));

    left = arg1->var->sdVar.sdvConst ? load_immediate(inst->bb, NULL, GetConstVal(arg1->var, 0)->cvValue.cvIval, arg1->var->sdType->tdTypeKind, pmydata)
                                                 : getreg (arg1, pmydata->reg_map);
    if  (!left)
    {
        left = GetTreeNode (arg1, pmydata);
    }
    right = arg2->var->sdVar.sdvConst ? load_immediate(inst->bb, NULL, GetConstVal(arg2->var, 0)->cvValue.cvIval, arg2->var->sdType->tdTypeKind, pmydata)
                                                 : getreg (arg2, pmydata->reg_map);
    if  (!right)
    {
        right = GetTreeNode (arg2, pmydata);
    }

    p = tree (pmydata->tree_nodes, SDIV, left, right);
    p->operand.vreg = gen_vregArm32 (pmydata->virtual_regs, -1, GENERAL_REGS);

    p = tree (pmydata->tree_nodes, MUL, p, right);
    p->operand.vreg = gen_vregArm32 (pmydata->virtual_regs, -1, GENERAL_REGS);

    p = tree (pmydata->tree_nodes, SUB, left, p);
    dest_reg = get_registerArm32 (dest, NULL, dest->var->sdType->tdTypeKind == TYP_FLOAT ? VFP_REGS : GENERAL_REGS, FALSE);
    if  (!dest_reg)
    {
        /* 如果目标操作数不在寄存器，为它新建一个。  */
        dest_reg = gen_vregArm32 (pmydata->virtual_regs, -1, dest->var->sdType->tdTypeKind == TYP_FLOAT ? VFP_REGS : GENERAL_REGS);
        register_descriptorArm32 (dest_reg, NULL, dest, NULL, FALSE);
        location_descriptorArm32 (dest, NULL, dest_reg, FALSE, FALSE);
    }
    p->operand.vreg = dest_reg;

    p = handler (pmydata->set, inst, p, dest_reg, pmydata);
    setreg (dest, p, pmydata);

    return TRUE;
}
/* 逻辑左移  */
static BOOL translate_lsl (IRInst inst, PINTERNAL_DATA pmydata) 
{
    return translate_addop (inst, LSL, pmydata);
}
/* 算数右移  */
static BOOL translate_asr (IRInst inst, PINTERNAL_DATA pmydata) 
{
    return translate_addop (inst, ASR, pmydata);
}
/* 逻辑右移  */
static BOOL translate_lsr (IRInst inst, PINTERNAL_DATA pmydata) 
{
    return translate_addop (inst, LSR, pmydata);
}
static BOOL translate_neg (IRInst inst, PINTERNAL_DATA pmydata) 
{
    NODEPTR_TYPE left, right, p;
    varpool_node dest, arg1;
    vreg_tArm32 dest_reg;

    dest = varpool_get_node (pmydata->set, IRInstGetOperand (inst, 0));
    dest_reg = get_registerArm32 (dest, NULL, dest->var->sdType->tdTypeKind == TYP_FLOAT ? VFP_REGS : GENERAL_REGS, FALSE);
    if  (!dest_reg)
    {
        /* 如果目标操作数不在寄存器，为它新建一个。  */
        dest_reg = gen_vregArm32 (pmydata->virtual_regs, -1, dest->var->sdType->tdTypeKind == TYP_FLOAT ? VFP_REGS : GENERAL_REGS);
        register_descriptorArm32 (dest_reg, NULL, dest, NULL, FALSE);
        location_descriptorArm32 (dest, NULL, dest_reg, FALSE, FALSE);
    }

    arg1 = varpool_get_node (pmydata->set, IRInstGetOperand (inst, 1));

    left = arg1->var->sdVar.sdvConst ? load_immediate(inst->bb, NULL, GetConstVal(arg1->var, 0)->cvValue.cvIval, arg1->var->sdType->tdTypeKind, pmydata)
                                                 : getreg (arg1, pmydata->reg_map);
    if  (!left)
    {
        left = GetTreeNode (arg1, pmydata);
    }

    if  (dest->var->sdType->tdTypeKind == TYP_FLOAT)
    {
        p = tree (pmydata->tree_nodes, VNEG, left, NULL);
        p->operand.vreg = dest_reg;
    }
    else
    {
        right = tree (pmydata->tree_nodes, simm8, NULL, NULL);
        right->operand.cval.cvValue.cvIval = 0;
        p = tree (pmydata->tree_nodes, RSB, left, right);
        p->operand.vreg = dest_reg;
    }

    p = handler (pmydata->set, inst, p, dest_reg, pmydata);
    setreg (dest, p, pmydata);

    return TRUE;
}
static BOOL translate_i2f (IRInst inst, PINTERNAL_DATA pmydata) 
{
    NODEPTR_TYPE left;
    varpool_node dest, arg1;
    NODEPTR_TYPE p;
    vreg_tArm32 dest_reg;

    dest = varpool_get_node (pmydata->set, IRInstGetOperand (inst, 0));
    dest_reg = get_registerArm32 (dest, NULL, dest->var->sdType->tdTypeKind == TYP_FLOAT ? VFP_REGS : GENERAL_REGS, FALSE);
    if  (!dest_reg)
    {
        /* 如果目标操作数不在寄存器，为它新建一个。  */
        dest_reg = gen_vregArm32 (pmydata->virtual_regs, -1, dest->var->sdType->tdTypeKind == TYP_FLOAT ? VFP_REGS : GENERAL_REGS);
        register_descriptorArm32 (dest_reg, NULL, dest, NULL, FALSE);
        location_descriptorArm32 (dest, NULL, dest_reg, FALSE, FALSE);
    }

    arg1 = varpool_get_node (pmydata->set, IRInstGetOperand (inst, 1));

    left = arg1->var->sdVar.sdvConst ? load_immediate(inst->bb, NULL, GetConstVal(arg1->var, 0)->cvValue.cvIval, arg1->var->sdType->tdTypeKind, pmydata)
                                                 : getreg (arg1, pmydata->reg_map);
    if  (!left)
    {
        left = GetTreeNode (arg1, pmydata);
    }

    p = tree (pmydata->tree_nodes, VMOV, left, NULL);
    p->operand.vreg = gen_vregArm32 (pmydata->virtual_regs, -1, VFP_REGS);
    p = tree (pmydata->tree_nodes, vcvt_signedToFloatingPoint, p, NULL);
    p->operand.vreg = dest_reg;

    p = handler (pmydata->set, inst, p, dest_reg, pmydata);
    setreg (dest, p, pmydata);

    return TRUE;
}
static BOOL translate_f2i (IRInst inst, PINTERNAL_DATA pmydata) 
{
    NODEPTR_TYPE left;
    varpool_node dest, arg1;
    NODEPTR_TYPE p;
    vreg_tArm32 dest_reg;

    dest = varpool_get_node (pmydata->set, IRInstGetOperand (inst, 0));
    dest_reg = get_registerArm32 (dest, NULL, dest->var->sdType->tdTypeKind == TYP_FLOAT ? VFP_REGS : GENERAL_REGS, FALSE);
    if  (!dest_reg)
    {
        /* 如果目标操作数不在寄存器，为它新建一个。  */
        dest_reg = gen_vregArm32 (pmydata->virtual_regs, -1, dest->var->sdType->tdTypeKind == TYP_FLOAT ? VFP_REGS : GENERAL_REGS);
        register_descriptorArm32 (dest_reg, NULL, dest, NULL, FALSE);
        location_descriptorArm32 (dest, NULL, dest_reg, FALSE, FALSE);
    }

    arg1 = varpool_get_node (pmydata->set, IRInstGetOperand (inst, 1));

    left = arg1->var->sdVar.sdvConst ? load_immediate(inst->bb, NULL, GetConstVal(arg1->var, 0)->cvValue.cvIval, arg1->var->sdType->tdTypeKind, pmydata)
                                                 : getreg (arg1, pmydata->reg_map);
    if  (!left)
    {
        left = GetTreeNode (arg1, pmydata);
    }

    p = tree (pmydata->tree_nodes, vcvt_floatingPointToSigned, left, NULL);
    p->operand.vreg = gen_vregArm32 (pmydata->virtual_regs, -1, arg1->var->sdType->tdTypeKind==TYP_FLOAT ? VFP_REGS : GENERAL_REGS);
    p = tree (pmydata->tree_nodes, VMOV, p, NULL);
    p->operand.vreg = dest_reg;

    p = handler (pmydata->set, inst, p, dest_reg, pmydata);
    setreg (dest, p, pmydata);

    return TRUE;
}
static BOOL
emit_compare (IRInst inst, varpool_node arg1, varpool_node arg2, BOOL *swaped, PINTERNAL_DATA pmydata)
{
    NODEPTR_TYPE left, right;
    varpool_node temp;
    int compare;
    NODEPTR_TYPE p;

    *swaped = FALSE;

    if  ((arg1->var->sdVar.sdvConst && arg1->var->sdType->tdTypeKind != TYP_FLOAT &&
         is_simm8 (GetConstVal (arg1->var, 0)->cvValue.cvIval)) ||
         (arg1->var->sdVar.sdvConst && arg1->var->sdType->tdTypeKind == TYP_FLOAT && 
         GetConstVal (arg1->var, 0)->cvValue.cvFval == 0))
    {
        /* 若源操作数1是立即数，交换两操作数。  */
        temp = arg1;
        arg1 = arg2;
        arg2 = temp;
        *swaped = TRUE;
    }

    left = arg1->var->sdVar.sdvConst ? load_immediate(inst->bb, NULL, GetConstVal(arg1->var, 0)->cvValue.cvIval, arg1->var->sdType->tdTypeKind, pmydata)
                                                 : getreg (arg1, pmydata->reg_map);
    if  (!left)
    {
        left = GetTreeNode (arg1, pmydata);
    }

    if  (((arg2->var->sdType->tdTypeKind != TYP_FLOAT) && (!arg2->var->sdVar.sdvConst ||
         !is_simm8(GetConstVal(arg2->var, 0)->cvValue.cvIval))) ||
         (arg2->var->sdType->tdTypeKind == TYP_FLOAT && (!arg2->var->sdVar.sdvConst ||
         GetConstVal(arg2->var, 0)->cvValue.cvFval != 0)))
    {
        right = arg2->var->sdVar.sdvConst ? load_immediate(inst->bb, NULL, GetConstVal(arg2->var, 0)->cvValue.cvIval, arg2->var->sdType->tdTypeKind, pmydata)
                                                     : getreg (arg2, pmydata->reg_map);
        if  (!right)
            right = GetTreeNode (arg2, pmydata);
        compare = arg2->var->sdType->tdTypeKind == TYP_FLOAT ? VCMP : CMP;
    }
    else if (GetConstVal(arg2->var, 0)->cvValue.cvFval == 0 &&
             arg2->var->sdType->tdTypeKind == TYP_FLOAT)
    {
        right = NULL;
        compare = VCMPz;
    }
    else
    {
        right = tree (pmydata->tree_nodes, simm8, NULL, NULL);
        right->operand.cval = *GetConstVal (arg2->var, 0);
        compare = CMP;
    }

    /* 产生比较指令。  */
    p = tree (pmydata->tree_nodes, compare, left, right);
    commit (inst->bb, p, 1, pmydata);

    if  (arg2->var->sdType->tdTypeKind == TYP_FLOAT)
    {
        p = tree (pmydata->tree_nodes, VMRS, NULL, NULL);
        commit (inst->bb, p, 1, pmydata);
    }

    return TRUE;
}
static BOOL translate_branches (IRInst inst, Condition condition, PINTERNAL_DATA pmydata) 
{
    BOOL bSuccess = FALSE;
    NODEPTR_TYPE label;
    BOOL swaped = FALSE;
    BOOL IfFalse;
    NODEPTR_TYPE p;

    /* 添加一个标记，供if_convert使用。  */
    p = tree(pmydata->tree_nodes, STRING, NULL, NULL);
    p->operand.ds = dyn_string_new (32);
    dyn_string_insert_cstr (p->operand.ds, 0, "\t@ Jump to the target.");
    commit (inst->bb, p, 1, pmydata);

    if (!emit_compare (inst, varpool_get_node(pmydata->set, IRInstGetOperand(inst, 0)), varpool_get_node(pmydata->set, IRInstGetOperand(inst, 1)), &swaped, pmydata))
        goto fail;

    if  (swaped)
    {
        switch (condition)
        {
        case ConditionGE: condition = ConditionLE; break;
        case ConditionGT: condition = ConditionLT; break;
        case ConditionLE: condition = ConditionGE; break;
        case ConditionLT: condition = ConditionGT; break;
        case ConditionNE: break;
        case ConditionEQ: break;
        default: fatal("internal compiler error");
        }
    }

    /* 避免生成冗余的goto指令。  */
    IfFalse = (((struct BblockArm32 *) inst->bb->param)->next_block && (*(IRInst *) List_First (((struct BblockArm32 *) inst->bb->param)->next_block->insns))->uid == GetConstVal (IRInstGetOperand (inst, 2)->var, 0)->cvValue.cvIval);
    if  (IfFalse)
    {
        switch (condition)
        {
        case ConditionGE: condition = ConditionLT; break;
        case ConditionGT: condition = ConditionLE; break;
        case ConditionLE: condition = ConditionGT; break;
        case ConditionLT: condition = ConditionGE; break;
        case ConditionNE: condition = ConditionEQ; break;
        case ConditionEQ: condition = ConditionNE; break;
        default: fatal("internal compiler error");
        }
    }

    /* 产生分支指令。  */
    label = tree (pmydata->tree_nodes, CNSTI4, NULL, NULL);
    label->operand.cval.cvValue.cvIval = GetConstVal(IRInstGetOperand (inst, IfFalse ? 3 : 2)->var, 0)->cvValue.cvIval;
    p = tree (pmydata->tree_nodes, JUMPV, label, NULL);
    p->condition = condition;
    commit (inst->bb, p, 1, pmydata);

    /* 产生跳转指令。  */
    label = tree (pmydata->tree_nodes, CNSTI4, NULL, NULL);
    label->operand.cval.cvValue.cvIval = GetConstVal(IRInstGetOperand (inst, IfFalse ? 2 : 3)->var, 0)->cvValue.cvIval;
    p = tree (pmydata->tree_nodes, JUMPV, label, NULL);
    commit (inst->bb, p, 1, pmydata);

    bSuccess = TRUE;

fail:
    return bSuccess;
}
static BOOL translate_ifeq (IRInst inst, PINTERNAL_DATA pmydata) 
{
    if  (pmydata->ddata)
        dwarfout_line (inst, pmydata);

    return translate_branches (inst, ConditionEQ, pmydata);
}
static BOOL translate_ifne (IRInst inst, PINTERNAL_DATA pmydata) 
{
    if  (pmydata->ddata)
        dwarfout_line (inst, pmydata);

    return translate_branches (inst, ConditionNE, pmydata);
}
static BOOL translate_iflt (IRInst inst, PINTERNAL_DATA pmydata) 
{
    if  (pmydata->ddata)
        dwarfout_line (inst, pmydata);

    return translate_branches (inst, ConditionLT, pmydata);
}
static BOOL translate_ifge (IRInst inst, PINTERNAL_DATA pmydata) 
{
    if  (pmydata->ddata)
        dwarfout_line (inst, pmydata);

    return translate_branches (inst, ConditionGE, pmydata);
}
static BOOL translate_ifgt (IRInst inst, PINTERNAL_DATA pmydata) 
{
    if  (pmydata->ddata)
        dwarfout_line (inst, pmydata);

    return translate_branches (inst, ConditionGT, pmydata);
}
static BOOL translate_ifle (IRInst inst, PINTERNAL_DATA pmydata) 
{
    if  (pmydata->ddata)
        dwarfout_line (inst, pmydata);

    return translate_branches (inst, ConditionLE, pmydata);
}
static BOOL translate_relop (IRInst inst, PINTERNAL_DATA pmydata) 
{
    Condition cond1 = ConditionInvalid, cond2 = ConditionInvalid;
    enum IRInstOperator opcode;
    NODEPTR_TYPE p;
    BOOL bSuccess = FALSE;
    NODEPTR_TYPE conv;
    BOOL swaped = FALSE;
    varpool_node dest;
    vreg_tArm32 dest_reg;

    dest = varpool_get_node (pmydata->set, IRInstGetOperand(inst, 0));
    dest_reg = get_registerArm32 (dest, NULL, dest->var->sdType->tdTypeKind == TYP_FLOAT ? VFP_REGS : GENERAL_REGS, FALSE);
    if  (!dest_reg)
    {
        /* 如果目标操作数不在寄存器，为它新建一个。  */
        dest_reg = gen_vregArm32 (pmydata->virtual_regs, -1, dest->var->sdType->tdTypeKind == TYP_FLOAT ? VFP_REGS : GENERAL_REGS);
        register_descriptorArm32 (dest_reg, NULL, dest, NULL, FALSE);
        location_descriptorArm32 (dest, NULL, dest_reg, FALSE, FALSE);
    }

    if (!emit_compare (inst, varpool_get_node(pmydata->set, IRInstGetOperand(inst, 1)), varpool_get_node(pmydata->set, IRInstGetOperand(inst, 2)), &swaped, pmydata))
        goto fail;

    opcode = inst->opcode;

    if  (swaped)
    {
        switch (opcode)
        {
        case IRINST_OP_lt: opcode = IRINST_OP_gt; break;
        case IRINST_OP_ge: opcode = IRINST_OP_le; break;
        case IRINST_OP_gt: opcode = IRINST_OP_lt; break;
        case IRINST_OP_le: opcode = IRINST_OP_ge; break;
        case IRINST_OP_eq: break;
        case IRINST_OP_ne: break;
        default: fatal("internal compiler error");
        }
    }

    switch (opcode)
    {
    case IRINST_OP_eq: cond1 = ConditionEQ; cond2 = ConditionNE; break;
    case IRINST_OP_ne: cond1 = ConditionNE; cond2 = ConditionEQ; break;
    case IRINST_OP_lt: cond1 = ConditionLT; cond2 = ConditionGE; break;
    case IRINST_OP_ge: cond1 = ConditionGE; cond2 = ConditionLT; break;
    case IRINST_OP_gt: cond1 = ConditionGT; cond2 = ConditionLE; break;
    case IRINST_OP_le: cond1 = ConditionLE; cond2 = ConditionGT; break;
    default: fatal("internal compiler error");
    }

    /* 产生条件mov指令。  */
    conv = tree (pmydata->tree_nodes, imm16, NULL, NULL);
    conv->operand.cval.cvValue.cvIval = 1;
    p = tree (pmydata->tree_nodes, MOV, conv, NULL);
    p->condition = cond1;
    p->operand.vreg = dest_reg;
    commit (inst->bb, p, burmArm32_reg_NT, pmydata);

    conv = tree (pmydata->tree_nodes, imm16, NULL, NULL);
    conv->operand.cval.cvValue.cvIval = 0;
    p = tree (pmydata->tree_nodes, MOV, conv, NULL);
    p->condition = cond2;
    p->operand.vreg = dest_reg;
    commit (inst->bb, p, burmArm32_reg_NT, pmydata);

    p = tree(pmydata->tree_nodes, REGISTER, NULL, NULL);
    p->operand.vreg = dest_reg;
    setreg (dest, p, pmydata);

    bSuccess = TRUE;

fail:
    return bSuccess;
}
static BOOL translate_eq (IRInst inst, PINTERNAL_DATA pmydata) 
{
    return translate_relop (inst, pmydata);
}
static BOOL translate_ne (IRInst inst, PINTERNAL_DATA pmydata) 
{
    return translate_relop (inst, pmydata);
}
static BOOL translate_lt (IRInst inst, PINTERNAL_DATA pmydata) 
{
    return translate_relop (inst, pmydata);
}
static BOOL translate_ge (IRInst inst, PINTERNAL_DATA pmydata) 
{
    return translate_relop (inst, pmydata);
}
static BOOL translate_gt (IRInst inst, PINTERNAL_DATA pmydata) 
{
    return translate_relop (inst, pmydata);
}
static BOOL translate_le (IRInst inst, PINTERNAL_DATA pmydata) 
{
    return translate_relop (inst, pmydata);
}
static BOOL translate_not (IRInst inst, PINTERNAL_DATA pmydata) 
{
    NODEPTR_TYPE left;
    BOOL bSuccess = FALSE;
    varpool_node arg1, arg2, dest;
    NODEPTR_TYPE conv;
    NODEPTR_TYPE num;
    BOOL dummy;
    IRInst tmp_insn;
    NODEPTR_TYPE p;
    vreg_tArm32 dest_reg;

    dest = varpool_get_node (pmydata->set, IRInstGetOperand(inst, 0));
    dest_reg = get_registerArm32 (dest, NULL, dest->var->sdType->tdTypeKind == TYP_FLOAT ? VFP_REGS : GENERAL_REGS, FALSE);
    if  (!dest_reg)
    {
        /* 如果目标操作数不在寄存器，为它新建一个。  */
        dest_reg = gen_vregArm32 (pmydata->virtual_regs, -1, dest->var->sdType->tdTypeKind == TYP_FLOAT ? VFP_REGS : GENERAL_REGS);
        register_descriptorArm32 (dest_reg, NULL, dest, NULL, FALSE);
        location_descriptorArm32 (dest, NULL, dest_reg, FALSE, FALSE);
    }

    arg1 = varpool_get_node (pmydata->set, IRInstGetOperand(inst, 1));

    left = arg1->var->sdVar.sdvConst ? load_immediate(inst->bb, NULL, GetConstVal(arg1->var, 0)->cvValue.cvIval, arg1->var->sdType->tdTypeKind, pmydata)
                                                 : getreg (arg1, pmydata->reg_map);
    if  (!left)
    {
        left = GetTreeNode (arg1, pmydata);
    }

    if  (arg1->var->sdType->tdTypeKind== TYP_FLOAT)
    {
        /* 我们只能通过这种方式新建一个浮点立即数0。  */
        tmp_insn = IRInstEmitInst(IRINST_OP_nop, inst->line, inst->column);
        IRInstSetOperand (tmp_insn, 0, stCreateFconNode (pmydata->stab, 0.0f));
        arg2 = varpool_get_node (pmydata->set, IRInstGetOperand (tmp_insn, 0));

        if (!emit_compare (inst, arg1, arg2, &dummy, pmydata))
            goto fail;

        IRInstDelInst (tmp_insn);

        /* 产生条件mov指令。  */
        conv = tree (pmydata->tree_nodes, imm16, NULL, NULL);
        conv->operand.cval.cvValue.cvIval = 1;
        p = tree (pmydata->tree_nodes, MOV, conv, NULL);
        p->condition = ConditionEQ;
        p->operand.vreg = dest_reg;
        commit (inst->bb, p, burmArm32_reg_NT, pmydata);

        conv = tree (pmydata->tree_nodes, imm16, NULL, NULL);
        conv->operand.cval.cvValue.cvIval = 0;
        p = tree (pmydata->tree_nodes, MOV, conv, NULL);
        p->condition = ConditionNE;
        p->operand.vreg = dest_reg;
        commit (inst->bb, p, burmArm32_reg_NT, pmydata);

        p = tree(pmydata->tree_nodes, REGISTER, NULL, NULL);
        p->operand.vreg = dest_reg;
    }
    else
    {
        num = tree (pmydata->tree_nodes, CLZ, left, NULL);
        num->operand.vreg = gen_vregArm32 (pmydata->virtual_regs, -1, GENERAL_REGS);
        conv = tree (pmydata->tree_nodes, imm5, NULL, NULL);
        conv->operand.cval.cvValue.cvIval = 5;
        p = tree (pmydata->tree_nodes, LSR, num, conv);
        p->operand.vreg = dest_reg;

        p = handler (pmydata->set, inst, p, dest_reg, pmydata);

    }
    setreg (dest, p, pmydata);

    bSuccess = TRUE;

fail:
    return bSuccess;
}
static BOOL translate_goto (IRInst inst, PINTERNAL_DATA pmydata) 
{
    NODEPTR_TYPE label;
    NODEPTR_TYPE p;

    if  (pmydata->ddata)
        dwarfout_line (inst, pmydata);
    
    /* 添加一个标记，供if_convert使用。  */
    p = tree(pmydata->tree_nodes, STRING, NULL, NULL);
    p->operand.ds = dyn_string_new (32);
    dyn_string_insert_cstr (p->operand.ds, 0, "\t@ Jump to the target.");
    commit (inst->bb, p, 1, pmydata);

    label = tree (pmydata->tree_nodes, CNSTI4, NULL, NULL);
    label->operand.cval.cvValue.cvIval = GetConstVal(IRInstGetOperand(inst, 0)->var, 0)->cvValue.cvIval;
    p = tree (pmydata->tree_nodes, JUMPV, label, NULL);
    commit (inst->bb, p, 1, pmydata);
    return TRUE;
}
static int compare_symbols ( varpool_node arg1, varpool_node arg2 )
{
   return arg1->var->uid - arg2->var->uid;
}

/* 在函数开头加载所有全局变量地址。  */
static void
load_global_address (basic_block block, PINTERNAL_DATA pmydata)
{
    struct avl_traverser trav;
    varpool_node iter;
    NODEPTR_TYPE p;
    struct avl_table *loaded = avl_create ((avl_comparison_func *)compare_symbols, NULL, NULL);
    varpool_node vnode;
    BOOL found;

    for(  iter = (varpool_node)avl_t_first (&trav, pmydata->set->nodes)
       ;  iter != NULL
       ;  iter = (varpool_node)avl_t_next (&trav)
       )
    {
        if  (iter->var->sdSymKind == SYM_VAR &&
             ((is_global_var (iter->var) && !iter->var->sdVar.sdvConst)
             || iter->var->sdType->tdTypeKind > TYP_lastIntrins) &&
             ! iter->var->sdIsImplicit)
        {
            found = TRUE;
            /* 使SSA形式的全局变量地址也仅被加载一次。  */
            vnode = (varpool_node) avl_find (loaded, iter);
            if  (vnode == NULL)
            {
                avl_insert (loaded, iter);
                vnode = (varpool_node) avl_find (loaded, iter);
                found = FALSE;
            }
            p = getAddress (NULL, vnode, NULL, block, pmydata);
            if  (found)
            {
                register_descriptorArm32 (p->operand.vreg, NULL, iter, NULL, TRUE);
                location_descriptorArm32 (iter, NULL, p->operand.vreg, FALSE, TRUE);
            }
            else
            {
                commit (block, p, burmArm32_reg_NT, pmydata);
            }
        }
    }

    avl_destroy (loaded, NULL);
}

static BOOL translate_call (IRInst inst, PINTERNAL_DATA pmydata) 
{
    int funcCallArgCnt;
    int icnt = 0;
    int fcnt = 0;
    IRInst *curs;
    NODEPTR_TYPE p;
    NODEPTR_TYPE tmp;
    varpool_node param;
    varpool_node dest;
    vreg_tArm32 vreg;

    if  (pmydata->ddata)
        dwarfout_line (inst, pmydata);

    dest = varpool_get_node (pmydata->set, IRInstGetOperand (inst, 0));
    vreg = get_registerArm32 (dest, NULL, dest->var->sdType->tdTypeKind == TYP_FLOAT ? VFP_REGS : GENERAL_REGS, FALSE);
    if  (!vreg && dest->var->sdType->tdTypeKind != TYP_VOID)
    {
        /* 如果目标操作数不在寄存器，为它新建一个。  */
        vreg = gen_vregArm32 (pmydata->virtual_regs, -1, dest->var->sdType->tdTypeKind == TYP_FLOAT ? VFP_REGS : GENERAL_REGS);
        register_descriptorArm32 (vreg, NULL, dest, NULL, FALSE);
        location_descriptorArm32 (dest, NULL, vreg, FALSE, FALSE);
    }

    /* 统计各类参数的个数。  */
    funcCallArgCnt = GetConstVal (IRInstGetOperand (inst, 2)->var, 0)->cvValue.cvIval;
    for(  curs=(IRInst *)InterCodeGetCursor (inst->bb->cfg->code, inst)
       ;  curs!=NULL
       ;  curs = (IRInst *)List_Prev((void *)curs)
       )
        if  ((*curs)->opcode == IRINST_OP_param)
        {
            if  (IRInstGetOperand (*curs, 0)->var->sdType->tdTypeKind == TYP_FLOAT)
                fcnt++;
            else
                icnt++;
            if  (fcnt + icnt >= funcCallArgCnt)
                break;
        }

    /* 添加一个标记，供寄存器分配使用。  */
    p = tree(pmydata->tree_nodes, STRING, NULL, NULL);
    p->operand.ds = dyn_string_new (32);
    dyn_string_insert_cstr (p->operand.ds, 0, "\t@ Prepare arguments for call.");
    commit (inst->bb, p, 1, pmydata);

    /* 产生放置参数的指令。  */
    for(  curs=(IRInst *)InterCodeGetCursor (inst->bb->cfg->code, inst)
       ;  curs!=NULL
       ;  curs = (IRInst *)List_Prev((void *)curs)
       )
        if  ((*curs)->opcode == IRINST_OP_param)
        {
            param = varpool_get_node(pmydata->set, IRInstGetOperand(*curs, 0));
            if  (param->var->sdType->tdTypeKind == TYP_FLOAT)
            {
                fcnt--;
                if  (fcnt < NUM_VFP_ARG_REGS)
                {
                    /* 通过寄存器传递参数。  */
                    p = param->var->sdVar.sdvConst ? load_immediate(inst->bb, NULL, GetConstVal(param->var, 0)->cvValue.cvIval, param->var->sdType->tdTypeKind, pmydata)
                                                                 : getreg (param, pmydata->reg_map);
                    if  (!p)
                        p = GetTreeNode (param, pmydata);
                    p = tree (pmydata->tree_nodes, VMOV, p, NULL);
                    p->operand.vreg = gen_vregArm32(pmydata->virtual_regs, VREG (fcnt), VFP_REGS);
                    commit (inst->bb, p, burmArm32_reg_NT, pmydata);
                }
                else
                {
                    /* 通过栈传递参数。  */
                    p = param->var->sdVar.sdvConst ? load_immediate(inst->bb, NULL, GetConstVal(param->var, 0)->cvValue.cvIval, param->var->sdType->tdTypeKind, pmydata)
                                                                 : getreg (param, pmydata->reg_map);
                    if  (!p)
                        p = GetTreeNode (param, pmydata);
                    tmp = tree (pmydata->tree_nodes, REGISTER, NULL, NULL);
                    tmp->operand.vreg = gen_vregArm32(pmydata->virtual_regs, SP_REGNUM, GENERAL_REGS);
                    p = store_base (inst->bb, p, tmp, (max(icnt - NUM_ARG_REGS, 0)+max(0, fcnt - NUM_VFP_ARG_REGS)) * UNITS_PER_WORD, pmydata);
                    commit (inst->bb, p, 1, pmydata);
                }
            }
            else
            {
                icnt--;
                if  (icnt < NUM_ARG_REGS)
                {
                    /* 通过寄存器传递参数。  */
                    if  (param->var->sdVar.sdvConst &&
                         param->var->sdType->tdTypeKind <= TYP_lastIntrins &&
                         is_imm16(GetConstVal(param->var, 0)->cvValue.cvIval))
                    {
                        p = tree (pmydata->tree_nodes, imm16, NULL, NULL);
                        p->operand.cval.cvValue.cvIval = GetConstVal (param->var, 0)->cvValue.cvIval;
                    }
                    else
                    {
                        p = param->var->sdVar.sdvConst ? load_immediate(inst->bb, NULL, GetConstVal(param->var, 0)->cvValue.cvIval, param->var->sdType->tdTypeKind, pmydata)
                                                                     : getreg (param, pmydata->reg_map);
                        if  (!p)
                            p = GetTreeNode (param, pmydata);
                    }
                    p = tree (pmydata->tree_nodes, MOV, p, NULL);
                    p->operand.vreg = gen_vregArm32(pmydata->virtual_regs, icnt, GENERAL_REGS);
                    commit (inst->bb, p, burmArm32_reg_NT, pmydata);
                }
                else
                {
                    /* 通过栈传递参数。  */
                    p = param->var->sdVar.sdvConst ? load_immediate(inst->bb, NULL, GetConstVal(param->var, 0)->cvValue.cvIval, param->var->sdType->tdTypeKind, pmydata)
                                                                 : getreg (param, pmydata->reg_map);
                    if  (!p)
                        p = GetTreeNode (param, pmydata);
                    tmp = tree (pmydata->tree_nodes, REGISTER, NULL, NULL);
                    tmp->operand.vreg = gen_vregArm32(pmydata->virtual_regs, SP_REGNUM, GENERAL_REGS);
                    p = store_base (inst->bb, p, tmp, (max(icnt - NUM_ARG_REGS, 0)+max(0, fcnt - NUM_VFP_ARG_REGS)) * UNITS_PER_WORD, pmydata);
                    commit (inst->bb, p, 1, pmydata);
                }
            }
            if  (fcnt + icnt == 0)
                break;
        }

    /* 产生函数调用指令。  */
    p = tree (pmydata->tree_nodes, LABELV, NULL, NULL);
    p->operand.sym = IRInstGetOperand (inst, 1)->var;
    p = tree (pmydata->tree_nodes, BL, p, NULL);
    commit (inst->bb, p, 1, pmydata);

    if  (IRInstGetOperand (inst, 0)->var->sdType->tdTypeKind == TYP_FLOAT)
    {
        p = tree (pmydata->tree_nodes, REGISTER, NULL, NULL);
        p->operand.vreg = gen_vregArm32 (pmydata->virtual_regs, R0_REGNUM + FIRST_VFP_REGNUM, VFP_REGS);
        p = tree(pmydata->tree_nodes, VMOV, p, NULL);
        p->operand.vreg = vreg;

        /* 必须立即进行指令选择，若进行统一指令选择可能导致r0被覆盖。  */
        commit (inst->bb, p, burmArm32_reg_NT, pmydata);

        p = tree (pmydata->tree_nodes, REGISTER, NULL, NULL);
        p->operand.vreg = vreg;
        setreg (dest, p, pmydata);
    }
    else if (IRInstGetOperand (inst, 0)->var->sdType->tdTypeKind != TYP_VOID)
    {
        p = tree (pmydata->tree_nodes, REGISTER, NULL, NULL);
        p->operand.vreg = gen_vregArm32 (pmydata->virtual_regs, R0_REGNUM, GENERAL_REGS);
        p = tree(pmydata->tree_nodes, MOV, p, NULL);
        p->operand.vreg = vreg;

        /* 必须立即进行指令选择，若进行统一指令选择可能导致r0被覆盖。  */
        commit (inst->bb, p, burmArm32_reg_NT, pmydata);

        p = tree (pmydata->tree_nodes, REGISTER, NULL, NULL);
        p->operand.vreg = vreg;
        setreg (dest, p, pmydata);
    }

    return TRUE;
}
static BOOL translate_param (IRInst inst, PINTERNAL_DATA pmydata) 
{
    /* do nothing */
    return TRUE;
}
static BOOL translate_fparam (IRInst inst, PINTERNAL_DATA pmydata) 
{
    /* do nothing */
    return TRUE;
}
static BOOL translate_entry (IRInst inst, PINTERNAL_DATA pmydata) 
{
    int icnt = 0;
    int fcnt = 0;
    IRInst *curs;
    NODEPTR_TYPE pivot; /* 帧指针，用于帮助定位放在栈上的参数。  */
    NODEPTR_TYPE p;
    NODEPTR_TYPE tmp;
    varpool_node param;
    vreg_tArm32 vreg;

    if  (pmydata->ddata)
        dwarfout_line (inst, pmydata);

    /* 将callee_save寄存器压栈。  */
    p = tree (pmydata->tree_nodes, PUSH, NULL, NULL);
    commit (inst->bb, p, 1, pmydata);

    p = tree (pmydata->tree_nodes, VPUSH, NULL, NULL);
    commit (inst->bb, p, 1, pmydata);

    /* 初始化帧指针。  */
    p = tree (pmydata->tree_nodes, STRING, NULL, NULL);
    p->operand.ds = dyn_string_new (32);
    dyn_string_insert_cstr (p->operand.ds, 0, "\t@ Set up the frame pointer.");
    commit (inst->bb, p, 1, pmydata);

    tmp = tree (pmydata->tree_nodes, simm8, NULL, NULL);
    tmp->operand.cval.cvValue.cvIval = 100;
    p = tree (pmydata->tree_nodes, REGISTER, NULL, NULL);
    p->operand.vreg = gen_vregArm32(pmydata->virtual_regs, SP_REGNUM, GENERAL_REGS);
    p = tree (pmydata->tree_nodes, ADD, p, tmp);
    p->operand.vreg = gen_vregArm32(pmydata->virtual_regs, FP_REGNUM, GENERAL_REGS);
    commit (inst->bb, p, burmArm32_reg_NT, pmydata);

    pivot = tree (pmydata->tree_nodes, REGISTER, NULL, NULL);
    pivot->operand.vreg = p->operand.vreg;

    /* 移动栈顶指针。  */
    p = tree (pmydata->tree_nodes, STRING, NULL, NULL);
    p->operand.ds = dyn_string_new (32);
    dyn_string_insert_cstr (p->operand.ds, 0, "\t@ Decrement the stack pointer.");
    commit (inst->bb, p, 1, pmydata);

    p = tree (pmydata->tree_nodes, simm8, NULL, NULL);
    p->operand.cval.cvValue.cvIval = 100;
    tmp = tree (pmydata->tree_nodes, REGISTER, NULL, NULL);
    tmp->operand.vreg = gen_vregArm32 (pmydata->virtual_regs, SP_REGNUM, GENERAL_REGS);
    p = tree (pmydata->tree_nodes, SUB, tmp, p);
    p->operand.vreg = gen_vregArm32 (pmydata->virtual_regs, SP_REGNUM, GENERAL_REGS);
    commit (inst->bb, p, burmArm32_reg_NT, pmydata);

    for(  curs=(IRInst *)InterCodeGetCursor (inst->bb->cfg->code, inst)
       ;  curs!=NULL
       ;  curs = (IRInst *)List_Next((void *)curs)
       )
    {
        if  ((*(IRInst *)curs)->opcode == IRINST_OP_fparam)
        {
            param = varpool_get_node (pmydata->set, IRInstGetOperand(*curs, 0));
            if  (param->var->sdType->tdTypeKind == TYP_FLOAT)
            {
                if  (fcnt < NUM_VFP_ARG_REGS)
                {
                    /* 参数在寄存器。  */
                    p = tree (pmydata->tree_nodes, REGISTER, NULL, NULL);
                    p->operand.vreg = gen_vregArm32(pmydata->virtual_regs, VREG (fcnt), VFP_REGS);
                    p = tree (pmydata->tree_nodes, VMOV, p, NULL);
                    p->operand.vreg = gen_vregArm32(pmydata->virtual_regs, -1, VFP_REGS);
                }
                else
                {
                    /* 参数在栈上。  */
                    p = load_base (inst->bb, NULL, TYP_FLOAT, pivot, UNITS_PER_WORD * (max(icnt - NUM_ARG_REGS, 0) + max(fcnt - NUM_VFP_ARG_REGS, 0)), pmydata);
                }
                fcnt++;
            }
            else
            {
                if  (icnt < NUM_ARG_REGS)
                {
                    /* 参数在寄存器。  */
                    p = tree (pmydata->tree_nodes, REGISTER, NULL, NULL);
                    p->operand.vreg = gen_vregArm32(pmydata->virtual_regs, icnt, GENERAL_REGS);
                    p = tree (pmydata->tree_nodes, MOV, p, NULL);
                    p->operand.vreg = gen_vregArm32(pmydata->virtual_regs, -1, GENERAL_REGS);
                }
                else
                {
                    /* 参数在栈上。  */
                    p = load_base (inst->bb, NULL, TYP_INT, pivot, UNITS_PER_WORD * (max(icnt - NUM_ARG_REGS, 0) + max(fcnt - NUM_VFP_ARG_REGS, 0)), pmydata);
                }
                icnt++;
            }

            commit (inst->bb, p, burmArm32_reg_NT, pmydata);
            vreg = p->operand.vreg;
            p = tree (pmydata->tree_nodes, REGISTER, NULL, NULL);
            p->operand.vreg = vreg;
            setreg (param, p, pmydata);

            register_descriptorArm32 (p->operand.vreg, NULL, param, NULL, param->var->sdType->tdTypeKind > TYP_lastIntrins);
            location_descriptorArm32 (param, NULL, NULL, TRUE, param->var->sdType->tdTypeKind > TYP_lastIntrins);
            location_descriptorArm32 (param, NULL, p->operand.vreg, FALSE, param->var->sdType->tdTypeKind > TYP_lastIntrins);
        }
    }

    load_global_address (inst->bb, pmydata);

    if  (pmydata->ddata)
        dwarfout_begin_function (inst, pmydata);

    return TRUE;
}
static BOOL translate_exit (IRInst inst, PINTERNAL_DATA pmydata) 
{
    NODEPTR_TYPE p;
    NODEPTR_TYPE vreg;
    varpool_node retval;

    if  (pmydata->ddata)
        dwarfout_end_function (inst, pmydata);

    if  (pmydata->ddata)
        dwarfout_line (inst, pmydata);

    /* 设置返回值。  */
    retval = varpool_get_node (pmydata->set, IRInstGetOperand(inst, 0));
    if      (retval->var->sdType->tdTypeKind == TYP_FLOAT)
    {
        p = retval->var->sdVar.sdvConst ? load_immediate(inst->bb, NULL, GetConstVal(retval->var, 0)->cvValue.cvIval, retval->var->sdType->tdTypeKind, pmydata)
                                                     : getreg (retval, pmydata->reg_map);
        if  (!p)
            p = GetTreeNode (retval, pmydata);
        p = tree (pmydata->tree_nodes, VMOV, p, NULL);
        p->operand.vreg = gen_vregArm32 (pmydata->virtual_regs, VREG (R0_REGNUM), VFP_REGS);

        commit (inst->bb, p, burmArm32_reg_NT, pmydata);
    }
    else if (retval->var->sdType->tdTypeKind != TYP_VOID)
    {
        p = retval->var->sdVar.sdvConst ? load_immediate(inst->bb, NULL, GetConstVal(retval->var, 0)->cvValue.cvIval, retval->var->sdType->tdTypeKind, pmydata)
                                                     : getreg (retval, pmydata->reg_map);
        if  (!p)
            p = GetTreeNode (retval, pmydata);
        p = tree (pmydata->tree_nodes, MOV, p, NULL);
        p->operand.vreg = gen_vregArm32 (pmydata->virtual_regs, R0_REGNUM, GENERAL_REGS);

        commit (inst->bb, p, burmArm32_reg_NT, pmydata);
    }

    /* 移动栈顶指针。  */
    p = tree (pmydata->tree_nodes, STRING, NULL, NULL);
    p->operand.ds = dyn_string_new (32);
    dyn_string_insert_cstr (p->operand.ds, 0, "\t@ Recover the stack pointer.");
    commit (inst->bb, p, 1, pmydata);

    p = tree (pmydata->tree_nodes, simm8, NULL, NULL);
    p->operand.cval.cvValue.cvIval = 100;
    vreg = tree (pmydata->tree_nodes, REGISTER, NULL, NULL);
    vreg->operand.vreg = gen_vregArm32 (pmydata->virtual_regs, SP_REGNUM, GENERAL_REGS);
    p = tree (pmydata->tree_nodes, ADD, vreg, p);
    p->operand.vreg = gen_vregArm32 (pmydata->virtual_regs, SP_REGNUM, GENERAL_REGS);
    commit (inst->bb, p, burmArm32_reg_NT, pmydata);

    /* 将callee_save寄存器弹出。  */
    p = tree (pmydata->tree_nodes, VPOP, NULL, NULL);
    commit (inst->bb, p, 1, pmydata);

    p = tree (pmydata->tree_nodes, POP, NULL, NULL);
    commit (inst->bb, p, 1, pmydata);

    /* 跳转。  */
    vreg = tree (pmydata->tree_nodes, REGISTER, NULL, NULL);
    vreg->operand.vreg = gen_vregArm32 (pmydata->virtual_regs, LR_REGNUM, GENERAL_REGS);
    p = tree (pmydata->tree_nodes, BX, vreg, NULL);
    commit (inst->bb, p, 1, pmydata);

    return TRUE;
}
static BOOL translate_phi (IRInst inst, PINTERNAL_DATA pmydata) 
{
    return TRUE;
}
/* Output a marker (i.e. a label) for the beginning of the generated code for
   a lexical block.  */
static BOOL translate_begin_block (IRInst inst, PINTERNAL_DATA pmydata) 
{
    char label[30];
    NODEPTR_TYPE p;

    if  (pmydata->ddata)
    {
        sprintf (label, ".L_B%u:", GetConstVal(IRInstGetOperand(inst, 0)->var, 0)->cvValue.cvIval);
        p = tree(pmydata->tree_nodes, STRING, NULL, NULL);
        p->operand.ds = dyn_string_new (3);
        dyn_string_insert_cstr (p->operand.ds, 0, label);

        commit (inst->bb, p, 1, pmydata);
    }

    return TRUE;
}
/* Output a marker (i.e. a label) for the end of the generated code for a
   lexical block.  */
static BOOL translate_end_block (IRInst inst, PINTERNAL_DATA pmydata) 
{
    char label[30];
    NODEPTR_TYPE p;

    if  (pmydata->ddata)
    {
        sprintf (label, ".L_B%u_e:", GetConstVal(IRInstGetOperand(inst, 0)->var, 0)->cvValue.cvIval);
        p = tree(pmydata->tree_nodes, STRING, NULL, NULL);
        p->operand.ds = dyn_string_new (3);
        dyn_string_insert_cstr (p->operand.ds, 0, label);

        commit (inst->bb, p, 1, pmydata);
    }

    return TRUE;
}
static BOOL translate (IRInst inst, PINTERNAL_DATA pmydata) 
{
    switch (inst->opcode)
    {
#define OPDEF(OPCODE, OPNAME, name_zenglj, OPKIND, operand_count) \
    case IRINST_OP_ ## OPCODE: return translate_ ## OPCODE (inst, pmydata);
#include "IROpcode.h"
#undef  OPDEF
    default:
        return FALSE;
    }
}

static BOOL
find_if_block (basic_block test_bb)
{
    basic_block then_bb;
    basic_block else_bb;
    basic_block join_bb = NULL;
    edge then_succ;
    ArmInst instr;
    ArmInst jump_insn;
    ArmInst new_insn;
    edge *ei;
    BOOL bswap = FALSE;

    if  (List_Card (test_bb->succs) != 2)
        return FALSE;

    for(  instr=(ArmInst) List_Last(((BblockArm32)test_bb->param)->code)
       ;  instr!=NULL
       ;  instr = (ArmInst) List_Prev((void *)instr)
       )
    {
        if  (instr->opcode == ARMINST_OP_STRING &&
             !strcmp (dyn_string_buf (ArmInstGetOperand (instr, 0)->ds), "\t@ Jump to the target."))
            return FALSE;
        if  (instr->condition != ConditionAL)
            break;
    }

    then_bb = InterCodeGetInstByID (test_bb->cfg->code, GetConstVal (IRInstGetOperand (*(IRInst *)List_Last (test_bb->insns), 2)->var, 0)->cvValue.cvIval)->bb;
    else_bb = InterCodeGetInstByID (test_bb->cfg->code, GetConstVal (IRInstGetOperand (*(IRInst *)List_Last (test_bb->insns), 3)->var, 0)->cvValue.cvIval)->bb;

    if  (GetConstVal (IRInstGetOperand (*(IRInst *)List_Last (test_bb->insns), 2)->var, 0)->cvValue.cvIval != ArmInstGetOperand(instr, 0)->cval.cvValue.cvIval)
        bswap = TRUE;

    /* IF-THEN组合的THEN块必须只有一个前驱。  */
    if  (List_Card (then_bb->preds) != 1)
        return FALSE;

    /* IF-THEN组合的THEN块必须只有一个后继。  */
    if  (List_Card (then_bb->succs) != 1)
        return FALSE;
    then_succ = *(edge *) List_First(then_bb->succs);

    if (then_succ->dest == else_bb)
    {
        join_bb = else_bb;
        else_bb = NULL;
    }
    else if (List_Card(else_bb->succs) == 1
            && then_succ->dest == (*(edge *)List_First (else_bb->succs))->dest
            && List_Card(else_bb->preds) == 1)
        join_bb = (*(edge *)List_First (else_bb->succs))->dest;
    else
        return FALSE;	

    /* 查找是否已经用了条件指令。  */
    for(  instr=(ArmInst) List_First(((BblockArm32)then_bb->param)->code)
       ;  instr!=NULL
       ;  instr = (ArmInst) List_Next((void *)instr)
       )
    {
        if  (instr->opcode == ARMINST_OP_STRING &&
             !strcmp (dyn_string_buf (ArmInstGetOperand (instr, 0)->ds), "\t@ Jump to the target."))
            break;
        if  (instr->condition != ConditionAL ||
             ArmInst_is_call (instr))
            return FALSE;
    }
    if  (else_bb)
    {
        for(  instr=(ArmInst) List_First(((BblockArm32)else_bb->param)->code)
           ;  instr!=NULL
           ;  instr = (ArmInst) List_Next((void *)instr)
           )
        {
            if  (instr->opcode == ARMINST_OP_STRING &&
                 !strcmp (dyn_string_buf (ArmInstGetOperand (instr, 0)->ds), "\t@ Jump to the target."))
                break;
            if  (instr->condition != ConditionAL ||
                 ArmInst_is_call (instr))
                return FALSE;
        }
    }

    /* 找到要插入的位置。  */
    for(  jump_insn=(ArmInst) List_Last(((BblockArm32)test_bb->param)->code)
       ;  jump_insn!=NULL
       ;  jump_insn = (ArmInst) List_Prev((void *)jump_insn)
       )
    {
        if  (jump_insn->opcode == ARMINST_OP_STRING &&
             !strcmp (dyn_string_buf (ArmInstGetOperand (jump_insn, 0)->ds), "\t@ Jump to the target."))
            return FALSE;
        if  (jump_insn->condition != ConditionAL)
            break;
    }

    if  (bswap)
    {
        switch (jump_insn->condition)
        {
        case ConditionGE: jump_insn->condition = ConditionLT; break;
        case ConditionGT: jump_insn->condition = ConditionLE; break;
        case ConditionLE: jump_insn->condition = ConditionGT; break;
        case ConditionLT: jump_insn->condition = ConditionGE; break;
        case ConditionNE: jump_insn->condition = ConditionEQ; break;
        case ConditionEQ: jump_insn->condition = ConditionNE; break;
        default: fatal("internal compiler error");
        }
    }

    for(  instr=(ArmInst) List_Prev((void *)jump_insn)
       ;  instr!=NULL
       ;  instr = (ArmInst) List_Prev((void *)instr)
       )
    {
        if  (instr->opcode == ARMINST_OP_STRING &&
             !strcmp (dyn_string_buf (ArmInstGetOperand (instr, 0)->ds), "\t@ Jump to the target."))
        {
            deleteArmInst (instr);
            break;
        }
    }

    for(  instr=(ArmInst) List_First(((BblockArm32)then_bb->param)->code)
       ;  instr!=NULL
       ;  instr = (ArmInst) List_First(((BblockArm32)then_bb->param)->code)
       )
    {
        if  (instr->opcode == ARMINST_OP_STRING &&
             !strcmp (dyn_string_buf (ArmInstGetOperand (instr, 0)->ds), "\t@ Jump to the target."))
            break;
        new_insn = (ArmInst) List_NewBefore (((BblockArm32)test_bb->param)->code, jump_insn, sizeof (*new_insn));
        memcpy (new_insn, instr, sizeof (*new_insn));
        new_insn->condition = jump_insn->condition;
        List_Delete (instr);
    }

    if  (else_bb)
    {
        switch (jump_insn->condition)
        {
        case ConditionGE: jump_insn->condition = ConditionLT; break;
        case ConditionGT: jump_insn->condition = ConditionLE; break;
        case ConditionLE: jump_insn->condition = ConditionGT; break;
        case ConditionLT: jump_insn->condition = ConditionGE; break;
        case ConditionNE: jump_insn->condition = ConditionEQ; break;
        case ConditionEQ: jump_insn->condition = ConditionNE; break;
        default: fatal("internal compiler error");
        }

        for(  instr=(ArmInst) List_First(((BblockArm32)else_bb->param)->code)
           ;  instr!=NULL
           ;  instr = (ArmInst) List_First(((BblockArm32)else_bb->param)->code)
           )
        {
            if  (instr->opcode == ARMINST_OP_STRING &&
                 !strcmp (dyn_string_buf (ArmInstGetOperand (instr, 0)->ds), "\t@ Jump to the target."))
                break;
            new_insn = (ArmInst) List_NewBefore (((BblockArm32)test_bb->param)->code, jump_insn, sizeof (*new_insn));
            memcpy (new_insn, instr, sizeof (*new_insn));
            new_insn->condition = jump_insn->condition;
            List_Delete (instr);
        }
    }

    for(  instr=(ArmInst) List_Next((void *)jump_insn)
       ;  instr!=NULL
       ;  instr = (ArmInst) List_Next((void *)jump_insn)
       )
        deleteArmInst (instr);
    jump_insn->condition = ConditionAL;

    for(  ei=(edge *) List_First(then_bb->preds)
       ;  ei!=NULL
       ;  ei = (edge *) List_First(then_bb->preds)
       )
        remove_edge (*ei);
    for(  ei=(edge *) List_First(then_bb->succs)
       ;  ei!=NULL
       ;  ei = (edge *) List_First(then_bb->succs)
       )
        remove_edge (*ei);
    if  (else_bb)
    {
        for(  ei=(edge *) List_First(else_bb->preds)
           ;  ei!=NULL
           ;  ei = (edge *) List_First(else_bb->preds)
           )
            remove_edge (*ei);
        for(  ei=(edge *) List_First(else_bb->succs)
           ;  ei!=NULL
           ;  ei = (edge *) List_First(else_bb->succs)
           )
            remove_edge (*ei);
    }

    make_edge (test_bb, join_bb);
    ArmInstGetOperand (jump_insn, 0)->cval.cvValue.cvIval = (*(IRInst *) List_First(join_bb->insns))->uid;

    return TRUE;   
}

/* if-conversion.  */
void
if_convertArm32 (control_flow_graph func)
{
    basic_block *block;
    LIST blocks;

    blocks = List_Create ();
    pre_and_rev_post_order_compute (func, NULL, blocks, TRUE, FALSE);

    for(  block=(basic_block *) List_Last(blocks)
       ;  block!=NULL
       ;  block = (basic_block *) List_Prev((void *)block)
       )
        find_if_block (*block);

    List_Destroy(&blocks);
}

BOOL InstSelectorArm32 (control_flow_graph func, varpool_node_set set, SymTab stab, struct avl_table *virtual_regs, struct dwarf_data *ddata) 
{
    BOOL bSuccess = FALSE;
    PINTERNAL_DATA pmydata;
    basic_block *block;
    LIST blocks;
    NODEPTR_TYPE label;
    IRInst *instr;
    PTREEINFO pti;

    /* 初始化内存池，供iburg产生的程序使用。  */
    mempool = List_Create ();

    /* 创建内部数据。  */
    pmydata = (PINTERNAL_DATA) xmalloc (sizeof (*pmydata));

    /* 初始化。  */
    pmydata->set = set;
    pmydata->stab = stab;
    pmydata->tree_nodes = List_Create ();
    pmydata->trees = List_Create ();
    pmydata->_constants = List_Create ();
    pmydata->reg_map = avl_create ((avl_comparison_func *)compare, NULL, NULL);
    pmydata->node_map = avl_create ((avl_comparison_func *)compare_tree, NULL, NULL);
    pmydata->func = func;
    pmydata->virtual_regs = virtual_regs;
    pmydata->ddata = ddata;

    compute_dominators (func, FALSE);

    blocks = List_Create ();
    pre_and_rev_post_order_compute (func, NULL, blocks, TRUE, FALSE);

    for(  block=(basic_block *) List_First(blocks)
       ;  block!=NULL
       ;  block = (basic_block *) List_Next((void *)block)
       )
    {
        ((struct BblockArm32 *) (*block)->param)->next_block = (basic_block *) List_Next((void *)block) ? *(basic_block *) List_Next((void *)block) : NULL;

        /* 在基本块入口添加标签。  */
        if  ((*(IRInst *)List_First((*block)->insns))->opcode != IRINST_OP_entry)
        {
            label = tree (pmydata->tree_nodes, LABEL, NULL, NULL);
            label->operand.cval.cvValue.cvIval = (*(IRInst *)List_First((*block)->insns))->uid;
            commit (*block, label, 1, pmydata);
        }

        /* 清空常量池。  */
        List_Clear (pmydata->_constants);

        /* 翻译每条指令。  */
        for(  instr=(IRInst *) List_First((*block)->insns)
           ;  instr!=NULL
           ;  instr = (IRInst *) List_Next((void *)instr)
           )
            if  (!translate (*instr, pmydata))
                goto fail;
    }

    /* 指令选择，同时处理有向无环图的情况。  */
    for(  pti=(PTREEINFO) List_First(pmydata->trees)
       ;  pti!=NULL
       ;  pti = (PTREEINFO) List_Next((void *)pti)
       )
    {
        burmArm32_label (pti->p);
#if !defined(NDEBUG)
/*      dumpCover (pti->p, pti->goalnt, 0); */
#endif
        burmArm32_select (pti->block, pti->p, pti->goalnt, pmydata->virtual_regs);
    }

    bSuccess = TRUE;
fail:
    /* 执行清理。  */
    List_Destroy (&blocks);
    List_Destroy (&mempool);
    List_Destroy (&pmydata->tree_nodes);
    List_Destroy (&pmydata->trees);
    List_Destroy (&pmydata->_constants);
    avl_destroy (pmydata->reg_map, (avl_item_func *) free);
    avl_destroy (pmydata->node_map, (avl_item_func *) NULL);

    free (pmydata);
    return bSuccess; 
}
