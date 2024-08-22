/*****************************************************************************/
#ifndef _COMP_H_
#define _COMP_H_
/*****************************************************************************/

#if defined (_WIN32) && !defined(NDEBUG)
#include <crtdbg.h>
#endif

#if !defined(_WIN32) && !defined(NDEBUG)
#include <mcheck.h>
#endif

#include "avl.h"
#include "bitmap.h"
#include "list.h"
#include "dyn-string.h"


/* Flags */
/* 支持翻译成DragonIR。  */
#undef zenglj
#define zenglj

#undef DWARF_DEBUGGING_INFO
#define DWARF_DEBUGGING_INFO


/* Typedefs */
#define MR *

#ifndef MR
#error  Define "MR" to an empty string if using managed data, "*" otherwise.
#endif

/*---------------------------------------------------------------------------*/

#ifndef TRUE
#define FALSE          0
#define TRUE           !FALSE
typedef int BOOL;
#endif /* ! TRUE */

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

/*****************************************************************************/

struct   compiler;
typedef struct compiler     MR Compiler;

/*---------------------------------------------------------------------------*/

struct   symTab;
typedef struct symTab       MR SymTab;

struct   SymDefRec;
typedef struct SymDefRec    MR SymDef;

struct   TypDefRec;
typedef struct TypDefRec    MR TypDef;

struct   DimDefRec;
typedef struct DimDefRec    MR DimDef;

/*---------------------------------------------------------------------------*/

struct   constVal;
typedef struct constVal     MR ConstVal;

/*---------------------------------------------------------------------------*/

struct   TreeNode;
typedef struct TreeNode     MR Tree;

/*---------------------------------------------------------------------------*/

struct InterCode;
typedef struct InterCode MR InterCode;

struct IRInst;
typedef struct IRInst MR IRInst;

struct ssa_name;
typedef struct ssa_name MR ssa_name;

struct varpool_node_set_def;
typedef struct varpool_node_set_def MR varpool_node_set;

struct varpool_node;
typedef struct varpool_node MR varpool_node;

/*---------------------------------------------------------------------------*/

struct basic_block_def;
typedef struct basic_block_def MR basic_block;

struct control_flow_graph;
typedef struct control_flow_graph MR control_flow_graph;

struct edge_def;
typedef struct edge_def MR edge;

/*---------------------------------------------------------------------------*/

struct tree;
typedef struct tree MR NODEPTR_TYPE;

/*---------------------------------------------------------------------------*/

/* Various DIE's use offsets relative to the beginning of the
   .debug_info section to refer to each other.  */
typedef long int dw_offset;

/* Define typedefs here to avoid circular dependencies.  */
typedef struct die_struct *dw_die_ref;
typedef struct dw_attr_struct *dw_attr_ref;
typedef struct dw_val_struct *dw_val_ref;
typedef struct dw_line_info_struct *dw_line_info_ref;
typedef struct dw_loc_descr_struct *dw_loc_descr_ref;
typedef struct dw_cfi_struct *dw_cfi_ref;
typedef struct dw_fde_struct *dw_fde_ref;
typedef union  dw_cfi_oprnd_struct *dw_cfi_oprnd_ref;
typedef struct pubname_struct *pubname_ref;
typedef struct dw_loc_list_struct *dw_loc_list_ref;

/*---------------------------------------------------------------------------*/


enum treeOps
{
    #define TREEOP(en,tk,sn,IL,pr,ok) en,
    #include "toplist.h"

    TN_COUNT
};


enum    var_types_classification
{
    VTF_ANY = 0x0000,
    VTF_INT = 0x0001,
    VTF_UNS = 0x0002,
    VTF_FLT = 0x0004,
    VTF_SCL = 0x0008,
    VTF_ARI = 0x0010,
    VTF_IND = 0x0080,
};

#ifndef __SMC__

enum    var_types
{
    #define DEF_TP(tn,sz,al,nm,nm_zenglj,tf) TYP_##tn,
    #include "typelist.h"
    #undef  DEF_TP

    TYP_COUNT,

    TYP_lastIntrins = TYP_DOUBLE
};

#endif

#ifdef  FAST
typedef unsigned    varType_t;
#else
typedef enum var_types   varType_t;
#endif

/*****************************************************************************/

#ifndef __SMC__
extern  unsigned char        varTypeClassification[TYP_COUNT];
#endif

static  BOOL        varTypeIsIntegral  (enum var_types vt)
{
    return  (BOOL)((varTypeClassification[vt] & (VTF_INT)) != 0);
}

static  BOOL        varTypeIsIntArith  (enum var_types vt)
{
    return  (BOOL)((varTypeClassification[vt] & (VTF_INT|VTF_ARI)) == (VTF_INT|VTF_ARI));
}

static  BOOL        varTypeIsUnsigned  (enum var_types vt)
{
    return  (BOOL)((varTypeClassification[vt] & (VTF_UNS)) != 0);
}

static  BOOL        varTypeIsFloating  (enum var_types vt)
{
    return  (BOOL)((varTypeClassification[vt] & (VTF_FLT)) != 0);
}

static  BOOL        varTypeIsArithmetic(enum var_types vt)
{
    return  (BOOL)((varTypeClassification[vt] & (VTF_ARI)) != 0);
}

static  BOOL        varTypeIsScalar    (enum var_types vt)
{
    return  (BOOL)((varTypeClassification[vt] & (VTF_SCL)) != 0);
}

static  BOOL        varTypeIsSclOrFP   (enum var_types vt)
{
    return  (BOOL)((varTypeClassification[vt] & (VTF_SCL|VTF_FLT)) != 0);
}

static  BOOL        varTypeIsIndirect  (enum var_types vt)
{
    return  (BOOL)((varTypeClassification[vt] & (VTF_IND)) != 0);
}


/*****************************************************************************
 *
 *  下面的枚举定义了一组位标志，用于对表达式树节点进行分类。请注
 *  意，某些运算符可能会设置多个位，如下所示：
 *
 *          TNK_CONST    意味着    TNK_LEAF
 *          TNK_RELOP    意味着    TNK_BINOP
 *          TNK_LOGOP    意味着    TNK_BINOP
 */

enum genTreeKinds
{
    TNK_NONE        = 0x0000,   // 未分类的运算符

    TNK_CONST       = 0x0001,   // 常量运算符
    TNK_LEAF        = 0x0002,   // 叶子运算符
    TNK_UNOP        = 0x0004,   // 一元运算符
    TNK_BINOP       = 0x0008,   // 二元运算符
    TNK_RELOP       = 0x0010,   // 关系运算符
    TNK_LOGOP       = 0x0020,   // 逻辑运算符
    TNK_ASGOP       = 0x0040,   // 赋值运算符

    /* 定义复合值 */

    TNK_SMPOP       = TNK_UNOP|TNK_BINOP|TNK_RELOP|TNK_LOGOP
};

/*****************************************************************************
 *
 *  下面是用于 TreeNode 的 'tnFlags' 字段的值。
 *
 *  第一组标志可以与大量节点一起使用，因此它们的值需要是唯一的。
 *  也就是说，可以对任何表达式节点进行安全地测试其中的一个标志。
 */

enum treeNodeFlags
{
    TNF_BOUND       = 0x0001,   // 绑定（'compiled'）节点
    TNF_LVALUE      = 0x0002,   // 树是一个左值
    TNF_NOT_USER    = 0x0004,   // 编译器添加的代码
    TNF_ASG_DEST    = 0x0008,   // 表达式是赋值目标
    TNF_BEEN_CAST   = 0x0010,   // 此值已被强制转换
    TNF_COND_TRUE   = 0x0020,   // 条件已知为 true
    TNF_PAREN       = 0x0040,   // 表达式已显式括起来

    //---------------------------------------------------------------------
    //  剩余的标志只能与一个或少数几个节点一起使用，因此它们的值
    //  不需要是唯一的（当然，除了与特定节点一起使用的集合内部）。也就是
    //  说，只有在测试节点类型以确保它是特定标志所需的正确类型时，测试其
    //  中一个标志才有意义。
    //---------------------------------------------------------------------

    TNF_IF_HASELSE  = 0x8000,   // TN_IF        是否存在“else”部分？

    TNF_LCL_BASE    = 0x8000,   // TN_LCL_SYM   这是“baseclass”引用

    TNF_VAR_ARG     = 0x8000,   // TN_VAR_DECL  这是参数声明
    TNF_VAR_INIT    = 0x4000,   // TN_VAR_DECL  存在初始化器
    TNF_VAR_STATIC  = 0x2000,   // TN_VAR_DECL  变量是静态的
    TNF_VAR_CONST   = 0x1000,   // TN_VAR_DECL  变量是常量
    TNF_VAR_SEALED  = 0x0800,   // TN_VAR_DECL  变量是只读的
    TNF_VAR_UNREAL  = 0x0400,   // TN_VAR_DECL  变量是编译器生成的

    TNF_ADR_IMPLICIT= 0x8000,   // TN_ADDROF    数组/函数的自动“&”
    TNF_ADR_OUTARG  = 0x4000,   // TN_ADDROF    这是一个“out”参数

    TNF_EXP_CAST    = 0x8000,   // TN_CAST      显式强制转换？
    TNF_CHK_CAST    = 0x4000,   // TN_CAST      需要检查的强制转换？
    TNF_CTX_CAST    = 0x2000,   // TN_CAST      上下文包装/解包强制转换？

    TNF_STR_ASCII   = 0x8000,   // TN_CNS_STR   A"string"
    TNF_STR_WIDE    = 0x4000,   // TN_CNS_STR   L"string"
    TNF_STR_STR     = 0x2000,   // TN_CNS_STR   S"string"

    TNF_BLK_CATCH   = 0x8000,   // TN_BLOCK     这是一个“catch”块
    TNF_BLK_FOR     = 0x4000,   // TN_BLOCK     这是一个隐式的for循环作用域
    TNF_BLK_NUSER   = 0x2000,   // TN_BLOCK     编译器添加的作用域

    TNF_BLK_HASFIN  = 0x8000,   // TN_TRY       是否存在“finally”部分？

    TNF_ADD_NOCAT   = 0x8000,   // TN_ADD       操作数绑定，不是字符串连接

    TNF_ASG_INIT    = 0x8000,   // TN_ASG       初始化赋值

    TNF_REL_NANREV  = 0x8000,   // TN_GT/LT/..  NaN的相反意义

#ifdef  SETS

    TNF_LIST_DES    = 0x8000,   // TN_LIST      排序列表条目方向 = 降序

    TNF_LIST_SORT   = 0x8000,   // TN_LIST      排序     funclet body
    TNF_LIST_PROJ   = 0x4000,   // TN_LIST      投影     funclet body

#endif

    TNF_CALL_NVIRT  = 0x8000,   // TN_CALL      调用是非虚拟的
    TNF_CALL_VARARG = 0x4000,   // TN_CALL      调用有“额外”的参数
    TNF_CALL_MODOBJ = 0x2000,   // TN_CALL      参数可能修改实例指针
    TNF_CALL_STRCAT = 0x1000,   // TN_CALL      字符串连接赋值
    TNF_CALL_ASGOP  = 0x0800,   // TN_CALL      赋值运算符
    TNF_CALL_ASGPRE = 0x0400,   // TN_CALL      前增/减运算符
    TNF_CALL_GOTADR = 0x0200,   // TN_CALL      结果的地址已计算
    TNF_CALL_CHKOVL = 0x0100,   // TN_CALL      检查重载运算符

    TNF_NAME_TYPENS = 0x8000,   // TN_NAME      名称应该是一个类型
};

/*****************************************************************************/

struct TreeNode
{
    /* 结点的唯一标识符。  */
    int uid;
    enum treeOps         tnOper;                 // operator
    enum var_types       tnVtyp;                 // var_type of the node
    SymDef          sym;

    unsigned short  tnFlags;                // see TNF_xxxx above

    unsigned        tnLineNo;               // for error reporting
    unsigned        tnColumn;               // for error reporting

    TypDef          tnType;                 // type of the node (if bound)
    struct TreeNode *   lpParent;           /* pointer to parent */
    LIST children;       /* array is alloced */

    LIST true_list, false_list;
    LIST next_list;

    //----------------------------------------------------------------

    union
    {
        /* tnIntCon -- integer constant (TN_CNS_INT) */

        struct
        {
            unsigned int      tnIconVal;
        }
            tnIntCon;

        /* tnFltCon -- float   constant (TN_CNS_FLT) */

        struct
        {
            float           tnFconVal;
        }
            tnFltCon;

        /* tnDblCon -- double  constant (TN_CNS_DBL) */

        struct
        {
            double          tnDconVal;
        }
            tnDblCon;


        /* tnName   -- unbound name reference */

        struct
        {
            char           *tnNameId;
        }
            tnName;

        /* tnSym    -- unbound symbol reference */

        struct
        {
            SymDef          tnSym;
            SymDef          tnScp;
        }
            tnSym;

    };
};


typedef struct _OPTIONS {
    char *input_file_name;
    char *output_file_name;
    int optimize;
    BOOL debug_info;
#if defined(zenglj)
    BOOL DragonIR;
#endif
} OPTIONS;

/* pass的元数据。  */
struct pass_data
{
    /* 名称。 */
    const char *name;

    void (*callback)(InterCode, SymTab);
};

/*****************************************************************************/

struct DimDefRec
{
    unsigned            ddLoTree;                   // low  bound expression
    SymDef            ddHiTree;                   // high bound expression

    unsigned        ddIsConst   :1;             // constant fixed dimension
    unsigned        ddNoDim     :1;             // "*" in this position

    unsigned        ddSize;                     // constant dimension value
};

/*****************************************************************************/

struct TypDefRec
{
    /* We store the 'kind' as a simple byte for speed (enum for debugging) */

    enum var_types       tdTypeKind;

    /*
        Since we'd waste 24 bits if we didn't put anything here,
        we'll put some flags that apply only to one of the type
        variants here to use at least some of the bits (which
        would otherwise be wasted on padding anyway).
     */

    unsigned char   tdIsManaged     :1;         // all  : managed type?
    unsigned char   tdIsGenArg      :1;         // all  : uses a generic type arg?

    unsigned char   tdIsValArray    :1;         // array: managed value elems?
    unsigned char   tdIsUndimmed    :1;         // array: no dimension(s) given?
    unsigned char   tdIsGenArray    :1;         // array: non-zero low bound?

    unsigned char   tdIsDelegate    :1;         // class: delegate?
    unsigned char   tdIsIntrinsic   :1;         // class: intrinsic value type?

    unsigned char   tdIsImplicit    :1;         // ref  : implicit managed ref?
    unsigned char   tdIsObjRef      :1;         // ref  : Object ref?

    // .... 16 bits available for various flags and things ....

    union/* tdTypeKind  */
    {

    /* CASE(TYP_PTR)  */

        struct  // Note: the following used for both refs and ptrs
        {
            TypDef          tdrBase;            // type the ref/ptr points to
        }
            tdRef;

    /* CASE(TYP_FNC)  */

        struct
        {
            TypDef          tdfRett;            // return type of the function
            LIST       tdfArgs;            // argument list contains SymDef
        }
            tdFnc;

    /* CASE(TYP_ARRAY)  */

        struct
        {
            TypDef          tdaElem;            // element type
            DimDef          tdaDims;            /* dimension list */
        }
            tdArr;

        struct
        {
            // no additional fields needed for intrinsic types
            int dummy;
        }
            tdIntrinsic;
    };
};

struct constVal
{
    enum var_types           cvVtyp;
    unsigned    index;

    union
    {
        int             cvIval;
        float               cvFval;
        double              cvDval;
    }
                        cvValue;
};

/*****************************************************************************/

enum symbolKinds
{
    SYM_ERR,
    SYM_VAR,

    /*
        The symbol kinds that follow are the only ones that define
        scopes (i.e. they may own other symbols). This is relied
        upon in the function symDef::sdHasScope().
     */

    SYM_FNC,
    SYM_COMPUNIT,
    SYM_SCOPE,

    SYM_FIRST_SCOPED = SYM_FNC,
};

/*****************************************************************************/

struct SymDefRec
{
    char           *sdName;                 // name of the symbol
    TypDef          sdType;                 // type of the symbol
    int uid;

    int line, column;

#if defined (DWARF_DEBUGGING_INFO)
    dw_loc_descr_ref loc_descr;
#endif /* DWARF_DEBUGGING_INFO */

    SymDef          sdParent;               // owning symbol
    SymTab stab;

    enum symbolKinds     sdSymKind;

    unsigned        sdIsDefined         :1; // has definition (e.g. fn body)
    unsigned        sdIsImport          :1; // comes from another program?

    unsigned        sdIsImplicit        :1; // declared by compiler

    unsigned        sdIsStatic          :1; // variables, functions

    union/* (sdSymKind)  */
    {

    /* CASE(SYM_SCOPE)     // scope  */

        struct
        {
            /* This symbol owns scopes, the first field must be 'scopeFields' */

            struct avl_table     *sdScope;     /* contains SymDef */

            /* Here are the fields specific to this symbol kind */

            int             sdSWscopeId;    // scope id for the scope
        }
                sdScope;

    /* CASE(SYM_COMPUNIT)  // compilation unit  */

        struct
        {
            /* This symbol owns scopes, the first field must be 'scopeFields' */

            struct avl_table     *sdScope;     /* contains SymDef */

            /* Here are the fields specific to this symbol kind */

            char      *sdcSrcFile;     // name of source file
        }
                sdComp;

    /* CASE(SYM_FNC)       // function member  */

        struct
        {
            /* This symbol owns scopes, the first field must be 'scopeFields' */

            struct avl_table     *sdScope;     /* contains SymDef */

            /*  */

            SymDef            retv;


            /* Here are the fields specific to this symbol kind */

            unsigned        sdfEntryPt  :1; // could be an entry point?
        }
                sdFnc;

    /* CASE(SYM_VAR)       // variable (local or global) or data member  */

        struct
        {
            unsigned        sdvLocal    :1; // local (auto) (including arguments)
            unsigned        sdvArgument :1; // local : is this an argument ?
            unsigned        sdvHadInit  :1; // have we found an initializer?
            unsigned        sdvConst    :1; // compile time constant?

            struct avl_table        *sdvCnsVal;  /*used for constants contains ConstVal */ 
        }
                sdVar;

        /* The following is used only for sizing purposes */
        struct  {int dummy;}     sdBase;
    };

    void* param;
};


static
BOOL            sdHasScope(enum symbolKinds symkind)
{
    return  (BOOL)(  symkind >= SYM_FIRST_SCOPED);
}

struct symTab
{
    SymDef sym;
    SymDef cmpCurScp;          // the local scope we're in
    struct avl_table *id_map;
};

struct compiler
{
    OPTIONS cmpConfig;
    InterCode code;
    Tree syntaxTree;
    SymTab cmpCurST;           // current symbol table
};

/*****************************************************************************/

struct ssa_name
{
    /* _DECL wrapped by this SSA name.  */
    SymDef var;

    /* SSA version number.  */
    unsigned int version;
    unsigned int new_version;

    IRInst inst;

    void* param;
};

#define LOAD            0x0001
#define ARRAY           0x0002
#define STORE           0x0004
#define BINOP           0x0008
#define UNOP            0x0010
#define CONVERT         0x0020
#define BRANCH          0x0040
#define INVOKE          0x0080
#define SPECIAL         0x0100

enum    IRInstOperator
{
    #define OPDEF(OPCODE, OPNAME, name_zenglj, OPKIND, operand_count) IRINST_OP_ ## OPCODE,
    #include "IROpcode.h"
    #undef  OPDEF
};

struct IRInst {
    /* 结点的唯一标识符，必须位于结构体的首位。  */
    int uid;

    /* 操作数代码。用于区分不同类型的指令。  */
    enum IRInstOperator opcode;

    /* 中间指令所属基本块。  */
    basic_block bb;

    /* 各个操作数。除了PHI结点外的所有指令都有四个或更少的操作数。  */
    LIST operands;     /* contains ssa_name */

    int line;
    int column;
};

struct InterCode
{
    int counter;
    LIST code;
    LIST funcs;     /* contains control_flow_graph */

    struct avl_table *addr_map;
    struct avl_table *id_map;
};

struct basic_block_def {
    /* 进入和离开基本块的边。  */
    LIST preds;     /* contains edge */
    LIST succs;     /* contains edge */

    /* 包含block的最内层循环。  */
    struct loop *loop_father;

    /* 支配和反向支配信息节点。  */
    bitmap dom[2];

    /* 此基本块的索引。  */
    int index;

    /* 此基本块的控制流图。  */
    control_flow_graph cfg;

    LIST insns;     /* contains IRInst */

    bitmap live_in;
    bitmap live_out;

    void* param;
};

struct control_flow_graph {
    /* 函数的入口和出口的基本块指针。
       它们始终是基本块列表的头部和尾部。  */
    basic_block entry_block_ptr;
    basic_block exit_block_ptr;

    /* 按基本块编号索引，获取基本块结构信息。  */
    LIST basic_block_info;     /* contains basic_block */

    InterCode code;

    int local_size;         /* Locals.  */
    int arg_size;           /* Stdarg spills (bytes).  */
    int spill_size;
    int pad_size;   /* Stack pad size.  */

    unsigned funcdef_number;

    /* Number of bytes saved on the stack for callee saved registers.  */
    int callee_saved_reg_size;

    struct avl_table *addr_map;
    struct avl_table *id_map;

    struct loops *loops;

    void* param;
};

/* Control flow edge information.  */
struct edge_def {
    /* The two blocks at the ends of the edge.  */
    basic_block src;
    basic_block dest;
    int uid;
    void* param;
};

struct base_offset
{
    BOOL is_addr;
    varpool_node base, offset;
};

/* A varpool node set is a collection of varpool nodes.  A varpool node
   can appear in multiple sets.  */
struct varpool_node_set_def
{
    struct avl_table *map;
    struct avl_table *nodes;
    int count;
};

struct varpool_node
{
    /* _DECL wrapped by this SSA name.  */
    SymDef var;

    /* SSA version number.  */
    unsigned int version;

    unsigned sdvOffset;

    /* 地址描述符。  */
    LIST addr;           /* struct location */
    LIST value;           /* struct location */

    int uid;
    bitmap     _defines;
    bitmap use_chain;

    void* param;
};

/* 保存每个自然循环的信息的结构。  */
struct loop {
    /* 循环数组的索引。  */
    int num;

    /* 循环头的基本块。  */
    basic_block header;

    /* latch的基本块。  */
    basic_block latch;

    /* 循环中包含的块数。  */
    int num_nodes;

    /* 循环的超循环，从最外层的循环开始。  */
    LIST superloops;           /* contains pointer to struct loop */

    /* 第一个内部(子)循环，如果是最内层循环则为Empty。  */
    LIST inner;           /* contains pointer to struct loop */
};

/* 用于保存函数中自然循环的结构。  */
struct loops {
    /* Array of the loops.  */
    LIST larray;           /* contains pointer to struct loop */

    /* Pointer to root of loop hierarchy tree.  */
    struct loop *tree_root;
};


struct dwarf_data
{
    dw_die_ref comp_unit_die;
    FILE *asm_out_file;
    char *main_input_filename;

    dw_die_ref *abbrev_die_table;
    unsigned int abbrev_die_table_allocated;
    unsigned int abbrev_die_table_in_use;

    /* A pointer to the base of a list of references to DIE's that describe
       types.  The table is indexed by TYPE_UID() which is a unique number,
       indentifying each type.  */
    dw_die_ref *type_die_table;
    /* Number of elements currently allocated for type_die_table.  */
    unsigned type_die_table_allocated;
    /* Number of elements in type_die_table currently in use.  */
    unsigned type_die_table_in_use;

    /* Record the DIE associated with a given base type  This table is
       parallel to the base_type_table, and records the DIE genereated
       to describe base type that has been previously referenced.  */
    dw_die_ref *base_type_die_table;

    /* A pointer to the base of a table that contains frame description
       information for each routine.  */
    dw_fde_ref fde_table;
    
    /* Number of elements currently allocated for fde_table.  */
    unsigned fde_table_allocated;
    
    /* Number of elements in fde_table currently in use.  */
    unsigned fde_table_in_use;
    /* Some DWARF extensions (e.g., MIPS/SGI) implement a subprogram
       attribute that accelerates the lookup of the FDE associated
       with the subprogram.  This variable holds the table index of the FDE 
       associated with the current function (body) definition.  */
    unsigned current_funcdef_fde;
    
    /* Record the size of the frame, so that the DW_AT_frame_base
       attribute can be set properly in gen_subprogram_die.  */
    long int current_funcdef_frame_size ;

    /* A pointer to the base of a table that contains line information
       for each source code line in the compilation unit.  */
    dw_line_info_ref line_info_table;
    
    /* Number of elements currently allocated for line_info_table.  */
    unsigned line_info_table_allocated;
    
    /* Number of elements in line_info_table currently in use.  */
    unsigned line_info_table_in_use;

    /* A pointer to the base of a table that contains a list of publicly
       accessible names.  */
    pubname_ref pubname_table;
    
    /* Number of elements currently allocated for pubname_table.  */
    unsigned pubname_table_allocated;
    
    /* Number of elements in pubname_table currently in use.  */
    unsigned pubname_table_in_use;

    /* memory_pool.  */
    LIST mem;
};


#define STATE_TYPE void*
#define OP_LABEL(p) ((p)->op)
#define LEFT_CHILD(p) ((p)->kids[0])
#define RIGHT_CHILD(p) ((p)->kids[1])
#define STATE_LABEL(p) ((p)->state_label)
#define PANIC printf

/* Tag names and codes.  */

enum dwarf_tag
  {
    DW_TAG_padding = 0x00,
    DW_TAG_array_type = 0x01,
    DW_TAG_class_type = 0x02,
    DW_TAG_entry_point = 0x03,
    DW_TAG_enumeration_type = 0x04,
    DW_TAG_formal_parameter = 0x05,
    DW_TAG_imported_declaration = 0x08,
    DW_TAG_label = 0x0a,
    DW_TAG_lexical_block = 0x0b,
    DW_TAG_member = 0x0d,
    DW_TAG_pointer_type = 0x0f,
    DW_TAG_reference_type = 0x10,
    DW_TAG_compile_unit = 0x11,
    DW_TAG_string_type = 0x12,
    DW_TAG_structure_type = 0x13,
    DW_TAG_subroutine_type = 0x15,
    DW_TAG_typedef = 0x16,
    DW_TAG_union_type = 0x17,
    DW_TAG_unspecified_parameters = 0x18,
    DW_TAG_variant = 0x19,
    DW_TAG_common_block = 0x1a,
    DW_TAG_common_inclusion = 0x1b,
    DW_TAG_inheritance = 0x1c,
    DW_TAG_inlined_subroutine = 0x1d,
    DW_TAG_module = 0x1e,
    DW_TAG_ptr_to_member_type = 0x1f,
    DW_TAG_set_type = 0x20,
    DW_TAG_subrange_type = 0x21,
    DW_TAG_with_stmt = 0x22,
    DW_TAG_access_declaration = 0x23,
    DW_TAG_base_type = 0x24,
    DW_TAG_catch_block = 0x25,
    DW_TAG_const_type = 0x26,
    DW_TAG_constant = 0x27,
    DW_TAG_enumerator = 0x28,
    DW_TAG_file_type = 0x29,
    DW_TAG_friend = 0x2a,
    DW_TAG_namelist = 0x2b,
    DW_TAG_namelist_item = 0x2c,
    DW_TAG_packed_type = 0x2d,
    DW_TAG_subprogram = 0x2e,
    DW_TAG_template_type_param = 0x2f,
    DW_TAG_template_value_param = 0x30,
    DW_TAG_thrown_type = 0x31,
    DW_TAG_try_block = 0x32,
    DW_TAG_variant_part = 0x33,
    DW_TAG_variable = 0x34,
    DW_TAG_volatile_type = 0x35,
  };

#define DW_TAG_lo_user	0x4080
#define DW_TAG_hi_user	0xffff

/* flag that tells whether entry has a child or not */
#define DW_children_no   0
#define	DW_children_yes  1

/* Form names and codes.  */
enum dwarf_form
  {
    DW_FORM_addr = 0x01,
    DW_FORM_block2 = 0x03,
    DW_FORM_block4 = 0x04,
    DW_FORM_data2 = 0x05,
    DW_FORM_data4 = 0x06,
    DW_FORM_data8 = 0x07,
    DW_FORM_string = 0x08,
    DW_FORM_block = 0x09,
    DW_FORM_block1 = 0x0a,
    DW_FORM_data1 = 0x0b,
    DW_FORM_flag = 0x0c,
    DW_FORM_sdata = 0x0d,
    DW_FORM_strp = 0x0e,
    DW_FORM_udata = 0x0f,
    DW_FORM_ref_addr = 0x10,
    DW_FORM_ref1 = 0x11,
    DW_FORM_ref2 = 0x12,
    DW_FORM_ref4 = 0x13,
    DW_FORM_ref8 = 0x14,
    DW_FORM_ref_udata = 0x15,
    DW_FORM_indirect = 0x16
  };

/* Attribute names and codes.  */

enum dwarf_attribute
  {
    DW_AT_sibling = 0x01,
    DW_AT_location = 0x02,
    DW_AT_name = 0x03,
    DW_AT_ordering = 0x09,
    DW_AT_subscr_data = 0x0a,
    DW_AT_byte_size = 0x0b,
    DW_AT_bit_offset = 0x0c,
    DW_AT_bit_size = 0x0d,
    DW_AT_element_list = 0x0f,
    DW_AT_stmt_list = 0x10,
    DW_AT_low_pc = 0x11,
    DW_AT_high_pc = 0x12,
    DW_AT_language = 0x13,
    DW_AT_member = 0x14,
    DW_AT_discr = 0x15,
    DW_AT_discr_value = 0x16,
    DW_AT_visibility = 0x17,
    DW_AT_import = 0x18,
    DW_AT_string_length = 0x19,
    DW_AT_common_reference = 0x1a,
    DW_AT_comp_dir = 0x1b,
    DW_AT_const_value = 0x1c,
    DW_AT_containing_type = 0x1d,
    DW_AT_default_value = 0x1e,
    DW_AT_inline = 0x20,
    DW_AT_is_optional = 0x21,
    DW_AT_lower_bound = 0x22,
    DW_AT_producer = 0x25,
    DW_AT_prototyped = 0x27,
    DW_AT_return_addr = 0x2a,
    DW_AT_start_scope = 0x2c,
    DW_AT_stride_size = 0x2e,
    DW_AT_upper_bound = 0x2f,
    DW_AT_abstract_origin = 0x31,
    DW_AT_accessibility = 0x32,
    DW_AT_address_class = 0x33,
    DW_AT_artificial = 0x34,
    DW_AT_base_types = 0x35,
    DW_AT_calling_convention = 0x36,
    DW_AT_count = 0x37,
    DW_AT_data_member_location = 0x38,
    DW_AT_decl_column = 0x39,
    DW_AT_decl_file = 0x3a,
    DW_AT_decl_line = 0x3b,
    DW_AT_declaration = 0x3c,
    DW_AT_discr_list = 0x3d,
    DW_AT_encoding = 0x3e,
    DW_AT_external = 0x3f,
    DW_AT_frame_base = 0x40,
    DW_AT_friend = 0x41,
    DW_AT_identifier_case = 0x42,
    DW_AT_macro_info = 0x43,
    DW_AT_namelist_items = 0x44,
    DW_AT_priority = 0x45,
    DW_AT_segment = 0x46,
    DW_AT_specification = 0x47,
    DW_AT_static_link = 0x48,
    DW_AT_type = 0x49,
    DW_AT_use_location = 0x4a,
    DW_AT_variable_parameter = 0x4b,
    DW_AT_virtuality = 0x4c,
    DW_AT_vtable_elem_location = 0x4d,
    /* SGI/MIPS Extensions */
    DW_AT_MIPS_fde = 0x2001,
    DW_AT_MIPS_loop_begin = 0x2002,
    DW_AT_MIPS_tail_loop_begin = 0x2003,
    DW_AT_MIPS_epilog_begin = 0x2004,
    DW_AT_MIPS_loop_unroll_factor = 0x2005,
    DW_AT_MIPS_software_pipeline_depth = 0x2006,
    DW_AT_MIPS_linkage_name = 0x2007,
    /* GNU extensions.  */
    DW_AT_sf_names = 0x2101,
    DW_AT_src_info = 0x2102,
    DW_AT_mac_info = 0x2103,
    DW_AT_src_coords = 0x2104,
    DW_AT_body_begin = 0x2105,
    DW_AT_body_end = 0x2106
  };

#define DW_AT_lo_user	0x2000	/* implementation-defined range start */
#define DW_AT_hi_user	0x3ff0	/* implementation-defined range end */

/* Location atom names and codes.  */

enum dwarf_location_atom
  {
    DW_OP_addr = 0x03,
    DW_OP_deref = 0x06,
    DW_OP_const1u = 0x08,
    DW_OP_const1s = 0x09,
    DW_OP_const2u = 0x0a,
    DW_OP_const2s = 0x0b,
    DW_OP_const4u = 0x0c,
    DW_OP_const4s = 0x0d,
    DW_OP_const8u = 0x0e,
    DW_OP_const8s = 0x0f,
    DW_OP_constu = 0x10,
    DW_OP_consts = 0x11,
    DW_OP_dup = 0x12,
    DW_OP_drop = 0x13,
    DW_OP_over = 0x14,
    DW_OP_pick = 0x15,
    DW_OP_swap = 0x16,
    DW_OP_rot = 0x17,
    DW_OP_xderef = 0x18,
    DW_OP_abs = 0x19,
    DW_OP_and = 0x1a,
    DW_OP_div = 0x1b,
    DW_OP_minus = 0x1c,
    DW_OP_mod = 0x1d,
    DW_OP_mul = 0x1e,
    DW_OP_neg = 0x1f,
    DW_OP_not = 0x20,
    DW_OP_or = 0x21,
    DW_OP_plus = 0x22,
    DW_OP_plus_uconst = 0x23,
    DW_OP_shl = 0x24,
    DW_OP_shr = 0x25,
    DW_OP_shra = 0x26,
    DW_OP_xor = 0x27,
    DW_OP_bra = 0x28,
    DW_OP_eq = 0x29,
    DW_OP_ge = 0x2a,
    DW_OP_gt = 0x2b,
    DW_OP_le = 0x2c,
    DW_OP_lt = 0x2d,
    DW_OP_ne = 0x2e,
    DW_OP_skip = 0x2f,
    DW_OP_lit0 = 0x30,
    DW_OP_lit1 = 0x31,
    DW_OP_lit2 = 0x32,
    DW_OP_lit3 = 0x33,
    DW_OP_lit4 = 0x34,
    DW_OP_lit5 = 0x35,
    DW_OP_lit6 = 0x36,
    DW_OP_lit7 = 0x37,
    DW_OP_lit8 = 0x38,
    DW_OP_lit9 = 0x39,
    DW_OP_lit10 = 0x3a,
    DW_OP_lit11 = 0x3b,
    DW_OP_lit12 = 0x3c,
    DW_OP_lit13 = 0x3d,
    DW_OP_lit14 = 0x3e,
    DW_OP_lit15 = 0x3f,
    DW_OP_lit16 = 0x40,
    DW_OP_lit17 = 0x41,
    DW_OP_lit18 = 0x42,
    DW_OP_lit19 = 0x43,
    DW_OP_lit20 = 0x44,
    DW_OP_lit21 = 0x45,
    DW_OP_lit22 = 0x46,
    DW_OP_lit23 = 0x47,
    DW_OP_lit24 = 0x48,
    DW_OP_lit25 = 0x49,
    DW_OP_lit26 = 0x4a,
    DW_OP_lit27 = 0x4b,
    DW_OP_lit28 = 0x4c,
    DW_OP_lit29 = 0x4d,
    DW_OP_lit30 = 0x4e,
    DW_OP_lit31 = 0x4f,
    DW_OP_reg0 = 0x50,
    DW_OP_reg1 = 0x51,
    DW_OP_reg2 = 0x52,
    DW_OP_reg3 = 0x53,
    DW_OP_reg4 = 0x54,
    DW_OP_reg5 = 0x55,
    DW_OP_reg6 = 0x56,
    DW_OP_reg7 = 0x57,
    DW_OP_reg8 = 0x58,
    DW_OP_reg9 = 0x59,
    DW_OP_reg10 = 0x5a,
    DW_OP_reg11 = 0x5b,
    DW_OP_reg12 = 0x5c,
    DW_OP_reg13 = 0x5d,
    DW_OP_reg14 = 0x5e,
    DW_OP_reg15 = 0x5f,
    DW_OP_reg16 = 0x60,
    DW_OP_reg17 = 0x61,
    DW_OP_reg18 = 0x62,
    DW_OP_reg19 = 0x63,
    DW_OP_reg20 = 0x64,
    DW_OP_reg21 = 0x65,
    DW_OP_reg22 = 0x66,
    DW_OP_reg23 = 0x67,
    DW_OP_reg24 = 0x68,
    DW_OP_reg25 = 0x69,
    DW_OP_reg26 = 0x6a,
    DW_OP_reg27 = 0x6b,
    DW_OP_reg28 = 0x6c,
    DW_OP_reg29 = 0x6d,
    DW_OP_reg30 = 0x6e,
    DW_OP_reg31 = 0x6f,
    DW_OP_breg0 = 0x70,
    DW_OP_breg1 = 0x71,
    DW_OP_breg2 = 0x72,
    DW_OP_breg3 = 0x73,
    DW_OP_breg4 = 0x74,
    DW_OP_breg5 = 0x75,
    DW_OP_breg6 = 0x76,
    DW_OP_breg7 = 0x77,
    DW_OP_breg8 = 0x78,
    DW_OP_breg9 = 0x79,
    DW_OP_breg10 = 0x7a,
    DW_OP_breg11 = 0x7b,
    DW_OP_breg12 = 0x7c,
    DW_OP_breg13 = 0x7d,
    DW_OP_breg14 = 0x7e,
    DW_OP_breg15 = 0x7f,
    DW_OP_breg16 = 0x80,
    DW_OP_breg17 = 0x81,
    DW_OP_breg18 = 0x82,
    DW_OP_breg19 = 0x83,
    DW_OP_breg20 = 0x84,
    DW_OP_breg21 = 0x85,
    DW_OP_breg22 = 0x86,
    DW_OP_breg23 = 0x87,
    DW_OP_breg24 = 0x88,
    DW_OP_breg25 = 0x89,
    DW_OP_breg26 = 0x8a,
    DW_OP_breg27 = 0x8b,
    DW_OP_breg28 = 0x8c,
    DW_OP_breg29 = 0x8d,
    DW_OP_breg30 = 0x8e,
    DW_OP_breg31 = 0x8f,
    DW_OP_regx = 0x90,
    DW_OP_fbreg = 0x91,
    DW_OP_bregx = 0x92,
    DW_OP_piece = 0x93,
    DW_OP_deref_size = 0x94,
    DW_OP_xderef_size = 0x95,
    DW_OP_nop = 0x96
  };

#define DW_OP_lo_user	0x80	/* implementation-defined range start */
#define DW_OP_hi_user	0xff	/* implementation-defined range end */

/* Type encodings.  */

enum dwarf_type
  {
    DW_ATE_void = 0x0,
    DW_ATE_address = 0x1,
    DW_ATE_boolean = 0x2,
    DW_ATE_complex_float = 0x3,
    DW_ATE_float = 0x4,
    DW_ATE_signed = 0x5,
    DW_ATE_signed_char = 0x6,
    DW_ATE_unsigned = 0x7,
    DW_ATE_unsigned_char = 0x8
  };

#define	DW_ATE_lo_user 0x80
#define	DW_ATE_hi_user 0xff

/* line number opcodes */
enum dwarf_line_number_ops
  {
    DW_LNS_extended_op = 0,
    DW_LNS_copy = 1,
    DW_LNS_advance_pc = 2,
    DW_LNS_advance_line = 3,
    DW_LNS_set_file = 4,
    DW_LNS_set_column = 5,
    DW_LNS_negate_stmt = 6,
    DW_LNS_set_basic_block = 7,
    DW_LNS_const_add_pc = 8,
    DW_LNS_fixed_advance_pc = 9
  };

/* line number extended opcodes */
enum dwarf_line_number_x_ops
  {
    DW_LNE_end_sequence = 1,
    DW_LNE_set_address = 2,
    DW_LNE_define_file = 3
  };

/* call frame information */
enum dwarf_call_frame_info
  {
    DW_CFA_advance_loc = 0x40,
    DW_CFA_offset = 0x80,
    DW_CFA_restore = 0xc0,
    DW_CFA_nop = 0x00,
    DW_CFA_set_loc = 0x01,
    DW_CFA_advance_loc1 = 0x02,
    DW_CFA_advance_loc2 = 0x03,
    DW_CFA_advance_loc4 = 0x04,
    DW_CFA_offset_extended = 0x05,
    DW_CFA_restore_extended = 0x06,
    DW_CFA_undefined = 0x07,
    DW_CFA_same_value = 0x08,
    DW_CFA_register = 0x09,
    DW_CFA_remember_state = 0x0a,
    DW_CFA_restore_state = 0x0b,
    DW_CFA_def_cfa = 0x0c,
    DW_CFA_def_cfa_register = 0x0d,
    DW_CFA_def_cfa_offset = 0x0e,
    /* SGI/MIPS specific */
    DW_CFA_MIPS_advance_loc8 = 0x1d
  };

#define DW_CIE_ID	  0xffffffff
#define DW_CIE_VERSION	  1

#define DW_CFA_extended   0
#define DW_CFA_low_user   0x1c
#define DW_CFA_high_user  0x3f

/* Each entry in the line_info_table maintains the file and
   line nuber associated with the label generated for that
   entry.  The label gives the PC value associated with
   the line number entry.  */
typedef struct dw_line_info_struct
  {
    unsigned long dw_file_num;
    unsigned long dw_line_num;
    unsigned int column_num;
  }
dw_line_info_entry;

/* Call frames are described using a sequence of Call Frame
   Information instructions.  The register number, offset
   and address fields are provided as possible operands;
   their use is selected by the opcode field.  */
typedef union dw_cfi_oprnd_struct
  {
    unsigned long dw_cfi_reg_num;
    long int dw_cfi_offset;
    char *dw_cfi_addr;
  }
dw_cfi_oprnd;

typedef struct dw_cfi_struct
  {
    dw_cfi_ref dw_cfi_next;
    enum dwarf_call_frame_info dw_cfi_opc;
    dw_cfi_oprnd dw_cfi_oprnd1;
    dw_cfi_oprnd dw_cfi_oprnd2;
  }
dw_cfi_node;

/* All call frame descriptions (FDE's) in the GCC generated DWARF
   refer to a signle Common Information Entry (CIE), defined at
   the beginning of the .debug_frame section.  This used of a single
   CIE obviates the need to keep track of multiple CIE's
   in the DWARF generation routines below.  */
typedef struct dw_fde_struct
  {
    unsigned long dw_fde_offset;
    char *dw_fde_begin;
    char *dw_fde_end_prolog;
    char *dw_fde_begin_epilogue;
    char *dw_fde_end;
    char *dw_fde_current_label;
    dw_cfi_ref dw_fde_cfi;
  }
dw_fde_node;

/* The Debugging Information Entry (DIE) structure */
typedef struct die_struct
  {
    enum dwarf_tag die_tag;
    dw_attr_ref die_attr;
    dw_attr_ref die_attr_last;
    dw_die_ref die_parent;
    dw_die_ref die_child;
    dw_die_ref die_child_last;
    dw_die_ref die_sib;
    dw_offset die_offset;
    unsigned long die_abbrev;

    TypDef          type;
    int label_num;
  }
die_node;

/* Each DIE may have a series of attribute/value pairs.  Values
   can take on several forms.  The forms that are used in this
   impelementation are listed below.  */
typedef enum
  {
    dw_val_class_addr,
    dw_val_class_loc,
    dw_val_class_loc_list,
    dw_val_class_const,
    dw_val_class_unsigned_const,
    dw_val_class_double_const,
    dw_val_class_flag,
    dw_val_class_die_ref,
    dw_val_class_fde_ref,
    dw_val_class_lbl_id,
    dw_val_class_section_offset,
    dw_val_class_str
  }
dw_val_class;

/* The dw_val_node describes an attibute's value, as it is
   represnted internally.  */
typedef struct dw_val_struct
  {
    dw_val_class val_class;
    union
      {
        char *val_addr;
        dw_loc_list_ref  val_loc_list;
        dw_loc_descr_ref val_loc;
        long int val_int;
        long unsigned val_unsigned;
        dw_die_ref val_die_ref;
        unsigned val_fde_index;
        char *val_str;
        char *val_lbl_id;
        char *val_section;
        unsigned char val_flag;
      }
    v;
  }
dw_val_node;

/* Each DIE attribute has a field specifying the attribute kind,
   a link to the next attribute in the chain, and an attribute value.
   Attributes are typically linked below the DIE they modify.  */
typedef struct dw_attr_struct
  {
    enum dwarf_attribute dw_attr;
    dw_attr_ref dw_attr_next;
    dw_val_node dw_attr_val;
  }
dw_attr_node;

/* Locations in memory are described using a sequence of stack machine
   operations.  */
typedef struct dw_loc_descr_struct
  {
    dw_loc_descr_ref dw_loc_next;
    enum dwarf_location_atom dw_loc_opc;
    dw_val_node dw_loc_oprnd1;
    dw_val_node dw_loc_oprnd2;

  /* 供指令选择使用。  */
    int line, column;
  }
dw_loc_descr_node;

/* Location lists are ranges + location descriptions for that range,
   so you can track variables that are in different places over
   their entire life. */
typedef struct dw_loc_list_struct
{
    dw_loc_list_ref dw_loc_next;
    char *begin; /* Label for begin address of range */
    char *end;  /* Label for end address of range */
    char *ll_symbol; /* Label for beginning of location list. Only on head of list */
    char *section; /* Section this loclist is relative to */
    dw_loc_descr_ref expr;
} dw_loc_list_node;

/* The pubname structure */
typedef struct pubname_struct
{
  dw_die_ref die;
  char * name;
}
pubname_entry;




/* EXTERN decls for data */

extern int LineNum;
extern int Column;
extern Compiler comp;

extern  int num_caller_save_registers;
extern  int caller_save_registers[];
extern  int num_callee_save_registers;
extern  int callee_save_registers[];


/* EXTERN procs */
/* procs in main.c */
char *Abbrev (char *lpsz);
void fatal (const char *str, ...);
void error (const char *fname, int line, const char *msg, ...);
void sorry (const char *fname, int line, const char *msg, ...);
void warning (const char *fname, int line, const char *msg, ...);
BOOL cmpInit(Compiler comp);
void cmpDone(Compiler comp);
BOOL cmpStart(Compiler comp, const char *defOutFileName);


/* procs in alloc.c */
void * xmalloc (size_t);
void * xrealloc (void *, size_t);
void * xcalloc (size_t, size_t);

/* procs in tree.c */
Tree parseAllocNode ();
void parseDeleteNode( Tree node );
Tree parseCreateNode(enum treeOps op, int tnLineNo, int tnColumn);
Tree parseCreateNameNode(const char *name, size_t length, int tnLineNo, int tnColumn);
void InsertChildNode (Tree lpParent, Tree lpChildNode);
Tree parseCreateIconNode(unsigned int ival, enum var_types typ, int tnLineNo, int tnColumn);
Tree parseCreateFconNode(float fval, int tnLineNo, int tnColumn);
void DestroyTree(Tree node);


/* procs in IRGenerator.c */
void ASTDumper (Tree syntaxTree, const char *filename);
BOOL genIR(Tree root, InterCode code, SymTab cmpCurST);


/* procs in IRCode.c */
void InterCodeDelete( InterCode code );
InterCode InterCodeNew (void);
void InterCodeAddInst_nobb(InterCode code, IRInst insn, BOOL fNewID);
IRInst *InterCodeAddExistingInst (basic_block bb, IRInst insn);
IRInst *InterCodeAddInst (basic_block bb, IRInst insn, BOOL fNewID);
void InterCodeRemoveInst_nobb(InterCode code, IRInst insn);
void InterCodeRemoveInst (InterCode code, IRInst insn, varpool_node_set set);
void InterCodeDump (InterCode code, FILE *dump_file);
IRInst InterCodeGetInstByID (InterCode code, int id);
IRInst *InterCodeGetCursor (InterCode code, IRInst inst);
IRInst *InterCodeInsertBefore (basic_block bb, IRInst *Curs, IRInst inst, BOOL fNewID, varpool_node_set set);
IRInst *InterCodeInsertAfter (basic_block bb, IRInst *Curs, IRInst inst, BOOL fNewID, varpool_node_set set);
void update_destinations (basic_block bb, int orig_insn, int new_insn, varpool_node_set set);


/* procs in symbol.c */
void stInit(SymTab stab);
void stDeinit(SymTab stab);
void stRemoveSym(SymDef sym);
SymDef stDeclareSym(SymTab stab, char *name, enum symbolKinds kind);
SymDef stLookupSym(SymTab stab, char *name, enum symbolKinds kind);
SymDef stGetSymByID (SymTab stab, int id);
BOOL stMatchTypes(TypDef typ1, TypDef typ2);
void stSetName(SymDef sym, const char *name);
void stDumpSymbol(SymDef sym, int indent, BOOL recurse, FILE *dump_file);
void DumpSymName_zenglj(SymDef sym, FILE *dump_file);
void DumpSymbolTable(SymTab stab, FILE *dump_file);
char *stGetSymName(SymDef sym);
SymDef stCreateFconNode(SymTab stab, float fval);
SymDef stCreateIconNode(SymTab stab, int ival);
SymDef stCreateDconNode(SymTab stab, double dval);
ConstVal GetConstVal(SymDef sym, unsigned offset);
BOOL SetIconVal(SymDef sym, int ival, unsigned index);
BOOL SetFconVal(SymDef sym, float fval, unsigned index);
BOOL SetDconVal(SymDef sym, double dval, unsigned index);
BOOL is_global_var (SymDef sym);
BOOL in_global_scope(SymDef scp);
void traverse_symtree (SymDef st, void (*sym_func) (SymDef, void *), void *param);
void traverse_global_variables (SymTab stab, void (*sym_func) (SymDef, void *), void *param);
LIST get_globals (SymTab stab);
int compare_constant (ConstVal t1, ConstVal t2, void *unused);
int CompareSymbolID (SymDef a, SymDef b, void *unused);
SymDef duplicate_symbol (SymDef sym);

/* procs in type.c */
TypDef stAllocTypDef(enum var_types kind);
DimDef stNewDimDesc(unsigned size, SymDef sym);
TypDef stNewArrType(DimDef dims, TypDef elem);
TypDef stNewPtrType(TypDef elem, BOOL impl);
void stDeleteTypDef(TypDef typ);
BOOL stMatchType2(TypDef typ1, TypDef typ2);
BOOL stArgsMatch(TypDef typ1, TypDef typ2);
BOOL stMatchArrays(TypDef typ1, TypDef typ2);
TypDef stNewFncType(TypDef rett);
void stTypeName(TypDef typ, SymDef sym, FILE *dump_file);
TypDef CopyType(TypDef srcType);
int type_size(TypDef type);
const char *stIntrinsicTypeName(enum var_types vt);
const char *stIntrinsicTypeName_zenglj(enum var_types vt);
void stTypeName_zenglj(TypDef typ, SymDef sym, FILE *dump_file);
TypDef stGetBaseType(TypDef type);

/* procs in IRInst.c */
void IRInstSetOperand(IRInst insn, int index, SymDef var);
int IRInstGetNumOperands(IRInst insn);
ssa_name IRInstGetOperand(IRInst insn, int index);
void IRInstDump (IRInst insn, BOOL pretty, FILE *dump_file);
void IRInstDelInst(IRInst insn);
void IRInstResize(IRInst insn, int length);
IRInst IRInstEmitInst(enum IRInstOperator opcode, int line, int column);
const char *IRInstGetOperatorSpelling(enum IRInstOperator Operator);
int IRInstGetOpKind(enum IRInstOperator Operator);
void IRInstDump_zenglj (IRInst insn, SymTab stab, FILE *dump_file);
BOOL IRInstIsOutput (IRInst insn, int opnum);
int cmp_ssa_name (ssa_name a, ssa_name b, void *no_backend);
IRInst IRInstCopy (IRInst orig);


/* procs in cfg.c */
basic_block alloc_block (void);
void free_edge (edge e);
edge alloc_edge (void);
basic_block split_edge (edge e);
void copy_block (basic_block dest, basic_block source);
void free_block (basic_block bb);
void remove_edge (edge e);
edge make_edge (basic_block src, basic_block dst);
void free_cfg (InterCode code, control_flow_graph cfg);
control_flow_graph alloc_flow (InterCode code);
void unlink_block (basic_block b);
void link_block (control_flow_graph cfg, basic_block b);
void cfgbuild (InterCode code, SymTab stab);
void dump_cfg (InterCode code, const char *file);
void dump_cfg_zenglj (InterCode code, SymTab stab, FILE *dump_file);
void pre_and_rev_post_order_compute (control_flow_graph fn, LIST pre_order, LIST rev_post_order, BOOL include_entry_exit, BOOL reverse);
basic_block lookup_block_by_id (control_flow_graph cfg, int id);
void compute_liveness (control_flow_graph cfun, varpool_node_set set);
void cleanup_cfg (control_flow_graph cfun, SymTab stab);


/* procs in ssa.c */
void build_ssa (InterCode code, SymTab stab);
basic_block get_immediate_dominator (control_flow_graph cfun, BOOL reverse, basic_block bb);
void compute_dominators (control_flow_graph cfun, BOOL reverse);
void remove_ssa_form (InterCode code, SymTab stab);
void rewrite_out_of_ssa (InterCode code, SymTab stab);

/* procs in sccp.c */
void SparseCondConstProp(InterCode code, SymTab stab);

/* procs in varpool.c */
varpool_node_set varpool_node_set_new (control_flow_graph cfun, BOOL no_backend);
void free_varpool_node_set (varpool_node_set set);
varpool_node varpool_node_set_find (varpool_node_set set, int id);
varpool_node varpool_node_set_add (varpool_node_set set, ssa_name node);
varpool_node varpool_get_node (varpool_node_set set, ssa_name decl);
int compare_varpool_node( varpool_node a, varpool_node b, void *no_backend );
void varpool_node_set_update (control_flow_graph cfun, varpool_node_set set, BOOL no_backend);


/* procs in loop.c */
void flow_loops_free (struct loops **loops);
struct loops *flow_loops_find (control_flow_graph cfun);
void flow_loops_dump (struct loops *current_loops, FILE *file);
void flow_loop_dump (const struct loop *loop, FILE *file);
void copy_loop_headers (control_flow_graph cfun);
void create_preheaders (control_flow_graph cfun, SymTab stab);
int loop_depth (basic_block bb);
BOOL is_loop_head (control_flow_graph cfun, basic_block header);
LIST get_loop_body (const struct loop *loop);
LIST get_loop_latch_edges (const struct loop *loop);

/* procs in gvn.c */
void GlobalValueNumbering (InterCode code, SymTab stab);

/* procs in dce.c */
void perform_ssa_dce (InterCode code, SymTab stab);

/* procs in inline.c */
void inline_transform (InterCode code, SymTab stab);
BOOL pure_function (InterCode code, SymDef function);

/* procs in lcm.c */
void LazyCodeMotion (InterCode code, SymTab stab);
void compute_available (control_flow_graph cfun, struct avl_table *block_map, bitmap ONES, bitmap *avloc, bitmap *kill, bitmap *avout, bitmap *avin);
bitmap *bitmap_vector_alloc (int n_vecs);
void bitmap_vector_free (bitmap * vec, int n_vecs);

/* procs in copyprop.c */
void copyprop (InterCode code, SymTab stab);
void CopyPropPass (InterCode code, SymTab stab);
BOOL local_copyprop_pass (basic_block block, varpool_node_set set);

/* procs in StraightLineStrengthReduce.c */
void StraightLineStrengthReduce (InterCode code, SymTab stab);

/* procs in global-variable-localization.c */
void GlobalVariableLocalization (InterCode code, SymTab stab);

/* procs in osr.c */
void OSR (InterCode code, SymTab stab);

/* procs in loop_unroll.c */
void unroll_loops (InterCode code, SymTab stab);

/* procs in treeheight.c */
void treeheight (InterCode code, SymTab stab);


/* procs in codegen.c */
BOOL codegen (InterCode code, SymTab stab, const char *name, FILE *file, BOOL debug_info) ;
void dwarfout_end_epilogue (struct dwarf_data *ddata, control_flow_graph func);
void dwarfout_header (FILE *asm_out_file);
void dwarfout_footer (FILE *asm_out_file);
void add_cfi (dw_cfi_ref *list_head, dw_cfi_ref cfi);
dw_cfi_ref new_cfi (struct dwarf_data *ddata);
dw_die_ref new_die (enum dwarf_tag tag_value, dw_die_ref parent_die, struct dwarf_data *ddata);
void add_AT_flag (dw_die_ref die, enum dwarf_attribute attr_kind, unsigned flag, struct dwarf_data *ddata);
void add_name_and_src_coords_attributes (dw_die_ref die, SymDef decl, struct dwarf_data *ddata);
void add_type_attribute (dw_die_ref object_die, TypDef type, dw_die_ref context_die, struct dwarf_data *ddata);
void add_AT_lbl_id (dw_die_ref die, enum dwarf_attribute attr_kind, const char *lbl_id, struct dwarf_data *ddata);
void add_pubname (SymDef decl, dw_die_ref die, struct dwarf_data *ddata);
dw_loc_descr_ref new_loc_descr (enum dwarf_location_atom op, unsigned long oprnd1, unsigned long oprnd2, struct dwarf_data *ddata);
void add_AT_loc (dw_die_ref die, enum dwarf_attribute attr_kind, dw_loc_descr_ref loc, struct dwarf_data *ddata);
void gen_decl_die (SymDef decl, dw_die_ref context_die, struct dwarf_data *ddata);
void add_loc_descr (dw_loc_descr_ref *list_head, dw_loc_descr_ref descr);
dw_loc_descr_ref mem_loc_descriptor (SymDef rtl, struct dwarf_data *ddata);
dw_loc_descr_ref reg_loc_descriptor (unsigned reg, struct dwarf_data *ddata);
void decls_for_scope (SymDef stmt, dw_die_ref context_die, struct dwarf_data *ddata);
dw_loc_descr_ref based_loc_descr (unsigned reg, int offset, struct dwarf_data *ddata);


#include "CodeGeneratorArm32.h"

/*****************************************************************************/
#endif
/*****************************************************************************/
