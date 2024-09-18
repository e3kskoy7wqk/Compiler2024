/*****************************************************************************
File       : grammar.y
Title      : the SysY2022 grammar file
           :
Description: contains the syntactic and semantic handling of the
           : sy file
History    :
        03-May-2023     MTC Create
        04-April-2024 MTC     Rewrite for COMPILER 2.0
*****************************************************************************/
%{
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "all.h"
#include "lex.yy.h"

#define YYMAXDEPTH 238609293

/* Cause the `yydebug' variable to be defined.  */
#define YYDEBUG 1

Tree g_savedTree;

#if !defined(NDEBUG)

#if 0

  static int nCount = 0; /* 规约次数  */

  /* helper macro for short definition of trace-output inside code  */
  #define TRACE_PARSER(code)       \
    if (1) {       \
      code;                                    \
    }
#else
  #define TRACE_PARSER(code)
#endif

#else
  #define TRACE_PARSER(code)
#endif


static void
yyerror (const char *msg)
{
  fprintf (stderr, "%s: %d: %s\n", comp->cmpConfig.input_file_name, LineNum, msg);
  exit (1);
}

%}

%nonassoc LOWER_THAN_ELSE
%nonassoc L_ELSE

%union
{
    float floatValue;
    unsigned intValue;
    Tree node;
}

%token <node> L_CONST      "const"
%token <node> L_INT        "int"
%token <node> L_FLOAT      "float"
%token <node> L_VOID       "void"
%token <node> L_AND        "&"
%token <node> L_ANDAND     "&&"
%token <node> L_ANDEQ      "&="
%token <node> L_ASSIGN      "="
%token <node> L_COLON       ":"
%token <node> L_COMMA       ","
%token <node> L_DECR        "--"
%token <node> L_DIV         "/"
%token <node> L_DIVEQ       "/="
%token <node> L_EQUALS      "=="
%token <node> L_EXCLAIM     "!"
%token <node> L_GT          ">"
%token <node> L_GTEQ        ">="
%token <node> L_LBRACK      "["
%token <node> L_LT          "<"
%token <node> L_LTEQ        "<="
%token <node> L_MINUS       "-"
%token <node> L_MINUSEQ     "-="
%token <node> L_MOD         "%"
%token <node> L_MULT        "*"
%token <node> L_MULTEQ      "*="
%token <node> L_NOTEQ       "!="
%token <node> L_OR          "|"
%token <node> L_OREQ        "|="
%token <node> L_OROR        "||"
%token <node> L_PERIOD      "."
%token <node> L_PLUS        "+"
%token <node> L_QUEST       "?"
%token <node> L_TILDE       "~"
%token <node> L_XOR         "^"
%token <node> L_XOREQ       "^="
%token <node> L_BREAK       "break"
%token <node> L_CASE        "case"
%token <node> L_CONTINUE    "continue"
%token <node> L_DEFAULT     "default"
%token <node> L_DO          "do"
%token <node> L_ELSE        "else"
%token <node> L_FOR         "for"
%token <node> L_GOTO        "goto"
%token <node> L_IF          "if"
%token <node> L_LCURLY      "{"
%token <node> L_LPAREN      "("
%token <node> L_RBRACK      "]"
%token <node> L_RCURLY      "}"
%token <node> L_RETURN      "return"
%token <node> L_RPAREN      ")"
%token <node> L_SEMI        ";"
%token <node> L_SIZEOF      "sizeof"
%token <node> L_SW          "switch"
%token <node> L_WHILE       "while"
%token <node> Identifier
%token <node>    IntConst 
%token <node>  floatConst


%start CompUnit

/* 此处优先级似乎无用  */
%left L_ASSIGN
%left L_PLUS L_MINUS
%left L_MULT L_DIV L_MOD
%left L_OROR
%left L_ANDAND
%left L_EXCLAIM

/* 指定文法的非终结符号，<>可指定文法属性  */
%type <node> CompUnit
%type <node> Decl
%type <node> ConstDecl
%type <node> ConstDefGroup
%type <node> ConstDef
%type <node> ConstExpGroup
%type <node> ConstInitVal
%type <node> ConstInitValGroup
%type <node> VarDecl
%type <node> VarDefGroup
%type <node> VarDef
%type <node> InitVal
%type <node> InitValGroup
%type <node> FuncBody
%type <node> FuncDef
%type <node> FuncFParams
%type <node> FuncFParamGroup
%type <node> FuncFParam
%type <node> ExpGroup
%type <node> Block
%type <node> BlockItemGroup
%type <node> BlockItem
%type <node> Stmt
%type <node> Exp
%type <node> Cond
%type <node> LVal
%type <node> Number
%type <node> PrimaryExp
%type <node> UnaryExp
%type <node> FuncRParams
%type <node> FuncRParamsGroup
%type <node> MulExp
%type <node> AddExp
%type <node> RelExp
%type <node> EqExp
%type <node> LAndExp
%type <node> LOrExp
%type <node> ConstExp

%type <node> FuncDecl
%type <node> TN_FuncBodys

%type <node> BType
%type <node> UnaryOp

%% /* SysY2022文法  */
/* 翻译单元  */
CompUnit: 
  FuncDef 
    {
      TRACE_PARSER (fprintf (stderr, "%d: CompUnit -> FuncDef\n", ++nCount));
      $$ = parseCreateNode(TN_CompUnit, $1->tnLineNo, $1->tnColumn);
      InsertChildNode($$, $1);
      g_savedTree = $$;
    }
| CompUnit FuncDef 
    {
      TRACE_PARSER (fprintf (stderr, "%d: CompUnit -> CompUnit FuncDef\n", ++nCount));
      $$ = $1;
      InsertChildNode($$, $2);
    }
| Decl  
    {
      TRACE_PARSER (fprintf (stderr, "%d: CompUnit -> Decl\n", ++nCount));
      $$ = parseCreateNode(TN_CompUnit, $1->tnLineNo, $1->tnColumn);
      InsertChildNode($$, $1);
      g_savedTree = $$;
    }
| CompUnit Decl  
    {
      TRACE_PARSER (fprintf (stderr, "%d: CompUnit -> CompUnit Decl\n", ++nCount));
      $$ = $1;
      InsertChildNode($$, $2);
    }
;

/* 声明  */
Decl:
  ConstDecl 
    {
      TRACE_PARSER (fprintf (stderr, "%d: Decl -> ConstDecl\n", ++nCount));
      $$ = $1;
    }
| VarDecl 
    {
      TRACE_PARSER (fprintf (stderr, "%d: Decl -> VarDecl\n", ++nCount));
      $$ = $1;
    }
| FuncDecl 
    {
      TRACE_PARSER (fprintf (stderr, "%d: Decl -> FuncDecl\n", ++nCount));
      $$ = $1;
    }
;

/* 常量声明  */
ConstDecl:
  L_CONST BType ConstDef ConstDefGroup L_SEMI 
    {
      TRACE_PARSER (fprintf (stderr, "%d: ConstDecl -> \"const\" BType ConstDef ConstDefGroup \";\"\n", ++nCount));
      $$ = $4;
      $$->tnVtyp = $2->tnVtyp;
      InsertChildNode ($$, $3);
      $$->tnLineNo = $1->tnLineNo;
      $$->tnColumn = $1->tnColumn;
      parseDeleteNode ($1);
      parseDeleteNode ($5);
      parseDeleteNode ($2);
    }
;

ConstDefGroup:
  L_COMMA ConstDef ConstDefGroup   
    {
      TRACE_PARSER (fprintf (stderr, "%d: ConstDefGroup -> \",\" ConstDef ConstDefGroup\n", ++nCount));
      $$= $3;
      InsertChildNode($$, $2);
      parseDeleteNode ($1);
    }
| 
    {
      TRACE_PARSER (fprintf (stderr, "%d: ConstDefGroup -> (null)\n", ++nCount));
      $$ = parseCreateNode(TN_VarDecl, LineNum, Column);
    }
;

/* 基本类型  */
BType:
  L_INT  
    {
      TRACE_PARSER (fprintf (stderr, "%d: BType -> \"int\"\n", ++nCount));
      $$ = $1;
      $$->tnVtyp = TYP_INT;
    }
| L_FLOAT 
    {
      TRACE_PARSER (fprintf (stderr, "%d: BType -> \"float\"\n", ++nCount));
      $$ = $1;
      $$->tnVtyp = TYP_FLOAT;
    }
;

/* 常数定义  */
ConstDef:
  Identifier ConstExpGroup L_ASSIGN ConstInitVal  
    {
      TRACE_PARSER (fprintf (stderr, "%d: ConstDef -> Identifier ConstExpGroup \"=\" ConstInitVal\n", ++nCount));
      $$ = parseCreateNode(TN_VarDef, $1->tnLineNo, $1->tnColumn);
      InsertChildNode($$, $2);
      InsertChildNode($$, $4);
      $$->tnFlags |= TNF_VAR_INIT | TNF_VAR_CONST | TNF_VAR_SEALED;
      while ($2->tnOper != TN_NAME)
        $2 = *(Tree *)List_First($2->children);
      $2->tnName.tnNameId = $1->tnName.tnNameId;
      $1->tnName.tnNameId = NULL;
      parseDeleteNode($1);
      parseDeleteNode($3);
    }
;

ConstExpGroup:
  ConstExpGroup L_LBRACK ConstExp L_RBRACK
    {
      TRACE_PARSER (fprintf (stderr, "%d: ConstExpGroup -> ConstExpGroup \"[\" ConstExp \"]\"\n", ++nCount));
      $$ = parseCreateNode(TN_INDEX, $1->tnLineNo, $1->tnColumn);
      InsertChildNode($$, $1);
      InsertChildNode($$, $3);
      parseDeleteNode($2);
      parseDeleteNode($4);
    }
|
    {
      TRACE_PARSER (fprintf (stderr, "%d: ConstExpGroup -> (null)\n", ++nCount));
      $$ = parseCreateNode(TN_NAME, LineNum, Column);
    }
;

/* 常量初值  */
ConstInitVal:
  ConstExp  
    {
      TRACE_PARSER (fprintf (stderr, "%d: ConstInitVal -> ConstExp\n", ++nCount));
      $$ = parseCreateNode(TN_ConstInitVal, $1->tnLineNo, $1->tnColumn);
      InsertChildNode($$, $1);
    }
| L_LCURLY L_RCURLY  
    {
      TRACE_PARSER (fprintf (stderr, "%d: ConstInitVal -> \"{\" \"}\"\n", ++nCount));
      $$ = parseCreateNode(TN_SLV_INIT, $1->tnLineNo, $1->tnColumn);
      parseDeleteNode($1);
      parseDeleteNode($2);
    }
| L_LCURLY ConstInitVal ConstInitValGroup L_RCURLY 
    {
      TRACE_PARSER (fprintf (stderr, "%d: ConstInitVal -> \"{\" ConstInitVal ConstInitValGroup \"}\"\n", ++nCount));
      $$ = $3;
      $$->tnLineNo = $1->tnLineNo;
      $$->tnColumn = $1->tnColumn;
      InsertChildNode($$, $2);
      parseDeleteNode($1);
      parseDeleteNode($4);
    }
;

ConstInitValGroup:
  L_COMMA ConstInitVal ConstInitValGroup 
    { 
      TRACE_PARSER (fprintf (stderr, "%d: ConstInitValGroup -> \",\" ConstInitVal ConstInitValGroup\n", ++nCount));
      $$ = $3;
      $$->tnLineNo = $1->tnLineNo;
      $$->tnColumn = $1->tnColumn;
      InsertChildNode($$, $2);
      parseDeleteNode($1);
    }
|  
    { 
      TRACE_PARSER (fprintf (stderr, "%d: ConstInitValGroup -> (null)\n", ++nCount));
      $$ = parseCreateNode(TN_SLV_INIT, LineNum, Column);
    }
;

/* 变量声明  */
VarDecl:
  BType VarDef VarDefGroup L_SEMI  
    { 
      TRACE_PARSER (fprintf (stderr, "%d: VarDecl -> BType VarDef VarDefGroup \";\"\n", ++nCount));
      $$ = $3;
      $$->tnVtyp = $1->tnVtyp;
      $$->tnLineNo = $1->tnLineNo;
      $$->tnColumn = $1->tnColumn;
      InsertChildNode($$, $2);
      parseDeleteNode($1);
      parseDeleteNode($4);
    }
;

VarDefGroup:
  L_COMMA VarDef VarDefGroup 
    { 
      TRACE_PARSER (fprintf (stderr, "%d: VarDefGroup -> \",\" VarDef VarDefGroup\n", ++nCount));
      $$ = $3;
      $$->tnLineNo = $1->tnLineNo;
      $$->tnColumn = $1->tnColumn;
      InsertChildNode($$, $2);
      parseDeleteNode($1);
    }
|  
    { 
      TRACE_PARSER (fprintf (stderr, "%d: VarDefGroup -> (null)\n", ++nCount));
      $$ = parseCreateNode(TN_VarDecl, LineNum, Column);
    }
;

/* 变量定义  */
VarDef:
  Identifier ConstExpGroup 
    { 
      TRACE_PARSER (fprintf (stderr, "%d: VarDef -> Identifier ConstExpGroup\n", ++nCount));
      $$ = parseCreateNode(TN_VarDef, $1->tnLineNo, $1->tnColumn);
      InsertChildNode($$, $2);
      while ($2->tnOper != TN_NAME)
        $2 = *(Tree *)List_First($2->children);
      $2->tnName.tnNameId = $1->tnName.tnNameId;
      $1->tnName.tnNameId = NULL;
      parseDeleteNode($1);
    }
| Identifier ConstExpGroup L_ASSIGN InitVal  
    { 
      TRACE_PARSER (fprintf (stderr, "%d: VarDef -> Identifier ConstExpGroup \"=\" InitVal\n", ++nCount));
      $$ = parseCreateNode(TN_VarDef, $1->tnLineNo, $1->tnColumn);
      $$->tnFlags |= TNF_VAR_INIT;
      InsertChildNode($$, $2);
      InsertChildNode($$, $4);
      while ($2->tnOper != TN_NAME)
        $2 = *(Tree *)List_First($2->children);
      $2->tnName.tnNameId = $1->tnName.tnNameId;
      $1->tnName.tnNameId = NULL;
      parseDeleteNode($1);
      parseDeleteNode($3);
    }
;

/* 变量初值  */
InitVal:
  Exp  
    {
      TRACE_PARSER (fprintf (stderr, "%d: InitVal -> Exp\n", ++nCount));
      $$ = parseCreateNode(TN_InitVal, $1->tnLineNo, $1->tnColumn);
      InsertChildNode($$, $1);
    }
| L_LCURLY L_RCURLY  
    {
      TRACE_PARSER (fprintf (stderr, "%d: InitVal -> \"{\" \"}\"\n", ++nCount));
      $$ = parseCreateNode(TN_SLV_INIT, $1->tnLineNo, $1->tnColumn);
      parseDeleteNode($1);
      parseDeleteNode($2);
    }
| L_LCURLY InitVal InitValGroup L_RCURLY 
    {
      TRACE_PARSER (fprintf (stderr, "%d: InitVal -> \"{\" InitVal InitValGroup \"}\"\n", ++nCount));
      $$ = $3;
      $$->tnLineNo = $1->tnLineNo;
      $$->tnColumn = $1->tnColumn;
      InsertChildNode($$, $2);
      parseDeleteNode($1);
      parseDeleteNode($4);
    }
;

InitValGroup:
  L_COMMA InitVal InitValGroup 
    {
      TRACE_PARSER (fprintf (stderr, "%d: InitValGroup -> \",\" InitVal InitValGroup\n", ++nCount));
      $$ = $3;
      $$->tnLineNo = $1->tnLineNo;
      $$->tnColumn = $1->tnColumn;
      InsertChildNode($$, $2);
      parseDeleteNode($1);
    }
|
    {
      TRACE_PARSER (fprintf (stderr, "%d: InitValGroup -> (null)\n", ++nCount));
      $$ = parseCreateNode(TN_SLV_INIT, LineNum, Column);
    }
;

FuncBody:
  Identifier L_LPAREN L_RPAREN
    {
      TRACE_PARSER (fprintf (stderr, "%d: FuncBody -> Identifier \"(\" \")\"\n", ++nCount));
      $$ = parseCreateNode(TN_FuncBody, $1->tnLineNo, $1->tnColumn);
      InsertChildNode($$, $1);
      InsertChildNode($$, parseCreateNode(TN_FuncFParams, LineNum, Column));
      parseDeleteNode($2);
      parseDeleteNode($3);
    }
| Identifier L_LPAREN FuncFParams L_RPAREN 
    {
      TRACE_PARSER (fprintf (stderr, "%d: FuncBody -> Identifier \"(\" FuncFParams \")\"\n", ++nCount));
      $$ = parseCreateNode(TN_FuncBody, $1->tnLineNo, $1->tnColumn);
      InsertChildNode($$, $1);
      InsertChildNode($$, $3);
      parseDeleteNode($2);
      parseDeleteNode($4);
    }
;

/* 函数定义  */
FuncDef:
  BType FuncBody Block 
    { 
      TRACE_PARSER (fprintf (stderr, "%d: FuncDef -> BType FuncBody Block\n", ++nCount));
      $$ = parseCreateNode(TN_FuncDef, $1->tnLineNo, $1->tnColumn);
      $$->tnVtyp = $1->tnVtyp;
      InsertChildNode($$, $2);
      InsertChildNode($$, $3);
      parseDeleteNode($1);
    }
| L_VOID FuncBody Block 
    { 
      TRACE_PARSER (fprintf (stderr, "%d: FuncDef -> \"void\" FuncBody Block\n", ++nCount));
      $$ = parseCreateNode(TN_FuncDef, $1->tnLineNo, $1->tnColumn);
      $$->tnVtyp = TYP_VOID;
      InsertChildNode($$, $2);
      InsertChildNode($$, $3);
      parseDeleteNode($1);
    }
;

/* 函数形参表  */
FuncFParams:
  FuncFParam FuncFParamGroup 
    { 
      TRACE_PARSER (fprintf (stderr, "%d: FuncFParams -> FuncFParam FuncFParamGroup\n", ++nCount));
      $$ = $2;
      $$->tnLineNo = $1->tnLineNo;
      $$->tnColumn = $1->tnColumn;
      InsertChildNode($$, $1);
    }
;

FuncFParamGroup:
  L_COMMA FuncFParam FuncFParamGroup 
    { 
      TRACE_PARSER (fprintf (stderr, "%d: FuncFParamGroup -> \",\" FuncFParam FuncFParamGroup\n", ++nCount));
      $$ = $3;
      $$->tnLineNo = $1->tnLineNo;
      $$->tnColumn = $1->tnColumn;
      InsertChildNode($$, $2);
      parseDeleteNode($1);
    }
|  
    { 
      TRACE_PARSER (fprintf (stderr, "%d: FuncFParamGroup -> (null)\n", ++nCount));
      $$ = parseCreateNode(TN_FuncFParams, LineNum, Column);
    }
;

/* 函数形参  */
FuncFParam:
  BType 
    {
      Tree node;
      TRACE_PARSER (fprintf (stderr, "%d: FuncFParam -> BType\n", ++nCount));
      $$ = parseCreateNode(TN_VarDecl, $1->tnLineNo, $1->tnColumn);
      node = parseCreateNode(TN_VarDef, $1->tnLineNo, $1->tnColumn);
      InsertChildNode($$, node);
      $$->tnVtyp = $1->tnVtyp;
      InsertChildNode(node, parseCreateNode(TN_NAME, $1->tnLineNo, $1->tnColumn));
      $$->tnFlags |= TNF_VAR_ARG;
      node->tnFlags |= TNF_VAR_ARG;
      parseDeleteNode($1);
    }
| BType Identifier 
    {
      Tree node;
      TRACE_PARSER (fprintf (stderr, "%d: FuncFParam -> BType Identifier\n", ++nCount));
      $$ = parseCreateNode(TN_VarDecl, $1->tnLineNo, $1->tnColumn);
      node = parseCreateNode(TN_VarDef, $1->tnLineNo, $1->tnColumn);
      InsertChildNode($$, node);
      $$->tnVtyp = $1->tnVtyp;
      InsertChildNode(node, $2);
      $$->tnFlags |= TNF_VAR_ARG;
      node->tnFlags |= TNF_VAR_ARG;
      parseDeleteNode($1);
    }
| BType Identifier L_LBRACK L_RBRACK ExpGroup 
    {
      Tree temp1, temp2;
      Tree node;
      TRACE_PARSER (fprintf (stderr, "%d: FuncFParam -> BType Identifier \"[\" \"]\" ExpGroup\n", ++nCount));
      $$ = parseCreateNode(TN_VarDecl, $1->tnLineNo, $1->tnColumn);
      node = parseCreateNode(TN_VarDef, $1->tnLineNo, $1->tnColumn);
      InsertChildNode($$, node);
      $$->tnVtyp = $1->tnVtyp;
      $$->tnFlags |= TNF_VAR_ARG;
      node->tnFlags |= TNF_VAR_ARG;

      temp1 = $5;
      while (temp1->tnOper != TN_NAME)
        temp1 = *(Tree *)List_First(temp1->children);

      /* 插入一个孩子，代表数组  */
      temp2 = parseCreateNode(TN_INDEX, $1->tnLineNo, $1->tnColumn);
      if (temp1->lpParent)
        {
          *(Tree *)List_First(temp1->lpParent->children) = temp2;
          temp2->lpParent = temp1->lpParent;
        }
      else
        $5 = temp2;
      InsertChildNode(temp2, temp1);
      InsertChildNode(temp2, parseCreateIconNode(0, TYP_INT, $1->tnLineNo, $1->tnColumn));

      InsertChildNode(node, $5);
      temp1->tnName.tnNameId = $2->tnName.tnNameId;
      $2->tnName.tnNameId = NULL;
      parseDeleteNode($2);
      parseDeleteNode($1);
      parseDeleteNode($3);
      parseDeleteNode($4);
    }
;

ExpGroup:
  ExpGroup L_LBRACK Exp L_RBRACK
    { 
      TRACE_PARSER (fprintf (stderr, "%d: ExpGroup -> ExpGroup \"[\" Exp \"]\"\n", ++nCount));
      $$ = parseCreateNode(TN_INDEX, $1->tnLineNo, $1->tnColumn);
      InsertChildNode($$, $1);
      InsertChildNode($$, $3);
      parseDeleteNode($2);
      parseDeleteNode($4);
    }
|  
    { 
      TRACE_PARSER (fprintf (stderr, "%d: ExpGroup -> (null)\n", ++nCount));
      $$ = parseCreateNode(TN_NAME, LineNum, Column);
    }
;

/* 语句块  */
Block:
  L_LCURLY BlockItemGroup L_RCURLY  
    { 
      TRACE_PARSER (fprintf (stderr, "%d: Block -> \"{\" BlockItemGroup \"}\"\n", ++nCount));
      $$ = $2;
      $$->tnLineNo = $1->tnLineNo;
      $$->tnColumn = $1->tnColumn;
      parseDeleteNode($1);
      parseDeleteNode($3);
    }
; 

BlockItemGroup:
  BlockItem BlockItemGroup 
    { 
      TRACE_PARSER (fprintf (stderr, "%d: BlockItemGroup -> BlockItem BlockItemGroup\n", ++nCount));
      $$ = $2;
      $$->tnLineNo = $1->tnLineNo;
      $$->tnColumn = $1->tnColumn;
      InsertChildNode($$, $1);
    }
|  
    { 
      TRACE_PARSER (fprintf (stderr, "%d: BlockItemGroup -> (null)\n", ++nCount));
      $$ = parseCreateNode(TN_BLOCK, LineNum, Column);
    }
;

/* 语句块项  */
BlockItem:
  Decl  
    { 
      TRACE_PARSER (fprintf (stderr, "%d: BlockItem -> Decl\n", ++nCount));
      $$ = $1;
    }
| Stmt 
    { 
      TRACE_PARSER (fprintf (stderr, "%d: BlockItem -> Stmt\n", ++nCount));
      $$ = $1;
    }
;

/* 语句  */
Stmt:
  LVal L_ASSIGN Exp L_SEMI 
    { 
      TRACE_PARSER (fprintf (stderr, "%d: Stmt -> LVal \"=\" Exp \";\"\n", ++nCount));
      $$ = parseCreateNode(TN_ASG, $2->tnLineNo, $2->tnColumn);
      InsertChildNode($$, $1);
      InsertChildNode($$, $3);
      parseDeleteNode($2);
      parseDeleteNode($4);
    }
| L_SEMI  
    { 
      TRACE_PARSER (fprintf (stderr, "%d: Stmt -> \";\"\n", ++nCount));
      $$ = parseCreateNode(TN_EmptyStmt, $1->tnLineNo, $1->tnColumn);
      parseDeleteNode($1);
    }
| Exp L_SEMI  
    { 
      TRACE_PARSER (fprintf (stderr, "%d: Stmt -> Exp \";\"\n", ++nCount));
      $$ = $1;
      parseDeleteNode($2);
    }
| Block 
    {
      TRACE_PARSER (fprintf (stderr, "%d: Stmt -> Block\n", ++nCount));
      $$ = $1;
    }
| L_IF L_LPAREN Cond L_RPAREN Stmt %prec LOWER_THAN_ELSE 
    { 
      TRACE_PARSER (fprintf (stderr, "%d: Stmt -> \"if\" \"(\" Cond \")\" Stmt\n", ++nCount));
      $$ = parseCreateNode(TN_IF, $1->tnLineNo, $1->tnColumn);
      InsertChildNode($$, $3);
      InsertChildNode($$, $5) ;
      parseDeleteNode($1);
      parseDeleteNode($2);
      parseDeleteNode($4);
    }
| L_IF L_LPAREN Cond L_RPAREN Stmt L_ELSE Stmt 
    { 
      TRACE_PARSER (fprintf (stderr, "%d: Stmt -> \"if\" \"(\" Cond \")\" Stmt \"else\" Stmt\n", ++nCount));
      $$ = parseCreateNode(TN_IF, $1->tnLineNo, $1->tnColumn);
      InsertChildNode($$, $3);
      InsertChildNode($$, $5);
      InsertChildNode($$, $7);
      $$->tnFlags |= TNF_IF_HASELSE;
      parseDeleteNode($1);
      parseDeleteNode($2);
      parseDeleteNode($4);
      parseDeleteNode($6);
    }
| L_WHILE L_LPAREN Cond L_RPAREN Stmt 
    { 
      TRACE_PARSER (fprintf (stderr, "%d: Stmt -> \"while\" \"(\" Cond \")\" Stmt\n", ++nCount));
      $$ = parseCreateNode(TN_WHILE, $1->tnLineNo, $1->tnColumn);
      InsertChildNode($$, $3);
      InsertChildNode($$, $5);
      parseDeleteNode($1);
      parseDeleteNode($2);
      parseDeleteNode($4);
    }
| L_BREAK L_SEMI 
    { 
      TRACE_PARSER (fprintf (stderr, "%d: Stmt -> \"break\" \";\"\n", ++nCount));
      $$ = parseCreateNode(TN_BREAK, $1->tnLineNo, $1->tnColumn);
      parseDeleteNode($1);
      parseDeleteNode($2);
    }
| L_CONTINUE L_SEMI 
    { 
      TRACE_PARSER (fprintf (stderr, "%d: Stmt -> \"continue\" \";\"\n", ++nCount));
      $$ = parseCreateNode(TN_CONTINUE, $1->tnLineNo, $1->tnColumn);
      parseDeleteNode($1);
      parseDeleteNode($2);
    }
| L_RETURN L_SEMI  
    { 
      TRACE_PARSER (fprintf (stderr, "%d: Stmt -> \"return\" \";\"\n", ++nCount));
      $$ = parseCreateNode(TN_RETURN, $1->tnLineNo, $1->tnColumn);
      parseDeleteNode($1);
      parseDeleteNode($2);
    }
| L_RETURN Exp L_SEMI  
    { 
      TRACE_PARSER (fprintf (stderr, "%d: Stmt -> \"return\" Exp \";\"\n", ++nCount));
      $$ = parseCreateNode(TN_RETURN, $1->tnLineNo, $1->tnColumn);
      InsertChildNode($$, $2);
      parseDeleteNode($1);
      parseDeleteNode($3);
    }
;

/* 表达式  */
Exp:
  AddExp 
    { 
      TRACE_PARSER (fprintf (stderr, "%d: Exp -> AddExp\n", ++nCount));
      $$ = $1;
    }
;

/* 条件表达式  */
Cond:
  LOrExp 
    { 
      TRACE_PARSER (fprintf (stderr, "%d: Cond -> LOrExp\n", ++nCount));
      $$ = $1;
    }
;

/* 左值表达式  */
LVal:
  Identifier ExpGroup 
    { 
      TRACE_PARSER (fprintf (stderr, "%d: LVal -> Identifier ExpGroup\n", ++nCount));
      $$ = $2;
      $$->tnLineNo = $1->tnLineNo;
      $$->tnColumn = $1->tnColumn;
      while ($2->tnOper != TN_NAME)
        $2 = *(Tree *)List_First($2->children);
      $2->tnName.tnNameId = $1->tnName.tnNameId;
      $1->tnName.tnNameId = NULL;
      parseDeleteNode($1);
      $$->tnFlags |= TNF_LVALUE;
      $2->tnFlags |= TNF_LVALUE;
    }
;

/* 数值  */
Number:
  IntConst  
    { 
      TRACE_PARSER (fprintf (stderr, "%d: Number -> IntConst\n", ++nCount));
      $$ = $1;
    } 
| floatConst  
    { 
      TRACE_PARSER (fprintf (stderr, "%d: Number -> floatConst\n", ++nCount));
      $$ = $1;
    }
;

/* 基本表达式  */
PrimaryExp:
  L_LPAREN Exp L_RPAREN 
    { 
      TRACE_PARSER (fprintf (stderr, "%d: PrimaryExp -> \"(\" Exp \")\"\n", ++nCount));
      $$ = $2;
      parseDeleteNode($1);
      parseDeleteNode($3);
    }
| LVal    
    { 
      TRACE_PARSER (fprintf (stderr, "%d: PrimaryExp -> LVal\n", ++nCount));
      $$ = $1;
    }   
| Number 
    { 
      TRACE_PARSER (fprintf (stderr, "%d: PrimaryExp -> Number\n", ++nCount));
      $$ = $1;
    }
;

/* 一元表达式  */
UnaryExp:
  PrimaryExp 
    {
      TRACE_PARSER (fprintf (stderr, "%d: UnaryExp -> PrimaryExp\n", ++nCount));
      $$ = $1;
    }
| Identifier L_LPAREN L_RPAREN  
    {
      TRACE_PARSER (fprintf (stderr, "%d: UnaryExp -> Identifier \"(\" \")\"\n", ++nCount));
      $$ = $1;
      $$->tnLineNo = $1->tnLineNo;
      $$->tnColumn = $1->tnColumn;
      $$->tnOper = TN_CALL;
      parseDeleteNode($2);
      parseDeleteNode($3);
    }
| Identifier L_LPAREN FuncRParams L_RPAREN  
    {
      TRACE_PARSER (fprintf (stderr, "%d: UnaryExp -> Identifier \"(\" FuncRParams \")\"\n", ++nCount));
      $$ = $3;
      $$->tnLineNo = $1->tnLineNo;
      $$->tnColumn = $1->tnColumn;
      $$->tnOper = TN_CALL;
      $$->tnName.tnNameId = $1->tnName.tnNameId;
      $1->tnName.tnNameId = NULL;
      parseDeleteNode($1);
      parseDeleteNode($2);
      parseDeleteNode($4);
    }
| UnaryOp UnaryExp 
    {
      TRACE_PARSER (fprintf (stderr, "%d: UnaryExp -> UnaryOp UnaryExp\n", ++nCount));
      $$ = $1;
      InsertChildNode($$, $2);
    }
;

/* 单目运算符  */
UnaryOp:
  L_PLUS 
    { 
      TRACE_PARSER (fprintf (stderr, "%d: UnaryOp -> \"+\"\n", ++nCount));
      $$ = parseCreateNode(TN_NOP, $1->tnLineNo, $1->tnColumn);
      parseDeleteNode($1);
    }
| L_MINUS  
    { 
      TRACE_PARSER (fprintf (stderr, "%d: UnaryOp -> \"-\"\n", ++nCount));
      $$ = parseCreateNode(TN_NEG, $1->tnLineNo, $1->tnColumn);
      parseDeleteNode($1);
    }
| L_EXCLAIM  
    { 
      TRACE_PARSER (fprintf (stderr, "%d: UnaryOp -> \"!\"\n", ++nCount));
      $$ = parseCreateNode(TN_LOG_NOT, $1->tnLineNo, $1->tnColumn);
      parseDeleteNode($1);
    }
;

/* 函数实参表  */
FuncRParams:
  Exp FuncRParamsGroup 
    { 
      TRACE_PARSER (fprintf (stderr, "%d: FuncRParams -> Exp FuncRParamsGroup\n", ++nCount));
      $$ = $2;
      $$->tnLineNo = $1->tnLineNo;
      $$->tnColumn = $1->tnColumn;
      InsertChildNode($$, $1);
    }
;

FuncRParamsGroup:
  L_COMMA Exp FuncRParamsGroup 
    { 
      TRACE_PARSER (fprintf (stderr, "%d: FuncRParamsGroup -> \",\" Exp FuncRParamsGroup\n", ++nCount));
      $$ = $3;
      $$->tnLineNo = $1->tnLineNo;
      $$->tnColumn = $1->tnColumn;
      InsertChildNode($$, $2);
      parseDeleteNode($1);
    }
|
    { 
      TRACE_PARSER (fprintf (stderr, "%d: FuncRParamsGroup -> (null)\n", ++nCount));
      $$ = parseCreateNode(TN_CALL, LineNum, Column);
    }
;

/* 乘除模表达式  */
MulExp:
  UnaryExp 
    { 
      TRACE_PARSER (fprintf (stderr, "%d: MulExp -> UnaryExp\n", ++nCount));
      $$ = $1;
    }
| MulExp L_MULT UnaryExp  
    { 
      TRACE_PARSER (fprintf (stderr, "%d: MulExp -> MulExp \"*\" UnaryExp\n", ++nCount));
      $$ = parseCreateNode(TN_MUL, $2->tnLineNo, $2->tnColumn);
      InsertChildNode($$, $1);
      InsertChildNode($$, $3);
      parseDeleteNode($2);
    }
| MulExp L_DIV UnaryExp  
    { 
      TRACE_PARSER (fprintf (stderr, "%d: MulExp -> MulExp \"/\" UnaryExp\n", ++nCount));
      $$ = parseCreateNode(TN_DIV, $2->tnLineNo, $2->tnColumn);
      InsertChildNode($$, $1);
      InsertChildNode($$, $3);
      parseDeleteNode($2);
    }
| MulExp L_MOD UnaryExp  
    { 
      TRACE_PARSER (fprintf (stderr, "%d: MulExp -> MulExp \"%%\" UnaryExp\n", ++nCount));
      $$ = parseCreateNode(TN_MOD, $2->tnLineNo, $2->tnColumn);
      InsertChildNode($$, $1);
      InsertChildNode($$, $3);
      parseDeleteNode($2);
    }
;

/* 加减表达式  */
AddExp:
  MulExp 
    { 
      TRACE_PARSER (fprintf (stderr, "%d: AddExp -> MulExp\n", ++nCount));
      $$ = $1;
    }
| AddExp L_PLUS MulExp  
    { 
      TRACE_PARSER (fprintf (stderr, "%d: AddExp -> AddExp \"+\" MulExp\n", ++nCount));
      $$ = parseCreateNode(TN_ADD, $2->tnLineNo, $2->tnColumn);
      InsertChildNode($$, $1);
      InsertChildNode($$, $3);
      parseDeleteNode($2);
    }
| AddExp L_MINUS MulExp  
    { 
      TRACE_PARSER (fprintf (stderr, "%d: AddExp -> AddExp \"-\" MulExp\n", ++nCount));
      $$ = parseCreateNode(TN_SUB, $2->tnLineNo, $2->tnColumn);
      InsertChildNode($$, $1);
      InsertChildNode($$, $3);
      parseDeleteNode($2);
    }
;

/* 关系表达式  */
RelExp:
  AddExp 
    { 
      TRACE_PARSER (fprintf (stderr, "%d: RelExp -> AddExp\n", ++nCount));
      $$ = $1;
    }
| RelExp L_LT AddExp  
    { 
      TRACE_PARSER (fprintf (stderr, "%d: RelExp -> RelExp \"<\" AddExp\n", ++nCount));
      $$ = parseCreateNode(TN_LT, $2->tnLineNo, $2->tnColumn);
      InsertChildNode($$, $1);
      InsertChildNode($$, $3);
      parseDeleteNode($2);
    }
| RelExp L_GT AddExp 
    { 
      TRACE_PARSER (fprintf (stderr, "%d: RelExp -> RelExp \">\" AddExp\n", ++nCount));
      $$ = parseCreateNode(TN_GT, $2->tnLineNo, $2->tnColumn);
      InsertChildNode($$, $1);
      InsertChildNode($$, $3);
      parseDeleteNode($2);
    }
| RelExp L_LTEQ AddExp  
    { 
      TRACE_PARSER (fprintf (stderr, "%d: RelExp -> RelExp \"<=\" AddExp\n", ++nCount));
      $$ = parseCreateNode(TN_LE, $2->tnLineNo, $2->tnColumn);
      InsertChildNode($$, $1);
      InsertChildNode($$, $3);
      parseDeleteNode($2);
    }
| RelExp L_GTEQ AddExp 
    { 
      TRACE_PARSER (fprintf (stderr, "%d: RelExp -> RelExp \">=\" AddExp\n", ++nCount));
      $$ = parseCreateNode(TN_GE, $2->tnLineNo, $2->tnColumn);
      InsertChildNode($$, $1);
      InsertChildNode($$, $3);
      parseDeleteNode($2);
    } 
;

/* 相等性表达式  */
EqExp:
  RelExp  
    { 
      TRACE_PARSER (fprintf (stderr, "%d: EqExp -> RelExp\n", ++nCount));
      $$ = $1;
    }
| EqExp L_EQUALS RelExp  
    { 
      TRACE_PARSER (fprintf (stderr, "%d: EqExp -> EqExp \"==\" RelExp\n", ++nCount));
      $$ = parseCreateNode(TN_EQ, $2->tnLineNo, $2->tnColumn);
      InsertChildNode($$, $1);
      InsertChildNode($$, $3);
      parseDeleteNode($2);
    }
| EqExp L_NOTEQ RelExp  
    { 
      TRACE_PARSER (fprintf (stderr, "%d: EqExp -> EqExp \"!=\" RelExp\n", ++nCount));
      $$ = parseCreateNode(TN_NE, $2->tnLineNo, $2->tnColumn);
      InsertChildNode($$, $1);
      InsertChildNode($$, $3);
      parseDeleteNode($2);
    }
;

/* 逻辑与表达式  */
LAndExp:
  EqExp  
    { 
      TRACE_PARSER (fprintf (stderr, "%d: LAndExp -> EqExp\n", ++nCount));
      $$ = $1;
    }
| LAndExp L_ANDAND EqExp  
    { 
      TRACE_PARSER (fprintf (stderr, "%d: LAndExp -> LAndExp \"&&\" EqExp\n", ++nCount));
      $$ = parseCreateNode(TN_LOG_AND, $2->tnLineNo, $2->tnColumn);
      InsertChildNode($$, $1);
      InsertChildNode($$, $3);
      parseDeleteNode($2);
    }
;

/* 逻辑或表达式  */
LOrExp:
  LAndExp  
    { 
      TRACE_PARSER (fprintf (stderr, "%d: LOrExp -> LAndExp\n", ++nCount));
      $$ = $1;
    }
| LOrExp L_OROR LAndExp  
    { 
      TRACE_PARSER (fprintf (stderr, "%d: LOrExp -> LOrExp \"||\" LAndExp\n", ++nCount));
      $$ = parseCreateNode(TN_LOG_OR, $2->tnLineNo, $2->tnColumn);
      InsertChildNode($$, $1);
      InsertChildNode($$, $3);
      parseDeleteNode($2);
    }
;

/* 常量表达式  */
ConstExp:
  AddExp 
    { 
      TRACE_PARSER (fprintf (stderr, "%d: ConstExp -> AddExp\n", ++nCount));
      $$ = $1;
    }
;

/* MiniC文法  */
TN_FuncBodys:
  TN_FuncBodys L_COMMA FuncBody
    { 
      TRACE_PARSER (fprintf (stderr, "%d: TN_FuncBodys -> TN_FuncBodys \",\" FuncBody\n", ++nCount));
      $$ = $1;
      InsertChildNode($$, $3);
      parseDeleteNode($2);
    }
| FuncBody
    { 
      TRACE_PARSER (fprintf (stderr, "%d: TN_FuncBodys -> FuncBody\n", ++nCount));
      $$ = parseCreateNode(TN_FuncDecl, $1->tnLineNo, $1->tnColumn);
      InsertChildNode($$, $1);
    }
;

/* “伪”函数声明  */
FuncDecl:
  BType TN_FuncBodys L_SEMI 
    {
      TRACE_PARSER (fprintf (stderr, "%d: FuncDecl -> BType TN_FuncBodys \";\"\n", ++nCount));
      $$ = $2;
      $$->tnLineNo = $1->tnLineNo;
      $$->tnColumn = $1->tnColumn;
      $$->tnVtyp = $1->tnVtyp;
      parseDeleteNode($1);
      parseDeleteNode($3);
    }
| L_VOID TN_FuncBodys L_SEMI 
    {
      TRACE_PARSER (fprintf (stderr, "%d: FuncDecl -> \"void\" TN_FuncBodys \";\"\n", ++nCount));
      $$ = $2;
      $$->tnLineNo = $1->tnLineNo;
      $$->tnColumn = $1->tnColumn;
      $$->tnVtyp = TYP_VOID;
      parseDeleteNode($1);
      parseDeleteNode($3);
    }
;

%%
