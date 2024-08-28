/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
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
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 11 "D:/compiler/frontend/flexbison/grammar.y"

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


#line 114 "grammar.tab.c"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
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

#include "grammar.tab.h"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_LOWER_THAN_ELSE = 3,            /* LOWER_THAN_ELSE  */
  YYSYMBOL_L_CONST = 4,                    /* "const"  */
  YYSYMBOL_L_INT = 5,                      /* "int"  */
  YYSYMBOL_L_FLOAT = 6,                    /* "float"  */
  YYSYMBOL_L_VOID = 7,                     /* "void"  */
  YYSYMBOL_L_AND = 8,                      /* "&"  */
  YYSYMBOL_L_ANDAND = 9,                   /* "&&"  */
  YYSYMBOL_L_ANDEQ = 10,                   /* "&="  */
  YYSYMBOL_L_ASSIGN = 11,                  /* "="  */
  YYSYMBOL_L_COLON = 12,                   /* ":"  */
  YYSYMBOL_L_COMMA = 13,                   /* ","  */
  YYSYMBOL_L_DECR = 14,                    /* "--"  */
  YYSYMBOL_L_DIV = 15,                     /* "/"  */
  YYSYMBOL_L_DIVEQ = 16,                   /* "/="  */
  YYSYMBOL_L_EQUALS = 17,                  /* "=="  */
  YYSYMBOL_L_EXCLAIM = 18,                 /* "!"  */
  YYSYMBOL_L_GT = 19,                      /* ">"  */
  YYSYMBOL_L_GTEQ = 20,                    /* ">="  */
  YYSYMBOL_L_LBRACK = 21,                  /* "["  */
  YYSYMBOL_L_LT = 22,                      /* "<"  */
  YYSYMBOL_L_LTEQ = 23,                    /* "<="  */
  YYSYMBOL_L_MINUS = 24,                   /* "-"  */
  YYSYMBOL_L_MINUSEQ = 25,                 /* "-="  */
  YYSYMBOL_L_MOD = 26,                     /* "%"  */
  YYSYMBOL_L_MULT = 27,                    /* "*"  */
  YYSYMBOL_L_MULTEQ = 28,                  /* "*="  */
  YYSYMBOL_L_NOTEQ = 29,                   /* "!="  */
  YYSYMBOL_L_OR = 30,                      /* "|"  */
  YYSYMBOL_L_OREQ = 31,                    /* "|="  */
  YYSYMBOL_L_OROR = 32,                    /* "||"  */
  YYSYMBOL_L_PERIOD = 33,                  /* "."  */
  YYSYMBOL_L_PLUS = 34,                    /* "+"  */
  YYSYMBOL_L_QUEST = 35,                   /* "?"  */
  YYSYMBOL_L_TILDE = 36,                   /* "~"  */
  YYSYMBOL_L_XOR = 37,                     /* "^"  */
  YYSYMBOL_L_XOREQ = 38,                   /* "^="  */
  YYSYMBOL_L_BREAK = 39,                   /* "break"  */
  YYSYMBOL_L_CASE = 40,                    /* "case"  */
  YYSYMBOL_L_CONTINUE = 41,                /* "continue"  */
  YYSYMBOL_L_DEFAULT = 42,                 /* "default"  */
  YYSYMBOL_L_DO = 43,                      /* "do"  */
  YYSYMBOL_L_ELSE = 44,                    /* "else"  */
  YYSYMBOL_L_FOR = 45,                     /* "for"  */
  YYSYMBOL_L_GOTO = 46,                    /* "goto"  */
  YYSYMBOL_L_IF = 47,                      /* "if"  */
  YYSYMBOL_L_LCURLY = 48,                  /* "{"  */
  YYSYMBOL_L_LPAREN = 49,                  /* "("  */
  YYSYMBOL_L_RBRACK = 50,                  /* "]"  */
  YYSYMBOL_L_RCURLY = 51,                  /* "}"  */
  YYSYMBOL_L_RETURN = 52,                  /* "return"  */
  YYSYMBOL_L_RPAREN = 53,                  /* ")"  */
  YYSYMBOL_L_SEMI = 54,                    /* ";"  */
  YYSYMBOL_L_SIZEOF = 55,                  /* "sizeof"  */
  YYSYMBOL_L_SW = 56,                      /* "switch"  */
  YYSYMBOL_L_WHILE = 57,                   /* "while"  */
  YYSYMBOL_Identifier = 58,                /* Identifier  */
  YYSYMBOL_IntConst = 59,                  /* IntConst  */
  YYSYMBOL_floatConst = 60,                /* floatConst  */
  YYSYMBOL_YYACCEPT = 61,                  /* $accept  */
  YYSYMBOL_CompUnit = 62,                  /* CompUnit  */
  YYSYMBOL_Decl = 63,                      /* Decl  */
  YYSYMBOL_ConstDecl = 64,                 /* ConstDecl  */
  YYSYMBOL_ConstDefGroup = 65,             /* ConstDefGroup  */
  YYSYMBOL_BType = 66,                     /* BType  */
  YYSYMBOL_ConstDef = 67,                  /* ConstDef  */
  YYSYMBOL_ConstExpGroup = 68,             /* ConstExpGroup  */
  YYSYMBOL_ConstInitVal = 69,              /* ConstInitVal  */
  YYSYMBOL_ConstInitValGroup = 70,         /* ConstInitValGroup  */
  YYSYMBOL_VarDecl = 71,                   /* VarDecl  */
  YYSYMBOL_VarDefGroup = 72,               /* VarDefGroup  */
  YYSYMBOL_VarDef = 73,                    /* VarDef  */
  YYSYMBOL_InitVal = 74,                   /* InitVal  */
  YYSYMBOL_InitValGroup = 75,              /* InitValGroup  */
  YYSYMBOL_FuncBody = 76,                  /* FuncBody  */
  YYSYMBOL_FuncDef = 77,                   /* FuncDef  */
  YYSYMBOL_FuncFParams = 78,               /* FuncFParams  */
  YYSYMBOL_FuncFParamGroup = 79,           /* FuncFParamGroup  */
  YYSYMBOL_FuncFParam = 80,                /* FuncFParam  */
  YYSYMBOL_ExpGroup = 81,                  /* ExpGroup  */
  YYSYMBOL_Block = 82,                     /* Block  */
  YYSYMBOL_BlockItemGroup = 83,            /* BlockItemGroup  */
  YYSYMBOL_BlockItem = 84,                 /* BlockItem  */
  YYSYMBOL_Stmt = 85,                      /* Stmt  */
  YYSYMBOL_Exp = 86,                       /* Exp  */
  YYSYMBOL_Cond = 87,                      /* Cond  */
  YYSYMBOL_LVal = 88,                      /* LVal  */
  YYSYMBOL_Number = 89,                    /* Number  */
  YYSYMBOL_PrimaryExp = 90,                /* PrimaryExp  */
  YYSYMBOL_UnaryExp = 91,                  /* UnaryExp  */
  YYSYMBOL_UnaryOp = 92,                   /* UnaryOp  */
  YYSYMBOL_FuncRParams = 93,               /* FuncRParams  */
  YYSYMBOL_FuncRParamsGroup = 94,          /* FuncRParamsGroup  */
  YYSYMBOL_MulExp = 95,                    /* MulExp  */
  YYSYMBOL_AddExp = 96,                    /* AddExp  */
  YYSYMBOL_RelExp = 97,                    /* RelExp  */
  YYSYMBOL_EqExp = 98,                     /* EqExp  */
  YYSYMBOL_LAndExp = 99,                   /* LAndExp  */
  YYSYMBOL_LOrExp = 100,                   /* LOrExp  */
  YYSYMBOL_ConstExp = 101,                 /* ConstExp  */
  YYSYMBOL_TN_FuncBodys = 102,             /* TN_FuncBodys  */
  YYSYMBOL_FuncDecl = 103                  /* FuncDecl  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_uint8 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

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


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
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

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if !defined yyoverflow

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
#endif /* !defined yyoverflow */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
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
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
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
#define YYLAST   237

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  61
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  43
/* YYNRULES -- Number of rules.  */
#define YYNRULES  101
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  183

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   315


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
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
static const yytype_int16 yyrline[] =
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

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if YYDEBUG || 0
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "LOWER_THAN_ELSE",
  "\"const\"", "\"int\"", "\"float\"", "\"void\"", "\"&\"", "\"&&\"",
  "\"&=\"", "\"=\"", "\":\"", "\",\"", "\"--\"", "\"/\"", "\"/=\"",
  "\"==\"", "\"!\"", "\">\"", "\">=\"", "\"[\"", "\"<\"", "\"<=\"",
  "\"-\"", "\"-=\"", "\"%\"", "\"*\"", "\"*=\"", "\"!=\"", "\"|\"",
  "\"|=\"", "\"||\"", "\".\"", "\"+\"", "\"?\"", "\"~\"", "\"^\"",
  "\"^=\"", "\"break\"", "\"case\"", "\"continue\"", "\"default\"",
  "\"do\"", "\"else\"", "\"for\"", "\"goto\"", "\"if\"", "\"{\"", "\"(\"",
  "\"]\"", "\"}\"", "\"return\"", "\")\"", "\";\"", "\"sizeof\"",
  "\"switch\"", "\"while\"", "Identifier", "IntConst", "floatConst",
  "$accept", "CompUnit", "Decl", "ConstDecl", "ConstDefGroup", "BType",
  "ConstDef", "ConstExpGroup", "ConstInitVal", "ConstInitValGroup",
  "VarDecl", "VarDefGroup", "VarDef", "InitVal", "InitValGroup",
  "FuncBody", "FuncDef", "FuncFParams", "FuncFParamGroup", "FuncFParam",
  "ExpGroup", "Block", "BlockItemGroup", "BlockItem", "Stmt", "Exp",
  "Cond", "LVal", "Number", "PrimaryExp", "UnaryExp", "UnaryOp",
  "FuncRParams", "FuncRParamsGroup", "MulExp", "AddExp", "RelExp", "EqExp",
  "LAndExp", "LOrExp", "ConstExp", "TN_FuncBodys", "FuncDecl", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-137)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     130,    93,  -137,  -137,   -45,   204,  -137,  -137,   -41,  -137,
    -137,  -137,   -37,   -24,    -9,    -4,  -137,  -137,  -137,   -24,
      28,    -9,    -1,  -137,    32,     2,    72,  -137,   -45,  -137,
      11,     8,    20,  -137,  -137,    17,   -37,    51,  -137,    37,
       6,    97,   -45,  -137,  -137,  -137,    58,    60,    66,   171,
       9,  -137,    69,    73,  -137,  -137,  -137,   -41,  -137,    76,
      72,  -137,    87,   117,  -137,  -137,  -137,   171,    74,    41,
    -137,   126,   171,  -137,    28,  -137,   143,    32,  -137,   122,
    -137,    93,  -137,  -137,  -137,  -137,   171,    89,  -137,  -137,
      95,   171,   164,   133,  -137,  -137,  -137,   171,  -137,   171,
     171,   171,   171,   171,    13,  -137,  -137,    41,   112,  -137,
     121,  -137,  -137,  -137,   113,    97,   111,    41,   174,     1,
     156,   134,  -137,  -137,   115,  -137,   158,   120,   171,   124,
    -137,  -137,  -137,    74,    74,  -137,   163,  -137,  -137,   170,
    -137,  -137,    99,   171,   171,   171,   171,   171,   171,   171,
     171,    99,   171,  -137,  -137,   137,  -137,   126,   139,   143,
     148,   133,   162,    41,    41,    41,    41,   174,   174,     1,
     156,  -137,   158,  -137,   163,  -137,   170,  -137,    99,  -137,
    -137,  -137,  -137
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int8 yydefact[] =
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
    -137,  -137,    46,  -137,   123,    10,   176,   184,  -107,    38,
    -137,   141,   185,  -100,    44,    12,   214,  -137,   106,   144,
      86,     5,   167,  -137,  -136,   -48,   142,   -26,  -137,  -137,
      -7,  -137,  -137,    56,     0,   -62,   -40,    83,    84,  -137,
     165,    -3,  -137
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_uint8 yydefgoto[] =
{
       0,     5,    56,     7,    37,     8,    24,    30,   111,   160,
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
      63,    87,    90,   139,   136,    22,   162,     2,     3,    28,
     107,    12,    28,    13,   107,   171,    14,    19,   147,    27,
      21,    23,    71,   106,   117,    25,    33,    43,    76,   117,
     148,    43,    72,    44,    63,    39,    57,    44,    72,    26,
      70,    31,   182,    45,   126,    36,     6,    45,   107,   129,
      29,    17,   176,    34,    22,    38,   106,   174,    49,    80,
      98,   104,    49,    89,   135,   102,    73,    53,    54,    55,
      57,    53,    54,    55,    75,   103,     1,     2,     3,    42,
     155,   163,   164,   165,   166,   117,   117,   117,   117,    99,
      43,    39,   130,   131,   132,    79,    44,   107,     2,     3,
     100,   101,   133,   134,   172,    78,    45,   167,   168,   106,
      81,    46,    84,    47,    85,    86,    63,    43,    91,    48,
      26,    49,    92,    44,    50,    63,    51,    94,    97,    52,
      53,    54,    55,    45,     1,     2,     3,     4,    46,    43,
      47,    96,   122,   114,    43,    44,    48,    26,    49,   123,
      44,    50,    63,    51,   128,    45,    52,    53,    54,    55,
      45,    43,   137,   140,   142,   149,   150,    44,   151,   110,
      49,   152,   138,   154,   104,    49,   157,    45,   156,    53,
      54,    55,    43,   159,    53,    54,    55,   173,    44,    43,
     175,   110,    49,   143,   144,    44,   145,   146,    45,   177,
     113,    53,    54,    55,    16,    45,   178,    35,     1,     2,
       3,     4,    77,    49,   181,   109,    74,   125,   180,    18,
      49,   141,    53,    54,    55,   115,   161,    95,   179,    53,
      54,    55,   169,   124,   170,     0,     0,   108
};

static const yytype_int16 yycheck[] =
{
      26,    49,    50,   110,   104,     8,   142,     5,     6,    13,
      72,     1,    13,    58,    76,   151,     4,    58,    17,    14,
       8,    58,    11,    71,    86,    49,    21,    18,    11,    91,
      29,    18,    21,    24,    60,    25,    26,    24,    21,    48,
      28,    13,   178,    34,    92,    13,     0,    34,   110,    97,
      54,     5,   159,    54,    57,    53,   104,   157,    49,    53,
      67,    48,    49,    54,    51,    24,    58,    58,    59,    60,
      60,    58,    59,    60,    54,    34,     4,     5,     6,     7,
     128,   143,   144,   145,   146,   147,   148,   149,   150,    15,
      18,    81,    99,   100,   101,    58,    24,   159,     5,     6,
      26,    27,   102,   103,   152,    54,    34,   147,   148,   157,
      13,    39,    54,    41,    54,    49,   142,    18,    49,    47,
      48,    49,    49,    24,    52,   151,    54,    51,    11,    57,
      58,    59,    60,    34,     4,     5,     6,     7,    39,    18,
      41,    54,    53,    21,    18,    24,    47,    48,    49,    54,
      24,    52,   178,    54,    21,    34,    57,    58,    59,    60,
      34,    18,    50,    50,    53,     9,    32,    24,    53,    48,
      49,    13,    51,    53,    48,    49,    13,    34,    54,    58,
      59,    60,    18,    13,    58,    59,    60,    50,    24,    18,
      51,    48,    49,    19,    20,    24,    22,    23,    34,    51,
      77,    58,    59,    60,     0,    34,    44,    23,     4,     5,
       6,     7,    36,    49,   176,    74,    31,    53,   174,     5,
      49,   115,    58,    59,    60,    81,   140,    60,   172,    58,
      59,    60,   149,    91,   150,    -1,    -1,    72
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,     4,     5,     6,     7,    62,    63,    64,    66,    71,
      77,   103,    66,    58,    76,   102,     0,    63,    77,    58,
      73,    76,   102,    58,    67,    49,    48,    82,    13,    54,
      68,    13,    72,    82,    54,    68,    13,    65,    53,    66,
      78,    80,     7,    18,    24,    34,    39,    41,    47,    49,
      52,    54,    57,    58,    59,    60,    63,    66,    82,    83,
      84,    85,    86,    88,    89,    90,    91,    92,    95,    96,
      76,    11,    21,    58,    73,    54,    11,    67,    54,    58,
      53,    13,    79,    76,    54,    54,    49,    86,    88,    54,
      86,    49,    49,    81,    51,    83,    54,    11,    91,    15,
      26,    27,    24,    34,    48,    74,    86,    96,   101,    72,
      48,    69,   101,    65,    21,    80,    87,    96,    97,    98,
      99,   100,    53,    54,    87,    53,    86,    93,    21,    86,
      91,    91,    91,    95,    95,    51,    74,    50,    51,    69,
      50,    79,    53,    19,    20,    22,    23,    17,    29,     9,
      32,    53,    13,    94,    53,    86,    54,    13,    75,    13,
      70,    81,    85,    96,    96,    96,    96,    97,    97,    98,
      99,    85,    86,    50,    74,    51,    69,    51,    44,    94,
      75,    70,    85
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
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

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
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


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


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

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF


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




# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
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
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)]);
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
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
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






/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep)
{
  YY_USE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/* Lookahead token kind.  */
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
    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

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
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


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

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
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
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
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
  case 2: /* CompUnit: FuncDef  */
#line 183 "D:/compiler/frontend/flexbison/grammar.y"
    {
      TRACE_PARSER (fprintf (stderr, "%d: CompUnit -> FuncDef\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_CompUnit, (yyvsp[0].node)->tnLineNo, (yyvsp[0].node)->tnColumn);
      InsertChildNode((yyval.node), (yyvsp[0].node));
      g_savedTree = (yyval.node);
    }
#line 1355 "grammar.tab.c"
    break;

  case 3: /* CompUnit: CompUnit FuncDef  */
#line 190 "D:/compiler/frontend/flexbison/grammar.y"
    {
      TRACE_PARSER (fprintf (stderr, "%d: CompUnit -> CompUnit FuncDef\n", ++nCount));
      (yyval.node) = (yyvsp[-1].node);
      InsertChildNode((yyval.node), (yyvsp[0].node));
    }
#line 1365 "grammar.tab.c"
    break;

  case 4: /* CompUnit: Decl  */
#line 196 "D:/compiler/frontend/flexbison/grammar.y"
    {
      TRACE_PARSER (fprintf (stderr, "%d: CompUnit -> Decl\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_CompUnit, (yyvsp[0].node)->tnLineNo, (yyvsp[0].node)->tnColumn);
      InsertChildNode((yyval.node), (yyvsp[0].node));
      g_savedTree = (yyval.node);
    }
#line 1376 "grammar.tab.c"
    break;

  case 5: /* CompUnit: CompUnit Decl  */
#line 203 "D:/compiler/frontend/flexbison/grammar.y"
    {
      TRACE_PARSER (fprintf (stderr, "%d: CompUnit -> CompUnit Decl\n", ++nCount));
      (yyval.node) = (yyvsp[-1].node);
      InsertChildNode((yyval.node), (yyvsp[0].node));
    }
#line 1386 "grammar.tab.c"
    break;

  case 6: /* Decl: ConstDecl  */
#line 213 "D:/compiler/frontend/flexbison/grammar.y"
    {
      TRACE_PARSER (fprintf (stderr, "%d: Decl -> ConstDecl\n", ++nCount));
      (yyval.node) = (yyvsp[0].node);
    }
#line 1395 "grammar.tab.c"
    break;

  case 7: /* Decl: VarDecl  */
#line 218 "D:/compiler/frontend/flexbison/grammar.y"
    {
      TRACE_PARSER (fprintf (stderr, "%d: Decl -> VarDecl\n", ++nCount));
      (yyval.node) = (yyvsp[0].node);
    }
#line 1404 "grammar.tab.c"
    break;

  case 8: /* Decl: FuncDecl  */
#line 223 "D:/compiler/frontend/flexbison/grammar.y"
    {
      TRACE_PARSER (fprintf (stderr, "%d: Decl -> FuncDecl\n", ++nCount));
      (yyval.node) = (yyvsp[0].node);
    }
#line 1413 "grammar.tab.c"
    break;

  case 9: /* ConstDecl: "const" BType ConstDef ConstDefGroup ";"  */
#line 232 "D:/compiler/frontend/flexbison/grammar.y"
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
#line 1429 "grammar.tab.c"
    break;

  case 10: /* ConstDefGroup: "," ConstDef ConstDefGroup  */
#line 247 "D:/compiler/frontend/flexbison/grammar.y"
    {
      TRACE_PARSER (fprintf (stderr, "%d: ConstDefGroup -> \",\" ConstDef ConstDefGroup\n", ++nCount));
      (yyval.node)= (yyvsp[0].node);
      InsertChildNode((yyval.node), (yyvsp[-1].node));
      parseDeleteNode ((yyvsp[-2].node));
    }
#line 1440 "grammar.tab.c"
    break;

  case 11: /* ConstDefGroup: %empty  */
#line 254 "D:/compiler/frontend/flexbison/grammar.y"
    {
      TRACE_PARSER (fprintf (stderr, "%d: ConstDefGroup -> (null)\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_VarDecl, LineNum, Column);
    }
#line 1449 "grammar.tab.c"
    break;

  case 12: /* BType: "int"  */
#line 263 "D:/compiler/frontend/flexbison/grammar.y"
    {
      TRACE_PARSER (fprintf (stderr, "%d: BType -> \"int\"\n", ++nCount));
      (yyval.node) = (yyvsp[0].node);
      (yyval.node)->tnVtyp = TYP_INT;
    }
#line 1459 "grammar.tab.c"
    break;

  case 13: /* BType: "float"  */
#line 269 "D:/compiler/frontend/flexbison/grammar.y"
    {
      TRACE_PARSER (fprintf (stderr, "%d: BType -> \"float\"\n", ++nCount));
      (yyval.node) = (yyvsp[0].node);
      (yyval.node)->tnVtyp = TYP_FLOAT;
    }
#line 1469 "grammar.tab.c"
    break;

  case 14: /* ConstDef: Identifier ConstExpGroup "=" ConstInitVal  */
#line 279 "D:/compiler/frontend/flexbison/grammar.y"
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
#line 1487 "grammar.tab.c"
    break;

  case 15: /* ConstExpGroup: ConstExpGroup "[" ConstExp "]"  */
#line 296 "D:/compiler/frontend/flexbison/grammar.y"
    {
      TRACE_PARSER (fprintf (stderr, "%d: ConstExpGroup -> ConstExpGroup \"[\" ConstExp \"]\"\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_INDEX, (yyvsp[-3].node)->tnLineNo, (yyvsp[-3].node)->tnColumn);
      InsertChildNode((yyval.node), (yyvsp[-3].node));
      InsertChildNode((yyval.node), (yyvsp[-1].node));
      parseDeleteNode((yyvsp[-2].node));
      parseDeleteNode((yyvsp[0].node));
    }
#line 1500 "grammar.tab.c"
    break;

  case 16: /* ConstExpGroup: %empty  */
#line 305 "D:/compiler/frontend/flexbison/grammar.y"
    {
      TRACE_PARSER (fprintf (stderr, "%d: ConstExpGroup -> (null)\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_NAME, LineNum, Column);
    }
#line 1509 "grammar.tab.c"
    break;

  case 17: /* ConstInitVal: ConstExp  */
#line 314 "D:/compiler/frontend/flexbison/grammar.y"
    {
      TRACE_PARSER (fprintf (stderr, "%d: ConstInitVal -> ConstExp\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_ConstInitVal, (yyvsp[0].node)->tnLineNo, (yyvsp[0].node)->tnColumn);
      InsertChildNode((yyval.node), (yyvsp[0].node));
    }
#line 1519 "grammar.tab.c"
    break;

  case 18: /* ConstInitVal: "{" "}"  */
#line 320 "D:/compiler/frontend/flexbison/grammar.y"
    {
      TRACE_PARSER (fprintf (stderr, "%d: ConstInitVal -> \"{\" \"}\"\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_SLV_INIT, (yyvsp[-1].node)->tnLineNo, (yyvsp[-1].node)->tnColumn);
      parseDeleteNode((yyvsp[-1].node));
      parseDeleteNode((yyvsp[0].node));
    }
#line 1530 "grammar.tab.c"
    break;

  case 19: /* ConstInitVal: "{" ConstInitVal ConstInitValGroup "}"  */
#line 327 "D:/compiler/frontend/flexbison/grammar.y"
    {
      TRACE_PARSER (fprintf (stderr, "%d: ConstInitVal -> \"{\" ConstInitVal ConstInitValGroup \"}\"\n", ++nCount));
      (yyval.node) = (yyvsp[-1].node);
      (yyval.node)->tnLineNo = (yyvsp[-3].node)->tnLineNo;
      (yyval.node)->tnColumn = (yyvsp[-3].node)->tnColumn;
      InsertChildNode((yyval.node), (yyvsp[-2].node));
      parseDeleteNode((yyvsp[-3].node));
      parseDeleteNode((yyvsp[0].node));
    }
#line 1544 "grammar.tab.c"
    break;

  case 20: /* ConstInitValGroup: "," ConstInitVal ConstInitValGroup  */
#line 340 "D:/compiler/frontend/flexbison/grammar.y"
    { 
      TRACE_PARSER (fprintf (stderr, "%d: ConstInitValGroup -> \",\" ConstInitVal ConstInitValGroup\n", ++nCount));
      (yyval.node) = (yyvsp[0].node);
      (yyval.node)->tnLineNo = (yyvsp[-2].node)->tnLineNo;
      (yyval.node)->tnColumn = (yyvsp[-2].node)->tnColumn;
      InsertChildNode((yyval.node), (yyvsp[-1].node));
      parseDeleteNode((yyvsp[-2].node));
    }
#line 1557 "grammar.tab.c"
    break;

  case 21: /* ConstInitValGroup: %empty  */
#line 349 "D:/compiler/frontend/flexbison/grammar.y"
    { 
      TRACE_PARSER (fprintf (stderr, "%d: ConstInitValGroup -> (null)\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_SLV_INIT, LineNum, Column);
    }
#line 1566 "grammar.tab.c"
    break;

  case 22: /* VarDecl: BType VarDef VarDefGroup ";"  */
#line 358 "D:/compiler/frontend/flexbison/grammar.y"
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
#line 1581 "grammar.tab.c"
    break;

  case 23: /* VarDefGroup: "," VarDef VarDefGroup  */
#line 372 "D:/compiler/frontend/flexbison/grammar.y"
    { 
      TRACE_PARSER (fprintf (stderr, "%d: VarDefGroup -> \",\" VarDef VarDefGroup\n", ++nCount));
      (yyval.node) = (yyvsp[0].node);
      (yyval.node)->tnLineNo = (yyvsp[-2].node)->tnLineNo;
      (yyval.node)->tnColumn = (yyvsp[-2].node)->tnColumn;
      InsertChildNode((yyval.node), (yyvsp[-1].node));
      parseDeleteNode((yyvsp[-2].node));
    }
#line 1594 "grammar.tab.c"
    break;

  case 24: /* VarDefGroup: %empty  */
#line 381 "D:/compiler/frontend/flexbison/grammar.y"
    { 
      TRACE_PARSER (fprintf (stderr, "%d: VarDefGroup -> (null)\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_VarDecl, LineNum, Column);
    }
#line 1603 "grammar.tab.c"
    break;

  case 25: /* VarDef: Identifier ConstExpGroup  */
#line 390 "D:/compiler/frontend/flexbison/grammar.y"
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
#line 1618 "grammar.tab.c"
    break;

  case 26: /* VarDef: Identifier ConstExpGroup "=" InitVal  */
#line 401 "D:/compiler/frontend/flexbison/grammar.y"
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
#line 1636 "grammar.tab.c"
    break;

  case 27: /* InitVal: Exp  */
#line 419 "D:/compiler/frontend/flexbison/grammar.y"
    {
      TRACE_PARSER (fprintf (stderr, "%d: InitVal -> Exp\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_InitVal, (yyvsp[0].node)->tnLineNo, (yyvsp[0].node)->tnColumn);
      InsertChildNode((yyval.node), (yyvsp[0].node));
    }
#line 1646 "grammar.tab.c"
    break;

  case 28: /* InitVal: "{" "}"  */
#line 425 "D:/compiler/frontend/flexbison/grammar.y"
    {
      TRACE_PARSER (fprintf (stderr, "%d: InitVal -> \"{\" \"}\"\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_SLV_INIT, (yyvsp[-1].node)->tnLineNo, (yyvsp[-1].node)->tnColumn);
      parseDeleteNode((yyvsp[-1].node));
      parseDeleteNode((yyvsp[0].node));
    }
#line 1657 "grammar.tab.c"
    break;

  case 29: /* InitVal: "{" InitVal InitValGroup "}"  */
#line 432 "D:/compiler/frontend/flexbison/grammar.y"
    {
      TRACE_PARSER (fprintf (stderr, "%d: InitVal -> \"{\" InitVal InitValGroup \"}\"\n", ++nCount));
      (yyval.node) = (yyvsp[-1].node);
      (yyval.node)->tnLineNo = (yyvsp[-3].node)->tnLineNo;
      (yyval.node)->tnColumn = (yyvsp[-3].node)->tnColumn;
      InsertChildNode((yyval.node), (yyvsp[-2].node));
      parseDeleteNode((yyvsp[-3].node));
      parseDeleteNode((yyvsp[0].node));
    }
#line 1671 "grammar.tab.c"
    break;

  case 30: /* InitValGroup: "," InitVal InitValGroup  */
#line 445 "D:/compiler/frontend/flexbison/grammar.y"
    {
      TRACE_PARSER (fprintf (stderr, "%d: InitValGroup -> \",\" InitVal InitValGroup\n", ++nCount));
      (yyval.node) = (yyvsp[0].node);
      (yyval.node)->tnLineNo = (yyvsp[-2].node)->tnLineNo;
      (yyval.node)->tnColumn = (yyvsp[-2].node)->tnColumn;
      InsertChildNode((yyval.node), (yyvsp[-1].node));
      parseDeleteNode((yyvsp[-2].node));
    }
#line 1684 "grammar.tab.c"
    break;

  case 31: /* InitValGroup: %empty  */
#line 454 "D:/compiler/frontend/flexbison/grammar.y"
    {
      TRACE_PARSER (fprintf (stderr, "%d: InitValGroup -> (null)\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_SLV_INIT, LineNum, Column);
    }
#line 1693 "grammar.tab.c"
    break;

  case 32: /* FuncBody: Identifier "(" ")"  */
#line 462 "D:/compiler/frontend/flexbison/grammar.y"
    {
      TRACE_PARSER (fprintf (stderr, "%d: FuncBody -> Identifier \"(\" \")\"\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_FuncBody, (yyvsp[-2].node)->tnLineNo, (yyvsp[-2].node)->tnColumn);
      InsertChildNode((yyval.node), (yyvsp[-2].node));
      InsertChildNode((yyval.node), parseCreateNode(TN_FuncFParams, LineNum, Column));
      parseDeleteNode((yyvsp[-1].node));
      parseDeleteNode((yyvsp[0].node));
    }
#line 1706 "grammar.tab.c"
    break;

  case 33: /* FuncBody: Identifier "(" FuncFParams ")"  */
#line 471 "D:/compiler/frontend/flexbison/grammar.y"
    {
      TRACE_PARSER (fprintf (stderr, "%d: FuncBody -> Identifier \"(\" FuncFParams \")\"\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_FuncBody, (yyvsp[-3].node)->tnLineNo, (yyvsp[-3].node)->tnColumn);
      InsertChildNode((yyval.node), (yyvsp[-3].node));
      InsertChildNode((yyval.node), (yyvsp[-1].node));
      parseDeleteNode((yyvsp[-2].node));
      parseDeleteNode((yyvsp[0].node));
    }
#line 1719 "grammar.tab.c"
    break;

  case 34: /* FuncDef: BType FuncBody Block  */
#line 484 "D:/compiler/frontend/flexbison/grammar.y"
    { 
      TRACE_PARSER (fprintf (stderr, "%d: FuncDef -> BType FuncBody Block\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_FuncDef, (yyvsp[-2].node)->tnLineNo, (yyvsp[-2].node)->tnColumn);
      (yyval.node)->tnVtyp = (yyvsp[-2].node)->tnVtyp;
      InsertChildNode((yyval.node), (yyvsp[-1].node));
      InsertChildNode((yyval.node), (yyvsp[0].node));
      parseDeleteNode((yyvsp[-2].node));
    }
#line 1732 "grammar.tab.c"
    break;

  case 35: /* FuncDef: "void" FuncBody Block  */
#line 493 "D:/compiler/frontend/flexbison/grammar.y"
    { 
      TRACE_PARSER (fprintf (stderr, "%d: FuncDef -> \"void\" FuncBody Block\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_FuncDef, (yyvsp[-2].node)->tnLineNo, (yyvsp[-2].node)->tnColumn);
      (yyval.node)->tnVtyp = TYP_VOID;
      InsertChildNode((yyval.node), (yyvsp[-1].node));
      InsertChildNode((yyval.node), (yyvsp[0].node));
      parseDeleteNode((yyvsp[-2].node));
    }
#line 1745 "grammar.tab.c"
    break;

  case 36: /* FuncFParams: FuncFParam FuncFParamGroup  */
#line 506 "D:/compiler/frontend/flexbison/grammar.y"
    { 
      TRACE_PARSER (fprintf (stderr, "%d: FuncFParams -> FuncFParam FuncFParamGroup\n", ++nCount));
      (yyval.node) = (yyvsp[0].node);
      (yyval.node)->tnLineNo = (yyvsp[-1].node)->tnLineNo;
      (yyval.node)->tnColumn = (yyvsp[-1].node)->tnColumn;
      InsertChildNode((yyval.node), (yyvsp[-1].node));
    }
#line 1757 "grammar.tab.c"
    break;

  case 37: /* FuncFParamGroup: "," FuncFParam FuncFParamGroup  */
#line 517 "D:/compiler/frontend/flexbison/grammar.y"
    { 
      TRACE_PARSER (fprintf (stderr, "%d: FuncFParamGroup -> \",\" FuncFParam FuncFParamGroup\n", ++nCount));
      (yyval.node) = (yyvsp[0].node);
      (yyval.node)->tnLineNo = (yyvsp[-2].node)->tnLineNo;
      (yyval.node)->tnColumn = (yyvsp[-2].node)->tnColumn;
      InsertChildNode((yyval.node), (yyvsp[-1].node));
      parseDeleteNode((yyvsp[-2].node));
    }
#line 1770 "grammar.tab.c"
    break;

  case 38: /* FuncFParamGroup: %empty  */
#line 526 "D:/compiler/frontend/flexbison/grammar.y"
    { 
      TRACE_PARSER (fprintf (stderr, "%d: FuncFParamGroup -> (null)\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_FuncFParams, LineNum, Column);
    }
#line 1779 "grammar.tab.c"
    break;

  case 39: /* FuncFParam: BType  */
#line 535 "D:/compiler/frontend/flexbison/grammar.y"
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
#line 1796 "grammar.tab.c"
    break;

  case 40: /* FuncFParam: BType Identifier  */
#line 548 "D:/compiler/frontend/flexbison/grammar.y"
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
#line 1813 "grammar.tab.c"
    break;

  case 41: /* FuncFParam: BType Identifier "[" "]" ExpGroup  */
#line 561 "D:/compiler/frontend/flexbison/grammar.y"
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
#line 1853 "grammar.tab.c"
    break;

  case 42: /* ExpGroup: ExpGroup "[" Exp "]"  */
#line 600 "D:/compiler/frontend/flexbison/grammar.y"
    { 
      TRACE_PARSER (fprintf (stderr, "%d: ExpGroup -> ExpGroup \"[\" Exp \"]\"\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_INDEX, (yyvsp[-3].node)->tnLineNo, (yyvsp[-3].node)->tnColumn);
      InsertChildNode((yyval.node), (yyvsp[-3].node));
      InsertChildNode((yyval.node), (yyvsp[-1].node));
      parseDeleteNode((yyvsp[-2].node));
      parseDeleteNode((yyvsp[0].node));
    }
#line 1866 "grammar.tab.c"
    break;

  case 43: /* ExpGroup: %empty  */
#line 609 "D:/compiler/frontend/flexbison/grammar.y"
    { 
      TRACE_PARSER (fprintf (stderr, "%d: ExpGroup -> (null)\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_NAME, LineNum, Column);
    }
#line 1875 "grammar.tab.c"
    break;

  case 44: /* Block: "{" BlockItemGroup "}"  */
#line 618 "D:/compiler/frontend/flexbison/grammar.y"
    { 
      TRACE_PARSER (fprintf (stderr, "%d: Block -> \"{\" BlockItemGroup \"}\"\n", ++nCount));
      (yyval.node) = (yyvsp[-1].node);
      (yyval.node)->tnLineNo = (yyvsp[-2].node)->tnLineNo;
      (yyval.node)->tnColumn = (yyvsp[-2].node)->tnColumn;
      parseDeleteNode((yyvsp[-2].node));
      parseDeleteNode((yyvsp[0].node));
    }
#line 1888 "grammar.tab.c"
    break;

  case 45: /* BlockItemGroup: BlockItem BlockItemGroup  */
#line 630 "D:/compiler/frontend/flexbison/grammar.y"
    { 
      TRACE_PARSER (fprintf (stderr, "%d: BlockItemGroup -> BlockItem BlockItemGroup\n", ++nCount));
      (yyval.node) = (yyvsp[0].node);
      (yyval.node)->tnLineNo = (yyvsp[-1].node)->tnLineNo;
      (yyval.node)->tnColumn = (yyvsp[-1].node)->tnColumn;
      InsertChildNode((yyval.node), (yyvsp[-1].node));
    }
#line 1900 "grammar.tab.c"
    break;

  case 46: /* BlockItemGroup: %empty  */
#line 638 "D:/compiler/frontend/flexbison/grammar.y"
    { 
      TRACE_PARSER (fprintf (stderr, "%d: BlockItemGroup -> (null)\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_BLOCK, LineNum, Column);
    }
#line 1909 "grammar.tab.c"
    break;

  case 47: /* BlockItem: Decl  */
#line 647 "D:/compiler/frontend/flexbison/grammar.y"
    { 
      TRACE_PARSER (fprintf (stderr, "%d: BlockItem -> Decl\n", ++nCount));
      (yyval.node) = (yyvsp[0].node);
    }
#line 1918 "grammar.tab.c"
    break;

  case 48: /* BlockItem: Stmt  */
#line 652 "D:/compiler/frontend/flexbison/grammar.y"
    { 
      TRACE_PARSER (fprintf (stderr, "%d: BlockItem -> Stmt\n", ++nCount));
      (yyval.node) = (yyvsp[0].node);
    }
#line 1927 "grammar.tab.c"
    break;

  case 49: /* Stmt: LVal "=" Exp ";"  */
#line 661 "D:/compiler/frontend/flexbison/grammar.y"
    { 
      TRACE_PARSER (fprintf (stderr, "%d: Stmt -> LVal \"=\" Exp \";\"\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_ASG, (yyvsp[-2].node)->tnLineNo, (yyvsp[-2].node)->tnColumn);
      InsertChildNode((yyval.node), (yyvsp[-3].node));
      InsertChildNode((yyval.node), (yyvsp[-1].node));
      parseDeleteNode((yyvsp[-2].node));
      parseDeleteNode((yyvsp[0].node));
    }
#line 1940 "grammar.tab.c"
    break;

  case 50: /* Stmt: ";"  */
#line 670 "D:/compiler/frontend/flexbison/grammar.y"
    { 
      TRACE_PARSER (fprintf (stderr, "%d: Stmt -> \";\"\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_EmptyStmt, (yyvsp[0].node)->tnLineNo, (yyvsp[0].node)->tnColumn);
      parseDeleteNode((yyvsp[0].node));
    }
#line 1950 "grammar.tab.c"
    break;

  case 51: /* Stmt: Exp ";"  */
#line 676 "D:/compiler/frontend/flexbison/grammar.y"
    { 
      TRACE_PARSER (fprintf (stderr, "%d: Stmt -> Exp \";\"\n", ++nCount));
      (yyval.node) = (yyvsp[-1].node);
      parseDeleteNode((yyvsp[0].node));
    }
#line 1960 "grammar.tab.c"
    break;

  case 52: /* Stmt: Block  */
#line 682 "D:/compiler/frontend/flexbison/grammar.y"
    {
      TRACE_PARSER (fprintf (stderr, "%d: Stmt -> Block\n", ++nCount));
      (yyval.node) = (yyvsp[0].node);
    }
#line 1969 "grammar.tab.c"
    break;

  case 53: /* Stmt: "if" "(" Cond ")" Stmt  */
#line 687 "D:/compiler/frontend/flexbison/grammar.y"
    { 
      TRACE_PARSER (fprintf (stderr, "%d: Stmt -> \"if\" \"(\" Cond \")\" Stmt\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_IF, (yyvsp[-4].node)->tnLineNo, (yyvsp[-4].node)->tnColumn);
      InsertChildNode((yyval.node), (yyvsp[-2].node));
      InsertChildNode((yyval.node), (yyvsp[0].node)) ;
      parseDeleteNode((yyvsp[-4].node));
      parseDeleteNode((yyvsp[-3].node));
      parseDeleteNode((yyvsp[-1].node));
    }
#line 1983 "grammar.tab.c"
    break;

  case 54: /* Stmt: "if" "(" Cond ")" Stmt "else" Stmt  */
#line 697 "D:/compiler/frontend/flexbison/grammar.y"
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
#line 2000 "grammar.tab.c"
    break;

  case 55: /* Stmt: "while" "(" Cond ")" Stmt  */
#line 710 "D:/compiler/frontend/flexbison/grammar.y"
    { 
      TRACE_PARSER (fprintf (stderr, "%d: Stmt -> \"while\" \"(\" Cond \")\" Stmt\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_WHILE, (yyvsp[-4].node)->tnLineNo, (yyvsp[-4].node)->tnColumn);
      InsertChildNode((yyval.node), (yyvsp[-2].node));
      InsertChildNode((yyval.node), (yyvsp[0].node));
      parseDeleteNode((yyvsp[-4].node));
      parseDeleteNode((yyvsp[-3].node));
      parseDeleteNode((yyvsp[-1].node));
    }
#line 2014 "grammar.tab.c"
    break;

  case 56: /* Stmt: "break" ";"  */
#line 720 "D:/compiler/frontend/flexbison/grammar.y"
    { 
      TRACE_PARSER (fprintf (stderr, "%d: Stmt -> \"break\" \";\"\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_BREAK, (yyvsp[-1].node)->tnLineNo, (yyvsp[-1].node)->tnColumn);
      parseDeleteNode((yyvsp[-1].node));
      parseDeleteNode((yyvsp[0].node));
    }
#line 2025 "grammar.tab.c"
    break;

  case 57: /* Stmt: "continue" ";"  */
#line 727 "D:/compiler/frontend/flexbison/grammar.y"
    { 
      TRACE_PARSER (fprintf (stderr, "%d: Stmt -> \"continue\" \";\"\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_CONTINUE, (yyvsp[-1].node)->tnLineNo, (yyvsp[-1].node)->tnColumn);
      parseDeleteNode((yyvsp[-1].node));
      parseDeleteNode((yyvsp[0].node));
    }
#line 2036 "grammar.tab.c"
    break;

  case 58: /* Stmt: "return" ";"  */
#line 734 "D:/compiler/frontend/flexbison/grammar.y"
    { 
      TRACE_PARSER (fprintf (stderr, "%d: Stmt -> \"return\" \";\"\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_RETURN, (yyvsp[-1].node)->tnLineNo, (yyvsp[-1].node)->tnColumn);
      parseDeleteNode((yyvsp[-1].node));
      parseDeleteNode((yyvsp[0].node));
    }
#line 2047 "grammar.tab.c"
    break;

  case 59: /* Stmt: "return" Exp ";"  */
#line 741 "D:/compiler/frontend/flexbison/grammar.y"
    { 
      TRACE_PARSER (fprintf (stderr, "%d: Stmt -> \"return\" Exp \";\"\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_RETURN, (yyvsp[-2].node)->tnLineNo, (yyvsp[-2].node)->tnColumn);
      InsertChildNode((yyval.node), (yyvsp[-1].node));
      parseDeleteNode((yyvsp[-2].node));
      parseDeleteNode((yyvsp[0].node));
    }
#line 2059 "grammar.tab.c"
    break;

  case 60: /* Exp: AddExp  */
#line 753 "D:/compiler/frontend/flexbison/grammar.y"
    { 
      TRACE_PARSER (fprintf (stderr, "%d: Exp -> AddExp\n", ++nCount));
      (yyval.node) = (yyvsp[0].node);
    }
#line 2068 "grammar.tab.c"
    break;

  case 61: /* Cond: LOrExp  */
#line 762 "D:/compiler/frontend/flexbison/grammar.y"
    { 
      TRACE_PARSER (fprintf (stderr, "%d: Cond -> LOrExp\n", ++nCount));
      (yyval.node) = (yyvsp[0].node);
    }
#line 2077 "grammar.tab.c"
    break;

  case 62: /* LVal: Identifier ExpGroup  */
#line 771 "D:/compiler/frontend/flexbison/grammar.y"
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
#line 2095 "grammar.tab.c"
    break;

  case 63: /* Number: IntConst  */
#line 789 "D:/compiler/frontend/flexbison/grammar.y"
    { 
      TRACE_PARSER (fprintf (stderr, "%d: Number -> IntConst\n", ++nCount));
      (yyval.node) = (yyvsp[0].node);
    }
#line 2104 "grammar.tab.c"
    break;

  case 64: /* Number: floatConst  */
#line 794 "D:/compiler/frontend/flexbison/grammar.y"
    { 
      TRACE_PARSER (fprintf (stderr, "%d: Number -> floatConst\n", ++nCount));
      (yyval.node) = (yyvsp[0].node);
    }
#line 2113 "grammar.tab.c"
    break;

  case 65: /* PrimaryExp: "(" Exp ")"  */
#line 803 "D:/compiler/frontend/flexbison/grammar.y"
    { 
      TRACE_PARSER (fprintf (stderr, "%d: PrimaryExp -> \"(\" Exp \")\"\n", ++nCount));
      (yyval.node) = (yyvsp[-1].node);
      parseDeleteNode((yyvsp[-2].node));
      parseDeleteNode((yyvsp[0].node));
    }
#line 2124 "grammar.tab.c"
    break;

  case 66: /* PrimaryExp: LVal  */
#line 810 "D:/compiler/frontend/flexbison/grammar.y"
    { 
      TRACE_PARSER (fprintf (stderr, "%d: PrimaryExp -> LVal\n", ++nCount));
      (yyval.node) = (yyvsp[0].node);
    }
#line 2133 "grammar.tab.c"
    break;

  case 67: /* PrimaryExp: Number  */
#line 815 "D:/compiler/frontend/flexbison/grammar.y"
    { 
      TRACE_PARSER (fprintf (stderr, "%d: PrimaryExp -> Number\n", ++nCount));
      (yyval.node) = (yyvsp[0].node);
    }
#line 2142 "grammar.tab.c"
    break;

  case 68: /* UnaryExp: PrimaryExp  */
#line 824 "D:/compiler/frontend/flexbison/grammar.y"
    {
      TRACE_PARSER (fprintf (stderr, "%d: UnaryExp -> PrimaryExp\n", ++nCount));
      (yyval.node) = (yyvsp[0].node);
    }
#line 2151 "grammar.tab.c"
    break;

  case 69: /* UnaryExp: Identifier "(" ")"  */
#line 829 "D:/compiler/frontend/flexbison/grammar.y"
    {
      TRACE_PARSER (fprintf (stderr, "%d: UnaryExp -> Identifier \"(\" \")\"\n", ++nCount));
      (yyval.node) = (yyvsp[-2].node);
      (yyval.node)->tnLineNo = (yyvsp[-2].node)->tnLineNo;
      (yyval.node)->tnColumn = (yyvsp[-2].node)->tnColumn;
      (yyval.node)->tnOper = TN_CALL;
      parseDeleteNode((yyvsp[-1].node));
      parseDeleteNode((yyvsp[0].node));
    }
#line 2165 "grammar.tab.c"
    break;

  case 70: /* UnaryExp: Identifier "(" FuncRParams ")"  */
#line 839 "D:/compiler/frontend/flexbison/grammar.y"
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
#line 2182 "grammar.tab.c"
    break;

  case 71: /* UnaryExp: UnaryOp UnaryExp  */
#line 852 "D:/compiler/frontend/flexbison/grammar.y"
    {
      TRACE_PARSER (fprintf (stderr, "%d: UnaryExp -> UnaryOp UnaryExp\n", ++nCount));
      (yyval.node) = (yyvsp[-1].node);
      InsertChildNode((yyval.node), (yyvsp[0].node));
    }
#line 2192 "grammar.tab.c"
    break;

  case 72: /* UnaryOp: "+"  */
#line 862 "D:/compiler/frontend/flexbison/grammar.y"
    { 
      TRACE_PARSER (fprintf (stderr, "%d: UnaryOp -> \"+\"\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_NOP, (yyvsp[0].node)->tnLineNo, (yyvsp[0].node)->tnColumn);
      parseDeleteNode((yyvsp[0].node));
    }
#line 2202 "grammar.tab.c"
    break;

  case 73: /* UnaryOp: "-"  */
#line 868 "D:/compiler/frontend/flexbison/grammar.y"
    { 
      TRACE_PARSER (fprintf (stderr, "%d: UnaryOp -> \"-\"\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_NEG, (yyvsp[0].node)->tnLineNo, (yyvsp[0].node)->tnColumn);
      parseDeleteNode((yyvsp[0].node));
    }
#line 2212 "grammar.tab.c"
    break;

  case 74: /* UnaryOp: "!"  */
#line 874 "D:/compiler/frontend/flexbison/grammar.y"
    { 
      TRACE_PARSER (fprintf (stderr, "%d: UnaryOp -> \"!\"\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_LOG_NOT, (yyvsp[0].node)->tnLineNo, (yyvsp[0].node)->tnColumn);
      parseDeleteNode((yyvsp[0].node));
    }
#line 2222 "grammar.tab.c"
    break;

  case 75: /* FuncRParams: Exp FuncRParamsGroup  */
#line 884 "D:/compiler/frontend/flexbison/grammar.y"
    { 
      TRACE_PARSER (fprintf (stderr, "%d: FuncRParams -> Exp FuncRParamsGroup\n", ++nCount));
      (yyval.node) = (yyvsp[0].node);
      (yyval.node)->tnLineNo = (yyvsp[-1].node)->tnLineNo;
      (yyval.node)->tnColumn = (yyvsp[-1].node)->tnColumn;
      InsertChildNode((yyval.node), (yyvsp[-1].node));
    }
#line 2234 "grammar.tab.c"
    break;

  case 76: /* FuncRParamsGroup: "," Exp FuncRParamsGroup  */
#line 895 "D:/compiler/frontend/flexbison/grammar.y"
    { 
      TRACE_PARSER (fprintf (stderr, "%d: FuncRParamsGroup -> \",\" Exp FuncRParamsGroup\n", ++nCount));
      (yyval.node) = (yyvsp[0].node);
      (yyval.node)->tnLineNo = (yyvsp[-2].node)->tnLineNo;
      (yyval.node)->tnColumn = (yyvsp[-2].node)->tnColumn;
      InsertChildNode((yyval.node), (yyvsp[-1].node));
      parseDeleteNode((yyvsp[-2].node));
    }
#line 2247 "grammar.tab.c"
    break;

  case 77: /* FuncRParamsGroup: %empty  */
#line 904 "D:/compiler/frontend/flexbison/grammar.y"
    { 
      TRACE_PARSER (fprintf (stderr, "%d: FuncRParamsGroup -> (null)\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_CALL, LineNum, Column);
    }
#line 2256 "grammar.tab.c"
    break;

  case 78: /* MulExp: UnaryExp  */
#line 913 "D:/compiler/frontend/flexbison/grammar.y"
    { 
      TRACE_PARSER (fprintf (stderr, "%d: MulExp -> UnaryExp\n", ++nCount));
      (yyval.node) = (yyvsp[0].node);
    }
#line 2265 "grammar.tab.c"
    break;

  case 79: /* MulExp: MulExp "*" UnaryExp  */
#line 918 "D:/compiler/frontend/flexbison/grammar.y"
    { 
      TRACE_PARSER (fprintf (stderr, "%d: MulExp -> MulExp \"*\" UnaryExp\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_MUL, (yyvsp[-1].node)->tnLineNo, (yyvsp[-1].node)->tnColumn);
      InsertChildNode((yyval.node), (yyvsp[-2].node));
      InsertChildNode((yyval.node), (yyvsp[0].node));
      parseDeleteNode((yyvsp[-1].node));
    }
#line 2277 "grammar.tab.c"
    break;

  case 80: /* MulExp: MulExp "/" UnaryExp  */
#line 926 "D:/compiler/frontend/flexbison/grammar.y"
    { 
      TRACE_PARSER (fprintf (stderr, "%d: MulExp -> MulExp \"/\" UnaryExp\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_DIV, (yyvsp[-1].node)->tnLineNo, (yyvsp[-1].node)->tnColumn);
      InsertChildNode((yyval.node), (yyvsp[-2].node));
      InsertChildNode((yyval.node), (yyvsp[0].node));
      parseDeleteNode((yyvsp[-1].node));
    }
#line 2289 "grammar.tab.c"
    break;

  case 81: /* MulExp: MulExp "%" UnaryExp  */
#line 934 "D:/compiler/frontend/flexbison/grammar.y"
    { 
      TRACE_PARSER (fprintf (stderr, "%d: MulExp -> MulExp \"%%\" UnaryExp\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_MOD, (yyvsp[-1].node)->tnLineNo, (yyvsp[-1].node)->tnColumn);
      InsertChildNode((yyval.node), (yyvsp[-2].node));
      InsertChildNode((yyval.node), (yyvsp[0].node));
      parseDeleteNode((yyvsp[-1].node));
    }
#line 2301 "grammar.tab.c"
    break;

  case 82: /* AddExp: MulExp  */
#line 946 "D:/compiler/frontend/flexbison/grammar.y"
    { 
      TRACE_PARSER (fprintf (stderr, "%d: AddExp -> MulExp\n", ++nCount));
      (yyval.node) = (yyvsp[0].node);
    }
#line 2310 "grammar.tab.c"
    break;

  case 83: /* AddExp: AddExp "+" MulExp  */
#line 951 "D:/compiler/frontend/flexbison/grammar.y"
    { 
      TRACE_PARSER (fprintf (stderr, "%d: AddExp -> AddExp \"+\" MulExp\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_ADD, (yyvsp[-1].node)->tnLineNo, (yyvsp[-1].node)->tnColumn);
      InsertChildNode((yyval.node), (yyvsp[-2].node));
      InsertChildNode((yyval.node), (yyvsp[0].node));
      parseDeleteNode((yyvsp[-1].node));
    }
#line 2322 "grammar.tab.c"
    break;

  case 84: /* AddExp: AddExp "-" MulExp  */
#line 959 "D:/compiler/frontend/flexbison/grammar.y"
    { 
      TRACE_PARSER (fprintf (stderr, "%d: AddExp -> AddExp \"-\" MulExp\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_SUB, (yyvsp[-1].node)->tnLineNo, (yyvsp[-1].node)->tnColumn);
      InsertChildNode((yyval.node), (yyvsp[-2].node));
      InsertChildNode((yyval.node), (yyvsp[0].node));
      parseDeleteNode((yyvsp[-1].node));
    }
#line 2334 "grammar.tab.c"
    break;

  case 85: /* RelExp: AddExp  */
#line 971 "D:/compiler/frontend/flexbison/grammar.y"
    { 
      TRACE_PARSER (fprintf (stderr, "%d: RelExp -> AddExp\n", ++nCount));
      (yyval.node) = (yyvsp[0].node);
    }
#line 2343 "grammar.tab.c"
    break;

  case 86: /* RelExp: RelExp "<" AddExp  */
#line 976 "D:/compiler/frontend/flexbison/grammar.y"
    { 
      TRACE_PARSER (fprintf (stderr, "%d: RelExp -> RelExp \"<\" AddExp\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_LT, (yyvsp[-1].node)->tnLineNo, (yyvsp[-1].node)->tnColumn);
      InsertChildNode((yyval.node), (yyvsp[-2].node));
      InsertChildNode((yyval.node), (yyvsp[0].node));
      parseDeleteNode((yyvsp[-1].node));
    }
#line 2355 "grammar.tab.c"
    break;

  case 87: /* RelExp: RelExp ">" AddExp  */
#line 984 "D:/compiler/frontend/flexbison/grammar.y"
    { 
      TRACE_PARSER (fprintf (stderr, "%d: RelExp -> RelExp \">\" AddExp\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_GT, (yyvsp[-1].node)->tnLineNo, (yyvsp[-1].node)->tnColumn);
      InsertChildNode((yyval.node), (yyvsp[-2].node));
      InsertChildNode((yyval.node), (yyvsp[0].node));
      parseDeleteNode((yyvsp[-1].node));
    }
#line 2367 "grammar.tab.c"
    break;

  case 88: /* RelExp: RelExp "<=" AddExp  */
#line 992 "D:/compiler/frontend/flexbison/grammar.y"
    { 
      TRACE_PARSER (fprintf (stderr, "%d: RelExp -> RelExp \"<=\" AddExp\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_LE, (yyvsp[-1].node)->tnLineNo, (yyvsp[-1].node)->tnColumn);
      InsertChildNode((yyval.node), (yyvsp[-2].node));
      InsertChildNode((yyval.node), (yyvsp[0].node));
      parseDeleteNode((yyvsp[-1].node));
    }
#line 2379 "grammar.tab.c"
    break;

  case 89: /* RelExp: RelExp ">=" AddExp  */
#line 1000 "D:/compiler/frontend/flexbison/grammar.y"
    { 
      TRACE_PARSER (fprintf (stderr, "%d: RelExp -> RelExp \">=\" AddExp\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_GE, (yyvsp[-1].node)->tnLineNo, (yyvsp[-1].node)->tnColumn);
      InsertChildNode((yyval.node), (yyvsp[-2].node));
      InsertChildNode((yyval.node), (yyvsp[0].node));
      parseDeleteNode((yyvsp[-1].node));
    }
#line 2391 "grammar.tab.c"
    break;

  case 90: /* EqExp: RelExp  */
#line 1012 "D:/compiler/frontend/flexbison/grammar.y"
    { 
      TRACE_PARSER (fprintf (stderr, "%d: EqExp -> RelExp\n", ++nCount));
      (yyval.node) = (yyvsp[0].node);
    }
#line 2400 "grammar.tab.c"
    break;

  case 91: /* EqExp: EqExp "==" RelExp  */
#line 1017 "D:/compiler/frontend/flexbison/grammar.y"
    { 
      TRACE_PARSER (fprintf (stderr, "%d: EqExp -> EqExp \"==\" RelExp\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_EQ, (yyvsp[-1].node)->tnLineNo, (yyvsp[-1].node)->tnColumn);
      InsertChildNode((yyval.node), (yyvsp[-2].node));
      InsertChildNode((yyval.node), (yyvsp[0].node));
      parseDeleteNode((yyvsp[-1].node));
    }
#line 2412 "grammar.tab.c"
    break;

  case 92: /* EqExp: EqExp "!=" RelExp  */
#line 1025 "D:/compiler/frontend/flexbison/grammar.y"
    { 
      TRACE_PARSER (fprintf (stderr, "%d: EqExp -> EqExp \"!=\" RelExp\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_NE, (yyvsp[-1].node)->tnLineNo, (yyvsp[-1].node)->tnColumn);
      InsertChildNode((yyval.node), (yyvsp[-2].node));
      InsertChildNode((yyval.node), (yyvsp[0].node));
      parseDeleteNode((yyvsp[-1].node));
    }
#line 2424 "grammar.tab.c"
    break;

  case 93: /* LAndExp: EqExp  */
#line 1037 "D:/compiler/frontend/flexbison/grammar.y"
    { 
      TRACE_PARSER (fprintf (stderr, "%d: LAndExp -> EqExp\n", ++nCount));
      (yyval.node) = (yyvsp[0].node);
    }
#line 2433 "grammar.tab.c"
    break;

  case 94: /* LAndExp: LAndExp "&&" EqExp  */
#line 1042 "D:/compiler/frontend/flexbison/grammar.y"
    { 
      TRACE_PARSER (fprintf (stderr, "%d: LAndExp -> LAndExp \"&&\" EqExp\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_LOG_AND, (yyvsp[-1].node)->tnLineNo, (yyvsp[-1].node)->tnColumn);
      InsertChildNode((yyval.node), (yyvsp[-2].node));
      InsertChildNode((yyval.node), (yyvsp[0].node));
      parseDeleteNode((yyvsp[-1].node));
    }
#line 2445 "grammar.tab.c"
    break;

  case 95: /* LOrExp: LAndExp  */
#line 1054 "D:/compiler/frontend/flexbison/grammar.y"
    { 
      TRACE_PARSER (fprintf (stderr, "%d: LOrExp -> LAndExp\n", ++nCount));
      (yyval.node) = (yyvsp[0].node);
    }
#line 2454 "grammar.tab.c"
    break;

  case 96: /* LOrExp: LOrExp "||" LAndExp  */
#line 1059 "D:/compiler/frontend/flexbison/grammar.y"
    { 
      TRACE_PARSER (fprintf (stderr, "%d: LOrExp -> LOrExp \"||\" LAndExp\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_LOG_OR, (yyvsp[-1].node)->tnLineNo, (yyvsp[-1].node)->tnColumn);
      InsertChildNode((yyval.node), (yyvsp[-2].node));
      InsertChildNode((yyval.node), (yyvsp[0].node));
      parseDeleteNode((yyvsp[-1].node));
    }
#line 2466 "grammar.tab.c"
    break;

  case 97: /* ConstExp: AddExp  */
#line 1071 "D:/compiler/frontend/flexbison/grammar.y"
    { 
      TRACE_PARSER (fprintf (stderr, "%d: ConstExp -> AddExp\n", ++nCount));
      (yyval.node) = (yyvsp[0].node);
    }
#line 2475 "grammar.tab.c"
    break;

  case 98: /* TN_FuncBodys: TN_FuncBodys "," FuncBody  */
#line 1080 "D:/compiler/frontend/flexbison/grammar.y"
    { 
      TRACE_PARSER (fprintf (stderr, "%d: TN_FuncBodys -> TN_FuncBodys \",\" FuncBody\n", ++nCount));
      (yyval.node) = (yyvsp[-2].node);
      InsertChildNode((yyval.node), (yyvsp[0].node));
      parseDeleteNode((yyvsp[-1].node));
    }
#line 2486 "grammar.tab.c"
    break;

  case 99: /* TN_FuncBodys: FuncBody  */
#line 1087 "D:/compiler/frontend/flexbison/grammar.y"
    { 
      TRACE_PARSER (fprintf (stderr, "%d: TN_FuncBodys -> FuncBody\n", ++nCount));
      (yyval.node) = parseCreateNode(TN_FuncDecl, (yyvsp[0].node)->tnLineNo, (yyvsp[0].node)->tnColumn);
      InsertChildNode((yyval.node), (yyvsp[0].node));
    }
#line 2496 "grammar.tab.c"
    break;

  case 100: /* FuncDecl: BType TN_FuncBodys ";"  */
#line 1097 "D:/compiler/frontend/flexbison/grammar.y"
    {
      TRACE_PARSER (fprintf (stderr, "%d: FuncDecl -> BType TN_FuncBodys \";\"\n", ++nCount));
      (yyval.node) = (yyvsp[-1].node);
      (yyval.node)->tnLineNo = (yyvsp[-2].node)->tnLineNo;
      (yyval.node)->tnColumn = (yyvsp[-2].node)->tnColumn;
      (yyval.node)->tnVtyp = (yyvsp[-2].node)->tnVtyp;
      parseDeleteNode((yyvsp[-2].node));
      parseDeleteNode((yyvsp[0].node));
    }
#line 2510 "grammar.tab.c"
    break;

  case 101: /* FuncDecl: "void" TN_FuncBodys ";"  */
#line 1107 "D:/compiler/frontend/flexbison/grammar.y"
    {
      TRACE_PARSER (fprintf (stderr, "%d: FuncDecl -> \"void\" TN_FuncBodys \";\"\n", ++nCount));
      (yyval.node) = (yyvsp[-1].node);
      (yyval.node)->tnLineNo = (yyvsp[-2].node)->tnLineNo;
      (yyval.node)->tnColumn = (yyvsp[-2].node)->tnColumn;
      (yyval.node)->tnVtyp = TYP_VOID;
      parseDeleteNode((yyvsp[-2].node));
      parseDeleteNode((yyvsp[0].node));
    }
#line 2524 "grammar.tab.c"
    break;


#line 2528 "grammar.tab.c"

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
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

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
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      yyerror (YY_("syntax error"));
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
  ++yynerrs;

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

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
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
                  YY_ACCESSING_SYMBOL (yystate), yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
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
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif

  return yyresult;
}

#line 1118 "D:/compiler/frontend/flexbison/grammar.y"

