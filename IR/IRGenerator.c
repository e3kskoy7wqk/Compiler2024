/****************************************************/
/* File: IRGenerator.c                              */
/* AST遍历产生线性IR                                 */
/****************************************************/

#include <stdlib.h>
#include <string.h>
#if ! defined (_WIN32)
#include <unistd.h>
#endif
#include "all.h"

#define TESTFLAG(v,f)(((v)&(f))!=0)
#define NEXT_INSN(code) ((code)->counter)

#if !defined(NDEBUG)

#if 0

  // helper macro for short definition of trace-output inside code
  #define TRACE_SEMANTICS(code)       \
    if (1) {       \
      code;                                    \
    }
#else
  #define TRACE_SEMANTICS(code)
#endif

#else
  #define TRACE_SEMANTICS(code)
#endif

static BOOL ir_visit_ast_node (Tree node, InterCode code, SymTab cmpCurST);
static SymDef emit_load(SymDef memSym, InterCode code, SymTab cmpCurST, int line, int column);
static SymDef emit_conversion_insns(SymDef sym, TypDef type, InterCode code, SymTab cmpCurST, int line, int column);
static SymDef emit_binop (SymDef op1, SymDef op2, enum IRInstOperator c, InterCode code, SymTab cmpCurST, int line, int column);
static Tree * decl_initializer(SymDef sym, Tree *Cursor, TypDef type, unsigned long c, int first, InterCode code, SymTab cmpCurST);
static Tree * decl_designator(SymDef sym, Tree *node, TypDef type, unsigned long c, int *cur_index, InterCode code, SymTab cmpCurST);
static SymDef FoldIntBinop(enum treeOps tnOper, SymDef op1, SymDef op2, Tree node, SymTab cmpCurST);
static SymDef FoldIntUnop(enum treeOps tnOper, SymDef op1, SymTab cmpCurST);
static SymDef FoldFltBinop(enum treeOps tnOper, SymDef op1, SymDef op2, Tree node, SymTab cmpCurST);
static SymDef FoldFltUnop(enum treeOps tnOper, SymDef op1, SymTab cmpCurST);
static void DoBackpatch(LIST lst, SymDef number);
static void DoMerge(LIST To, LIST l1, LIST l2);
static BOOL create_cmp_insn (SymDef op1, SymDef op2, enum treeOps tnOper, Tree node, InterCode code, SymTab cmpCurST);

static void traverse( Tree t,
               void (* preProc) (Tree, void *),
               void (* postProc) (Tree, void *),
               void* lParam )
{
    if (t != NULL)
    {
        Tree *Cursor;
        if (preProc != NULL) preProc(t, lParam);
        for(  Cursor=(Tree *)List_First(t->children)
            ;  Cursor!=NULL
            ;  Cursor = List_Next((void *)Cursor)
            )
            traverse(*Cursor,preProc,postProc,lParam);
        if (postProc != NULL) postProc(t, lParam);
    }
}

static void graph_visit_ast_node(Tree t, FILE *fp)
{
    if (t->lpParent)
        fprintf(fp, "   n%03d -- n%03d ;\n", t->lpParent->uid, t->uid);
    else fprintf(fp, "   n%03d ;\n", t->uid);

    switch (t->tnOper)
    {
        case TN_NAME: fprintf(fp, "   n%03d [label=\"%s\"] ;\n", t->uid, t->tnName.tnNameId); return;
        case TN_CNS_INT: fprintf(fp, "   n%03d [label=\"%d\"] ;\n", t->uid, t->tnIntCon.tnIconVal); return;
        case TN_CNS_FLT: fprintf(fp, "   n%03d [label=\"%g\"] ;\n", t->uid, t->tnFltCon.tnFconVal); return;
        case TN_CNS_DBL: fprintf(fp, "   n%03d [label=\"%g\"] ;\n", t->uid, t->tnDblCon.tnDconVal); return;
        case TN_CALL: fprintf(fp, "   n%03d [label=\"%s ()\"] ;\n", t->uid, t->tnName.tnNameId); return;
        default: break;
    }

    switch (t->tnOper)
    {
#define TREEOP(en,tk,sn,IL,pr,ok) \
        case en: fprintf(fp, "   n%03d [label=\"%s\"] ;\n", t->uid, sn); break;
#include "toplist.h"
#undef TREEOP
        default: break;
    }
}

void
ASTDumper (Tree syntaxTree, const char *filename)
{
    FILE *fp;
    char *name;

    fp=fopen("psg.gv","w");
    if (NULL == fp)
    {
        fprintf (stderr, "error opening psg.gv\n");
        return;
    }

    fputs("graph \"\"\n", fp);
    fputs("   {\n", fp);
    fputs("    fontname=\"Helvetica,Arial,sans-serif\"\n", fp);
    fputs("    node [fontname=\"Helvetica,Arial,sans-serif\"]\n", fp);
    fputs("    edge [fontname=\"Helvetica,Arial,sans-serif\"]\n", fp);
    traverse(syntaxTree, (void (*) (Tree, void *))graph_visit_ast_node, NULL, fp);
    fputs("   }\n", fp);

    fclose (fp);

    name = (char *) xmalloc (strlen(filename) + 64);
    sprintf( name, "dot -Tpdf psg.gv > \"%s\"", filename );
    if (system(name) != 0) {
        fprintf( stderr, "Error executing %s\n", name );
        free (name);
        return;
    }

    free (name);
#if defined (_WIN32)
    _unlink("psg.gv");
#else
    unlink("psg.gv");
#endif
}

struct patch_info
{
  IRInst insn;
  BOOL is_true_list;
};

static BOOL VisitTN_ERROR(Tree node, InterCode code, SymTab cmpCurST)
{
    return FALSE;
}

static BOOL VisitTN_NONE(Tree node, InterCode code, SymTab cmpCurST)
{
    TRACE_SEMANTICS (fprintf (stderr, "enter VisitTN_NONE()"));
    /* do nothing */
    TRACE_SEMANTICS (fprintf (stderr, "exit  VisitTN_NONE with %x", TRUE));
    return TRUE;
}

static BOOL VisitTN_NAME(Tree node, InterCode code, SymTab cmpCurST)
{
    BOOL bSuccess = FALSE;

    TRACE_SEMANTICS (fprintf (stderr, "enter VisitTN_NAME()"));

    if (TESTFLAG (node->tnFlags, TNF_LVALUE))
    {
        node->sym = stLookupSym(cmpCurST, node->tnName.tnNameId, SYM_VAR);
        if (!node->sym)
        {
            error(comp->cmpConfig.input_file_name, node->tnLineNo, "use of undeclared identifier '%s'", node->tnName.tnNameId);
            goto fail;
        }
        node->tnType = CopyType(node->sym->sdType);
    }

    bSuccess = TRUE;

fail:

    TRACE_SEMANTICS (fprintf (stderr, "exit  VisitTN_NAME with %x", bSuccess));
    return bSuccess;
}

static BOOL VisitTN_CNS_INT(Tree node, InterCode code, SymTab cmpCurST)
{
    int iv1;
    TRACE_SEMANTICS (fprintf (stderr, "enter VisitTN_CNS_INT()"));
    memcpy (&iv1, &node->tnIntCon.tnIconVal, sizeof (iv1));
    node->sym = stCreateIconNode(cmpCurST, iv1);
    node->tnType = CopyType(node->sym->sdType);
    TRACE_SEMANTICS (fprintf (stderr, "exit  VisitTN_CNS_INT with %x", TRUE));
    return TRUE;
}

static BOOL VisitTN_CNS_FLT(Tree node, InterCode code, SymTab cmpCurST)
{
    TRACE_SEMANTICS (fprintf (stderr, "enter VisitTN_CNS_FLT()"));
    node->sym = stCreateFconNode(cmpCurST, node->tnFltCon.tnFconVal);
    node->tnType = CopyType(node->sym->sdType);
    TRACE_SEMANTICS (fprintf (stderr, "exit  VisitTN_CNS_FLT with %x", TRUE));
    return TRUE;
}

static BOOL VisitTN_CNS_DBL(Tree node, InterCode code, SymTab cmpCurST)
{
    TRACE_SEMANTICS (fprintf (stderr, "enter VisitTN_CNS_DBL()"));
    node->sym = stCreateDconNode(cmpCurST, node->tnDblCon.tnDconVal);
    node->tnType = CopyType(node->sym->sdType);
    TRACE_SEMANTICS (fprintf (stderr, "exit  VisitTN_CNS_DBL with %x", TRUE));
    return TRUE;
}

static BOOL VisitTN_LOG_NOT(Tree node, InterCode code, SymTab cmpCurST)
{
    BOOL bSuccess = FALSE;
    Tree first;
    SymDef sym;
    IRInst insn;
    BOOL relop = FALSE;
    Tree temp;

    TRACE_SEMANTICS (fprintf (stderr, "enter VisitTN_LOG_NOT()"));

    temp = node;
    while   (temp->tnOper == TN_LOG_NOT)
    {
        switch (temp->lpParent->tnOper)
        {
        case TN_LOG_OR: relop = TRUE; break;
        case TN_LOG_AND: relop = TRUE; break;
        case TN_IF: relop = TRUE; break;
        case TN_WHILE: relop = TRUE; break;
        case TN_DO: relop = TRUE; break;
        default     : break;
        }

        temp = temp->lpParent;
    }

    /* 访问当前结点的孩子。  */
    first = *(Tree *)List_First (node->children);
    if (!ir_visit_ast_node (first, code, cmpCurST))
        goto fail;

    if  (relop)
    {
        if  (first->sym)
        {
            if (!create_cmp_insn (first->sym, stCreateIconNode(cmpCurST, 0), TN_EQ, node, code, cmpCurST))
                goto fail;
        }
        else
        {
            List_Join(node->true_list, first->false_list);
            List_Join(node->false_list, first->true_list);
        }

    }
    else
    {
        /* 类型检查。  */
        if  (first->sym->sdType->tdTypeKind != TYP_INT && first->sym->sdType->tdTypeKind != TYP_FLOAT)
        {
            error (comp->cmpConfig.input_file_name, node->tnLineNo, "wrong type argument to unary minus");
            goto fail;
        }

        /* 加载操作数。  */
        sym = emit_load (first->sym, code, cmpCurST, first->tnLineNo, first->tnColumn);

        /* 尝试常数折叠。  */
        if      (sym->sdType->tdTypeKind == TYP_INT)
        {
            node->sym = FoldIntUnop(node->tnOper, sym, cmpCurST);
        }
        else if (sym->sdType->tdTypeKind == TYP_FLOAT)
        {
            node->sym = FoldFltUnop(node->tnOper, sym, cmpCurST);
        }

        /* 如果常数折叠不成功，则产生中间指令。  */
        if  (!node->sym)
        {
            node->sym = stDeclareSym(cmpCurST, NULL, SYM_VAR);
            node->sym->sdIsImplicit = TRUE;
            node->sym->sdType = stAllocTypDef(TYP_INT);

            insn = IRInstEmitInst(IRINST_OP_not, node->tnLineNo, node->tnColumn);
            IRInstSetOperand(insn, 1, sym);
            IRInstSetOperand(insn, 0, node->sym);
            InterCodeAddInst_nobb(code, insn, TRUE);
        }
    }

    if  (node->sym)
        node->tnType = CopyType(node->sym->sdType);
    else
        node->tnType = stAllocTypDef(TYP_BOOL);

    bSuccess = TRUE;

fail:

    TRACE_SEMANTICS (fprintf (stderr, "exit  VisitTN_LOG_NOT with %x", bSuccess));
    return bSuccess;
}

static BOOL VisitTN_NOP(Tree node, InterCode code, SymTab cmpCurST)
{
    BOOL bSuccess = FALSE;
    Tree first;

    TRACE_SEMANTICS (fprintf (stderr, "enter VisitTN_NOP()"));

    first = *(Tree *)List_First (node->children);
    if (!ir_visit_ast_node (first, code, cmpCurST))
        goto fail;
    node->sym = first->sym;
    node->tnType = CopyType(node->sym->sdType);

    bSuccess = TRUE;

fail:

    TRACE_SEMANTICS (fprintf (stderr, "exit  VisitTN_NOP with %x", bSuccess));
    return bSuccess;
}

static BOOL VisitTN_NEG(Tree node, InterCode code, SymTab cmpCurST)
{
    BOOL bSuccess = FALSE;
    Tree first;
    SymDef sym;
    IRInst insn;

    TRACE_SEMANTICS (fprintf (stderr, "enter VisitTN_NEG()"));

    /* 访问当前结点的孩子。  */
    first = *(Tree *)List_First (node->children);
    if (!ir_visit_ast_node (first, code, cmpCurST))
        goto fail;

    /* 类型检查。  */
    if  (first->sym->sdType->tdTypeKind != TYP_INT && first->sym->sdType->tdTypeKind != TYP_FLOAT)
    {
        error (comp->cmpConfig.input_file_name, node->tnLineNo, "wrong type argument to unary minus");
        goto fail;
    }

    /* 加载操作数。  */
    sym = emit_load (first->sym, code, cmpCurST, first->tnLineNo, first->tnColumn);

    /* 尝试常数折叠。  */
    if      (sym->sdType->tdTypeKind == TYP_INT)
    {
        node->sym = FoldIntUnop(node->tnOper, sym, cmpCurST);
    }
    else if (sym->sdType->tdTypeKind == TYP_FLOAT)
    {
        node->sym = FoldFltUnop(node->tnOper, sym, cmpCurST);
    }

    /* 如果常数折叠不成功，则产生中间指令。  */
    if  (!node->sym)
    {
        node->sym = stDeclareSym(cmpCurST, NULL, SYM_VAR);
        node->sym->sdIsImplicit = TRUE;
        node->sym->sdType = CopyType(first->sym->sdType);

        insn = IRInstEmitInst(IRINST_OP_neg, node->tnLineNo, node->tnColumn);
        IRInstSetOperand(insn, 1, sym);
        IRInstSetOperand(insn, 0, node->sym);
        InterCodeAddInst_nobb(code, insn, TRUE);
    }

    node->tnType = CopyType(node->sym->sdType);

    bSuccess = TRUE;

fail:

    TRACE_SEMANTICS (fprintf (stderr, "exit  VisitTN_NEG with %x", bSuccess));
    return bSuccess;
}

static BOOL VisitBinaryOperator(Tree node, InterCode code, SymTab cmpCurST)
{
    BOOL bSuccess = FALSE;
    void *Cursor;
    enum IRInstOperator opcode;
    Tree first, second;
    BOOL relops = TRUE;
    SymDef op1, op2;
    enum var_types tp1;
    enum var_types tp2;

    TRACE_SEMANTICS (fprintf (stderr, "enter VisitBinaryOperator()"));

    /* 预先判断此结点是布尔表达式还是算数表达式。  */
    switch (node->lpParent->tnOper)
    {
    case TN_EQ  : relops = FALSE; break;
    case TN_NE  : relops = FALSE; break;
    case TN_LT  : relops = FALSE; break;
    case TN_LE  : relops = FALSE; break;
    case TN_GE  : relops = FALSE; break;
    case TN_GT  : relops = FALSE; break;
    default     : break;
    }

    switch (node->tnOper)
    {
    case TN_ADD : opcode = IRINST_OP_add; relops = FALSE; break;
    case TN_SUB : opcode = IRINST_OP_sub; relops = FALSE; break;
    case TN_MUL : opcode = IRINST_OP_mul; relops = FALSE; break;
    case TN_DIV : opcode = IRINST_OP_div; relops = FALSE; break;
    case TN_MOD : opcode = IRINST_OP_rem; relops = FALSE; break;
    case TN_EQ  : opcode = relops ? IRINST_OP_ifeq : IRINST_OP_eq; break;
    case TN_NE  : opcode = relops ? IRINST_OP_ifne : IRINST_OP_ne; break;
    case TN_LT  : opcode = relops ? IRINST_OP_iflt : IRINST_OP_lt; break;
    case TN_LE  : opcode = relops ? IRINST_OP_ifle : IRINST_OP_le; break;
    case TN_GE  : opcode = relops ? IRINST_OP_ifge : IRINST_OP_ge; break;
    case TN_GT  : opcode = relops ? IRINST_OP_ifgt : IRINST_OP_gt; break;
    default     : goto fail; break;
    }

    Cursor=List_First (node->children);
    first = *(Tree *)Cursor;
    Cursor=List_Next ((void *)Cursor);
    second = *(Tree *)Cursor;

    /* 访问两操作数。  */
    if (!ir_visit_ast_node (first, code, cmpCurST))
        goto fail;
    if (!ir_visit_ast_node (second, code, cmpCurST))
        goto fail;

    /* 加载两操作数。  */
    op1 = emit_load (first->sym, code, cmpCurST, first->tnLineNo, first->tnColumn);
    op2 = emit_load (second->sym, code, cmpCurST, second->tnLineNo, second->tnColumn);

    /* 获得两操作数的类型。  */
    tp1 = op1->sdType->tdTypeKind;
    tp2 = op2->sdType->tdTypeKind;

    /* 操作数类型是否合法？  */
    if  ((tp1 != TYP_INT && tp1 != TYP_FLOAT) ||
        (tp2 != TYP_INT && tp2 != TYP_FLOAT))
    {
        error (comp->cmpConfig.input_file_name, node->tnLineNo, "invalid operands to binary expression");
        goto fail;
    }

    /* 将两个操作数转换为为更大的类型 */
    if      (tp1 < tp2)
    {
        op1 = emit_conversion_insns(op1, op2->sdType, code, cmpCurST, node->tnLineNo, node->tnColumn);
    }
    else if (tp1 > tp2)
    {
        op2 = emit_conversion_insns(op2, op1->sdType, code, cmpCurST, node->tnLineNo, node->tnColumn);
    }

    /* 尝试常量折叠。  */
    if      (op1->sdType->tdTypeKind == TYP_INT)
    {
        node->sym = FoldIntBinop(node->tnOper, op1, op2, node, cmpCurST);
    }
    else if (op1->sdType->tdTypeKind == TYP_FLOAT)
    {
        node->sym = FoldFltBinop(node->tnOper, op1, op2, node, cmpCurST);
    }

    /* 常量折叠成功了吗？  */
    if  (!node->sym)
    {
        if  (relops)
        {
            if  (!create_cmp_insn(op1, op2, node->tnOper, node, code, cmpCurST))
                goto fail;
        }
        else
        {
            node->sym = emit_binop(op1, op2, opcode, code, cmpCurST, node->tnLineNo, node->tnColumn);
        }
    }

    if  (node->sym)
        node->tnType = CopyType(node->sym->sdType);
    else
        node->tnType = stAllocTypDef(TYP_BOOL);

    bSuccess = TRUE;

fail:

    TRACE_SEMANTICS (fprintf (stderr, "exit  VisitBinaryOperator with %x", bSuccess));
    return bSuccess;
}

#define  VisitTN_ADD             VisitBinaryOperator
#define  VisitTN_SUB             VisitBinaryOperator
#define  VisitTN_MUL             VisitBinaryOperator
#define  VisitTN_DIV             VisitBinaryOperator
#define  VisitTN_MOD             VisitBinaryOperator
#define  VisitTN_EQ              VisitBinaryOperator
#define  VisitTN_NE              VisitBinaryOperator
#define  VisitTN_LT              VisitBinaryOperator
#define  VisitTN_LE              VisitBinaryOperator
#define  VisitTN_GE              VisitBinaryOperator
#define  VisitTN_GT              VisitBinaryOperator

static BOOL VisitTN_ASG(Tree node, InterCode code, SymTab cmpCurST)
{
    IRInst insn;
    BOOL bSuccess = FALSE;
    Tree *Cursor;
    Tree first, second;
    SymDef sym;
    Tree name;

    TRACE_SEMANTICS (fprintf (stderr, "enter VisitTN_ASG()"));

    Cursor=(Tree *)List_First (node->children);
    first = *Cursor;
    Cursor=(Tree *)List_Next ((void *)Cursor);
    second = *Cursor;

    /* 必须先访问第二个孩子。  */
    if (!ir_visit_ast_node (second, code, cmpCurST))
        goto fail;
    if (!ir_visit_ast_node (first, code, cmpCurST))
        goto fail;

    /* 获取被定值变量的标识符。  */
    name = first;
    while (name->tnOper == TN_INDEX)
        name = *(Tree *)List_First(name->children);

    /* 被定值变量是否是常量？  */
    if  (name->sym->sdVar.sdvConst)
    {
        error (comp->cmpConfig.input_file_name, node->tnLineNo, "assignment of read-only variable '%s'", stGetSymName(name->sym));
        goto fail;
    }

    /* 确保操作数在虚拟寄存器中。  */
    sym = emit_load (second->sym, code, cmpCurST, second->tnLineNo, second->tnColumn);

    /* 自动类型转换。  */
    sym = emit_conversion_insns (sym, first->tnType, code, cmpCurST, second->tnLineNo, second->tnColumn);

    /* 左孩子是否是数组元素的寻址？  */
    if  (first->tnOper == TN_INDEX)
    {
        /* 赋值是否合法？  */
        if  (first->tnType->tdTypeKind > TYP_lastIntrins)
        {
            error (comp->cmpConfig.input_file_name, node->tnLineNo, "assignment to expression with array type");
            goto fail;
        }

        insn = IRInstEmitInst(IRINST_OP_astore, node->tnLineNo, node->tnColumn);
        IRInstSetOperand(insn, 1, emit_load (first->sym, code, cmpCurST, first->tnLineNo, first->tnColumn));
        IRInstSetOperand(insn, 0, name->sym);
        IRInstSetOperand(insn, 2, sym);
        IRInstSetOperand(insn, 3, name->sym);
#if defined(zenglj)
        sym = stDeclareSym(cmpCurST, NULL, SYM_VAR);
        sym->sdIsImplicit = TRUE;
        sym->sdType = stNewPtrType(CopyType(stGetBaseType (name->sym->sdType)), TRUE);
        IRInstSetOperand(insn, 4, sym);
#endif

        node->sym = second->sym;
        node->tnType = CopyType(node->sym->sdType);
    }
    else 
    {
        /* 产生store指令。  */
        insn = IRInstEmitInst(IRINST_OP_store, node->tnLineNo, node->tnColumn);
        IRInstSetOperand(insn, 1, sym);
        IRInstSetOperand(insn, 0, first->sym);

        node->sym = first->sym;
        node->tnType = CopyType(node->sym->sdType);
    }

    InterCodeAddInst_nobb(code, insn, TRUE);

    bSuccess = TRUE;

fail:

    TRACE_SEMANTICS (fprintf (stderr, "exit  VisitTN_ASG with %x", bSuccess));
    return bSuccess;
}

static BOOL VisitTN_LOG_OR(Tree node, InterCode code, SymTab cmpCurST)
{
    Tree first, second;
    BOOL bSuccess = FALSE;
    void *Cursor;
    int next;

    TRACE_SEMANTICS (fprintf (stderr, "enter VisitTN_LOG_OR()"));

    Cursor=List_First (node->children);
    first = *(Tree *)Cursor;
    Cursor=List_Next ((void *)Cursor);
    second = *(Tree *)Cursor;

    Cursor=List_First (node->children);
    first = *(Tree *)Cursor;
    Cursor=List_Next ((void *)Cursor);
    second = *(Tree *)Cursor;

    /* 访问两操作数。  */
    if (!ir_visit_ast_node (first, code, cmpCurST))
        goto fail;

    /* 确保操作数为布尔表达式。  */
    if  (first->sym)
    {
        if (!create_cmp_insn (first->sym, stCreateIconNode(cmpCurST, 0), TN_NE, first, code, cmpCurST))
            goto fail;
    }

    next = NEXT_INSN(code);

    if (!ir_visit_ast_node (second, code, cmpCurST))
        goto fail;

    /* 确保操作数为布尔表达式。  */
    if  (second->sym)
    {
        if (!create_cmp_insn (second->sym, stCreateIconNode(cmpCurST, 0), TN_NE, second, code, cmpCurST))
            goto fail;
    }

    /* 拉链回填。  */
    DoBackpatch (first->false_list, stCreateIconNode(cmpCurST, next));
    DoMerge (node->true_list, first->true_list, second->true_list);
    List_Join (node->false_list, second->false_list);

    if  (node->sym)
        node->tnType = CopyType(node->sym->sdType);
    else
        node->tnType = stAllocTypDef(TYP_BOOL);

    bSuccess = TRUE;

fail:

    TRACE_SEMANTICS (fprintf (stderr, "exit  VisitTN_LOG_OR with %x", bSuccess));
    return bSuccess;
}

static BOOL VisitTN_LOG_AND(Tree node, InterCode code, SymTab cmpCurST)
{
    Tree first, second;
    BOOL bSuccess = FALSE;
    void *Cursor;
    int next;

    TRACE_SEMANTICS (fprintf (stderr, "enter VisitTN_LOG_OR()"));

    Cursor=List_First (node->children);
    first = *(Tree *)Cursor;
    Cursor=List_Next ((void *)Cursor);
    second = *(Tree *)Cursor;

    Cursor=List_First (node->children);
    first = *(Tree *)Cursor;
    Cursor=List_Next ((void *)Cursor);
    second = *(Tree *)Cursor;

    /* 访问两操作数。  */
    if (!ir_visit_ast_node (first, code, cmpCurST))
        goto fail;

    /* 确保操作数为布尔表达式。  */
    if  (first->sym)
    {
        if (!create_cmp_insn (first->sym, stCreateIconNode(cmpCurST, 0), TN_NE, first, code, cmpCurST))
            goto fail;
    }

    next = NEXT_INSN(code);

    if (!ir_visit_ast_node (second, code, cmpCurST))
        goto fail;

    /* 确保操作数为布尔表达式。  */
    if  (second->sym)
    {
        if (!create_cmp_insn (second->sym, stCreateIconNode(cmpCurST, 0), TN_NE, second, code, cmpCurST))
            goto fail;
    }

    /* 拉链回填。  */
    DoBackpatch (first->true_list, stCreateIconNode(cmpCurST, next));
    List_Join (node->true_list, second->true_list);
    DoMerge (node->false_list, first->false_list, second->false_list);

    if  (node->sym)
        node->tnType = CopyType(node->sym->sdType);
    else
        node->tnType = stAllocTypDef(TYP_BOOL);

    bSuccess = TRUE;

fail:

    TRACE_SEMANTICS (fprintf (stderr, "exit  VisitTN_LOG_OR with %x", bSuccess));
    return bSuccess;
}

static BOOL VisitTN_INDEX(Tree node, InterCode code, SymTab cmpCurST)
{
    BOOL bSuccess = FALSE;
    Tree *Cursor;
    Tree first, second;
    SymDef tmpSym;
    Tree name;
    IRInst insn;
    SymDef op1;
    SymDef op2;
    ConstVal cval;

    TRACE_SEMANTICS (fprintf (stderr, "enter VisitTN_INDEX()"));

    Cursor=(Tree *)List_First (node->children);
    first = *Cursor;
    Cursor=(Tree *)List_Next ((void *)Cursor);
    second = *Cursor;

    if (!ir_visit_ast_node (first, code, cmpCurST))
        goto fail;
    if (!ir_visit_ast_node (second, code, cmpCurST))
        goto fail;

    node->tnType = CopyType(first->tnType->tdArr.tdaElem);

    /* 下标数量是否超过了数组维数？  */
    if  (node->lpParent->tnOper == TN_INDEX &&
         *(Tree *)List_First (node->lpParent->children) == node &&
         node->tnType->tdTypeKind <= TYP_lastIntrins)
    {
        error (comp->cmpConfig.input_file_name, node->tnLineNo, "subscripted value is not an array, pointer, or vector");
        goto fail;
    }

    /* 下标是否是整型变量？  */
    if  (second->sym->sdType->tdTypeKind != TYP_INT)
    {
        error (comp->cmpConfig.input_file_name, node->tnLineNo, "array subscript is not an integer");
        goto fail;
    }

    /* 计算偏移量。  */
    if  (first->tnOper == TN_INDEX)
    {
        op1 = emit_load(first->sym, code, cmpCurST, first->tnLineNo, first->tnColumn);
        op2 = emit_load(second->sym, code, cmpCurST, second->tnLineNo, second->tnColumn);

        tmpSym = FoldIntBinop(TN_ADD, op1, op2, node, cmpCurST);
        tmpSym = tmpSym ? tmpSym
                        : emit_binop (op1, op2, IRINST_OP_add, code, cmpCurST, node->tnLineNo, node->tnColumn);
    }
    else 
    {
        tmpSym = emit_load (second->sym, code, cmpCurST, second->tnLineNo, second->tnColumn);
    }

    if  (node->tnType->tdTypeKind == TYP_ARRAY)
    {
        op1 = emit_load(tmpSym, code, cmpCurST, node->tnLineNo, node->tnColumn);
        op2 = emit_load(node->tnType->tdArr.tdaDims->ddHiTree, code, cmpCurST, node->tnLineNo, node->tnColumn);

        node->sym = FoldIntBinop(TN_MUL, op1, op2, node, cmpCurST);
        node->sym = node->sym ? node->sym
                              : emit_binop (op1, op2, IRINST_OP_mul, code, cmpCurST, node->tnLineNo, node->tnColumn);
    }
    else
    {
        node->sym = tmpSym;
    }

    /* 我们需要对最上层的TN_INDEX结点做特殊处理。  */
    if  (node->lpParent->tnOper != TN_INDEX ||
         *(Tree *)List_Last (node->lpParent->children) == node)
    {
        /* 计算偏移量。  */
        op1 = emit_load(node->sym, code, cmpCurST, node->tnLineNo, node->tnColumn);
        op2 = stCreateIconNode(cmpCurST, 4);
        tmpSym = FoldIntBinop(TN_MUL, op1, op2, node, cmpCurST);
        tmpSym = tmpSym ? tmpSym
                        : emit_binop (op1, op2, IRINST_OP_mul, code, cmpCurST, node->tnLineNo, node->tnColumn);
        if  (node->lpParent->tnOper != TN_ASG || *(Tree *)List_First (node->lpParent->children) != node)
        {
            /* 根据结点类型产生add指令或aload指令。  */

            name = node;
            while (name->tnOper == TN_INDEX)
                name = *(Tree *)List_First(name->children);

            if  (tmpSym->sdVar.sdvConst &&
                 node->tnType->tdTypeKind != TYP_ARRAY &&
                 name->sym->sdVar.sdvConst)
            {
                /* 可以常量折叠。  */
                cval = GetConstVal(name->sym, GetConstVal(tmpSym, 0)->cvValue.cvIval / type_size(node->tnType));
                if  (cval)
                {
                    node->sym = cval->cvVtyp == TYP_FLOAT ? stCreateFconNode(cmpCurST, cval->cvValue.cvFval) : stCreateIconNode(cmpCurST, cval->cvValue.cvIval);
                }
                else
                {
                    node->sym = node->tnType->tdTypeKind == TYP_FLOAT ? stCreateFconNode(cmpCurST, 0.0f) : stCreateIconNode(cmpCurST, 0);
                }
            }
            else
            {
                /* 无法常量折叠。  */
                node->sym = stDeclareSym(cmpCurST, NULL, SYM_VAR);
                node->sym->sdIsImplicit = TRUE;
                node->sym->sdType = CopyType(node->tnType);
                insn = IRInstEmitInst(node->tnType->tdTypeKind == TYP_ARRAY ? IRINST_OP_addptr : IRINST_OP_aload, node->tnLineNo, node->tnColumn);
                IRInstSetOperand(insn, 0,  node->sym);
                IRInstSetOperand(insn, 1, name->sym);
                IRInstSetOperand(insn, 2, tmpSym);
                if  (node->tnType->tdTypeKind == TYP_ARRAY)
                    IRInstSetOperand(insn, 3, name->sym);
                InterCodeAddInst_nobb(code, insn, TRUE);
#if defined(zenglj)
                if  (node->tnType->tdTypeKind != TYP_ARRAY)
                {
                    tmpSym = stDeclareSym(cmpCurST, NULL, SYM_VAR);
                    tmpSym->sdIsImplicit = TRUE;
                    tmpSym->sdType = stNewPtrType(CopyType(stGetBaseType (name->sym->sdType)), TRUE);
                    IRInstSetOperand(insn, 3,  tmpSym);
                }
#endif
            }
        }
        else
        {
            /* 如果当前结点是赋值语句的左孩子，则astore指令由赋值语句产生。  */
            node->sym = tmpSym;
        }
    }

    bSuccess = TRUE;

fail:

    TRACE_SEMANTICS (fprintf (stderr, "exit  VisitTN_INDEX with %x", bSuccess));
    return bSuccess;
}

static BOOL VisitTN_CALL(Tree node, InterCode code, SymTab cmpCurST)
{
    Tree *Cursor;
    BOOL bSuccess = FALSE;
    IRInst insn;
    SymDef func;
    SymDef tmpSym;
    SymDef scope;
    LIST params = NULL;
    SymDef *iter;
    char *funcname = node->tnName.tnNameId;
    int nparams = List_Card (node->children);

    TRACE_SEMANTICS (fprintf (stderr, "enter VisitTN_CALL()"));

    if      (!strcmp (funcname, "starttime"))
    {
        funcname = "_sysy_starttime";
        nparams = 1;
    }
    else if (!strcmp (funcname, "stoptime"))
    {
        funcname = "_sysy_stoptime";
        nparams = 1;
    }

    params = List_Create();

    func = stLookupSym(cmpCurST, funcname, SYM_FNC);
    if  (!func)
    {
        /* 函数没有声明。  */
        warning (comp->cmpConfig.input_file_name, node->tnLineNo, "implicit declaration of function '%s'", funcname);
    }
    else if (func->sdType->tdTypeKind != TYP_FNC)
    {
        /* 调用的不是函数。  */
        error (comp->cmpConfig.input_file_name, node->tnLineNo, "called object '%s' is not a function or function pointer", funcname);
        goto fail;
    }
    else
    {
        if  (nparams > List_Card(func->sdType->tdFnc.tdfArgs))
        {
            /* 实参个数大于形参个数。  */
            warning (comp->cmpConfig.input_file_name, node->tnLineNo, "too many arguments to function call, expected %d, have %d", List_Card(func->sdType->tdFnc.tdfArgs), nparams);
        }
        else if (nparams < List_Card(func->sdType->tdFnc.tdfArgs))
        {
            /* 实参个数小于形参个数。  */
            warning (comp->cmpConfig.input_file_name, node->tnLineNo, "too few arguments to function call, expected %d, have %d", List_Card(func->sdType->tdFnc.tdfArgs), nparams);
        }
    }

    /* 访问函数的参数。  */
    for(  Cursor=(Tree *)List_Last(node->children)
        ;  Cursor!=NULL
        ;  Cursor = (Tree *)List_Prev((void *)Cursor)
        )
        if (!ir_visit_ast_node (*Cursor, code, cmpCurST))
            goto fail;

    /* 如果函数没有声明，则添加函数声明。  */
    if  (!func)
    {
        /* 备份当前作用域。  */
        scope = cmpCurST->cmpCurScp;

        /* 退回到文件作用域。  */
        while (!in_global_scope (cmpCurST->cmpCurScp))
        {
            cmpCurST->cmpCurScp = cmpCurST->cmpCurScp->sdParent;
        }

        /* 函数声明。  */
        func = stDeclareSym(cmpCurST, funcname, SYM_FNC);
        func->sdIsImplicit = TRUE;
        func->sdType = stNewFncType(stAllocTypDef(TYP_INT));

        /* 进入函数作用域。  */
        cmpCurST->cmpCurScp = func;

        /* 遍历函数的参数。  */
        for(  Cursor=(Tree *)List_Last(node->children)
            ;  Cursor!=NULL
            ;  Cursor = (Tree *)List_Prev((void *)Cursor)
            )
        {
            tmpSym = stDeclareSym(cmpCurST, NULL, SYM_VAR);
            tmpSym->sdIsImplicit = TRUE;
            tmpSym->sdVar.sdvArgument = TRUE;
            tmpSym->sdVar.sdvLocal = TRUE;
            tmpSym->sdType = CopyType((*Cursor)->tnType);
            if  (tmpSym->sdType->tdTypeKind == TYP_ARRAY)
            {
                /* 参数是数组，将其改成符合要求的形式。  */
                tmpSym->sdType->tdArr.tdaDims->ddSize = 0;
                tmpSym->sdType->tdArr.tdaDims->ddHiTree = stCreateIconNode(cmpCurST, 0);
            }
            *(SymDef *)List_NewLast(func->sdType->tdFnc.tdfArgs, sizeof (SymDef)) = tmpSym;
        }

        /* 恢复到原始作用域。  */
        cmpCurST->cmpCurScp = scope;
    }

    for(  Cursor=(Tree *)List_Last(node->children), iter = (SymDef *)List_First (func->sdType->tdFnc.tdfArgs)
        ;  Cursor!=NULL && iter!=NULL
        ;  Cursor = (Tree *)List_Prev((void *)Cursor), iter = (SymDef *)List_Next ((void *)iter)
        )
    {
        tmpSym = emit_load((*Cursor)->sym, code, cmpCurST, (*Cursor)->tnLineNo, (*Cursor)->tnColumn);
        tmpSym = emit_conversion_insns(tmpSym, (*iter)->sdType, code, cmpCurST, (*Cursor)->tnLineNo, (*Cursor)->tnColumn);
        if  (tmpSym->sdType->tdTypeKind > TYP_lastIntrins &&
             ! tmpSym->sdIsImplicit)
        {
            insn = IRInstEmitInst(IRINST_OP_addptr, node->tnLineNo, node->tnColumn);
            IRInstSetOperand(insn, 1, tmpSym);
            IRInstSetOperand(insn, 2, stCreateIconNode(cmpCurST, 0));
            IRInstSetOperand(insn, 3, tmpSym);
            tmpSym = stDeclareSym(cmpCurST, NULL, SYM_VAR);
            tmpSym->sdIsImplicit = TRUE;
            tmpSym->sdType = CopyType(IRInstGetOperand(insn, 1)->var->sdType);
            IRInstSetOperand(insn, 0, tmpSym);
            InterCodeAddInst_nobb(code, insn, TRUE);
        }
        *(SymDef *)List_NewLast(params, sizeof (SymDef)) = tmpSym;
    }

    for(  iter=(SymDef *)List_First(params)
        ;  iter!=NULL
        ;  iter = (SymDef *)List_Next((void *)iter)
        )
    {
        insn = IRInstEmitInst(IRINST_OP_param, node->tnLineNo, node->tnColumn);
        IRInstSetOperand(insn, 0, *iter);
        InterCodeAddInst_nobb(code, insn, TRUE);
    }

    if  (!strcmp (funcname, "_sysy_stoptime") ||
         !strcmp (funcname, "_sysy_starttime"))
    {
        insn = IRInstEmitInst(IRINST_OP_param, node->tnLineNo, node->tnColumn);
        IRInstSetOperand(insn, 0, stCreateIconNode(cmpCurST, node->tnLineNo));
        InterCodeAddInst_nobb(code, insn, TRUE);
    }

    node->sym = stDeclareSym(cmpCurST, NULL, SYM_VAR);
    node->sym->sdIsImplicit = TRUE;
    node->sym->sdType = CopyType(func->sdType->tdFnc.tdfRett);
    node->tnType = CopyType(node->sym->sdType);
    insn = IRInstEmitInst(IRINST_OP_call, node->tnLineNo, node->tnColumn);
    IRInstSetOperand(insn, 0, node->sym);
    IRInstSetOperand(insn, 1, func);
    IRInstSetOperand(insn, 2, stCreateIconNode(cmpCurST, nparams));
    InterCodeAddInst_nobb(code, insn, TRUE);

    bSuccess = TRUE;

fail:

    List_Destroy(&params);
    TRACE_SEMANTICS (fprintf (stderr, "exit  VisitTN_CALL with %x", bSuccess));
    return bSuccess;
}

static BOOL VisitTN_EXPR_STMT(Tree node, InterCode code, SymTab cmpCurST)
{
    return TRUE;
}

static BOOL VisitTN_IF(Tree node, InterCode code, SymTab cmpCurST)
{
    Tree first, second, third;
    BOOL bSuccess = FALSE;
    Tree *Cursor;
    int next2;
    int next;
    IRInst insn;

    TRACE_SEMANTICS (fprintf (stderr, "enter VisitTN_IF()"));

    third = NULL;

    Cursor=(Tree *)List_First (node->children);
    first = *Cursor;
    Cursor=(Tree *)List_Next ((void *)Cursor);
    second = *Cursor;
    Cursor=(Tree *)List_Next ((void *)Cursor);
    if  (Cursor)
        third = *Cursor;

    /* 单目运算符对条件表达式不产生影响，可删除之。  */
    while (first->tnOper == TN_NOP || first->tnOper == TN_NEG)
    {
        Cursor = (Tree *)List_First (first->children);
        *(Tree *)List_First (node->children) = *Cursor;
        parseDeleteNode(first);
        first = *Cursor;
        first->lpParent = node;
    }

    if (!ir_visit_ast_node (first, code, cmpCurST))
        goto fail;

    /* 确保条件为布尔表达式。  */
    if  (first->sym)
    {
        if (!create_cmp_insn (first->sym, stCreateIconNode(cmpCurST, 0), TN_NE, first, code, cmpCurST))
            goto fail;
    }

    next = NEXT_INSN(code);

    if (!ir_visit_ast_node (second, code, cmpCurST))
        goto fail;

    /* 根据情况进行回填。  */
    if  (third)
    {
        insn = IRInstEmitInst(IRINST_OP_goto, node->tnLineNo, node->tnColumn);
        InterCodeAddInst_nobb(code, insn, TRUE);
        next2 = NEXT_INSN(code);
        if (!ir_visit_ast_node (third, code, cmpCurST))
            goto fail;

        DoBackpatch (first->true_list, stCreateIconNode(cmpCurST, next));
        DoBackpatch (first->false_list, stCreateIconNode(cmpCurST, next2));
        DoMerge (node->next_list, second->next_list, third->next_list);
        *(IRInst *)List_NewLast(node->next_list, sizeof (IRInst)) = insn;
    }
    else
    {
        DoBackpatch (first->true_list, stCreateIconNode(cmpCurST, next));
        DoMerge (node->next_list, first->false_list, second->next_list);
    }

    bSuccess = TRUE;

fail:

    TRACE_SEMANTICS (fprintf (stderr, "exit  VisitTN_IF with %x", bSuccess));
    return bSuccess;
}

static BOOL VisitTN_EmptyStmt(Tree node, InterCode code, SymTab cmpCurST)
{
    TRACE_SEMANTICS (fprintf (stderr, "enter VisitTN_EmptyStmt()"));
    /* do nothing */
    TRACE_SEMANTICS (fprintf (stderr, "exit  VisitTN_EmptyStmt with %x", TRUE));
    return TRUE;
}

static BOOL VisitTN_DO(Tree node, InterCode code, SymTab cmpCurST)
{
    TRACE_SEMANTICS (fprintf (stderr, "enter VisitTN_DO()"));
    /* do nothing */
    TRACE_SEMANTICS (fprintf (stderr, "exit  VisitTN_DO with %x", TRUE));
    return TRUE;
}

static BOOL VisitTN_WHILE(Tree node, InterCode code, SymTab cmpCurST)
{
    Tree first, second;
    BOOL bSuccess = FALSE;
    Tree *Cursor;
    SymDef next2;
    SymDef next;
    IRInst insn;

    TRACE_SEMANTICS (fprintf (stderr, "enter VisitTN_IF()"));

    Cursor=(Tree *)List_First (node->children);
    first = *Cursor;
    Cursor=(Tree *)List_Next ((void *)Cursor);
    second = *Cursor;

    next = stCreateIconNode(cmpCurST,NEXT_INSN(code));
    /* 设置它为continue跳转到的指令。  */
    node->sym = next;

    /* 单目运算符对条件表达式不产生影响，可删除之。  */
    while (first->tnOper == TN_NOP || first->tnOper == TN_NEG)
    {
        Cursor = (Tree *)List_First (first->children);
        *(Tree *)List_First (node->children) = *Cursor;
        parseDeleteNode(first);
        first = *Cursor;
        first->lpParent = node;
    }

    if (!ir_visit_ast_node (first, code, cmpCurST))
        goto fail;

    /* 确保条件为布尔表达式。  */
    if  (first->sym)
    {
        if (!create_cmp_insn (first->sym, stCreateIconNode(cmpCurST, 0), TN_NE, first, code, cmpCurST))
            goto fail;
    }

    next2 = stCreateIconNode(cmpCurST,NEXT_INSN(code));

    if (!ir_visit_ast_node (second, code, cmpCurST))
        goto fail;

    /* 回填。  */
    DoBackpatch (second->next_list, next);
    DoBackpatch (first->true_list,  next2);
    List_Join(node->next_list, first->false_list);
    insn = IRInstEmitInst(IRINST_OP_goto, node->tnLineNo, node->tnColumn);
    IRInstSetOperand(insn, 0, next);
    InterCodeAddInst_nobb(code, insn, TRUE);

    bSuccess = TRUE;

fail:

    TRACE_SEMANTICS (fprintf (stderr, "exit  VisitTN_IF with %x", bSuccess));
    return bSuccess;
}

static BOOL VisitTN_BREAK(Tree node, InterCode code, SymTab cmpCurST)
{
    BOOL bSuccess = FALSE;
    IRInst insn;
    Tree loop;

    TRACE_SEMANTICS (fprintf (stderr, "enter VisitTN_BREAK()"));

    /* (1) 跟踪外围循环语句S，
       (2) 为该break语句生成未完成的跳转指令，
       (3) 将这些指令放到S.nextlist中。  */

    loop = node;
    while (loop && loop->tnOper != TN_WHILE)
    {
        loop = loop->lpParent;
    }

    if  (!loop)
    {
        /* 没找到外围循环语句。  */
        error (comp->cmpConfig.input_file_name, node->tnLineNo, "break statement not within loop");
        goto fail;
    }

    insn = IRInstEmitInst(IRINST_OP_goto, node->tnLineNo, node->tnColumn);
    InterCodeAddInst_nobb(code, insn, TRUE);
    *(IRInst *)List_NewLast(loop->next_list, sizeof (IRInst)) = insn;

    bSuccess = TRUE;

fail:

    TRACE_SEMANTICS (fprintf (stderr, "exit  VisitTN_BREAK with %x", bSuccess));
    return bSuccess;
}

static BOOL VisitTN_CONTINUE(Tree node, InterCode code, SymTab cmpCurST)
{
    BOOL bSuccess = FALSE;
    IRInst insn;
    Tree loop;

    TRACE_SEMANTICS (fprintf (stderr, "enter VisitTN_CONTINUE()"));

    loop = node;
    while (loop && loop->tnOper != TN_WHILE)
    {
        loop = loop->lpParent;
    }

    if  (!loop)
    {
        /* 没找到外围循环语句。  */
        error (comp->cmpConfig.input_file_name, node->tnLineNo, "continue statement not within loop");
        goto fail;
    }

    insn = IRInstEmitInst(IRINST_OP_goto, node->tnLineNo, node->tnColumn);
    IRInstSetOperand(insn, 0, loop->sym);
    InterCodeAddInst_nobb(code, insn, TRUE);

    bSuccess = TRUE;

fail:

    TRACE_SEMANTICS (fprintf (stderr, "exit  VisitTN_CONTINUE with %x", bSuccess));
    return bSuccess;
}

static BOOL VisitTN_RETURN(Tree node, InterCode code, SymTab cmpCurST)
{
    BOOL bSuccess = FALSE;
    Tree first;
    SymDef func;
    IRInst insn;
    SymDef  temp;

    TRACE_SEMANTICS (fprintf (stderr, "enter VisitTN_RETURN()"));

    /* 找到函数的符号表项。  */
    func = cmpCurST->cmpCurScp;
    while   (func->sdType->tdTypeKind != TYP_FNC)
    {
        func = func->sdParent;
    }

    if  (!List_IsEmpty(node->children) &&
         func->sdType->tdFnc.tdfRett->tdTypeKind == TYP_VOID)
    {
        warning (comp->cmpConfig.input_file_name, node->tnLineNo, "'return' with a value, in function returning void");
    }
    if  (List_IsEmpty(node->children) &&
         func->sdType->tdFnc.tdfRett->tdTypeKind != TYP_VOID)
    {
        warning (comp->cmpConfig.input_file_name, node->tnLineNo, "'return' with no value, in function returning non-void");
    }

    /* 将返回值存储到局部变量。  */
    if  (!List_IsEmpty(node->children) &&
         func->sdType->tdFnc.tdfRett->tdTypeKind != TYP_VOID)
    {
        first=*(Tree *)List_First (node->children);

        if (!ir_visit_ast_node (first, code, cmpCurST))
            goto fail;

        insn = IRInstEmitInst(IRINST_OP_store, node->tnLineNo, node->tnColumn);
        IRInstSetOperand(insn, 0, func->sdFnc.retv);
        temp = emit_load(first->sym, code, cmpCurST, first->tnLineNo, first->tnColumn);
        temp = emit_conversion_insns(temp, func->sdType->tdFnc.tdfRett, code, cmpCurST, first->tnLineNo, first->tnColumn);
        if ( !temp)
        {
            fatal("internal compiler error");
            goto fail;
        }
        IRInstSetOperand(insn, 1, temp);
        InterCodeAddInst_nobb(code, insn, TRUE);
    }

    insn = IRInstEmitInst(IRINST_OP_goto, node->tnLineNo, node->tnColumn);

    /* 找到函数定义结点用于拉链回填。  */
    while (node && node->tnOper != TN_FuncDef)
    {
        node = node->lpParent;
    }

    InterCodeAddInst_nobb(code, insn, TRUE);
    *(IRInst *)List_NewLast(node->next_list, sizeof (IRInst)) = insn;

    bSuccess = TRUE;

fail:

    TRACE_SEMANTICS (fprintf (stderr, "exit  VisitTN_RETURN with %x", bSuccess));
    return bSuccess;
}

static BOOL VisitTN_BLOCK(Tree node, InterCode code, SymTab cmpCurST)
{
    Tree *Cursor;
    BOOL bSuccess = FALSE;
    SymDef cmpCurScp;
    int next;
    IRInst insn;

    TRACE_SEMANTICS (fprintf (stderr, "enter VisitTN_BLOCK()"));

    /* 进入作用域。  */
    cmpCurScp = cmpCurST->cmpCurScp;
    if  (node->lpParent->tnOper == TN_FuncDef)
    {
        cmpCurST->cmpCurScp = node->lpParent->sym;
    }
    else
    {
        cmpCurST->cmpCurScp = stDeclareSym(cmpCurST, NULL, SYM_SCOPE);
        node->sym = cmpCurST->cmpCurScp;
        node->sym->sdType = stAllocTypDef(TYP_UNDEF);
    }
    
    /* 插入lexical block标记。  */
    insn = IRInstEmitInst(IRINST_OP_begin_block, node->tnLineNo, node->tnColumn);
    IRInstSetOperand(insn, 0, stCreateIconNode(cmpCurST, cmpCurST->cmpCurScp->uid));
    InterCodeAddInst_nobb(code, insn, TRUE);

    /* 遍历块中的语句。  */
    for(  Cursor=(Tree *)List_Last(node->children)
        ;  Cursor!=NULL
        ;  Cursor = (Tree *)List_Prev((void *)Cursor)
        )
    {
        next = NEXT_INSN(code);
        if (!ir_visit_ast_node (*Cursor, code, cmpCurST))
            goto fail;
        DoBackpatch (node->next_list, stCreateIconNode(cmpCurST, next));
        List_Clear (node->next_list);
        List_Join (node->next_list, (*Cursor)->next_list);
    }
    
    bSuccess = TRUE;

fail:

    /* 插入lexical block标记。  */
    insn = IRInstEmitInst(IRINST_OP_end_block, node->tnLineNo, node->tnColumn);
    IRInstSetOperand(insn, 0, stCreateIconNode(cmpCurST, cmpCurST->cmpCurScp->uid));
    InterCodeAddInst_nobb(code, insn, TRUE);

    /* 退出作用域。  */
    cmpCurST->cmpCurScp = cmpCurScp;

    TRACE_SEMANTICS (fprintf (stderr, "exit  VisitTN_BLOCK with %x", bSuccess));
    return bSuccess;
}

static BOOL VisitTN_SLV_INIT(Tree node, InterCode code, SymTab cmpCurST)
{
    Tree *Cursor;
    BOOL bSuccess = FALSE;
    IRInst insn;
    SymDef func;
    SymDef   temp;

    TRACE_SEMANTICS (fprintf (stderr, "enter VisitTN_SLV_INIT()"));

    /* TODO: 将常量放到全局.  */
    if  (!is_global_var(node->lpParent->sym))
    {
        func = stLookupSym(cmpCurST, "memset", SYM_FNC);
        if  (!func)
        {
            fatal("internal compiler error");
            goto fail;
        }

        insn = IRInstEmitInst(IRINST_OP_addptr, node->tnLineNo, node->tnColumn);
        IRInstSetOperand (insn, 1, node->lpParent->sym);
        IRInstSetOperand (insn, 2, stCreateIconNode(cmpCurST, 0));
        IRInstSetOperand (insn, 3, node->lpParent->sym);
        temp = stDeclareSym(cmpCurST, NULL, SYM_VAR);
        temp->sdIsImplicit = TRUE;
        temp->sdType = CopyType(IRInstGetOperand(insn, 1)->var->sdType);
        IRInstSetOperand (insn, 0, temp);
        InterCodeAddInst_nobb(code, insn, TRUE);

        insn = IRInstEmitInst(IRINST_OP_param, node->tnLineNo, node->tnColumn);
        IRInstSetOperand(insn, 0, temp);
        InterCodeAddInst_nobb(code, insn, TRUE);

        insn = IRInstEmitInst(IRINST_OP_param, node->tnLineNo, node->tnColumn);
        IRInstSetOperand(insn, 0, stCreateIconNode(cmpCurST, 0));
        InterCodeAddInst_nobb(code, insn, TRUE);

        insn = IRInstEmitInst(IRINST_OP_param, node->tnLineNo, node->tnColumn);
        IRInstSetOperand(insn, 0, stCreateIconNode(cmpCurST, type_size(node->lpParent->sym->sdType)));
        InterCodeAddInst_nobb(code, insn, TRUE);

        temp = stDeclareSym(cmpCurST, NULL, SYM_VAR);
        temp->sdIsImplicit = TRUE;
        temp->sdType = stAllocTypDef(TYP_VOID);
        insn = IRInstEmitInst(IRINST_OP_call, node->tnLineNo, node->tnColumn);
        IRInstSetOperand(insn, 0, temp);
        IRInstSetOperand(insn, 1, func);
        IRInstSetOperand(insn, 2, stCreateIconNode(cmpCurST, 3));
        InterCodeAddInst_nobb(code, insn, TRUE);
    }

    /* 若decl_initializer函数成功，则Cursor应指向node。  */
    Cursor = decl_initializer(node->lpParent->sym, (Tree *)List_Last(node->lpParent->children), node->lpParent->tnType, 0, 1, code, cmpCurST);
    if  (!Cursor)
        goto fail;

    bSuccess = TRUE;

fail:

    TRACE_SEMANTICS (fprintf (stderr, "exit  VisitTN_SLV_INIT with %x", bSuccess));
    return bSuccess;
}

static BOOL VisitTN_CompUnit(Tree node, InterCode code, SymTab cmpCurST)
{
    Tree *Cursor;
    BOOL bSuccess = FALSE;
    SymDef outerScp;

    TRACE_SEMANTICS (fprintf (stderr, "enter VisitTN_CompUnit()"));

    outerScp = cmpCurST->cmpCurScp;
    cmpCurST->cmpCurScp = stDeclareSym(cmpCurST, NULL, SYM_COMPUNIT);
    cmpCurST->cmpCurScp->sdType = stAllocTypDef(TYP_UNDEF);

    for(  Cursor=(Tree *)List_First(node->children)
        ;  Cursor!=NULL
        ;  Cursor = (Tree *)List_Next((void *)Cursor)
        )
        if (!ir_visit_ast_node (*Cursor, code, cmpCurST))
            goto fail;

    cmpCurST->cmpCurScp = outerScp;

    bSuccess = TRUE;

fail:

    TRACE_SEMANTICS (fprintf (stderr, "exit  VisitTN_CompUnit with %x", bSuccess));
    return bSuccess;
}

static BOOL VisitTN_VarDecl(Tree node, InterCode code, SymTab cmpCurST)
{
    Tree *Cursor;
    BOOL bSuccess = FALSE;

    TRACE_SEMANTICS (fprintf (stderr, "enter VisitTN_VarDecl()"));

    for(  Cursor=(Tree *)List_Last(node->children)
        ;  Cursor!=NULL
        ;  Cursor = (Tree *)List_Prev((void *)Cursor)
        )
        if (!ir_visit_ast_node (*Cursor, code, cmpCurST))
            goto fail;

    bSuccess = TRUE;

fail:

    TRACE_SEMANTICS (fprintf (stderr, "exit  VisitTN_VarDecl with %x", bSuccess));
    return bSuccess;
}

static BOOL VisitTN_VarDef(Tree node, InterCode code, SymTab cmpCurST)
{
    Tree first, second = NULL; /* 当前结点的两个孩子  */
    BOOL bSuccess = FALSE;     /* 成功/失败标志  */
    Tree *Cursor;              /* 用于获取当前结点孩子的游标  */
    Tree temp;                 /* 用于创建数组的内情向量的临时树结点  */
    DimDef dim = NULL;         /* 数组的内情向量  */
    Tree size;                 /* 数组当前维度的大小  */
    Tree name;                 /* 要定义变量的标识符  */
    TypDef type;               /* 要定义变量的类型  */
    SymDef sym;                /* 临时符号表项  */

    TRACE_SEMANTICS (fprintf (stderr, "enter VisitTN_VarDef()"));

    /*
     *
     *          VarDef
     *            |
     *            []
     *           / \
     *          []  1        int e[4][2][1]
     *         / \
     *        []  2
     *       / \
     *      e   4
     */

    /* 找到第一个孩子。  */
    Cursor=(Tree *)List_First (node->children);
    first = *Cursor;

    /* 获取变量定义的标识符。  */
    name = first;
    while (name->tnOper != TN_NAME)
        name = *(Tree *)List_First(name->children);

    /* 变量是否已经被定义过？  */
    sym = stLookupSym(cmpCurST, name->tnName.tnNameId, SYM_VAR);
    if  (sym && sym->sdParent == cmpCurST->cmpCurScp)
    {
        error (comp->cmpConfig.input_file_name, node->tnLineNo, "redefinition of '%s'", name->tnName.tnNameId);
        goto fail;
    }

    /* 访问TN_NAME结点。  */
    if (!ir_visit_ast_node (name, code, cmpCurST))
        goto fail;

    temp = first;
    type = stAllocTypDef(node->lpParent->tnVtyp);
    while (temp->tnOper != TN_NAME)
    {
        size = *(Tree *)List_Last(temp->children);

        /* 访问这个结点  */
        if (!ir_visit_ast_node (size, code, cmpCurST))
            goto fail;
        if ( !size->sym)
        {
            fatal("internal compiler error");
            goto fail;
        }

        /* 数组的大小是不是常数？  */
        if ( !size->sym->sdVar.sdvConst)
        {
            error(comp->cmpConfig.input_file_name, node->tnLineNo, "variable length array declaration not allowed");
            goto fail;
        }

        /* 数组的大小是不是整数？  */
        if ( !varTypeIsIntegral(size->sym->sdType->tdTypeKind))
        {
            error(comp->cmpConfig.input_file_name, node->tnLineNo, "size of array '%s' has non-integer type", name->tnName.tnNameId);
            goto fail;
        }

        /* 插入内情向量。  */
        dim = stNewDimDesc(GetConstVal(size->sym, 0)->cvValue.cvIval, size->sym);
        type = stNewArrType(dim, type);

        temp = *(Tree *)List_First(temp->children);
    }

    /* 将变量插入符号表。  */
    node->sym = stDeclareSym(cmpCurST, name->tnName.tnNameId, SYM_VAR);
    node->sym->sdType = type;
    node->tnType = CopyType(type);
    node->sym->column = name->tnColumn;
    node->sym->line = name->tnLineNo;

    /* 是否是参数声明？  */
    if ( TESTFLAG(node->tnFlags, TNF_VAR_ARG))
    {
       node->sym->sdVar.sdvArgument = TRUE;
    }

    /* 是否是常量？  */
    if ( TESTFLAG(node->tnFlags, TNF_VAR_CONST))
    {
       node->sym->sdVar.sdvConst = TRUE;
    }

    /* 是否是局部变量？  */
    if ( cmpCurST->cmpCurScp->sdSymKind != SYM_COMPUNIT)
    {
       node->sym->sdVar.sdvLocal = TRUE;
    }

    /* 有没有初始化？  */
    Cursor = (Tree *)List_Next((void *)Cursor);
    if ( NULL != Cursor )
    {
       node->sym->sdVar.sdvHadInit = TRUE;
       second = *Cursor;
    }

    /* 访问初始化结点。  */
    if (
        second != NULL &&
        !ir_visit_ast_node (second, code, cmpCurST)
    ) {
        goto fail;
    }

    bSuccess = TRUE;

fail:

    TRACE_SEMANTICS (fprintf (stderr, "exit  VisitTN_VarDef with %x", bSuccess));
    return bSuccess;
}

static BOOL VisitTN_ConstInitVal(Tree node, InterCode code, SymTab cmpCurST)
{
    Tree *Cursor;
    BOOL bSuccess;
    TRACE_SEMANTICS (fprintf (stderr, "enter VisitTN_SLV_INIT()"));
    Cursor = decl_initializer(node->lpParent->sym, (Tree *)List_Last(node->lpParent->children), node->lpParent->tnType, 0, 1, code, cmpCurST);
    /* 若decl_initializer函数成功，则Cursor应指向node。  */
    bSuccess = !!Cursor;
    TRACE_SEMANTICS (fprintf (stderr, "exit  VisitTN_SLV_INIT with %x", bSuccess));
    return bSuccess;
}

static BOOL VisitTN_InitVal(Tree node, InterCode code, SymTab cmpCurST)
{
    Tree *Cursor;
    BOOL bSuccess;
    TRACE_SEMANTICS (fprintf (stderr, "enter VisitTN_SLV_INIT()"));
    Cursor = decl_initializer(node->lpParent->sym, (Tree *)List_Last(node->lpParent->children), node->lpParent->tnType, 0, 1, code, cmpCurST);
    /* 若decl_initializer函数成功，则Cursor应指向node。  */
    bSuccess = !!Cursor;
    TRACE_SEMANTICS (fprintf (stderr, "exit  VisitTN_SLV_INIT with %x", bSuccess));
    return bSuccess;
}

static BOOL VisitTN_FuncBody(Tree node, InterCode code, SymTab cmpCurST)
{
    Tree first, second, parent; /* 当前结点的两个孩子和双亲  */
    BOOL bSuccess = FALSE;      /* 成功/失败标志  */
    Tree *Cursor;               /* 用于获取当前结点孩子的游标  */
    SymDef sym;                 /* 从符号表中找到的符号  */
    SymDef cmpCurScp;           /* 用于备份当前的作用域  */

    TRACE_SEMANTICS (fprintf (stderr, "enter VisitTN_FuncBody()"));

    /* 保存当前的作用域，在结束后回退到当前作用域。  */
    parent = node->lpParent;
    cmpCurScp = cmpCurST->cmpCurScp;

    /* 将函数插入符号表。  */
    node->sym = stDeclareSym(cmpCurST, NULL, SYM_FNC);
    node->sym->sdIsDefined = (parent->tnOper == TN_FuncDef) ? TRUE : FALSE;
    node->sym->sdType = stNewFncType(stAllocTypDef(parent->tnVtyp));

    /* 进入到函数的作用域，因为参数必须定义在函数的作用域。  */
    cmpCurST->cmpCurScp = node->sym;

    /* 访问函数标识符。  */
    Cursor=(Tree *)List_First (node->children);
    first = *Cursor;
    if (!ir_visit_ast_node (first, code, cmpCurST))
        goto fail;

    /* 访问函数参数列表。  */
    Cursor = (Tree *)List_Next((void *)Cursor);
    second = *Cursor;
    if (!ir_visit_ast_node (second, code, cmpCurST))
        goto fail;

    /* 此标识符以前是否被定义/声明过？  */
    sym = stLookupSym(cmpCurST, first->tnName.tnNameId, SYM_FNC);
    if (sym)
    {
        /* 以前定义的不是函数。  */
        if  (sym->sdSymKind != SYM_FNC)
        {
            error(comp->cmpConfig.input_file_name, node->tnLineNo, "redefinition of '%s' as different kind of symbol", sym->sdName);
            goto fail;
        }

        /* 以前定义/声明的函数与现在定义/声明的函数类型不一致。  */
        else if (!stMatchTypes(sym->sdType, node->sym->sdType))
        {
            error(comp->cmpConfig.input_file_name, node->tnLineNo, "conflicting types for '%s'", sym->sdName);
            goto fail;
        }

        /* 函数定义了多次。  */
        else if (sym->sdIsDefined && node->sym->sdIsDefined)
        {
            error(comp->cmpConfig.input_file_name, node->tnLineNo, "redefinition of '%s'", sym->sdName);
            goto fail;
        }

        /* 曾经对此函数进行声明，现在对其定义，则删除以前的声明。  */
        else if (node->sym->sdIsDefined)
        {
            IRInst *curs;
            int i;

            for(  curs=(IRInst *) List_First(code->code)
               ;  curs!=NULL 
               ;  curs = (IRInst *) List_Next((void *)curs)
               )
            {
                for (i = 0; i < IRInstGetNumOperands (*curs); i++)
                {
                    if  (IRInstGetOperand(*curs, i)->var == sym)
                    {
                        IRInstSetOperand(*curs, i, node->sym);
                    }
                }
            }
            stRemoveSym (sym);
        }

        /* 曾经对此函数进行声明/定义，现在对其声明，则删除现在的函数声明。  */
        else
        {
            stRemoveSym(node->sym);
            node->sym = sym;
        }
    }

    /* 设置函数名称。  */
    stSetName(node->sym, first->tnName.tnNameId);

    /* 是否是入口函数？  */
    if ( !strcmp (first->tnName.tnNameId, "main") )
    {
        node->sym->sdFnc.sdfEntryPt = TRUE;
    }

    if  (parent->tnOper == TN_FuncDef)
    {
        parent->sym = node->sym;
        node->sym->column = first->tnColumn;
        node->sym->line = first->tnLineNo;
    }

    node->tnType = CopyType(node->sym->sdType);

    bSuccess = TRUE;

fail:

    /* 回退作用域。  */
    cmpCurST->cmpCurScp = cmpCurScp;
    TRACE_SEMANTICS (fprintf (stderr, "exit  VisitTN_FuncBody with %x", bSuccess));
    return bSuccess;
}

static BOOL VisitTN_FuncDef(Tree node, InterCode code, SymTab cmpCurST)
{
    BOOL bSuccess = FALSE;
    void *Cursor;
    IRInst entry, exit, move, insn;
    SymDef cmpCurScp;
    Tree first, second;

    TRACE_SEMANTICS (fprintf (stderr, "enter VisitTN_FuncDef()"));

    /* 产生entry和exit指令。  */
    entry = IRInstEmitInst(IRINST_OP_entry, node->tnLineNo, node->tnColumn);
    exit = IRInstEmitInst(IRINST_OP_exit, node->tnLineNo, node->tnColumn);

    Cursor=List_First (node->children);
    first = *(Tree *)Cursor;
    Cursor = List_Next((void *)Cursor);
    second = *(Tree *)Cursor;

    if (!ir_visit_ast_node (first, code, cmpCurST))
        goto fail;

    /* 插入一个临时局部变量作为返回值。  */
    cmpCurScp = cmpCurST->cmpCurScp;
    cmpCurST->cmpCurScp = node->sym;
    node->sym->sdFnc.retv = stDeclareSym (cmpCurST, NULL, SYM_VAR);
    node->sym->sdFnc.retv->sdIsImplicit = TRUE;
    node->sym->sdFnc.retv->sdVar.sdvLocal = TRUE;
    node->sym->sdFnc.retv->sdType = CopyType (node->sym->sdType->tdFnc.tdfRett);
    cmpCurST->cmpCurScp = cmpCurScp;

    /* 产生复制指令，用于对返回值初始化。  */
    move = IRInstEmitInst (IRINST_OP_store, node->tnLineNo, node->tnColumn);
    IRInstSetOperand (move, 0, node->sym->sdFnc.retv);

    /* 必须在访问函数体之前插入entry指令。  */
    InterCodeAddInst_nobb (code, entry, TRUE);

    /* 为形式参数产生fparam指令。  */
    for(  Cursor=List_First(node->sym->sdType->tdFnc.tdfArgs)
       ;  Cursor!=NULL
       ;  Cursor = List_Next((void *)Cursor)
       )
    {
        insn = IRInstEmitInst(IRINST_OP_fparam, node->tnLineNo, node->tnColumn);
        IRInstSetOperand(insn, 0, *(SymDef *)Cursor);
        InterCodeAddInst_nobb(code, insn, TRUE);
    }

    /* 插入初始化返回值的指令。  */
    InterCodeAddInst_nobb (code, move, TRUE);

    /* 访问函数体。  */
    if (!ir_visit_ast_node (second, code, cmpCurST))
        goto fail;

    /* 设置entry指令的函数。  */
    IRInstSetOperand(entry, 0, node->sym);

    /* 设置默认返回值。  */
    if  (node->sym->sdFnc.sdfEntryPt &&
         node->sym->sdFnc.retv->sdType->tdTypeKind == TYP_FLOAT)
    {
        IRInstSetOperand (move, 1, stCreateFconNode (comp->cmpCurST, 0.0f));
        move = NULL;
    }
    else if (node->sym->sdFnc.sdfEntryPt &&
             node->sym->sdFnc.retv->sdType->tdTypeKind != TYP_VOID)
    {
        IRInstSetOperand (move, 1, stCreateIconNode (comp->cmpCurST, 0));
        move = NULL;
    }
    if  (move)
    {
        InterCodeRemoveInst_nobb (code, move);
        IRInstDelInst (move);
    }

    /* 回填。  */
    DoBackpatch (second->next_list, stCreateIconNode(cmpCurST, NEXT_INSN(code)));
    DoBackpatch (node->next_list, stCreateIconNode(cmpCurST, NEXT_INSN(code)));

    /* 必须在最后插入exit指令。  */
    cmpCurST->cmpCurScp = node->sym;
    IRInstSetOperand(exit, 0, node->sym->sdType->tdFnc.tdfRett->tdTypeKind != TYP_VOID ? emit_load (node->sym->sdFnc.retv, code, cmpCurST, node->tnLineNo, node->tnColumn) : node->sym->sdFnc.retv);
    cmpCurST->cmpCurScp = node->sym->sdParent;
    InterCodeAddInst_nobb(code, exit, TRUE);

    bSuccess = TRUE;

fail:

    if (!bSuccess)
    {
        InterCodeRemoveInst_nobb(code, exit);
        IRInstDelInst(exit);
    }
    TRACE_SEMANTICS (fprintf (stderr, "exit  VisitTN_FuncDef with %x", bSuccess));
    return bSuccess;
}

static BOOL VisitTN_FuncFParams(Tree node, InterCode code, SymTab cmpCurST)
{
    Tree *Cursor;
    BOOL bSuccess = FALSE;

    TRACE_SEMANTICS (fprintf (stderr, "enter VisitTN_FuncFParams()"));

    for(  Cursor=(Tree *)List_Last(node->children)
        ;  Cursor!=NULL
        ;  Cursor = (Tree *)List_Prev((void *)Cursor)
        )
    {
        if (!ir_visit_ast_node (*Cursor, code, cmpCurST))
            goto fail;
        *(SymDef *)List_NewLast(node->lpParent->sym->sdType->tdFnc.tdfArgs, sizeof (SymDef)) = (*(Tree *)List_Last((*Cursor)->children))->sym;
    }

    bSuccess = TRUE;

fail:

    TRACE_SEMANTICS (fprintf (stderr, "exit  VisitTN_FuncFParams with %x", bSuccess));
    return bSuccess;
}

static BOOL VisitTN_FuncDecl(Tree node, InterCode code, SymTab cmpCurST)
{
    Tree *Cursor;
    BOOL bSuccess = FALSE;

    TRACE_SEMANTICS (fprintf (stderr, "enter VisitTN_FuncDecl()"));

    for(  Cursor=(Tree *)List_First(node->children)
        ;  Cursor!=NULL
        ;  Cursor = (Tree *)List_Next((void *)Cursor)
        )
        if (!ir_visit_ast_node (*Cursor, code, cmpCurST))
            goto fail;

    bSuccess = TRUE;

fail:

    TRACE_SEMANTICS (fprintf (stderr, "exit  VisitTN_FuncDecl with %x", bSuccess));
    return bSuccess;
}

static BOOL
ir_visit_ast_node (Tree node, InterCode code, SymTab cmpCurST)
{
    switch (node->tnOper)
    {
#define TREEOP(en,tk,sn,IL,pr,ok) \
        case en: return Visit##en(node, code, cmpCurST);
#include "toplist.h"
#undef TREEOP
        default: return FALSE;
    }
}

BOOL genIR(Tree root, InterCode code, SymTab cmpCurST)
{
    return ir_visit_ast_node(root, code, cmpCurST);
}

/* 如果操作数不是临时变量，则产生一“load”指令。这样做的目的是为了让临时变量的行为
   与寄存器相似。  */
static SymDef emit_load(SymDef memSym, InterCode code, SymTab cmpCurST, int line, int column)
{
    SymDef sym = memSym;
    IRInst insn;

    if ((memSym->sdSymKind == SYM_VAR) &&
        (memSym->sdVar.sdvLocal || is_global_var (memSym)) &&
        !memSym->sdVar.sdvConst &&
        (memSym->sdType->tdTypeKind <= TYP_lastIntrins))
    {
        insn = IRInstEmitInst(IRINST_OP_load, line, column);
        IRInstSetOperand(insn, 1, memSym);
        sym = stDeclareSym(cmpCurST, NULL, SYM_VAR);
        sym->sdIsImplicit = TRUE;
        sym->sdType = stAllocTypDef(memSym->sdType->tdTypeKind);
        IRInstSetOperand(insn, 0, sym);
        InterCodeAddInst_nobb(code, insn, TRUE);
    }
    return sym;
}

/* 如果可能，将给定的符号表项转换为指定的类型。  */
static SymDef emit_conversion_insns(SymDef sym, TypDef type, InterCode code, SymTab cmpCurST, int line, int column)
{
    TypDef srcType = sym->sdType;
    TypDef dstType = type;
    IRInst insn;
    enum var_types srcVtyp;
    enum var_types dstVtyp;
    ConstVal cval;

    srcVtyp = srcType->tdTypeKind;
    dstVtyp = dstType->tdTypeKind;

    /* Are the types identical? */

    if  (srcVtyp == dstVtyp)
    {
        return sym;
    }

    if  (sym->sdVar.sdvConst)
    {
        /* 常数折叠。  */
        cval = GetConstVal(sym, 0);
        sym = (dstVtyp == TYP_FLOAT) ? stCreateFconNode(cmpCurST, (float)cval->cvValue.cvIval)
                                     : stCreateIconNode(cmpCurST, (int)cval->cvValue.cvFval);
    }
    else
    {
        insn = IRInstEmitInst(dstVtyp == TYP_FLOAT ? IRINST_OP_i2f : IRINST_OP_f2i, line, column);
        IRInstSetOperand(insn, 1, sym);
        sym = stDeclareSym(cmpCurST, NULL, SYM_VAR);
        sym->sdIsImplicit = TRUE;
        sym->sdType = stAllocTypDef(dstVtyp);
        IRInstSetOperand(insn, 0, sym);
        InterCodeAddInst_nobb(code, insn, TRUE);
    }

    return sym;
}

static SymDef
emit_binop (SymDef op1, SymDef op2, enum IRInstOperator c, InterCode code, SymTab cmpCurST, int line, int column)
{
    enum var_types tp1;
    enum var_types tp2;
    IRInst insn;
    SymDef sym;

    /* 确保两个操作数在虚拟寄存器中。  */
    op1 = emit_load(op1, code, cmpCurST, line, column);
    op2 = emit_load(op2, code, cmpCurST, line, column);

    /* 获得两操作数的类型。  */
    tp1 = op1->sdType->tdTypeKind;
    tp2 = op2->sdType->tdTypeKind;

    /* 将两个操作数转换为为更大的类型 */
    if      (tp1 < tp2)
    {
        op1 = emit_conversion_insns(op1, op2->sdType, code, cmpCurST, line, column);
    }
    else if (tp1 > tp2)
    {
        op2 = emit_conversion_insns(op2, op1->sdType, code, cmpCurST, line, column);
    }

    /* 新建符号表项。  */
    sym = stDeclareSym(cmpCurST, NULL, SYM_VAR);
    sym->sdIsImplicit = TRUE;
    switch (c)
    {
    case IRINST_OP_eq : 
    case IRINST_OP_ne : 
    case IRINST_OP_lt : 
    case IRINST_OP_le : 
    case IRINST_OP_ge : 
    case IRINST_OP_gt : sym->sdType = stAllocTypDef(TYP_INT); break;
    default     : sym->sdType = CopyType(op1->sdType); break;
    }

    /* 产生指令。  */
    insn = IRInstEmitInst(c, line, column);
    IRInstSetOperand(insn, 0, sym);
    IRInstSetOperand(insn, 1, op1);
    IRInstSetOperand(insn, 2, op2);

    /* 插入指令。  */
    InterCodeAddInst_nobb(code, insn, TRUE);

    return sym;
}

static Tree *decl_designator(SymDef sym, Tree *Cursor, TypDef type, unsigned long c, int *cur_index, InterCode code, SymTab cmpCurST)
{
    int index;

    index = *cur_index;
    type = type->tdArr.tdaElem;
    c += index * type_size(type);
    return decl_initializer(sym, Cursor, type, c, 0, code, cmpCurST);
}

static Tree *decl_initializer(SymDef sym, Tree *Cursor, TypDef type, unsigned long c, int first, InterCode code, SymTab cmpCurST)
{
    int index, array_length, n, no_oblock;
    Tree *iter;
    SymDef tmpSym;
    IRInst insn;
    Tree node;

    if  (type->tdTypeKind == TYP_ARRAY)
    {
        n = type->tdArr.tdaDims->ddSize;
        array_length = 0;

        iter = Cursor;

        no_oblock = 1;
        if (first || 
            ((*Cursor)->tnOper == TN_SLV_INIT)) {
            Cursor=(Tree *)List_Last((*Cursor)->children);
            no_oblock = 0;
        }

        index = 0;
        while (Cursor != NULL)
        {
            Cursor = decl_designator(sym, Cursor, type, c, &index, code, cmpCurST);
            if (n >= 0 && index >= n)
            {
                error (comp->cmpConfig.input_file_name, (*Cursor)->tnLineNo, "index too large");
                return NULL;
            }
            index++;
            if (index > array_length)
                array_length = index;
            /* 对多维数组的特殊测试 */
            if (index >= n && no_oblock)
                break;
            if (Cursor == NULL)
                break;
            Cursor = (Tree *)List_Prev((void *)Cursor);
        }
        if (!no_oblock)
            Cursor = iter;
    }
    else
    {
        node = *(Tree *)List_First((*Cursor)->children);
        if (!ir_visit_ast_node (node, code, cmpCurST))
            return NULL;

        if (node->sym->sdSymKind != SYM_VAR)
            return NULL;

        if  (sym->sdVar.sdvConst ||
             is_global_var (sym))
        {
            /* 将全局变量和常量的初始化值保存在符号表中。  */

            tmpSym = emit_conversion_insns (node->sym, type, code, cmpCurST, (*Cursor)->tnLineNo, (*Cursor)->tnColumn);

            if (!tmpSym->sdVar.sdvConst)
            {
                error (comp->cmpConfig.input_file_name, node->tnLineNo, "initializer element is not constant");
                return NULL;
            }

            switch (node->tnType->tdTypeKind)
            {
            case TYP_INT:
                SetIconVal(sym, GetConstVal(tmpSym, 0)->cvValue.cvIval, c / type_size(type));
                break;

            case TYP_FLOAT:
                SetFconVal(sym, GetConstVal(tmpSym, 0)->cvValue.cvFval, c / type_size(type));
                break;

            case TYP_DOUBLE:
                SetDconVal(sym, GetConstVal(tmpSym, 0)->cvValue.cvDval, c / type_size(type));
                break;

            default:
                break;
            }
        }
        if  (!is_global_var (sym))
        {
            /* 局部变量的初始化直接产生中间指令。  */
            tmpSym = emit_load (node->sym, code, cmpCurST, (*Cursor)->tnLineNo, (*Cursor)->tnColumn);
            tmpSym = emit_conversion_insns (tmpSym, type, code, cmpCurST, (*Cursor)->tnLineNo, (*Cursor)->tnColumn);

            if  (sym->sdType->tdTypeKind > TYP_lastIntrins)
            {
                insn = IRInstEmitInst(IRINST_OP_astore, (*Cursor)->tnLineNo, (*Cursor)->tnColumn);
                IRInstSetOperand(insn, 0, sym);
                IRInstSetOperand(insn, 1, stCreateIconNode(cmpCurST, c));
                IRInstSetOperand(insn, 2, tmpSym);
                IRInstSetOperand(insn, 3, sym);
                InterCodeAddInst_nobb(code, insn, TRUE);
#if defined(zenglj)
                tmpSym = stDeclareSym(cmpCurST, NULL, SYM_VAR);
                tmpSym->sdIsImplicit = TRUE;
                tmpSym->sdType = stNewPtrType(CopyType(stGetBaseType (sym->sdType)), TRUE);
                IRInstSetOperand(insn, 4, tmpSym);
#endif
            }
            else if (! sym->sdVar.sdvConst)
            {
                insn = IRInstEmitInst(IRINST_OP_store, (*Cursor)->tnLineNo, (*Cursor)->tnColumn);
                IRInstSetOperand(insn, 0, sym);
                IRInstSetOperand(insn, 1, tmpSym);
                InterCodeAddInst_nobb(code, insn, TRUE);
            }
        }

    }

    return Cursor;
}

static SymDef FoldIntUnop(enum treeOps tnOper, SymDef op1, SymTab cmpCurST)
{
    int iv1;
    SymDef tmpSym;
    const unsigned u = 2147483648;

    if  (op1->sdSymKind != SYM_VAR ||
         !op1->sdVar.sdvConst ||
         op1->sdType->tdTypeKind != TYP_INT)
    {
        return NULL;
    }

    iv1 = GetConstVal(op1, 0)->cvValue.cvIval;

    switch (tnOper)
    {
    case TN_NEG:
        if  (memcmp (&iv1, &u, sizeof(u)) != 0)
            iv1 = -iv1;
        break;

    case TN_LOG_NOT:
        iv1 = !iv1;
        break;

    case TN_NOP:
        break;

    default:
        break;
    }

    tmpSym = stCreateIconNode(cmpCurST, iv1);

    return tmpSym;
}

static SymDef FoldIntBinop(enum treeOps tnOper, SymDef op1, SymDef op2, Tree node, SymTab cmpCurST)
{
    int iv1;
    int iv2;
    SymDef tmpSym;

    if  (op1->sdSymKind != SYM_VAR ||
         !op1->sdVar.sdvConst ||
         op1->sdType->tdTypeKind != TYP_INT)
    {
        return NULL;
    }

    if  (op2->sdSymKind != SYM_VAR ||
         !op2->sdVar.sdvConst ||
         op2->sdType->tdTypeKind != TYP_INT)
    {
        return NULL;
    }

    iv1 = GetConstVal(op1, 0)->cvValue.cvIval;
    iv2 = GetConstVal(op2, 0)->cvValue.cvIval;

    switch (tnOper)
    {
    case TN_ADD: iv1  += iv2; break;
    case TN_SUB: iv1  -= iv2; break;
    case TN_MUL: iv1  *= iv2; break;

    case TN_DIV: if (iv2 == 0) goto INT_DIV_ZERO;
                 iv1 /= iv2;
                 break;

    case TN_MOD: if (iv2 == 0) goto INT_DIV_ZERO;
                 iv1 %= iv2;
                 break;

    case TN_EQ : iv1 = (iv1 == iv2); break;
    case TN_NE : iv1 = (iv1 != iv2); break;

    case TN_LT : iv1 = (iv1 <  iv2);
                 break;
    case TN_LE : iv1 = (iv1 <= iv2);
                 break;
    case TN_GE : iv1 = (iv1 >= iv2);
                 break;
    case TN_GT : iv1 = (iv1 >  iv2);
                 break;

    case TN_LOG_OR : iv1 = iv1 || iv2; break;
    case TN_LOG_AND: iv1 = iv1 && iv2; break;

    default:
        break;
    }

    tmpSym = stCreateIconNode(cmpCurST, iv1);

    return tmpSym;

INT_DIV_ZERO:

    warning (comp->cmpConfig.input_file_name, node->tnLineNo, "Division by constant 0");
    return NULL;
}

static int
isnanf (float f)

{

  // checks whether f is a NaN

  unsigned int *fp;

  fp = (unsigned int *)&f;

  if (((fp[0] & 0x7f800000) == 0x7f800000) && ((fp[0] & 0x007fffff) != 0))
    return (1);
  else
    return (0);

}

static SymDef FoldFltUnop(enum treeOps tnOper, SymDef op1, SymTab cmpCurST)
{
    float fv1;
    SymDef tmpSym;
    int rel;

    if  (op1->sdSymKind != SYM_VAR ||
         !op1->sdVar.sdvConst ||
         op1->sdType->tdTypeKind != TYP_FLOAT)
    {
        return NULL;
    }

    fv1 = GetConstVal(op1, 0)->cvValue.cvFval;

    if  (isnanf(fv1))
        return  op1;

    switch (tnOper)
    {
    case TN_NEG:
        fv1 = -fv1;
        break;

    case TN_LOG_NOT:
        rel = !fv1;
        goto RELOP;

    case TN_NOP:
        break;

    default:
        break;
    }

    tmpSym = stCreateFconNode(cmpCurST, fv1);

    return tmpSym;

RELOP:

    tmpSym = stCreateIconNode(cmpCurST, rel);

    return tmpSym;
}

static SymDef FoldFltBinop(enum treeOps tnOper, SymDef op1, SymDef op2, Tree node, SymTab cmpCurST)
{
    float fv1;
    float fv2;
    int rel;
    SymDef tmpSym;

    if  (op1->sdSymKind != SYM_VAR ||
         !op1->sdVar.sdvConst ||
         op1->sdType->tdTypeKind != TYP_FLOAT)
    {
        return NULL;
    }

    if  (op2->sdSymKind != SYM_VAR ||
         !op2->sdVar.sdvConst ||
         op2->sdType->tdTypeKind != TYP_FLOAT)
    {
        return NULL;
    }

    fv1 = GetConstVal(op1, 0)->cvValue.cvFval;
    fv2 = GetConstVal(op2, 0)->cvValue.cvFval;

    if  (isnanf(fv1) || isnanf(fv2))
    {
        switch (tnOper)
        {
        case TN_EQ :
        case TN_NE :
        case TN_LT :
        case TN_LE :
        case TN_GE :
        case TN_GT :
            rel = (tnOper == TN_NE);
            goto RELOP;

        default: goto DONE;
        }
    }

    switch (tnOper)
    {
    case TN_ADD: fv1 += fv2; break;
    case TN_SUB: fv1 -= fv2; break;
    case TN_MUL: fv1 *= fv2; break;
    case TN_DIV: fv1 /= fv2; break;

    case TN_MOD: return op1;

    case TN_EQ : rel = (fv1 == fv2); goto RELOP;
    case TN_NE : rel = (fv1 != fv2); goto RELOP;
    case TN_LT : rel = (fv1 <  fv2); goto RELOP;
    case TN_LE : rel = (fv1 <= fv2); goto RELOP;
    case TN_GE : rel = (fv1 >= fv2); goto RELOP;
    case TN_GT : rel = (fv1 >  fv2); goto RELOP;

    default: break;
    }

DONE:

    tmpSym = stCreateFconNode(cmpCurST, fv1);

    return tmpSym;

RELOP:

    tmpSym = stCreateIconNode(cmpCurST, rel);

    return tmpSym;
}

static void DoBackpatch(LIST lst, SymDef number)
{
    struct patch_info *Cursor;
    for(  Cursor=(struct patch_info *)List_Last(lst)
        ;  Cursor!=NULL
        ;  Cursor = (struct patch_info *)List_Prev((void *)Cursor)
        )
    {
        if  (Cursor->insn->opcode != IRINST_OP_goto)
        {
            if  (!Cursor->is_true_list)
                IRInstSetOperand(Cursor->insn, 3, number);
            else
                IRInstSetOperand(Cursor->insn, 2, number);
        }
        else
        {
            IRInstSetOperand(Cursor->insn, 0, number);
        }
    }
}

static void DoMerge(LIST To, LIST l1, LIST l2)
{
    struct patch_info *Cursor;
    for(  Cursor=(struct patch_info *)List_First(l1)
       ;  Cursor!=NULL
       ;  Cursor = (struct patch_info *)List_Next((void *)Cursor)
       )
    {
        *(struct patch_info *)List_NewLast(To, sizeof(struct patch_info) ) = *Cursor;
    }
    for(  Cursor=(struct patch_info *)List_First(l2)
       ;  Cursor!=NULL
       ;  Cursor = (struct patch_info *)List_Next((void *)Cursor)
       )
    {
        *(struct patch_info *)List_NewLast(To, sizeof(struct patch_info) ) = *Cursor;
    }
}

static BOOL
create_cmp_insn (SymDef op1, SymDef op2, enum treeOps tnOper, Tree node, InterCode code, SymTab cmpCurST)
{
    IRInst insn;
    enum var_types tp1;
    enum var_types tp2;
    SymDef tmpSym = NULL;
    enum IRInstOperator opcode;
    void *Cursor;

    switch (tnOper)
    {
    case TN_EQ  : opcode = IRINST_OP_ifeq; break;
    case TN_NE  : opcode = IRINST_OP_ifne; break;
    case TN_LT  : opcode = IRINST_OP_iflt; break;
    case TN_LE  : opcode = IRINST_OP_ifle; break;
    case TN_GE  : opcode = IRINST_OP_ifge; break;
    case TN_GT  : opcode = IRINST_OP_ifgt; break;
    default     : return FALSE;
    }

    op1 = emit_load (op1, code, cmpCurST, node->tnLineNo, node->tnColumn);
    op2 = emit_load (op2, code, cmpCurST, node->tnLineNo, node->tnColumn);

    tp1 = op1->sdType->tdTypeKind;
    tp2 = op2->sdType->tdTypeKind;

    if  ((tp1 != TYP_INT && tp1 != TYP_FLOAT) ||
        (tp2 != TYP_INT && tp2 != TYP_FLOAT))
    {
        error (comp->cmpConfig.input_file_name, node->tnLineNo, "invalid operands to binary expression");
        return FALSE;
    }

    if      (tp1 < tp2)
    {
        op1 = emit_conversion_insns(op1, op2->sdType, code, cmpCurST, node->tnLineNo, node->tnColumn);
    }
    else if (tp1 > tp2)
    {
        op2 = emit_conversion_insns(op2, op1->sdType, code, cmpCurST, node->tnLineNo, node->tnColumn);
    }

    if      (op1->sdType->tdTypeKind == TYP_INT)
    {
        tmpSym = FoldIntBinop(tnOper, op1, op2, node, cmpCurST);
    }
    else if (op1->sdType->tdTypeKind == TYP_FLOAT)
    {
        tmpSym = FoldFltBinop(tnOper, op1, op2, node, cmpCurST);
    }

    if  (tmpSym)
    {
        insn = IRInstEmitInst(IRINST_OP_goto, node->tnLineNo, node->tnColumn);
        InterCodeAddInst_nobb(code, insn, TRUE);
        if  (GetConstVal(tmpSym, 0)->cvValue.cvIval)
        {
            Cursor = List_NewLast(node->true_list, sizeof (struct patch_info));
            ((struct patch_info *)Cursor)->insn = insn;
            ((struct patch_info *)Cursor)->is_true_list = TRUE;
        }
        else
        {
            Cursor = List_NewLast(node->false_list, sizeof (struct patch_info));
            ((struct patch_info *)Cursor)->insn = insn;
            ((struct patch_info *)Cursor)->is_true_list = FALSE;
        }
    }
    else
    {
        insn = IRInstEmitInst(opcode, node->tnLineNo, node->tnColumn);
        IRInstSetOperand(insn, 0, op1);
        IRInstSetOperand(insn, 1, op2);
        InterCodeAddInst_nobb(code, insn, TRUE);
#if defined(zenglj)
        tmpSym = stDeclareSym(cmpCurST, NULL, SYM_VAR);
        tmpSym->sdIsImplicit = TRUE;
        tmpSym->sdType = stAllocTypDef(TYP_BOOL);
        IRInstSetOperand(insn, 4, tmpSym);
#endif
        Cursor = List_NewLast(node->true_list, sizeof (struct patch_info));
        ((struct patch_info *)Cursor)->insn = insn;
        ((struct patch_info *)Cursor)->is_true_list = TRUE;
        Cursor = List_NewLast(node->false_list, sizeof (struct patch_info));
        ((struct patch_info *)Cursor)->insn = insn;
        ((struct patch_info *)Cursor)->is_true_list = FALSE;
    }

    node->sym = NULL;

    return TRUE;
}
