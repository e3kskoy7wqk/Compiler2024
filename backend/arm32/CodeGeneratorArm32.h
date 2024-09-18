
#ifndef MR
#error  Define "MR" to an empty string if using managed data, "*" otherwise.
#endif

/*---------------------------------------------------------------------------*/

struct ArmInst;
typedef struct ArmInst MR ArmInst;

struct tree;
typedef struct tree MR NODEPTR_TYPE;

/*---------------------------------------------------------------------------*/

#define R0_REGNUM         0     /* First CORE register  */
#define FP_REGNUM                   11
#define IP_REGNUM        12     /* Scratch register  */
#define SP_REGNUM        13     /* Stack pointer  */
#define LR_REGNUM        14     /* Return address register  */
#define PC_REGNUM        15     /* Program counter  */

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

#define STATE_TYPE void*
#define OP_LABEL(p) ((p)->op)
#define LEFT_CHILD(p) ((p)->kids[0])
#define RIGHT_CHILD(p) ((p)->kids[1])
#define STATE_LABEL(p) ((p)->state_label)
#define PANIC printf

/* Note: 需要跟arm32.c中的值保持一致。  */
#define burmArm32_reg_NT 2

/* Register classes.  */
enum reg_classArm32
{
    NO_REGS,
    GENERAL_REGS,
    VFP_REGS,
    ALL_REGS,
    LIM_REG_CLASSES
};

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
    bitmap D;
};

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

struct tree {
    int op;
    struct tree *kids[2];
    STATE_TYPE state_label;
    int id;
    int ref_counter;
    union LIR_Opr operand;
    Condition condition;
    int goalnt;
    int label;
} ;

struct ArmInst {
    struct LIR_Op lir;
    Condition condition;
};

extern int burmArm32_max_nt;
extern short *burmArm32_nts[] ;
extern char *burmArm32_string[] ;

/* procs in CodeGeneratorArm32.c */
vreg_t get_registerArm32 (varpool_node base, varpool_node offset, enum reg_classArm32 rclass, BOOL is_addr);
void location_descriptorArm32 (varpool_node base, varpool_node offset, vreg_t vreg, BOOL reset, BOOL is_addr);
void output_reg (FILE *file, vreg_t reg);
vreg_t gen_vregArm32 (struct avl_table *virtual_regs, int vregno, enum reg_classArm32 rclass);
void regdescArm32 (vreg_t vreg, vreg_t from, varpool_node base, varpool_node offset, BOOL is_addr);
struct descriptor_s *get_locationArm32(vreg_t vreg);

/* procs in arm32.c */
int burmArm32_rule(STATE_TYPE state, int goalnt) ;
STATE_TYPE burmArm32_state(int op, STATE_TYPE left, STATE_TYPE right) ;
STATE_TYPE burmArm32_label(NODEPTR_TYPE p) ;
NODEPTR_TYPE *burmArm32_kids(NODEPTR_TYPE p, int eruleno, NODEPTR_TYPE kids[]) ;
NODEPTR_TYPE burmArm32_child(NODEPTR_TYPE p, int index) ;

/* procs in InstSelectorArm32.c */
BOOL InstSelectorArm32 (control_flow_graph func, varpool_node_set set, SymTab stab, struct avl_table *virtual_regs, struct Backend* backend, struct dwarf_data *ddata) ;
void spill_inArm32 (ArmInst insn, vreg_t spill_reg, int slot, struct avl_table *virtual_regs);
void spill_outArm32 (ArmInst insn, vreg_t spill_reg, int slot, struct avl_table *virtual_regs);
void update_frame_layoutArm32 (control_flow_graph func, struct avl_table *virtual_regs, struct Backend* backend);
void if_convertArm32 (control_flow_graph func);

/* procs in ILocArm32.c */
ArmInst emit_ArmInst (basic_block bb, enum ArmInstOperator opcode);
void delete_ArmInst (ArmInst insn);
ArmInst emit_ArmInst_before (ArmInst before, enum ArmInstOperator opcode);
ArmInst emit_ArmInst_after (ArmInst after, enum ArmInstOperator opcode);
LIR_Opr ArmInst_get_operand (ArmInst insn, int index);
vreg_t ArmInst_get_as_Register (ArmInst insn, int index);
void ArmInst_set_operand (ArmInst insn, int index, LIR_Opr operand);
const char *ArmInstGetOperatorSpelling(enum ArmInstOperator Operator);
const char *ArmInstGetTypeString(enum ArmInstOperator Operator);
int ArmInstGetNumOperands(ArmInst insn);
BOOL ArmInstIsOutput (ArmInst inst, int index);
void ArmInstOutput (ArmInst inst, bitmap bmp);
void ArmInstInput (ArmInst inst, bitmap bmp);
void outputArmInst (ArmInst inst, FILE *file);
void outputArmCode (control_flow_graph fn, FILE *file);
BOOL ArmInst_is_call (ArmInst inst);
BOOL ArmInst_is_move (ArmInst instr);
