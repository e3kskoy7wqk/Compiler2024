#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "all.h"
#include "lex.yy.h"

Compiler comp;

/* 此程序被调用的名称。  */
char *progname;

/* 错误计数 */
int errors = 0;

/* 图18-23 优化顺序，

   引用:

     高级编译器设计与实现,
     Steven Muchnick, Morgan Kaufmann, 1997, Section 18.14.  */
static struct pass_data opt_pass[] = {
    { "Inline transform" , inline_transform  } ,    /* 内联替换 */
    { "Global Variable Localization" , GlobalVariableLocalization  } ,    /* 全局变量局部化 */
    { "builds the SSA form" , build_ssa  } ,    /* SSA形式的构建 */
    { "Sparse Cond Const Prop" , SparseCondConstProp  } ,    /* 稀疏条件常量传播 */
    { "Global Value Numbering" , GlobalValueNumbering  } ,    /* 全局值编号 */
    { "Dead Code Elimination" , perform_ssa_dce  } ,    /* 死代码消除 */
    { "Translating out of SSA form" , remove_ssa_form  } ,    /* SSA形式的消去 */
    { "Data-flow based copy propagation pass" , copyprop  } ,    /* 复写传播 */
    { "Lazy Code Motion" , LazyCodeMotion  } ,    /* 懒惰代码移动 */
    { "Operator Strength Reduction" , OSR  } ,    /* 运算符强度削减 */
#if 0
    { "Loop unrolling" , unroll_loops  } ,    /* 循环展开 */
#endif
    { "builds the SSA form" , build_ssa  } ,    /* SSA形式的构建 */
    { "Dead Code Elimination" , perform_ssa_dce  } ,    /* 死代码消除 */
    { "Tree-Height Reduction" , treeheight  } ,    /* 树高平衡 */
    { "Sparse Cond Const Prop" , SparseCondConstProp  } ,    /* 稀疏条件常量传播 */
    { "Global Value Numbering" , GlobalValueNumbering  } ,    /* 全局值编号 */
    { "Dead Code Elimination" , perform_ssa_dce  } ,    /* 死代码消除 */
    { "Translating out of SSA form" , remove_ssa_form  } ,    /* SSA形式的消去 */
    { "Data-flow based copy propagation pass" , copyprop  } ,    /* 复写传播 */
    { "Strength reduction" , StraightLineStrengthReduce  } ,    /* 弱强度削减 */
    { "builds the SSA form" , build_ssa  } ,    /* SSA形式的构建 */
    { "Dead Code Elimination" , perform_ssa_dce  } ,    /* 死代码消除 */
    { "Translating out of SSA form" , rewrite_out_of_ssa  } ,    /* SSA形式的消去 */
    { "Data-flow based copy propagation pass" , copyprop  } ,    /* 复写传播 */
};


/* 返回一个指向完全路径名中文件名部分的指针。  */
char *Abbrev (char *lpsz)
{
    char *lpszTemp;
   
    lpszTemp = lpsz + strlen(lpsz) - 1;
    while (lpszTemp > lpsz && lpszTemp[-1] != '\\' && lpszTemp[-1] != '/')
        lpszTemp--;
    return lpszTemp;
}

void
fatal (const char *str, ...)
{
    va_list va;
    fprintf (stderr, "%s: fatal error: ", progname);
    va_start(va, str);
    vfprintf (stderr, str, va);
    va_end(va);
    fprintf (stderr, "\n");
    exit (1);
}

/* 打印错误消息并增加错误计数。  */
void
error (const char *fname, int line, const char *msg, ...)
{
    va_list va;
    if (fname != NULL)
        fprintf (stderr, "%s:%d: error: ", fname, line);
    else
        fprintf (stderr, "%s: error: ", progname);
    va_start(va, msg);
    vfprintf (stderr, msg, va);
    va_end(va);
    fprintf (stderr, "\n");
    errors++;
}

void
sorry (const char *fname, int line, const char *msg, ...)
{
    va_list va;
    if (fname != NULL)
        fprintf (stderr, "%s:%d: sorry, not implemented: ", fname, line);
    else
        fprintf (stderr, "%s: sorry, not implemented: ", progname);
    va_start(va, msg);
    vfprintf (stderr, msg, va);
    va_end(va);
    fprintf (stderr, "\n");
}

void
warning (const char *fname, int line, const char *msg, ...)
{
    va_list va;
    if (fname != NULL)
        fprintf (stderr, "%s:%d: warning: ", fname, line);
    else
        fprintf (stderr, "%s: warning: ", progname);
    va_start(va, msg);
    vfprintf (stderr, msg, va);
    va_end(va);
    fprintf (stderr, "\n");
}

/* 打印出程序的用法表格。  */
static void display_help(char *szProgramname)
{
    fprintf(stdout, "Usage: %s [options] file...\n", szProgramname);
    fprintf(stdout, "Options:\n");
    fprintf(stdout, "  -?                       Display this information.\n");
    fprintf(stdout, "  -S                       Compile only; do not assemble or link.\n");
    fprintf(stdout, "  -o <file>                Place the output into <file>.\n");
    fprintf(stdout, "\nThe following options control optimizations:\n");
    fprintf(stdout, "  -O<number>\tSet optimization level to <number>.\n");
}

/* 验证传入的选项并相应地设置options结构。  */
static BOOL ValidOptions(char *argv[], int argc, Compiler comp)
{
    int i;
    memset (&comp->cmpConfig, 0, sizeof (comp->cmpConfig));

    for( i=1; i<argc; i++ ) {
        if( argv[i][0] == '-' ) {
            switch( argv[i][1] ) {
            case 'O' :
                if( strlen(argv[i]) > 2 )
                    comp->cmpConfig.optimize = atoi(&argv[i][2]);
                break;

            case 'o' :
                /* 首先检查是否还有一个参数。  */
                if (i + 1 >= argc)
                {
                    printf( "ERROR: FileName missing after -o switch\n" );
                    display_help(argv[0]);
                    return(FALSE);
                }

                comp->cmpConfig.output_file_name = argv[i+1];

                /* 表明我们已经处理了下一个参数。  */
                i++; 

                break;

            case 'S' :
                break;

            case 'g' :
                comp->cmpConfig.debug_info = TRUE;
                break;

#if defined(zenglj)
            case 'I' :
                comp->cmpConfig.DragonIR = TRUE;
                break;
#endif

            case '?' :
            case 'h' :
                display_help(argv[0]);
                return(FALSE);
                break;

            default:
                printf("  unknown options flag %s\n", argv[i]);
                display_help(argv[0]);
                return(FALSE);
                break;
            }
        } else {
            comp->cmpConfig.input_file_name = argv[i];
        }
    }

    return(TRUE);
}

static Tree ParseFile(const char *filename, Tree syntaxTree)
{
    extern int yyparse(void);

    BOOL fSuccess = TRUE; /* 假设成功 */
    extern FILE *yyin;
    extern Tree g_savedTree;
    Tree *Cursor;

    g_savedTree = NULL;
    LineNum = 1;

    yyin = fopen (filename, "r");
    if (NULL == yyin) {
       fatal ("Unable to open '%s'.", filename);
       fSuccess = FALSE;
    }
    if (0 == yyparse ()) {
#if !defined(NDEBUG)
       fprintf (stderr, "Parse of '%s' was successful.  %i lines were parsed.\n", filename, LineNum);
#endif
    } else {
#if !defined(NDEBUG)
       fprintf (stderr, "Parse of '%s' was not successful.\n", filename);
#endif
       fSuccess = FALSE;
    }

    fclose(yyin);
    yylex_destroy ();

    if  (syntaxTree && fSuccess)
    {
        for(  Cursor=(Tree *)List_First(g_savedTree->children)
            ;  Cursor!=NULL
            ;  Cursor = (Tree *)List_Next((void *)Cursor)
            )
            InsertChildNode(syntaxTree, *Cursor);
        parseDeleteNode(g_savedTree);
        g_savedTree = syntaxTree;
    }

    if(fSuccess)
        return g_savedTree;
    else
    {
        DestroyTree(g_savedTree);
        return NULL;
    }
}

static Tree ParseString(const char *lpBuffer, Tree syntaxTree)
{
    extern int yyparse(void);

    BOOL fSuccess = TRUE; /* 假设成功 */
    extern FILE *yyin;
    extern Tree g_savedTree;
    Tree *Cursor;

    g_savedTree = NULL;
    LineNum = 1;

    yy_scan_string(lpBuffer);
    if (0 == yyparse ()) {
#if !defined(NDEBUG)
       fprintf (stderr, "Parse of '%s' was successful.  %i lines were parsed.\n", lpBuffer, LineNum);
#endif
    } else {
#if !defined(NDEBUG)
       fprintf (stderr, "Parse of '%s' was not successful.\n", lpBuffer);
#endif
       fSuccess = FALSE;
    }

    yylex_destroy ();

    if  (syntaxTree && fSuccess)
    {
        for(  Cursor=(Tree *)List_First(g_savedTree->children)
            ;  Cursor!=NULL
            ;  Cursor = (Tree *)List_Next((void *)Cursor)
            )
            InsertChildNode(syntaxTree, *Cursor);
        parseDeleteNode(g_savedTree);
        g_savedTree = syntaxTree;
    }

    if(fSuccess)
        return g_savedTree;
    else
        return NULL;
}

BOOL cmpInit(Compiler comp)
{
    comp->code = NULL;
    comp->syntaxTree = NULL;

    /* 初始化符号表。  */
    comp->cmpCurST = (SymTab)xmalloc (sizeof (*comp->cmpCurST));
    stInit (comp->cmpCurST);

    /* 分配中间IR指令序列。  */
    comp->code = InterCodeNew();

    return TRUE;
}

void cmpDone(Compiler comp)
{
    /* 销毁中间IR指令序列。  */
    InterCodeDelete (comp->code);

    /* 销毁符号表。  */
    stDeinit (comp->cmpCurST);
    free (comp->cmpCurST);

    /* 销毁抽象语法树。  */
    DestroyTree (comp->syntaxTree);
}

BOOL cmpStart(Compiler comp, const char *defOutFileName)
{
    BOOL result = FALSE;
    const   char *  outfnm;
    int i;
    const char sylib[] =
    "/* Input & output functions */\n"
    "int getint(),getch(),getarray(int a[]);\n"
    "float getfloat();\n"
    "int getfarray(float a[]);\n"
    "\n"
    "void putint(int a),putch(int a),putarray(int n,int a[]);\n"
    "void putfloat(float a);\n"
    "void putfarray(int n, float a[]);\n"
    "\n"
    "/* Timing function implementation */\n"
    "void _sysy_starttime(int lineno);\n"
    "void _sysy_stoptime(int lineno);\n"
    "\n"
    "/* memset - set a section of memory to all one byte.  */\n"
    "void memset (int dest[], int ch, int len);\n"
    ;

    /* 创建抽象语法树。  */
    comp->syntaxTree = ParseString(sylib, NULL);
    if( !comp->syntaxTree )
        goto DONE;
    comp->syntaxTree = ParseFile(comp->cmpConfig.input_file_name, comp->syntaxTree);
    if( !comp->syntaxTree )
        goto DONE;

#if !defined(NDEBUG)

#if 0
    if (1)
    {
        /* 转储抽象语法树。  */
        char *lpTmpFile = NULL;
        lpTmpFile = (char *)xmalloc (strlen(comp->cmpConfig.input_file_name)+10);
        strcpy (lpTmpFile, comp->cmpConfig.input_file_name);
        strcat (lpTmpFile, ".ast.pdf");
        ASTDumper(comp->syntaxTree, lpTmpFile);
        free (lpTmpFile);
    }
#endif

#endif

    /* 遍历抽象语法树生成线性IR。  */
    if( !genIR(comp->syntaxTree, comp->code, comp->cmpCurST) )
        goto DONE;

#if !defined(NDEBUG)
#if 0
    /* 转储线性IR。  */
    InterCodeDump(comp->code, stderr);
#endif

#endif

    /* 在生成线性IR之后，仍会创建临时变量。
       我们创建一个临时作用域，用来存放这些临时变量。  */
    comp->cmpCurST->cmpCurScp = comp->cmpCurST->sym;
    comp->cmpCurST->cmpCurScp = stDeclareSym(comp->cmpCurST, NULL, SYM_SCOPE);
    comp->cmpCurST->cmpCurScp->sdType = stAllocTypDef(TYP_UNDEF);

    /* 构建控制流图。  */
    cfgbuild (comp->code, comp->cmpCurST);

    /* 机器无关优化。  */
    if  (comp->cmpConfig.optimize)
    {
        for (i = 0; i < sizeof(opt_pass) / sizeof(opt_pass[0]); i++)
        {
            opt_pass[i].callback (comp->code, comp->cmpCurST);
        }
    }

#if !defined(NDEBUG)

#if 1
    if (1)
    {
        /* 转储控制流图。  */
        char *lpTmpFile = NULL;
        lpTmpFile = (char *)xmalloc (strlen(comp->cmpConfig.input_file_name)+10);
        strcpy (lpTmpFile, comp->cmpConfig.input_file_name);
        strcat (lpTmpFile, ".cfg.pdf");
        dump_cfg(comp->code, lpTmpFile);
        free (lpTmpFile);
    }
#endif

#endif

    outfnm = comp->cmpConfig.output_file_name;
    if  (outfnm == NULL)
    {
        if (defOutFileName && defOutFileName[0])
        {
            outfnm = defOutFileName;
        }
        else
            fatal("Name of output file not specified, please use -o");
    }

#if defined(zenglj)
    if  (comp->cmpConfig.DragonIR)
    {
        FILE *fp;
        fp=fopen(outfnm,"w");
        if (NULL == fp)
        {
            fprintf (stderr, "error opening %s\n", outfnm);
            goto DONE;
        }
        dump_cfg_zenglj (comp->code, comp->cmpCurST, fp);
        fclose (fp);

        result = TRUE;
        goto DONE;
    }
#endif

    /* 代码生成。  */
    if (1)
    {
        FILE *fp;
        fp=fopen(outfnm,"w");
        if (NULL == fp)
        {
            fprintf (stderr, "error opening %s\n", outfnm);
            goto DONE;
        }

        if( !codegen (comp->code, comp->cmpCurST, comp->cmpConfig.input_file_name, fp, comp->cmpConfig.debug_info) )
            goto DONE;

        fclose (fp);

        result = TRUE;
        goto DONE;
    }

    result = TRUE;

DONE:

#if !defined(NDEBUG)

#if 0
    /* 转储符号表。  */
    DumpSymbolTable(comp->cmpCurST, stderr);
#endif

#endif

    return result;
}

int main(int argc, char **argv)
{
    int errorlevel = 1;

#if defined (_WIN32) && !defined(NDEBUG)
    /* 
    * Define the report destination(s) for each type of report
    * we are going to generate.  In this case, we are going to
    * generate a report for every report type: _CRT_WARN,
    * _CRT_ERROR, and _CRT_ASSERT.
    * The destination(s) is defined by specifying the report mode(s)
    * and report file for each report type.
    * This program sends all report types to STDOUT.
    */                                             
   _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
   _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);
   _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
   _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDOUT);
   _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
   _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDOUT);


#endif

#if !defined(_WIN32) && !defined(NDEBUG)
    mtrace ();
#endif

    progname = Abbrev(argv[0]);

    List_Init();

    comp = (Compiler)xmalloc(sizeof (*comp));

    if( !ValidOptions(argv, argc, comp) )
        goto cleanup;

    if (!comp->cmpConfig.input_file_name)
        fatal ("No input file name.");

    if( !cmpInit(comp) )
        goto cleanup;

    if( !cmpStart(comp, "a.s") )
        goto cleanup;

    errorlevel = 0;

cleanup:

    cmpDone(comp);
    free (comp);

    List_Term();
    bitmap_terminate ();

#if !defined(NDEBUG)
    fprintf (stderr, "Program exiting: return code = %d\n", errorlevel);
#endif

#if defined (_WIN32) && !defined(NDEBUG)
    _CrtDumpMemoryLeaks( );
#endif

#if !defined(_WIN32) && !defined(NDEBUG)
    muntrace ();
#endif

    return errorlevel;
}
