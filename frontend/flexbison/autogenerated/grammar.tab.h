/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_YY_GRAMMAR_TAB_H_INCLUDED
# define YY_YY_GRAMMAR_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    LOWER_THAN_ELSE = 258,         /* LOWER_THAN_ELSE  */
    L_CONST = 259,                 /* "const"  */
    L_INT = 260,                   /* "int"  */
    L_FLOAT = 261,                 /* "float"  */
    L_VOID = 262,                  /* "void"  */
    L_AND = 263,                   /* "&"  */
    L_ANDAND = 264,                /* "&&"  */
    L_ANDEQ = 265,                 /* "&="  */
    L_ASSIGN = 266,                /* "="  */
    L_COLON = 267,                 /* ":"  */
    L_COMMA = 268,                 /* ","  */
    L_DECR = 269,                  /* "--"  */
    L_DIV = 270,                   /* "/"  */
    L_DIVEQ = 271,                 /* "/="  */
    L_EQUALS = 272,                /* "=="  */
    L_EXCLAIM = 273,               /* "!"  */
    L_GT = 274,                    /* ">"  */
    L_GTEQ = 275,                  /* ">="  */
    L_LBRACK = 276,                /* "["  */
    L_LT = 277,                    /* "<"  */
    L_LTEQ = 278,                  /* "<="  */
    L_MINUS = 279,                 /* "-"  */
    L_MINUSEQ = 280,               /* "-="  */
    L_MOD = 281,                   /* "%"  */
    L_MULT = 282,                  /* "*"  */
    L_MULTEQ = 283,                /* "*="  */
    L_NOTEQ = 284,                 /* "!="  */
    L_OR = 285,                    /* "|"  */
    L_OREQ = 286,                  /* "|="  */
    L_OROR = 287,                  /* "||"  */
    L_PERIOD = 288,                /* "."  */
    L_PLUS = 289,                  /* "+"  */
    L_QUEST = 290,                 /* "?"  */
    L_TILDE = 291,                 /* "~"  */
    L_XOR = 292,                   /* "^"  */
    L_XOREQ = 293,                 /* "^="  */
    L_BREAK = 294,                 /* "break"  */
    L_CASE = 295,                  /* "case"  */
    L_CONTINUE = 296,              /* "continue"  */
    L_DEFAULT = 297,               /* "default"  */
    L_DO = 298,                    /* "do"  */
    L_ELSE = 299,                  /* "else"  */
    L_FOR = 300,                   /* "for"  */
    L_GOTO = 301,                  /* "goto"  */
    L_IF = 302,                    /* "if"  */
    L_LCURLY = 303,                /* "{"  */
    L_LPAREN = 304,                /* "("  */
    L_RBRACK = 305,                /* "]"  */
    L_RCURLY = 306,                /* "}"  */
    L_RETURN = 307,                /* "return"  */
    L_RPAREN = 308,                /* ")"  */
    L_SEMI = 309,                  /* ";"  */
    L_SIZEOF = 310,                /* "sizeof"  */
    L_SW = 311,                    /* "switch"  */
    L_WHILE = 312,                 /* "while"  */
    Identifier = 313,              /* Identifier  */
    IntConst = 314,                /* IntConst  */
    floatConst = 315               /* floatConst  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 58 "D:/compiler/frontend/flexbison/grammar.y"

    float floatValue;
    unsigned intValue;
    Tree node;

#line 130 "grammar.tab.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;


int yyparse (void);


#endif /* !YY_YY_GRAMMAR_TAB_H_INCLUDED  */
