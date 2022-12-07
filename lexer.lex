%{

#include <string.h>
#include "ast.h"
#include "y.tab.h"

/* keep track of current character number */
int col = 1;
/* keep track of current line number */
int line = 1;

%}

%option noyywrap

%%

[ \t]                    { col += yyleng; }

"\n"                     { col = 1; line += 1; }

"return"                 { col += yyleng; return RETURN; }
"if"                     { col += yyleng; return IF; }
"else"                   { col += yyleng; return ELSE; }
"while"                  { col += yyleng; return WHILE; }
"otherwise"              { col += yyleng; return OTHERWISE; }
"repeat"                 { col += yyleng; return REPEAT; }
"var"                    { col += yyleng; return VAR; }
"fun"                    { col += yyleng; return FUN; }

"("                      { col += yyleng; return LPAREN; }
")"                      { col += yyleng; return RPAREN; }
"{"                      { col += yyleng; return LCURL; }
"}"                      { col += yyleng; return RCURL; }

"+"                      { col += yyleng; return OP_PLUS; }
"-"                      { col += yyleng; return OP_MINUS; }
"*"                      { col += yyleng; return OP_TIMES; }
"/"                      { col += yyleng; return OP_DIV; }
"%"                      { col += yyleng; return OP_REM; }
"|"                      { col += yyleng; return OP_BOR; }
"&"                      { col += yyleng; return OP_BAND; }
"^"                      { col += yyleng; return OP_XOR; }
"||"                     { col += yyleng; return OP_OR; }
"&&"                     { col += yyleng; return OP_AND; }
"=="                     { col += yyleng; return OP_EQ; }
"<>"                     { col += yyleng; return OP_NE; }
"<"                      { col += yyleng; return OP_LT; }
">"                      { col += yyleng; return OP_GT; }
"<="                     { col += yyleng; return OP_LE; }
">="                     { col += yyleng; return OP_GE; }
"!"                      { col += yyleng; return OP_NOT; }

":="                     { col += yyleng; return ASSIGN; }

[0-9]+                   { col += yyleng; yylval.ival=atoi(yytext); return INT; }

"int"                    { col += yyleng; yylval.sval="int"; return TYPE;          }
"char"                   { col += yyleng; yylval.sval="char"; return TYPE;         }
"string"                 { col += yyleng; yylval.sval="string"; return TYPE;       }
"void"                   { col += yyleng; yylval.sval="void"; return TYPE;         }
[a-zA-Z][0-9a-zA-Z]*     { col += yyleng; yylval.sval=strdup(yytext); return NAME; }

\"[^\"]*\"                   { col += yyleng; yylval.sval=strdup(yytext); return STRING; }

";"                      { col += yyleng; return SEMICOLON; }
","                      { col += yyleng; return COMMA; }

.                        { fprintf(stderr,"illegal token\n"); exit(1); }
