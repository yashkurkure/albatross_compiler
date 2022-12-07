%{

#define _POSIX_C_SOURCE 200809L // enable strdup

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "ast.h"

extern program p;

int yylex(void); /* function prototype */

void yyerror(const char *s) {
    fprintf(stderr,"parsing error %s\n", s); exit(1);
}

%}


%union {
	int ival;
    char * sval;
    stmt_node * stmt;
    list * stmts;
    exp_node * exp;
    list * exps;
    list * vars;
    vardec_node * var;
    list * funcs;
    fundec_node * func;
    list * argdecls;
    param * argdecl;
	}

%define parse.error verbose

%token <sval> STRING
%token <sval> NAME
%token <sval> TYPE
%token <ival> INT

%token
  RETURN IF ELSE WHILE OTHERWISE REPEAT
  SEMICOLON LPAREN RPAREN LCURL RCURL
  ASSIGN VAR FUN COMMA

%left  OP_OR
%left  OP_AND
%left  OP_BOR
%left  OP_XOR
%left  OP_BAND
%left  OP_EQ OP_NE
%left  OP_LT OP_GT OP_LE OP_GE
%left  OP_PLUS OP_MINUS
%left  OP_TIMES OP_DIV OP_REM
%right OP_NOT

%type <stmt>   stmt
%type <stmts>  stmts
%type <exp>    exp
%type <exps>   exps
%type <vars>   vars
%type <var>    var
%type <funcs>  funcs
%type <func>   func
%type <argdecls>  argdecls
%type <argdecl>   argdecl

%start program

%%

program: vars funcs stmts   { p.variables = $1; p.functions = $2; p.statements = $3; }

vars: vars var    { $$ = ListAddLast($2,$1); }
    | /*empty*/   { $$ = NULL; }

var: VAR NAME TYPE ASSIGN exp SEMICOLON { $$ = VarDecNode($2,TyNode($3),$5); }

funcs: funcs func  { $$ = ListAddLast($2,$1); }
     | /*empty*/   { $$ = NULL; }

func: FUN NAME TYPE LPAREN argdecls RPAREN LCURL vars stmts RCURL { $$ = FunDecNode($2, TyNode($3), $5, $8, $9); }

argdecls: /*empty*/                { $$ = NULL; }
        | argdecl                  { $$ = ListAddLast($1, NULL); }
        | argdecl COMMA argdecls   { $$ = ListAddFirst($1, $3);   }

argdecl: NAME TYPE    { $$ = Param(TyNode($2), $1); }

stmts: stmts stmt { $$ = ListAddLast($2, $1);   }
     | stmt       { $$ = ListAddLast($1, NULL); }

stmt: RETURN exp SEMICOLON                       { $$ = RetNode($2); }
    | RETURN SEMICOLON                           { $$ = RetNode(NULL); }
    | IF LPAREN exp RPAREN LCURL stmts RCURL     { $$ = IfNode($3, $6, NULL); }
    | IF LPAREN exp RPAREN LCURL stmts RCURL ELSE LCURL stmts RCURL { $$ = IfNode($3, $6, $10); }
    | WHILE LPAREN exp RPAREN LCURL stmts RCURL  { $$ = WhileNode($3, $6, NULL); }
    | WHILE LPAREN exp RPAREN LCURL stmts RCURL OTHERWISE LCURL stmts RCURL  { $$ = WhileNode($3, $6, $10); }
    | REPEAT LPAREN exp RPAREN LCURL stmts RCURL { $$ = RepeatNode($3,$6); }
    | NAME ASSIGN exp SEMICOLON                  { $$ = AssignNode($1,$3); }
    | NAME LPAREN exps RPAREN SEMICOLON          { $$ = CallOpStmtNode($1, $3); }


exp: INT                     { $$ = IntNode($1); }
   | NAME                    { $$ = VarOpNode($1); }
   | STRING                  { $$ = StringNode($1); }
   | LPAREN exp RPAREN       { $$ = $2; }
   | NAME LPAREN exps RPAREN { $$ = CallOpNode($1, $3); }
   | exp OP_PLUS exp         { $$ = BinOpNode(plus_op, $1, $3); }
   | exp OP_MINUS exp        { $$ = BinOpNode(minus_op, $1, $3); }
   | exp OP_TIMES exp        { $$ = BinOpNode(times_op, $1, $3); }
   | exp OP_DIV exp          { $$ = BinOpNode(div_op, $1, $3); }
   | exp OP_REM exp          { $$ = BinOpNode(rem_op, $1, $3); }
   | exp OP_BOR exp          { $$ = BinOpNode(bor_op, $1, $3); }
   | exp OP_BAND exp         { $$ = BinOpNode(ban_op, $1, $3); }
   | exp OP_XOR exp          { $$ = BinOpNode(xor_op, $1, $3); }
   | exp OP_AND exp          { $$ = BinOpNode(and_op, $1, $3); }
   | exp OP_OR exp           { $$ = BinOpNode(or_op,  $1, $3); }
   | exp OP_EQ exp           { $$ = BinOpNode(eq_op,  $1, $3); }
   | exp OP_NE exp           { $$ = BinOpNode(ne_op,  $1, $3); }
   | exp OP_LT exp           { $$ = BinOpNode(lt_op,  $1, $3); }
   | exp OP_GT exp           { $$ = BinOpNode(gt_op,  $1, $3); }
   | exp OP_LE exp           { $$ = BinOpNode(le_op,  $1, $3); }
   | exp OP_GE exp           { $$ = BinOpNode(ge_op,  $1, $3); }
   | OP_NOT exp              { $$ = UnOpNode(not_op, $2); }

exps: exp COMMA exps { $$ = ListAddFirst($1, $3); }
    | exp            { $$ = ListAddLast($1, NULL); }
    | /*empty*/      { $$ = NULL; }
