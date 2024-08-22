%{ /* <-- needed */
/*****************************************************************************
File       : lex.l
Title      : The lex scanner for SysY2022.
           :
Description: contains the lex scanner of the
           : sy file
History    :
        03-May-2023     MTC Create
        04-April-2024 MTC     Rewrite for COMPILER 2.0
*****************************************************************************/
#undef YY_CHAR
#define YY_CHAR char

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "all.h"

#include "grammar.tab.h"

int LineNum = 1; /* 行号  */
int Column; /*列号  */
%}

%x COMMENT
%x INLINECOMMENT

decimalfloatingconstant ([0-9]*\.?[0-9]+|[0-9]+\.)((e|E)[-+]?[0-9]+)?
hexadecimalfloatingconstant (0x|0X)([0-9a-fA-F]*\.?[0-9a-fA-F]+|[0-9a-fA-F]+\.)((p|P)[-+]?[0-9a-fA-F]+)?
identifier [a-zA-Z_][a-zA-Z0-9_]*
decimalconst (0|[1-9][0-9]*)
octalconst 0[0-7]*
hexadecimalconst (0x|0X)[0-9a-fA-F]+


%%

 
[ \t]                    { Column = Column + yyleng; }        /* 忽略空白符  */
[\n]                     { LineNum = LineNum + 1; Column = 1; }    /* 匹配到换行符，行数+1  */
"int"                    { yylval.node = parseCreateNode (TN_NONE, LineNum, Column); Column = Column + yyleng; return (L_INT); }
"float"                  { yylval.node = parseCreateNode (TN_NONE, LineNum, Column); Column = Column + yyleng; return (L_FLOAT); }
"const"                  { yylval.node = parseCreateNode (TN_NONE, LineNum, Column); Column = Column + yyleng; return (L_CONST); }
"void"                   { yylval.node = parseCreateNode (TN_NONE, LineNum, Column); Column = Column + yyleng; return (L_VOID); }
"break"                  { yylval.node = parseCreateNode (TN_NONE, LineNum, Column); Column = Column + yyleng; return (L_BREAK); }
"continue"               { yylval.node = parseCreateNode (TN_NONE, LineNum, Column); Column = Column + yyleng; return (L_CONTINUE); }
"return"                 { yylval.node = parseCreateNode (TN_NONE, LineNum, Column); Column = Column + yyleng; return (L_RETURN); }
"if"                     { yylval.node = parseCreateNode (TN_NONE, LineNum, Column); Column = Column + yyleng; return (L_IF); }
"else"                   { yylval.node = parseCreateNode (TN_NONE, LineNum, Column); Column = Column + yyleng; return (L_ELSE); }
"while"                  { yylval.node = parseCreateNode (TN_NONE, LineNum, Column); Column = Column + yyleng; return (L_WHILE); }
"for"                    { yylval.node = parseCreateNode (TN_NONE, LineNum, Column); Column = Column + yyleng; return (L_FOR); }
"<"                      { yylval.node = parseCreateNode (TN_NONE, LineNum, Column); Column = Column + yyleng; return (L_LT); }
"<="                     { yylval.node = parseCreateNode (TN_NONE, LineNum, Column); Column = Column + yyleng; return (L_LTEQ); }
">"                      { yylval.node = parseCreateNode (TN_NONE, LineNum, Column); Column = Column + yyleng; return (L_GT); }
">="                     { yylval.node = parseCreateNode (TN_NONE, LineNum, Column); Column = Column + yyleng; return (L_GTEQ); }
"=="                     { yylval.node = parseCreateNode (TN_NONE, LineNum, Column); Column = Column + yyleng; return (L_EQUALS); }
"!="                     { yylval.node = parseCreateNode (TN_NONE, LineNum, Column); Column = Column + yyleng; return (L_NOTEQ); }
"="                      { yylval.node = parseCreateNode (TN_NONE, LineNum, Column); Column = Column + yyleng; return (L_ASSIGN); }
"+"                      { yylval.node = parseCreateNode (TN_NONE, LineNum, Column); Column = Column + yyleng; return (L_PLUS); }
"-"                      { yylval.node = parseCreateNode (TN_NONE, LineNum, Column); Column = Column + yyleng; return (L_MINUS); }
"*"                      { yylval.node = parseCreateNode (TN_NONE, LineNum, Column); Column = Column + yyleng; return (L_MULT); }
"/"                      { yylval.node = parseCreateNode (TN_NONE, LineNum, Column); Column = Column + yyleng; return (L_DIV); }
"%"                      { yylval.node = parseCreateNode (TN_NONE, LineNum, Column); Column = Column + yyleng; return (L_MOD); }
"!"                      { yylval.node = parseCreateNode (TN_NONE, LineNum, Column); Column = Column + yyleng; return (L_EXCLAIM); }
"&&"                     { yylval.node = parseCreateNode (TN_NONE, LineNum, Column); Column = Column + yyleng; return (L_ANDAND); }
"||"                     { yylval.node = parseCreateNode (TN_NONE, LineNum, Column); Column = Column + yyleng; return (L_OROR); }
";"                      { yylval.node = parseCreateNode (TN_NONE, LineNum, Column); Column = Column + yyleng; return (L_SEMI); }
":"                      { yylval.node = parseCreateNode (TN_NONE, LineNum, Column); Column = Column + yyleng; return (L_COLON); }
","                      { yylval.node = parseCreateNode (TN_NONE, LineNum, Column); Column = Column + yyleng; return (L_COMMA); }
"("                      { yylval.node = parseCreateNode (TN_NONE, LineNum, Column); Column = Column + yyleng; return (L_LPAREN); }
")"                      { yylval.node = parseCreateNode (TN_NONE, LineNum, Column); Column = Column + yyleng; return (L_RPAREN); }
"{"                      { yylval.node = parseCreateNode (TN_NONE, LineNum, Column); Column = Column + yyleng; return (L_LCURLY); }
"}"                      { yylval.node = parseCreateNode (TN_NONE, LineNum, Column); Column = Column + yyleng; return (L_RCURLY); }
"["                      { yylval.node = parseCreateNode (TN_NONE, LineNum, Column); Column = Column + yyleng; return (L_LBRACK); }
"]"                      { yylval.node = parseCreateNode (TN_NONE, LineNum, Column); Column = Column + yyleng; return (L_RBRACK); }

"/*"                             { BEGIN COMMENT; Column = Column + yyleng; }
"//"                             { BEGIN INLINECOMMENT; Column = Column + yyleng; }

<COMMENT>"*/"            { BEGIN(INITIAL); Column = Column + yyleng; }
<COMMENT>\n              { LineNum++; Column = 1; }
<COMMENT>([^*|\n])+|.    { ; Column = Column + yyleng; }
<COMMENT><<EOF>>         { fprintf(stderr, "%d: Unterminated comment\n", LineNum); exit(1); }

<INLINECOMMENT>\n                { BEGIN INITIAL; LineNum++; Column = 1; }
<INLINECOMMENT>.                 { Column = Column + yyleng; }

{decimalconst}                  {
                                  yylval.node = parseCreateIconNode ((unsigned)strtoul(yytext, NULL, 10), TYP_INT, LineNum, Column);
                                  Column = Column + yyleng;
                                  return (IntConst);
                                }

{octalconst}                    {
                                  yylval.node = parseCreateIconNode ((unsigned)strtoul(yytext, NULL, 8), TYP_INT, LineNum, Column);
                                  Column = Column + yyleng;
                                  return (IntConst);
                                }

{hexadecimalconst}              {
                                  yylval.node = parseCreateIconNode ((unsigned)strtoul(yytext, NULL, 16), TYP_INT, LineNum, Column);
                                  Column = Column + yyleng;
                                  return (IntConst);
                                }
                                
{hexadecimalfloatingconstant}   {
                                  yylval.node = parseCreateFconNode((float)strtod(yytext, NULL), LineNum, Column);
                                  Column = Column + yyleng;
                                  return (floatConst);
                                }
                                
{decimalfloatingconstant}       {
                                  yylval.node = parseCreateFconNode((float)atof(yytext), LineNum, Column);
                                  Column = Column + yyleng;
                                  return (floatConst);
                                }

{identifier}                    {
                                  yylval.node = parseCreateNameNode(yytext,yyleng, LineNum, Column);
                                  Column = Column + yyleng;
                                  return (Identifier);
                                }

%%
int yywrap()
{
    return 1;
}

