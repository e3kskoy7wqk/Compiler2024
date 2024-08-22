/* A Bison parser, made by GNU Bison 3.3.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2019 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Undocumented macros, especially those whose name start with YY_,
   are private implementation details.  Do not rely on them.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.3.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 11 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:337  */

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

  // helper macro for short definition of trace-output inside code
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


#line 113 "grammar.tab.c" /* yacc.c:337  */
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* In a future release of Bison, this section will be replaced
   by #include "grammar.tab.h".  */
#ifndef YY_YY_GRAMMAR_TAB_H_INCLUDED
# define YY_YY_GRAMMAR_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    LOWER_THAN_ELSE = 258,
    L_ELSE = 259,
    L_CONST = 260,
    L_INT = 261,
    L_FLOAT = 262,
    L_VOID = 263,
    L_AND = 264,
    L_ANDAND = 265,
    L_ANDEQ = 266,
    L_ASSIGN = 267,
    L_COLON = 268,
    L_COMMA = 269,
    L_DECR = 270,
    L_DIV = 271,
    L_DIVEQ = 272,
    L_EQUALS = 273,
    L_EXCLAIM = 274,
    L_GT = 275,
    L_GTEQ = 276,
    L_LBRACK = 277,
    L_LT = 278,
    L_LTEQ = 279,
    L_MINUS = 280,
    L_MINUSEQ = 281,
    L_MOD = 282,
    L_MULT = 283,
    L_MULTEQ = 284,
    L_NOTEQ = 285,
    L_OR = 286,
    L_OREQ = 287,
    L_OROR = 288,
    L_PERIOD = 289,
    L_PLUS = 290,
    L_QUEST = 291,
    L_TILDE = 292,
    L_XOR = 293,
    L_XOREQ = 294,
    L_BREAK = 295,
    L_CASE = 296,
    L_CONTINUE = 297,
    L_DEFAULT = 298,
    L_DO = 299,
    L_FOR = 300,
    L_GOTO = 301,
    L_IF = 302,
    L_LCURLY = 303,
    L_LPAREN = 304,
    L_RBRACK = 305,
    L_RCURLY = 306,
    L_RETURN = 307,
    L_RPAREN = 308,
    L_SEMI = 309,
    L_SIZEOF = 310,
    L_SW = 311,
    L_WHILE = 312,
    Identifier = 313,
    IntConst = 314,
    floatConst = 315
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 58 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:352  */

    float floatValue;
    unsigned intValue;
    Tree node;

#line 223 "grammar.tab.c" /* yacc.c:352  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_GRAMMAR_TAB_H_INCLUDED  */



#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

#ifndef YY_ATTRIBUTE
# if (defined __GNUC__                                               \
      && (2 < __GNUC__ || (__GNUC__ == 2 && 96 <= __GNUC_MINOR__)))  \
     || defined __SUNPRO_C && 0x5110 <= __SUNPRO_C
#  define YY_ATTRIBUTE(Spec) __attribute__(Spec)
# else
#  define YY_ATTRIBUTE(Spec) /* empty */
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# define YY_ATTRIBUTE_PURE   YY_ATTRIBUTE ((__pure__))
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# define YY_ATTRIBUTE_UNUSED YY_ATTRIBUTE ((__unused__))
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && ! defined __ICC && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")\
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif


#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYSIZE_T yynewbytes;                                            \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / sizeof (*yyptr);                          \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  16
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   238

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  61
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  43
/* YYNRULES -- Number of rules.  */
#define YYNRULES  101
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  183

#define YYUNDEFTOK  2
#define YYMAXUTOK   315

/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                                \
  ((unsigned) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   182,   182,   189,   195,   202,   212,   217,   222,   231,
     246,   254,   262,   268,   278,   295,   305,   313,   319,   326,
     339,   349,   357,   371,   381,   389,   400,   418,   424,   431,
     444,   454,   461,   470,   483,   492,   505,   516,   526,   534,
     547,   560,   599,   609,   617,   629,   638,   646,   651,   660,
     669,   675,   681,   686,   696,   709,   719,   726,   733,   740,
     752,   761,   770,   788,   793,   802,   809,   814,   823,   828,
     838,   851,   861,   867,   873,   883,   894,   904,   912,   917,
     925,   933,   945,   950,   958,   970,   975,   983,   991,   999,
    1011,  1016,  1024,  1036,  1041,  1053,  1058,  1070,  1079,  1086,
    1096,  1106
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "LOWER_THAN_ELSE", "\"else\"",
  "\"const\"", "\"int\"", "\"float\"", "\"void\"", "\"&\"", "\"&&\"",
  "\"&=\"", "\"=\"", "\":\"", "\",\"", "\"--\"", "\"/\"", "\"/=\"",
  "\"==\"", "\"!\"", "\">\"", "\">=\"", "\"[\"", "\"<\"", "\"<=\"",
  "\"-\"", "\"-=\"", "\"%\"", "\"*\"", "\"*=\"", "\"!=\"", "\"|\"",
  "\"|=\"", "\"||\"", "\".\"", "\"+\"", "\"?\"", "\"~\"", "\"^\"",
  "\"^=\"", "\"break\"", "\"case\"", "\"continue\"", "\"default\"",
  "\"do\"", "\"for\"", "\"goto\"", "\"if\"", "\"{\"", "\"(\"", "\"]\"",
  "\"}\"", "\"return\"", "\")\"", "\";\"", "\"sizeof\"", "\"switch\"",
  "\"while\"", "Identifier", "IntConst", "floatConst", "$accept",
  "CompUnit", "Decl", "ConstDecl", "ConstDefGroup", "BType", "ConstDef",
  "ConstExpGroup", "ConstInitVal", "ConstInitValGroup", "VarDecl",
  "VarDefGroup", "VarDef", "InitVal", "InitValGroup", "FuncBody",
  "FuncDef", "FuncFParams", "FuncFParamGroup", "FuncFParam", "ExpGroup",
  "Block", "BlockItemGroup", "BlockItem", "Stmt", "Exp", "Cond", "LVal",
  "Number", "PrimaryExp", "UnaryExp", "UnaryOp", "FuncRParams",
  "FuncRParamsGroup", "MulExp", "AddExp", "RelExp", "EqExp", "LAndExp",
  "LOrExp", "ConstExp", "TN_FuncBodys", "FuncDecl", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315
};
# endif

#define YYPACT_NINF -110

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-110)))

#define YYTABLE_NINF -1

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     137,    70,  -110,  -110,   -50,   155,  -110,  -110,   -41,  -110,
    -110,  -110,   -32,   -11,     3,     4,  -110,  -110,  -110,   -11,
      49,     3,    13,  -110,    76,     0,    54,  -110,   -50,  -110,
      18,    37,    44,  -110,  -110,    56,   -32,    46,  -110,    52,
      68,    93,   -50,  -110,  -110,  -110,    72,    77,    75,     6,
     134,  -110,    81,    87,  -110,  -110,  -110,   -41,  -110,    97,
      54,  -110,    96,   142,  -110,  -110,  -110,     6,    -7,   -13,
    -110,   147,     6,  -110,    49,  -110,   152,    76,  -110,   146,
    -110,    70,  -110,  -110,  -110,  -110,     6,   117,  -110,  -110,
     119,     6,   178,   156,  -110,  -110,  -110,     6,  -110,     6,
       6,     6,     6,     6,    98,  -110,  -110,   -13,   129,  -110,
     116,  -110,  -110,  -110,   130,    93,   128,   -13,    51,    -2,
     174,   153,  -110,  -110,   132,  -110,   175,   138,     6,   136,
    -110,  -110,  -110,    -7,    -7,  -110,   184,  -110,  -110,   185,
    -110,  -110,    80,     6,     6,     6,     6,     6,     6,     6,
       6,    80,     6,  -110,  -110,   154,  -110,   147,   151,   152,
     157,   156,   205,   -13,   -13,   -13,   -13,    51,    51,    -2,
     174,  -110,   175,  -110,   184,  -110,   185,  -110,    80,  -110,
    -110,  -110,  -110
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,    12,    13,     0,     0,     4,     6,     0,     7,
       2,     8,     0,     0,    99,     0,     1,     5,     3,    16,
      24,    99,     0,    16,    11,     0,    46,    35,     0,   101,
      25,     0,     0,    34,   100,     0,     0,     0,    32,    39,
       0,    38,     0,    74,    73,    72,     0,     0,     0,     0,
       0,    50,     0,    43,    63,    64,    47,     0,    52,     0,
      46,    48,     0,    66,    67,    68,    78,     0,    82,    60,
      98,     0,     0,    16,    24,    22,     0,    11,     9,    40,
      33,     0,    36,    99,    56,    57,     0,     0,    66,    58,
       0,     0,     0,    62,    44,    45,    51,     0,    71,     0,
       0,     0,     0,     0,     0,    26,    27,    97,     0,    23,
       0,    14,    17,    10,     0,    38,     0,    85,    90,    93,
      95,    61,    65,    59,     0,    69,    77,     0,     0,     0,
      80,    81,    79,    84,    83,    28,    31,    15,    18,    21,
      43,    37,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    75,    70,     0,    49,     0,     0,     0,
       0,    41,    53,    87,    89,    86,    88,    91,    92,    94,
      96,    55,    77,    42,    31,    29,    21,    19,     0,    76,
      30,    20,    54
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -110,  -110,    32,  -110,   139,    10,   179,   191,  -107,    41,
    -110,   144,   188,  -100,    47,    11,   215,  -110,   107,   143,
      83,    29,   165,  -110,  -109,   -48,   135,   -26,  -110,  -110,
     -54,  -110,  -110,    57,   -10,   -62,   -29,    79,    82,  -110,
     158,    -3,  -110
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     5,    56,     7,    37,     8,    24,    30,   111,   160,
       9,    32,    20,   105,   158,    83,    10,    40,    82,    41,
      93,    58,    59,    60,    61,    62,   116,    88,    64,    65,
      66,    67,   127,   153,    68,    69,   118,   119,   120,   121,
     112,    15,    11
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_uint8 yytable[] =
{
      63,    87,    90,   139,   136,    22,     2,     3,    13,    99,
     107,    12,   102,    98,   107,    14,   147,    19,    28,    21,
     100,   101,   103,   106,   117,    43,    23,    28,   148,   117,
      71,    44,     6,   162,    63,    39,    57,    17,    25,    70,
      72,    45,   171,    27,   126,   130,   131,   132,   107,   129,
      33,    26,   176,    38,    22,    49,   106,   174,    29,     1,
       2,     3,    42,    31,    53,    54,    55,    34,    76,   182,
      57,   143,   144,    43,   145,   146,     2,     3,    72,    44,
     155,   163,   164,   165,   166,   117,   117,   117,   117,    45,
      36,    39,   133,   134,    46,    73,    47,   107,    75,    43,
      78,    48,    26,    49,   172,    44,    50,    81,    51,   106,
      79,    52,    53,    54,    55,    45,    63,    43,   167,   168,
      46,    80,    47,    44,    86,    63,    84,    48,    26,    49,
      91,    85,    50,    45,    51,    43,    92,    52,    53,    54,
      55,    44,     1,     2,     3,     4,   104,    49,    94,   135,
      96,    45,    63,    43,    97,    16,    53,    54,    55,    44,
       1,     2,     3,     4,   110,    49,    43,   138,   114,    45,
     122,    43,    44,   123,    53,    54,    55,    44,   128,   137,
     140,   142,    45,    49,   149,   151,   150,    45,    89,   152,
     156,   154,    53,    54,    55,   104,    49,    43,   157,   159,
     110,    49,   175,    44,   173,    53,    54,    55,   177,   178,
      53,    54,    55,    45,    35,    77,   113,   181,   109,    74,
      18,   180,   141,   161,   115,    95,   124,    49,   169,   179,
     108,   125,   170,     0,     0,     0,    53,    54,    55
};

static const yytype_int16 yycheck[] =
{
      26,    49,    50,   110,   104,     8,     6,     7,    58,    16,
      72,     1,    25,    67,    76,     4,    18,    58,    14,     8,
      27,    28,    35,    71,    86,    19,    58,    14,    30,    91,
      12,    25,     0,   142,    60,    25,    26,     5,    49,    28,
      22,    35,   151,    14,    92,    99,   100,   101,   110,    97,
      21,    48,   159,    53,    57,    49,   104,   157,    54,     5,
       6,     7,     8,    14,    58,    59,    60,    54,    12,   178,
      60,    20,    21,    19,    23,    24,     6,     7,    22,    25,
     128,   143,   144,   145,   146,   147,   148,   149,   150,    35,
      14,    81,   102,   103,    40,    58,    42,   159,    54,    19,
      54,    47,    48,    49,   152,    25,    52,    14,    54,   157,
      58,    57,    58,    59,    60,    35,   142,    19,   147,   148,
      40,    53,    42,    25,    49,   151,    54,    47,    48,    49,
      49,    54,    52,    35,    54,    19,    49,    57,    58,    59,
      60,    25,     5,     6,     7,     8,    48,    49,    51,    51,
      54,    35,   178,    19,    12,     0,    58,    59,    60,    25,
       5,     6,     7,     8,    48,    49,    19,    51,    22,    35,
      53,    19,    25,    54,    58,    59,    60,    25,    22,    50,
      50,    53,    35,    49,    10,    53,    33,    35,    54,    14,
      54,    53,    58,    59,    60,    48,    49,    19,    14,    14,
      48,    49,    51,    25,    50,    58,    59,    60,    51,     4,
      58,    59,    60,    35,    23,    36,    77,   176,    74,    31,
       5,   174,   115,   140,    81,    60,    91,    49,   149,   172,
      72,    53,   150,    -1,    -1,    -1,    58,    59,    60
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     5,     6,     7,     8,    62,    63,    64,    66,    71,
      77,   103,    66,    58,    76,   102,     0,    63,    77,    58,
      73,    76,   102,    58,    67,    49,    48,    82,    14,    54,
      68,    14,    72,    82,    54,    68,    14,    65,    53,    66,
      78,    80,     8,    19,    25,    35,    40,    42,    47,    49,
      52,    54,    57,    58,    59,    60,    63,    66,    82,    83,
      84,    85,    86,    88,    89,    90,    91,    92,    95,    96,
      76,    12,    22,    58,    73,    54,    12,    67,    54,    58,
      53,    14,    79,    76,    54,    54,    49,    86,    88,    54,
      86,    49,    49,    81,    51,    83,    54,    12,    91,    16,
      27,    28,    25,    35,    48,    74,    86,    96,   101,    72,
      48,    69,   101,    65,    22,    80,    87,    96,    97,    98,
      99,   100,    53,    54,    87,    53,    86,    93,    22,    86,
      91,    91,    91,    95,    95,    51,    74,    50,    51,    69,
      50,    79,    53,    20,    21,    23,    24,    18,    30,    10,
      33,    53,    14,    94,    53,    86,    54,    14,    75,    14,
      70,    81,    85,    96,    96,    96,    96,    97,    97,    98,
      99,    85,    86,    50,    74,    51,    69,    51,     4,    94,
      75,    70,    85
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    61,    62,    62,    62,    62,    63,    63,    63,    64,
      65,    65,    66,    66,    67,    68,    68,    69,    69,    69,
      70,    70,    71,    72,    72,    73,    73,    74,    74,    74,
      75,    75,    76,    76,    77,    77,    78,    79,    79,    80,
      80,    80,    81,    81,    82,    83,    83,    84,    84,    85,
      85,    85,    85,    85,    85,    85,    85,    85,    85,    85,
      86,    87,    88,    89,    89,    90,    90,    90,    91,    91,
      91,    91,    92,    92,    92,    93,    94,    94,    95,    95,
      95,    95,    96,    96,    96,    97,    97,    97,    97,    97,
      98,    98,    98,    99,    99,   100,   100,   101,   102,   102,
     103,   103
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     2,     1,     2,     1,     1,     1,     5,
       3,     0,     1,     1,     4,     4,     0,     1,     2,     4,
       3,     0,     4,     3,     0,     2,     4,     1,     2,     4,
       3,     0,     3,     4,     3,     3,     2,     3,     0,     1,
       2,     5,     4,     0,     3,     2,     0,     1,     1,     4,
       1,     2,     1,     5,     7,     5,     2,     2,     2,     3,
       1,     1,     2,     1,     1,     3,     1,     1,     1,     3,
       4,     2,     1,     1,     1,     2,     3,     0,     1,     3,
       3,     3,     1,     3,     3,     1,     3,     3,     3,     3,
       1,     3,     3,     1,     3,     1,     3,     1,     3,     1,
       3,     3
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256



/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)

/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo, int yytype, YYSTYPE const * const yyvaluep)
{
  FILE *yyoutput = yyo;
  YYUSE (yyoutput);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyo, yytoknum[yytype], *yyvaluep);
# endif
  YYUSE (yytype);
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo, int yytype, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyo, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  yy_symbol_value_print (yyo, yytype, yyvaluep);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, int yyrule)
{
  unsigned long yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[yyssp[yyi + 1 - yynrhs]],
                       &yyvsp[(yyi + 1) - (yynrhs)]
                                              );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
yystrlen (const char *yystr)
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            else
              goto append;

          append:
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return (YYSIZE_T) (yystpcpy (yyres, yystr) - yyres);
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
                  if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
                    yysize = yysize1;
                  else
                    return 2;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
    default: /* Avoid compiler warnings. */
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    YYSIZE_T yysize1 = yysize + yystrlen (yyformat);
    if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
      yysize = yysize1;
    else
      return 2;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
{
  YYUSE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;


/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yynewstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  *yyssp = (yytype_int16) yystate;

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    goto yyexhaustedlab;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = (YYSIZE_T) (yyssp - yyss + 1);

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        YYSTYPE *yyvs1 = yyvs;
        yytype_int16 *yyss1 = yyss;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * sizeof (*yyssp),
                    &yyvs1, yysize * sizeof (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yytype_int16 *yyss1 = yyss;
        union yyalloc *yyptr =
          (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
# undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
                  (unsigned long) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 183 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    {
      TRACE_PARSER (fprintf (stderr, "%d: CompUnit -> FuncDef\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_CompUnit, (yyvsp[0].node)->tnLineNo, (yyvsp[0].node)->tnColumn);
      InsertChildNode((yyval.node), (yyvsp[0].node));
      g_savedTree = (yyval.node);
    }
#line 1474 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 3:
#line 190 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    {
      TRACE_PARSER (fprintf (stderr, "%d: CompUnit -> CompUnit FuncDef\n", ++nCount));
      (yyval.node) = (yyvsp[-1].node);
      InsertChildNode((yyval.node), (yyvsp[0].node));
    }
#line 1484 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 4:
#line 196 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    {
      TRACE_PARSER (fprintf (stderr, "%d: CompUnit -> Decl\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_CompUnit, (yyvsp[0].node)->tnLineNo, (yyvsp[0].node)->tnColumn);
      InsertChildNode((yyval.node), (yyvsp[0].node));
      g_savedTree = (yyval.node);
    }
#line 1495 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 5:
#line 203 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    {
      TRACE_PARSER (fprintf (stderr, "%d: CompUnit -> CompUnit Decl\n", ++nCount));
      (yyval.node) = (yyvsp[-1].node);
      InsertChildNode((yyval.node), (yyvsp[0].node));
    }
#line 1505 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 6:
#line 213 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    {
      TRACE_PARSER (fprintf (stderr, "%d: Decl -> ConstDecl\n", ++nCount));
      (yyval.node) = (yyvsp[0].node);
    }
#line 1514 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 7:
#line 218 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    {
      TRACE_PARSER (fprintf (stderr, "%d: Decl -> VarDecl\n", ++nCount));
      (yyval.node) = (yyvsp[0].node);
    }
#line 1523 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 8:
#line 223 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    {
      TRACE_PARSER (fprintf (stderr, "%d: Decl -> FuncDecl\n", ++nCount));
      (yyval.node) = (yyvsp[0].node);
    }
#line 1532 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 9:
#line 232 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    {
      TRACE_PARSER (fprintf (stderr, "%d: ConstDecl -> \"const\" BType ConstDef ConstDefGroup \";\"\n", ++nCount));
      (yyval.node) = (yyvsp[-1].node);
      (yyval.node)->tnVtyp = (yyvsp[-3].node)->tnVtyp;
      InsertChildNode ((yyval.node), (yyvsp[-2].node));
      (yyval.node)->tnLineNo = (yyvsp[-4].node)->tnLineNo;
      (yyval.node)->tnColumn = (yyvsp[-4].node)->tnColumn;
      parseDeleteNode ((yyvsp[-4].node));
      parseDeleteNode ((yyvsp[0].node));
      parseDeleteNode ((yyvsp[-3].node));
    }
#line 1548 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 10:
#line 247 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    {
      TRACE_PARSER (fprintf (stderr, "%d: ConstDefGroup -> \",\" ConstDef ConstDefGroup\n", ++nCount));
      (yyval.node)= (yyvsp[0].node);
      InsertChildNode((yyval.node), (yyvsp[-1].node));
      parseDeleteNode ((yyvsp[-2].node));
    }
#line 1559 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 11:
#line 254 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    {
      TRACE_PARSER (fprintf (stderr, "%d: ConstDefGroup -> (null)\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_VarDecl, LineNum, Column);
    }
#line 1568 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 12:
#line 263 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    {
      TRACE_PARSER (fprintf (stderr, "%d: BType -> \"int\"\n", ++nCount));
      (yyval.node) = (yyvsp[0].node);
      (yyval.node)->tnVtyp = TYP_INT;
    }
#line 1578 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 13:
#line 269 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    {
      TRACE_PARSER (fprintf (stderr, "%d: BType -> \"float\"\n", ++nCount));
      (yyval.node) = (yyvsp[0].node);
      (yyval.node)->tnVtyp = TYP_FLOAT;
    }
#line 1588 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 14:
#line 279 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    {
      TRACE_PARSER (fprintf (stderr, "%d: ConstDef -> Identifier ConstExpGroup \"=\" ConstInitVal\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_VarDef, (yyvsp[-3].node)->tnLineNo, (yyvsp[-3].node)->tnColumn);
      InsertChildNode((yyval.node), (yyvsp[-2].node));
      InsertChildNode((yyval.node), (yyvsp[0].node));
      (yyval.node)->tnFlags |= TNF_VAR_INIT | TNF_VAR_CONST | TNF_VAR_SEALED;
      while ((yyvsp[-2].node)->tnOper != TN_NAME)
        (yyvsp[-2].node) = *(Tree *)List_First((yyvsp[-2].node)->children);
      (yyvsp[-2].node)->tnName.tnNameId = (yyvsp[-3].node)->tnName.tnNameId;
      (yyvsp[-3].node)->tnName.tnNameId = NULL;
      parseDeleteNode((yyvsp[-3].node));
      parseDeleteNode((yyvsp[-1].node));
    }
#line 1606 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 15:
#line 296 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    {
      TRACE_PARSER (fprintf (stderr, "%d: ConstExpGroup -> ConstExpGroup \"[\" ConstExp \"]\"\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_INDEX, (yyvsp[-3].node)->tnLineNo, (yyvsp[-3].node)->tnColumn);
      InsertChildNode((yyval.node), (yyvsp[-3].node));
      InsertChildNode((yyval.node), (yyvsp[-1].node));
      parseDeleteNode((yyvsp[-2].node));
      parseDeleteNode((yyvsp[0].node));
    }
#line 1619 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 16:
#line 305 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    {
      TRACE_PARSER (fprintf (stderr, "%d: ConstExpGroup -> (null)\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_NAME, LineNum, Column);
    }
#line 1628 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 17:
#line 314 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    {
      TRACE_PARSER (fprintf (stderr, "%d: ConstInitVal -> ConstExp\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_ConstInitVal, (yyvsp[0].node)->tnLineNo, (yyvsp[0].node)->tnColumn);
      InsertChildNode((yyval.node), (yyvsp[0].node));
    }
#line 1638 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 18:
#line 320 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    {
      TRACE_PARSER (fprintf (stderr, "%d: ConstInitVal -> \"{\" \"}\"\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_SLV_INIT, (yyvsp[-1].node)->tnLineNo, (yyvsp[-1].node)->tnColumn);
      parseDeleteNode((yyvsp[-1].node));
      parseDeleteNode((yyvsp[0].node));
    }
#line 1649 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 19:
#line 327 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    {
      TRACE_PARSER (fprintf (stderr, "%d: ConstInitVal -> \"{\" ConstInitVal ConstInitValGroup \"}\"\n", ++nCount));
      (yyval.node) = (yyvsp[-1].node);
      (yyval.node)->tnLineNo = (yyvsp[-3].node)->tnLineNo;
      (yyval.node)->tnColumn = (yyvsp[-3].node)->tnColumn;
      InsertChildNode((yyval.node), (yyvsp[-2].node));
      parseDeleteNode((yyvsp[-3].node));
      parseDeleteNode((yyvsp[0].node));
    }
#line 1663 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 20:
#line 340 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    { 
      TRACE_PARSER (fprintf (stderr, "%d: ConstInitValGroup -> \",\" ConstInitVal ConstInitValGroup\n", ++nCount));
      (yyval.node) = (yyvsp[0].node);
      (yyval.node)->tnLineNo = (yyvsp[-2].node)->tnLineNo;
      (yyval.node)->tnColumn = (yyvsp[-2].node)->tnColumn;
      InsertChildNode((yyval.node), (yyvsp[-1].node));
      parseDeleteNode((yyvsp[-2].node));
    }
#line 1676 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 21:
#line 349 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    { 
      TRACE_PARSER (fprintf (stderr, "%d: ConstInitValGroup -> (null)\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_SLV_INIT, LineNum, Column);
    }
#line 1685 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 22:
#line 358 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    { 
      TRACE_PARSER (fprintf (stderr, "%d: VarDecl -> BType VarDef VarDefGroup \";\"\n", ++nCount));
      (yyval.node) = (yyvsp[-1].node);
      (yyval.node)->tnVtyp = (yyvsp[-3].node)->tnVtyp;
      (yyval.node)->tnLineNo = (yyvsp[-3].node)->tnLineNo;
      (yyval.node)->tnColumn = (yyvsp[-3].node)->tnColumn;
      InsertChildNode((yyval.node), (yyvsp[-2].node));
      parseDeleteNode((yyvsp[-3].node));
      parseDeleteNode((yyvsp[0].node));
    }
#line 1700 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 23:
#line 372 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    { 
      TRACE_PARSER (fprintf (stderr, "%d: VarDefGroup -> \",\" VarDef VarDefGroup\n", ++nCount));
      (yyval.node) = (yyvsp[0].node);
      (yyval.node)->tnLineNo = (yyvsp[-2].node)->tnLineNo;
      (yyval.node)->tnColumn = (yyvsp[-2].node)->tnColumn;
      InsertChildNode((yyval.node), (yyvsp[-1].node));
      parseDeleteNode((yyvsp[-2].node));
    }
#line 1713 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 24:
#line 381 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    { 
      TRACE_PARSER (fprintf (stderr, "%d: VarDefGroup -> (null)\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_VarDecl, LineNum, Column);
    }
#line 1722 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 25:
#line 390 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    { 
      TRACE_PARSER (fprintf (stderr, "%d: VarDef -> Identifier ConstExpGroup\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_VarDef, (yyvsp[-1].node)->tnLineNo, (yyvsp[-1].node)->tnColumn);
      InsertChildNode((yyval.node), (yyvsp[0].node));
      while ((yyvsp[0].node)->tnOper != TN_NAME)
        (yyvsp[0].node) = *(Tree *)List_First((yyvsp[0].node)->children);
      (yyvsp[0].node)->tnName.tnNameId = (yyvsp[-1].node)->tnName.tnNameId;
      (yyvsp[-1].node)->tnName.tnNameId = NULL;
      parseDeleteNode((yyvsp[-1].node));
    }
#line 1737 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 26:
#line 401 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    { 
      TRACE_PARSER (fprintf (stderr, "%d: VarDef -> Identifier ConstExpGroup \"=\" InitVal\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_VarDef, (yyvsp[-3].node)->tnLineNo, (yyvsp[-3].node)->tnColumn);
      (yyval.node)->tnFlags |= TNF_VAR_INIT;
      InsertChildNode((yyval.node), (yyvsp[-2].node));
      InsertChildNode((yyval.node), (yyvsp[0].node));
      while ((yyvsp[-2].node)->tnOper != TN_NAME)
        (yyvsp[-2].node) = *(Tree *)List_First((yyvsp[-2].node)->children);
      (yyvsp[-2].node)->tnName.tnNameId = (yyvsp[-3].node)->tnName.tnNameId;
      (yyvsp[-3].node)->tnName.tnNameId = NULL;
      parseDeleteNode((yyvsp[-3].node));
      parseDeleteNode((yyvsp[-1].node));
    }
#line 1755 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 27:
#line 419 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    {
      TRACE_PARSER (fprintf (stderr, "%d: InitVal -> Exp\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_InitVal, (yyvsp[0].node)->tnLineNo, (yyvsp[0].node)->tnColumn);
      InsertChildNode((yyval.node), (yyvsp[0].node));
    }
#line 1765 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 28:
#line 425 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    {
      TRACE_PARSER (fprintf (stderr, "%d: InitVal -> \"{\" \"}\"\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_SLV_INIT, (yyvsp[-1].node)->tnLineNo, (yyvsp[-1].node)->tnColumn);
      parseDeleteNode((yyvsp[-1].node));
      parseDeleteNode((yyvsp[0].node));
    }
#line 1776 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 29:
#line 432 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    {
      TRACE_PARSER (fprintf (stderr, "%d: InitVal -> \"{\" InitVal InitValGroup \"}\"\n", ++nCount));
      (yyval.node) = (yyvsp[-1].node);
      (yyval.node)->tnLineNo = (yyvsp[-3].node)->tnLineNo;
      (yyval.node)->tnColumn = (yyvsp[-3].node)->tnColumn;
      InsertChildNode((yyval.node), (yyvsp[-2].node));
      parseDeleteNode((yyvsp[-3].node));
      parseDeleteNode((yyvsp[0].node));
    }
#line 1790 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 30:
#line 445 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    {
      TRACE_PARSER (fprintf (stderr, "%d: InitValGroup -> \",\" InitVal InitValGroup\n", ++nCount));
      (yyval.node) = (yyvsp[0].node);
      (yyval.node)->tnLineNo = (yyvsp[-2].node)->tnLineNo;
      (yyval.node)->tnColumn = (yyvsp[-2].node)->tnColumn;
      InsertChildNode((yyval.node), (yyvsp[-1].node));
      parseDeleteNode((yyvsp[-2].node));
    }
#line 1803 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 31:
#line 454 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    {
      TRACE_PARSER (fprintf (stderr, "%d: InitValGroup -> (null)\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_SLV_INIT, LineNum, Column);
    }
#line 1812 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 32:
#line 462 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    {
      TRACE_PARSER (fprintf (stderr, "%d: FuncBody -> Identifier \"(\" \")\"\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_FuncBody, (yyvsp[-2].node)->tnLineNo, (yyvsp[-2].node)->tnColumn);
      InsertChildNode((yyval.node), (yyvsp[-2].node));
      InsertChildNode((yyval.node), parseCreateNode(TN_FuncFParams, LineNum, Column));
      parseDeleteNode((yyvsp[-1].node));
      parseDeleteNode((yyvsp[0].node));
    }
#line 1825 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 33:
#line 471 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    {
      TRACE_PARSER (fprintf (stderr, "%d: FuncBody -> Identifier \"(\" FuncFParams \")\"\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_FuncBody, (yyvsp[-3].node)->tnLineNo, (yyvsp[-3].node)->tnColumn);
      InsertChildNode((yyval.node), (yyvsp[-3].node));
      InsertChildNode((yyval.node), (yyvsp[-1].node));
      parseDeleteNode((yyvsp[-2].node));
      parseDeleteNode((yyvsp[0].node));
    }
#line 1838 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 34:
#line 484 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    { 
      TRACE_PARSER (fprintf (stderr, "%d: FuncDef -> BType FuncBody Block\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_FuncDef, (yyvsp[-2].node)->tnLineNo, (yyvsp[-2].node)->tnColumn);
      (yyval.node)->tnVtyp = (yyvsp[-2].node)->tnVtyp;
      InsertChildNode((yyval.node), (yyvsp[-1].node));
      InsertChildNode((yyval.node), (yyvsp[0].node));
      parseDeleteNode((yyvsp[-2].node));
    }
#line 1851 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 35:
#line 493 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    { 
      TRACE_PARSER (fprintf (stderr, "%d: FuncDef -> \"void\" FuncBody Block\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_FuncDef, (yyvsp[-2].node)->tnLineNo, (yyvsp[-2].node)->tnColumn);
      (yyval.node)->tnVtyp = TYP_VOID;
      InsertChildNode((yyval.node), (yyvsp[-1].node));
      InsertChildNode((yyval.node), (yyvsp[0].node));
      parseDeleteNode((yyvsp[-2].node));
    }
#line 1864 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 36:
#line 506 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    { 
      TRACE_PARSER (fprintf (stderr, "%d: FuncFParams -> FuncFParam FuncFParamGroup\n", ++nCount));
      (yyval.node) = (yyvsp[0].node);
      (yyval.node)->tnLineNo = (yyvsp[-1].node)->tnLineNo;
      (yyval.node)->tnColumn = (yyvsp[-1].node)->tnColumn;
      InsertChildNode((yyval.node), (yyvsp[-1].node));
    }
#line 1876 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 37:
#line 517 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    { 
      TRACE_PARSER (fprintf (stderr, "%d: FuncFParamGroup -> \",\" FuncFParam FuncFParamGroup\n", ++nCount));
      (yyval.node) = (yyvsp[0].node);
      (yyval.node)->tnLineNo = (yyvsp[-2].node)->tnLineNo;
      (yyval.node)->tnColumn = (yyvsp[-2].node)->tnColumn;
      InsertChildNode((yyval.node), (yyvsp[-1].node));
      parseDeleteNode((yyvsp[-2].node));
    }
#line 1889 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 38:
#line 526 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    { 
      TRACE_PARSER (fprintf (stderr, "%d: FuncFParamGroup -> (null)\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_FuncFParams, LineNum, Column);
    }
#line 1898 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 39:
#line 535 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    {
      Tree node;
      TRACE_PARSER (fprintf (stderr, "%d: FuncFParam -> BType\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_VarDecl, (yyvsp[0].node)->tnLineNo, (yyvsp[0].node)->tnColumn);
      node = parseCreateNode(TN_VarDef, (yyvsp[0].node)->tnLineNo, (yyvsp[0].node)->tnColumn);
      InsertChildNode((yyval.node), node);
      (yyval.node)->tnVtyp = (yyvsp[0].node)->tnVtyp;
      InsertChildNode(node, parseCreateNode(TN_NAME, (yyvsp[0].node)->tnLineNo, (yyvsp[0].node)->tnColumn));
      (yyval.node)->tnFlags |= TNF_VAR_ARG;
      node->tnFlags |= TNF_VAR_ARG;
      parseDeleteNode((yyvsp[0].node));
    }
#line 1915 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 40:
#line 548 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    {
      Tree node;
      TRACE_PARSER (fprintf (stderr, "%d: FuncFParam -> BType Identifier\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_VarDecl, (yyvsp[-1].node)->tnLineNo, (yyvsp[-1].node)->tnColumn);
      node = parseCreateNode(TN_VarDef, (yyvsp[-1].node)->tnLineNo, (yyvsp[-1].node)->tnColumn);
      InsertChildNode((yyval.node), node);
      (yyval.node)->tnVtyp = (yyvsp[-1].node)->tnVtyp;
      InsertChildNode(node, (yyvsp[0].node));
      (yyval.node)->tnFlags |= TNF_VAR_ARG;
      node->tnFlags |= TNF_VAR_ARG;
      parseDeleteNode((yyvsp[-1].node));
    }
#line 1932 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 41:
#line 561 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    {
      Tree temp1, temp2;
      Tree node;
      TRACE_PARSER (fprintf (stderr, "%d: FuncFParam -> BType Identifier \"[\" \"]\" ExpGroup\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_VarDecl, (yyvsp[-4].node)->tnLineNo, (yyvsp[-4].node)->tnColumn);
      node = parseCreateNode(TN_VarDef, (yyvsp[-4].node)->tnLineNo, (yyvsp[-4].node)->tnColumn);
      InsertChildNode((yyval.node), node);
      (yyval.node)->tnVtyp = (yyvsp[-4].node)->tnVtyp;
      (yyval.node)->tnFlags |= TNF_VAR_ARG;
      node->tnFlags |= TNF_VAR_ARG;

      temp1 = (yyvsp[0].node);
      while (temp1->tnOper != TN_NAME)
        temp1 = *(Tree *)List_First(temp1->children);

      /* 插入一个孩子，代表数组  */
      temp2 = parseCreateNode(TN_INDEX, (yyvsp[-4].node)->tnLineNo, (yyvsp[-4].node)->tnColumn);
      if (temp1->lpParent)
        {
          *(Tree *)List_First(temp1->lpParent->children) = temp2;
          temp2->lpParent = temp1->lpParent;
        }
      else
        (yyvsp[0].node) = temp2;
      InsertChildNode(temp2, temp1);
      InsertChildNode(temp2, parseCreateIconNode(0, TYP_INT, (yyvsp[-4].node)->tnLineNo, (yyvsp[-4].node)->tnColumn));

      InsertChildNode(node, (yyvsp[0].node));
      temp1->tnName.tnNameId = (yyvsp[-3].node)->tnName.tnNameId;
      (yyvsp[-3].node)->tnName.tnNameId = NULL;
      parseDeleteNode((yyvsp[-3].node));
      parseDeleteNode((yyvsp[-4].node));
      parseDeleteNode((yyvsp[-2].node));
      parseDeleteNode((yyvsp[-1].node));
    }
#line 1972 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 42:
#line 600 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    { 
      TRACE_PARSER (fprintf (stderr, "%d: ExpGroup -> ExpGroup \"[\" Exp \"]\"\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_INDEX, (yyvsp[-3].node)->tnLineNo, (yyvsp[-3].node)->tnColumn);
      InsertChildNode((yyval.node), (yyvsp[-3].node));
      InsertChildNode((yyval.node), (yyvsp[-1].node));
      parseDeleteNode((yyvsp[-2].node));
      parseDeleteNode((yyvsp[0].node));
    }
#line 1985 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 43:
#line 609 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    { 
      TRACE_PARSER (fprintf (stderr, "%d: ExpGroup -> (null)\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_NAME, LineNum, Column);
    }
#line 1994 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 44:
#line 618 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    { 
      TRACE_PARSER (fprintf (stderr, "%d: Block -> \"{\" BlockItemGroup \"}\"\n", ++nCount));
      (yyval.node) = (yyvsp[-1].node);
      (yyval.node)->tnLineNo = (yyvsp[-2].node)->tnLineNo;
      (yyval.node)->tnColumn = (yyvsp[-2].node)->tnColumn;
      parseDeleteNode((yyvsp[-2].node));
      parseDeleteNode((yyvsp[0].node));
    }
#line 2007 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 45:
#line 630 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    { 
      TRACE_PARSER (fprintf (stderr, "%d: BlockItemGroup -> BlockItem BlockItemGroup\n", ++nCount));
      (yyval.node) = (yyvsp[0].node);
      (yyval.node)->tnLineNo = (yyvsp[-1].node)->tnLineNo;
      (yyval.node)->tnColumn = (yyvsp[-1].node)->tnColumn;
      InsertChildNode((yyval.node), (yyvsp[-1].node));
    }
#line 2019 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 46:
#line 638 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    { 
      TRACE_PARSER (fprintf (stderr, "%d: BlockItemGroup -> (null)\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_BLOCK, LineNum, Column);
    }
#line 2028 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 47:
#line 647 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    { 
      TRACE_PARSER (fprintf (stderr, "%d: BlockItem -> Decl\n", ++nCount));
      (yyval.node) = (yyvsp[0].node);
    }
#line 2037 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 48:
#line 652 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    { 
      TRACE_PARSER (fprintf (stderr, "%d: BlockItem -> Stmt\n", ++nCount));
      (yyval.node) = (yyvsp[0].node);
    }
#line 2046 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 49:
#line 661 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    { 
      TRACE_PARSER (fprintf (stderr, "%d: Stmt -> LVal \"=\" Exp \";\"\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_ASG, (yyvsp[-2].node)->tnLineNo, (yyvsp[-2].node)->tnColumn);
      InsertChildNode((yyval.node), (yyvsp[-3].node));
      InsertChildNode((yyval.node), (yyvsp[-1].node));
      parseDeleteNode((yyvsp[-2].node));
      parseDeleteNode((yyvsp[0].node));
    }
#line 2059 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 50:
#line 670 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    { 
      TRACE_PARSER (fprintf (stderr, "%d: Stmt -> \";\"\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_EmptyStmt, (yyvsp[0].node)->tnLineNo, (yyvsp[0].node)->tnColumn);
      parseDeleteNode((yyvsp[0].node));
    }
#line 2069 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 51:
#line 676 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    { 
      TRACE_PARSER (fprintf (stderr, "%d: Stmt -> Exp \";\"\n", ++nCount));
      (yyval.node) = (yyvsp[-1].node);
      parseDeleteNode((yyvsp[0].node));
    }
#line 2079 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 52:
#line 682 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    {
      TRACE_PARSER (fprintf (stderr, "%d: Stmt -> Block\n", ++nCount));
      (yyval.node) = (yyvsp[0].node);
    }
#line 2088 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 53:
#line 687 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    { 
      TRACE_PARSER (fprintf (stderr, "%d: Stmt -> \"if\" \"(\" Cond \")\" Stmt\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_IF, (yyvsp[-4].node)->tnLineNo, (yyvsp[-4].node)->tnColumn);
      InsertChildNode((yyval.node), (yyvsp[-2].node));
      InsertChildNode((yyval.node), (yyvsp[0].node)) ;
      parseDeleteNode((yyvsp[-4].node));
      parseDeleteNode((yyvsp[-3].node));
      parseDeleteNode((yyvsp[-1].node));
    }
#line 2102 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 54:
#line 697 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    { 
      TRACE_PARSER (fprintf (stderr, "%d: Stmt -> \"if\" \"(\" Cond \")\" Stmt \"else\" Stmt\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_IF, (yyvsp[-6].node)->tnLineNo, (yyvsp[-6].node)->tnColumn);
      InsertChildNode((yyval.node), (yyvsp[-4].node));
      InsertChildNode((yyval.node), (yyvsp[-2].node));
      InsertChildNode((yyval.node), (yyvsp[0].node));
      (yyval.node)->tnFlags |= TNF_IF_HASELSE;
      parseDeleteNode((yyvsp[-6].node));
      parseDeleteNode((yyvsp[-5].node));
      parseDeleteNode((yyvsp[-3].node));
      parseDeleteNode((yyvsp[-1].node));
    }
#line 2119 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 55:
#line 710 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    { 
      TRACE_PARSER (fprintf (stderr, "%d: Stmt -> \"while\" \"(\" Cond \")\" Stmt\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_WHILE, (yyvsp[-4].node)->tnLineNo, (yyvsp[-4].node)->tnColumn);
      InsertChildNode((yyval.node), (yyvsp[-2].node));
      InsertChildNode((yyval.node), (yyvsp[0].node));
      parseDeleteNode((yyvsp[-4].node));
      parseDeleteNode((yyvsp[-3].node));
      parseDeleteNode((yyvsp[-1].node));
    }
#line 2133 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 56:
#line 720 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    { 
      TRACE_PARSER (fprintf (stderr, "%d: Stmt -> \"break\" \";\"\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_BREAK, (yyvsp[-1].node)->tnLineNo, (yyvsp[-1].node)->tnColumn);
      parseDeleteNode((yyvsp[-1].node));
      parseDeleteNode((yyvsp[0].node));
    }
#line 2144 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 57:
#line 727 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    { 
      TRACE_PARSER (fprintf (stderr, "%d: Stmt -> \"continue\" \";\"\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_CONTINUE, (yyvsp[-1].node)->tnLineNo, (yyvsp[-1].node)->tnColumn);
      parseDeleteNode((yyvsp[-1].node));
      parseDeleteNode((yyvsp[0].node));
    }
#line 2155 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 58:
#line 734 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    { 
      TRACE_PARSER (fprintf (stderr, "%d: Stmt -> \"return\" \";\"\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_RETURN, (yyvsp[-1].node)->tnLineNo, (yyvsp[-1].node)->tnColumn);
      parseDeleteNode((yyvsp[-1].node));
      parseDeleteNode((yyvsp[0].node));
    }
#line 2166 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 59:
#line 741 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    { 
      TRACE_PARSER (fprintf (stderr, "%d: Stmt -> \"return\" Exp \";\"\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_RETURN, (yyvsp[-2].node)->tnLineNo, (yyvsp[-2].node)->tnColumn);
      InsertChildNode((yyval.node), (yyvsp[-1].node));
      parseDeleteNode((yyvsp[-2].node));
      parseDeleteNode((yyvsp[0].node));
    }
#line 2178 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 60:
#line 753 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    { 
      TRACE_PARSER (fprintf (stderr, "%d: Exp -> AddExp\n", ++nCount));
      (yyval.node) = (yyvsp[0].node);
    }
#line 2187 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 61:
#line 762 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    { 
      TRACE_PARSER (fprintf (stderr, "%d: Cond -> LOrExp\n", ++nCount));
      (yyval.node) = (yyvsp[0].node);
    }
#line 2196 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 62:
#line 771 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    { 
      TRACE_PARSER (fprintf (stderr, "%d: LVal -> Identifier ExpGroup\n", ++nCount));
      (yyval.node) = (yyvsp[0].node);
      (yyval.node)->tnLineNo = (yyvsp[-1].node)->tnLineNo;
      (yyval.node)->tnColumn = (yyvsp[-1].node)->tnColumn;
      while ((yyvsp[0].node)->tnOper != TN_NAME)
        (yyvsp[0].node) = *(Tree *)List_First((yyvsp[0].node)->children);
      (yyvsp[0].node)->tnName.tnNameId = (yyvsp[-1].node)->tnName.tnNameId;
      (yyvsp[-1].node)->tnName.tnNameId = NULL;
      parseDeleteNode((yyvsp[-1].node));
      (yyval.node)->tnFlags |= TNF_LVALUE;
      (yyvsp[0].node)->tnFlags |= TNF_LVALUE;
    }
#line 2214 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 63:
#line 789 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    { 
      TRACE_PARSER (fprintf (stderr, "%d: Number -> IntConst\n", ++nCount));
      (yyval.node) = (yyvsp[0].node);
    }
#line 2223 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 64:
#line 794 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    { 
      TRACE_PARSER (fprintf (stderr, "%d: Number -> floatConst\n", ++nCount));
      (yyval.node) = (yyvsp[0].node);
    }
#line 2232 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 65:
#line 803 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    { 
      TRACE_PARSER (fprintf (stderr, "%d: PrimaryExp -> \"(\" Exp \")\"\n", ++nCount));
      (yyval.node) = (yyvsp[-1].node);
      parseDeleteNode((yyvsp[-2].node));
      parseDeleteNode((yyvsp[0].node));
    }
#line 2243 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 66:
#line 810 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    { 
      TRACE_PARSER (fprintf (stderr, "%d: PrimaryExp -> LVal\n", ++nCount));
      (yyval.node) = (yyvsp[0].node);
    }
#line 2252 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 67:
#line 815 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    { 
      TRACE_PARSER (fprintf (stderr, "%d: PrimaryExp -> Number\n", ++nCount));
      (yyval.node) = (yyvsp[0].node);
    }
#line 2261 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 68:
#line 824 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    {
      TRACE_PARSER (fprintf (stderr, "%d: UnaryExp -> PrimaryExp\n", ++nCount));
      (yyval.node) = (yyvsp[0].node);
    }
#line 2270 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 69:
#line 829 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    {
      TRACE_PARSER (fprintf (stderr, "%d: UnaryExp -> Identifier \"(\" \")\"\n", ++nCount));
      (yyval.node) = (yyvsp[-2].node);
      (yyval.node)->tnLineNo = (yyvsp[-2].node)->tnLineNo;
      (yyval.node)->tnColumn = (yyvsp[-2].node)->tnColumn;
      (yyval.node)->tnOper = TN_CALL;
      parseDeleteNode((yyvsp[-1].node));
      parseDeleteNode((yyvsp[0].node));
    }
#line 2284 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 70:
#line 839 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    {
      TRACE_PARSER (fprintf (stderr, "%d: UnaryExp -> Identifier \"(\" FuncRParams \")\"\n", ++nCount));
      (yyval.node) = (yyvsp[-1].node);
      (yyval.node)->tnLineNo = (yyvsp[-3].node)->tnLineNo;
      (yyval.node)->tnColumn = (yyvsp[-3].node)->tnColumn;
      (yyval.node)->tnOper = TN_CALL;
      (yyval.node)->tnName.tnNameId = (yyvsp[-3].node)->tnName.tnNameId;
      (yyvsp[-3].node)->tnName.tnNameId = NULL;
      parseDeleteNode((yyvsp[-3].node));
      parseDeleteNode((yyvsp[-2].node));
      parseDeleteNode((yyvsp[0].node));
    }
#line 2301 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 71:
#line 852 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    {
      TRACE_PARSER (fprintf (stderr, "%d: UnaryExp -> UnaryOp UnaryExp\n", ++nCount));
      (yyval.node) = (yyvsp[-1].node);
      InsertChildNode((yyval.node), (yyvsp[0].node));
    }
#line 2311 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 72:
#line 862 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    { 
      TRACE_PARSER (fprintf (stderr, "%d: UnaryOp -> \"+\"\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_NOP, (yyvsp[0].node)->tnLineNo, (yyvsp[0].node)->tnColumn);
      parseDeleteNode((yyvsp[0].node));
    }
#line 2321 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 73:
#line 868 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    { 
      TRACE_PARSER (fprintf (stderr, "%d: UnaryOp -> \"-\"\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_NEG, (yyvsp[0].node)->tnLineNo, (yyvsp[0].node)->tnColumn);
      parseDeleteNode((yyvsp[0].node));
    }
#line 2331 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 74:
#line 874 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    { 
      TRACE_PARSER (fprintf (stderr, "%d: UnaryOp -> \"!\"\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_LOG_NOT, (yyvsp[0].node)->tnLineNo, (yyvsp[0].node)->tnColumn);
      parseDeleteNode((yyvsp[0].node));
    }
#line 2341 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 75:
#line 884 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    { 
      TRACE_PARSER (fprintf (stderr, "%d: FuncRParams -> Exp FuncRParamsGroup\n", ++nCount));
      (yyval.node) = (yyvsp[0].node);
      (yyval.node)->tnLineNo = (yyvsp[-1].node)->tnLineNo;
      (yyval.node)->tnColumn = (yyvsp[-1].node)->tnColumn;
      InsertChildNode((yyval.node), (yyvsp[-1].node));
    }
#line 2353 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 76:
#line 895 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    { 
      TRACE_PARSER (fprintf (stderr, "%d: FuncRParamsGroup -> \",\" Exp FuncRParamsGroup\n", ++nCount));
      (yyval.node) = (yyvsp[0].node);
      (yyval.node)->tnLineNo = (yyvsp[-2].node)->tnLineNo;
      (yyval.node)->tnColumn = (yyvsp[-2].node)->tnColumn;
      InsertChildNode((yyval.node), (yyvsp[-1].node));
      parseDeleteNode((yyvsp[-2].node));
    }
#line 2366 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 77:
#line 904 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    { 
      TRACE_PARSER (fprintf (stderr, "%d: FuncRParamsGroup -> (null)\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_CALL, LineNum, Column);
    }
#line 2375 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 78:
#line 913 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    { 
      TRACE_PARSER (fprintf (stderr, "%d: MulExp -> UnaryExp\n", ++nCount));
      (yyval.node) = (yyvsp[0].node);
    }
#line 2384 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 79:
#line 918 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    { 
      TRACE_PARSER (fprintf (stderr, "%d: MulExp -> MulExp \"*\" UnaryExp\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_MUL, (yyvsp[-1].node)->tnLineNo, (yyvsp[-1].node)->tnColumn);
      InsertChildNode((yyval.node), (yyvsp[-2].node));
      InsertChildNode((yyval.node), (yyvsp[0].node));
      parseDeleteNode((yyvsp[-1].node));
    }
#line 2396 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 80:
#line 926 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    { 
      TRACE_PARSER (fprintf (stderr, "%d: MulExp -> MulExp \"/\" UnaryExp\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_DIV, (yyvsp[-1].node)->tnLineNo, (yyvsp[-1].node)->tnColumn);
      InsertChildNode((yyval.node), (yyvsp[-2].node));
      InsertChildNode((yyval.node), (yyvsp[0].node));
      parseDeleteNode((yyvsp[-1].node));
    }
#line 2408 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 81:
#line 934 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    { 
      TRACE_PARSER (fprintf (stderr, "%d: MulExp -> MulExp \"%%\" UnaryExp\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_MOD, (yyvsp[-1].node)->tnLineNo, (yyvsp[-1].node)->tnColumn);
      InsertChildNode((yyval.node), (yyvsp[-2].node));
      InsertChildNode((yyval.node), (yyvsp[0].node));
      parseDeleteNode((yyvsp[-1].node));
    }
#line 2420 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 82:
#line 946 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    { 
      TRACE_PARSER (fprintf (stderr, "%d: AddExp -> MulExp\n", ++nCount));
      (yyval.node) = (yyvsp[0].node);
    }
#line 2429 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 83:
#line 951 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    { 
      TRACE_PARSER (fprintf (stderr, "%d: AddExp -> AddExp \"+\" MulExp\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_ADD, (yyvsp[-1].node)->tnLineNo, (yyvsp[-1].node)->tnColumn);
      InsertChildNode((yyval.node), (yyvsp[-2].node));
      InsertChildNode((yyval.node), (yyvsp[0].node));
      parseDeleteNode((yyvsp[-1].node));
    }
#line 2441 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 84:
#line 959 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    { 
      TRACE_PARSER (fprintf (stderr, "%d: AddExp -> AddExp \"-\" MulExp\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_SUB, (yyvsp[-1].node)->tnLineNo, (yyvsp[-1].node)->tnColumn);
      InsertChildNode((yyval.node), (yyvsp[-2].node));
      InsertChildNode((yyval.node), (yyvsp[0].node));
      parseDeleteNode((yyvsp[-1].node));
    }
#line 2453 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 85:
#line 971 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    { 
      TRACE_PARSER (fprintf (stderr, "%d: RelExp -> AddExp\n", ++nCount));
      (yyval.node) = (yyvsp[0].node);
    }
#line 2462 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 86:
#line 976 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    { 
      TRACE_PARSER (fprintf (stderr, "%d: RelExp -> RelExp \"<\" AddExp\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_LT, (yyvsp[-1].node)->tnLineNo, (yyvsp[-1].node)->tnColumn);
      InsertChildNode((yyval.node), (yyvsp[-2].node));
      InsertChildNode((yyval.node), (yyvsp[0].node));
      parseDeleteNode((yyvsp[-1].node));
    }
#line 2474 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 87:
#line 984 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    { 
      TRACE_PARSER (fprintf (stderr, "%d: RelExp -> RelExp \">\" AddExp\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_GT, (yyvsp[-1].node)->tnLineNo, (yyvsp[-1].node)->tnColumn);
      InsertChildNode((yyval.node), (yyvsp[-2].node));
      InsertChildNode((yyval.node), (yyvsp[0].node));
      parseDeleteNode((yyvsp[-1].node));
    }
#line 2486 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 88:
#line 992 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    { 
      TRACE_PARSER (fprintf (stderr, "%d: RelExp -> RelExp \"<=\" AddExp\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_LE, (yyvsp[-1].node)->tnLineNo, (yyvsp[-1].node)->tnColumn);
      InsertChildNode((yyval.node), (yyvsp[-2].node));
      InsertChildNode((yyval.node), (yyvsp[0].node));
      parseDeleteNode((yyvsp[-1].node));
    }
#line 2498 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 89:
#line 1000 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    { 
      TRACE_PARSER (fprintf (stderr, "%d: RelExp -> RelExp \">=\" AddExp\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_GE, (yyvsp[-1].node)->tnLineNo, (yyvsp[-1].node)->tnColumn);
      InsertChildNode((yyval.node), (yyvsp[-2].node));
      InsertChildNode((yyval.node), (yyvsp[0].node));
      parseDeleteNode((yyvsp[-1].node));
    }
#line 2510 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 90:
#line 1012 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    { 
      TRACE_PARSER (fprintf (stderr, "%d: EqExp -> RelExp\n", ++nCount));
      (yyval.node) = (yyvsp[0].node);
    }
#line 2519 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 91:
#line 1017 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    { 
      TRACE_PARSER (fprintf (stderr, "%d: EqExp -> EqExp \"==\" RelExp\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_EQ, (yyvsp[-1].node)->tnLineNo, (yyvsp[-1].node)->tnColumn);
      InsertChildNode((yyval.node), (yyvsp[-2].node));
      InsertChildNode((yyval.node), (yyvsp[0].node));
      parseDeleteNode((yyvsp[-1].node));
    }
#line 2531 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 92:
#line 1025 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    { 
      TRACE_PARSER (fprintf (stderr, "%d: EqExp -> EqExp \"!=\" RelExp\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_NE, (yyvsp[-1].node)->tnLineNo, (yyvsp[-1].node)->tnColumn);
      InsertChildNode((yyval.node), (yyvsp[-2].node));
      InsertChildNode((yyval.node), (yyvsp[0].node));
      parseDeleteNode((yyvsp[-1].node));
    }
#line 2543 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 93:
#line 1037 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    { 
      TRACE_PARSER (fprintf (stderr, "%d: LAndExp -> EqExp\n", ++nCount));
      (yyval.node) = (yyvsp[0].node);
    }
#line 2552 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 94:
#line 1042 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    { 
      TRACE_PARSER (fprintf (stderr, "%d: LAndExp -> LAndExp \"&&\" EqExp\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_LOG_AND, (yyvsp[-1].node)->tnLineNo, (yyvsp[-1].node)->tnColumn);
      InsertChildNode((yyval.node), (yyvsp[-2].node));
      InsertChildNode((yyval.node), (yyvsp[0].node));
      parseDeleteNode((yyvsp[-1].node));
    }
#line 2564 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 95:
#line 1054 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    { 
      TRACE_PARSER (fprintf (stderr, "%d: LOrExp -> LAndExp\n", ++nCount));
      (yyval.node) = (yyvsp[0].node);
    }
#line 2573 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 96:
#line 1059 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    { 
      TRACE_PARSER (fprintf (stderr, "%d: LOrExp -> LOrExp \"||\" LAndExp\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_LOG_OR, (yyvsp[-1].node)->tnLineNo, (yyvsp[-1].node)->tnColumn);
      InsertChildNode((yyval.node), (yyvsp[-2].node));
      InsertChildNode((yyval.node), (yyvsp[0].node));
      parseDeleteNode((yyvsp[-1].node));
    }
#line 2585 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 97:
#line 1071 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    { 
      TRACE_PARSER (fprintf (stderr, "%d: ConstExp -> AddExp\n", ++nCount));
      (yyval.node) = (yyvsp[0].node);
    }
#line 2594 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 98:
#line 1080 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    { 
      TRACE_PARSER (fprintf (stderr, "%d: TN_FuncBodys -> TN_FuncBodys \",\" FuncBody\n", ++nCount));
      (yyval.node) = (yyvsp[-2].node);
      InsertChildNode((yyval.node), (yyvsp[0].node));
      parseDeleteNode((yyvsp[-1].node));
    }
#line 2605 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 99:
#line 1087 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    { 
      TRACE_PARSER (fprintf (stderr, "%d: TN_FuncBodys -> FuncBody\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_FuncDecl, (yyvsp[0].node)->tnLineNo, (yyvsp[0].node)->tnColumn);
      InsertChildNode((yyval.node), (yyvsp[0].node));
    }
#line 2615 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 100:
#line 1097 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    {
      TRACE_PARSER (fprintf (stderr, "%d: FuncDecl -> BType TN_FuncBodys \";\"\n", ++nCount));
      (yyval.node) = (yyvsp[-1].node);
      (yyval.node)->tnLineNo = (yyvsp[-2].node)->tnLineNo;
      (yyval.node)->tnColumn = (yyvsp[-2].node)->tnColumn;
      (yyval.node)->tnVtyp = (yyvsp[-2].node)->tnVtyp;
      parseDeleteNode((yyvsp[-2].node));
      parseDeleteNode((yyvsp[0].node));
    }
#line 2629 "grammar.tab.c" /* yacc.c:1652  */
    break;

  case 101:
#line 1107 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1652  */
    {
      TRACE_PARSER (fprintf (stderr, "%d: FuncDecl -> \"void\" TN_FuncBodys \";\"\n", ++nCount));
      (yyval.node) = (yyvsp[-1].node);
      (yyval.node)->tnLineNo = (yyvsp[-2].node)->tnLineNo;
      (yyval.node)->tnColumn = (yyvsp[-2].node)->tnColumn;
      (yyval.node)->tnVtyp = TYP_VOID;
      parseDeleteNode((yyvsp[-2].node));
      parseDeleteNode((yyvsp[0].node));
    }
#line 2643 "grammar.tab.c" /* yacc.c:1652  */
    break;


#line 2647 "grammar.tab.c" /* yacc.c:1652  */
      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYTERROR;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;


#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif


/*-----------------------------------------------------.
| yyreturn -- parsing is finished, return the result.  |
`-----------------------------------------------------*/
yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  return yyresult;
}
#line 1118 "/home/pi/compiler/frontend/flexbison/grammar.y" /* yacc.c:1918  */

