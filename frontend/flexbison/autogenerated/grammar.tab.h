/* A Bison parser, made by GNU Bison 3.5.1.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2020 Free Software Foundation,
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

/* Undocumented macros, especially those whose name start with YY_,
   are private implementation details.  Do not rely on them.  */

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
#line 58 "/data/miaotc/compiler/frontend/flexbison/grammar.y"

    float floatValue;
    unsigned intValue;
    Tree node;

#line 124 "grammar.tab.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_GRAMMAR_TAB_H_INCLUDED  */
