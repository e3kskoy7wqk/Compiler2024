#ifndef MR
#error  Define "MR" to an empty string if using managed data, "*" otherwise.
#endif

/*---------------------------------------------------------------------------*/

struct ArmInst;
typedef struct ArmInst MR ArmInst;

union Arm_Operand;
typedef union Arm_Operand MR Arm_Operand;

struct BfunctionArm32;
typedef struct BfunctionArm32 MR BfunctionArm32;

struct BblockArm32;
typedef struct BblockArm32 MR BblockArm32;

struct vreg_tArm32;
typedef struct vreg_tArm32 MR vreg_tArm32;

/*---------------------------------------------------------------------------*/

#define R0_REGNUM         0 /* First CORE register  */
#define R1_REGNUM         1 /* Second CORE register  */
#define FP_REGNUM                   11
#define IP_REGNUM        12 /* Scratch register  */
#define SP_REGNUM        13 /* Stack pointer  */
#define LR_REGNUM        14 /* Return address register  */
#define PC_REGNUM        15 /* Program counter  */

#define SPILL_REG        IP_REGNUM

#define LAST_ARM_REGNUM  15

#define FIRST_VFP_REGNUM 16

#define LAST_VFP_REGNUM \
  (FIRST_VFP_REGNUM + 31)

#define IS_VFP_REGNUM(REGNUM) \
  (((REGNUM) >= FIRST_VFP_REGNUM) && ((REGNUM) <= LAST_VFP_REGNUM))

#define VREG(X)  (FIRST_VFP_REGNUM + (X))

#define FIRST_VIRTUAL_REGISTER  (LAST_VFP_REGNUM + 1)

/* The number of (integer) argument register available.  */
#define NUM_ARG_REGS        4

/* And similarly for the VFP.  */
#define NUM_VFP_ARG_REGS    16

#define UNITS_PER_WORD  4

/* Register classes.  */
enum reg_classArm32
{
    NO_REGS,
    GENERAL_REGS,
    VFP_REGS,
    ALL_REGS,
    LIM_REG_CLASSES
};

struct vreg_tArm32 {
    int vregno;
    int hard_num;
    int spill_slot;

    /* 寄存器描述符。  */
    enum reg_classArm32 rclass;
    LIST base_offset;           /* pointer to struct base_offset */
};

struct locationArm32
{
    varpool_node offset;
    BOOL is_reg;
    union
    {
        vreg_tArm32 vreg;
        varpool_node base;
    };
};


/* Note: 需要跟arm32.c中的值保持一致。  */
#define burmArm32_reg_NT 2

/* Note: 需要跟arm32.brg中的值保持一致。  */
enum    ArmInstOperator
{
    #define OPDEF(OPNAME, OPCODE, name_string, type, operand_count) ARMINST_ ## OPNAME = OPCODE,
    #include "oparm32.h"
    #undef  OPDEF
};

typedef enum {
    ConditionAL,
    ConditionEQ,
    ConditionNE,
    ConditionHS, ConditionCS = ConditionHS,
    ConditionLO, ConditionCC = ConditionLO,
    ConditionMI,
    ConditionPL,
    ConditionVS,
    ConditionVC,
    ConditionHI,
    ConditionLS,
    ConditionGE,
    ConditionLT,
    ConditionGT,
    ConditionLE,
    ConditionInvalid
} Condition;

struct ArmInst {
    /* 结点的唯一标识符。  */
    int uid;

    Condition condition;

    /* 操作数代码。用于区分不同类型的指令。  */
    enum ArmInstOperator opcode;

    /* 所属基本块。  */
    basic_block bb;

    /* 各个操作数。  */
    LIST operands;     /* contains Arm_Operand */
};

union Arm_Operand {
    vreg_tArm32 vreg;
    SymDef sym;
    struct constVal cval;
    dyn_string_t ds;
};

struct tree {
    int op;
    struct tree *kids[2];
    STATE_TYPE state_label;
    int id;
    int ref_counter;
    union Arm_Operand operand;
    Condition condition;
} ;

struct BfunctionArm32
{
    int NumSpills;
    bitmap callee_saved_reg;     /* 供dwarfout使用。 */
};

struct BblockArm32
{
    LIST code;     /* contains struct ArmInst */
    bitmap liveout, livein;
    basic_block next_block;
};


extern int burmArm32_max_nt;
extern short *burmArm32_nts[] ;
extern char *burmArm32_string[] ;

/* procs in arm32.c */
int burmArm32_rule(STATE_TYPE state, int goalnt) ;
STATE_TYPE burmArm32_state(int op, STATE_TYPE left, STATE_TYPE right) ;
STATE_TYPE burmArm32_label(NODEPTR_TYPE p) ;
NODEPTR_TYPE *burmArm32_kids(NODEPTR_TYPE p, int eruleno, NODEPTR_TYPE kids[]) ;
NODEPTR_TYPE burmArm32_child(NODEPTR_TYPE p, int index) ;

/* procs in InstSelectorArm32.c */
BOOL InstSelectorArm32 (control_flow_graph func, varpool_node_set set, SymTab stab, struct avl_table *virtual_regs, struct dwarf_data *ddata) ;
void spill_inArm32 (ArmInst insn, vreg_tArm32 spill_reg, int slot, struct avl_table *virtual_regs);
void spill_outArm32 (ArmInst insn, vreg_tArm32 spill_reg, int slot, struct avl_table *virtual_regs);
void update_frame_layoutArm32 (control_flow_graph func, struct avl_table *virtual_regs);
void if_convertArm32 (control_flow_graph func);

/* procs in CodeGeneratorArm32.c */
BOOL CodeGeneratorArm32(InterCode code, SymTab stab, const char *name, FILE *file, struct dwarf_data *ddata) ;
vreg_tArm32 gen_vregArm32 (struct avl_table *virtual_regs, int vregno, enum reg_classArm32 rclass);
void register_descriptorArm32 (vreg_tArm32 vreg, vreg_tArm32 from, varpool_node base, varpool_node offset, BOOL is_addr);
vreg_tArm32 get_registerArm32 (varpool_node base, varpool_node offset, enum reg_classArm32 rclass, BOOL is_addr);
void location_descriptorArm32 (varpool_node base, varpool_node offset, vreg_tArm32 vreg, BOOL reset, BOOL is_addr);
BOOL is_virtual_registerArm32 (int num);
void output_reg (FILE *file, vreg_tArm32 reg);
void remove_locationArm32 (vreg_tArm32 vreg);
struct base_offset *get_locationArm32(vreg_tArm32 vreg);

/* procs in ArmInst.c */
ArmInst emitArmInst (basic_block bb, enum ArmInstOperator opcode);
void deleteArmInst(ArmInst Cursor);
const char *ArmInstGetOperatorSpelling(enum ArmInstOperator Operator);
void outputArmInst (ArmInst inst, FILE *file);
Arm_Operand ArmInstGetOperand(ArmInst insn, int index);
void ArmInstSetOperand(ArmInst insn, int index, union Arm_Operand operand);
void outputArmCode (control_flow_graph fn, FILE *file);
int ArmInstGetNumOperands(ArmInst insn);
BOOL ArmInstIsOutput (ArmInst inst, int index);
void ArmInstOutput (ArmInst inst, bitmap bmp);
void ArmInstInput (ArmInst inst, bitmap bmp);
ArmInst ArmInstInsertAfter (ArmInst insn, enum ArmInstOperator opcode);
ArmInst ArmInstInsertBefore (ArmInst insn, enum ArmInstOperator opcode);
vreg_tArm32 ArmInstGetOperandAsReg(ArmInst inst, int index);
BOOL ArmInst_is_call (ArmInst inst);
const char *ArmInstGetTypeString(enum ArmInstOperator Operator);
BOOL ArmInst_is_move (ArmInst instr);

/* procs in linearscan.c */
void LinearScanAllocator (control_flow_graph fn, struct avl_table *virtual_regs);
void regallocArm32 (control_flow_graph fn, struct avl_table *virtual_regs);
