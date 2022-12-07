#ifndef AST_H
#define AST_H

#include "symbol.h"

typedef struct list_ list;

typedef struct program_ {
    list * variables;
    list * functions;
    list * statements;
} program;

typedef struct ty_node {
    enum {
        int_ty,
        string_ty,
        void_ty
        // Add more types if needed
    } kind;
} ty_node;

ty_node* IntTyNode();
ty_node* StringTyNode();
ty_node* VoidTyNode();
ty_node* TyNode(char * type);
char* typeToStr(ty_node* type);

typedef enum {
    plus_op,
    minus_op,
    times_op,
    div_op,
    rem_op,
    bor_op,
    ban_op,
    xor_op,
    and_op,
    or_op,
    eq_op,
    ne_op,
    lt_op,
    gt_op,
    le_op,
    ge_op,
} binop;

typedef enum { not_op } unop;

typedef struct exp_node {
    enum { int_exp, string_exp, binop_exp, unop_exp, call_exp, intrinsic_exp, var_exp } kind;
    union {
        int ival;
        char* sval;
        struct { binop op; struct exp_node* e1; struct exp_node* e2; } bin_ops;
        struct { unop op; struct exp_node* e; } un_ops;
        struct { char* name; list * args; } call_ops;
        struct { char* name; list * args; } intrinsic_ops;
        struct { char* name; } var_ops;
    } data;
} exp_node;

exp_node* IntNode(int val);
exp_node* StringNode(char * str);
exp_node* BinOpNode(binop operation, exp_node * left, exp_node * right);
exp_node* UnOpNode(unop operation, exp_node * expr);
exp_node* VarOpNode(char * name);
exp_node* CallOpNode(char * name, list * args);
exp_node* IntrinsicNode(char * name, list * args);

typedef struct param {
    ty_node* ty;
    char* name;
} param ;

param * Param(ty_node * type, char * name);

typedef struct vardec_node {
    char * name;
    ty_node * type;
    exp_node * init;
} vardec_node;

vardec_node * VarDecNode(char * name, ty_node * type, exp_node * init);

typedef struct fundec_node {
    char * name;
    ty_node * type;
    list * args;
    list * locs;
    list * stmts;
} fundec_node;

fundec_node * FunDecNode(char * name, ty_node * type, list * args, list * locs, list * stmts);

typedef struct frame {
    S_table args_locs_types;
    S_table indexes;
    ty_node * ret;
} frame;

typedef struct stmt_node {
    enum { assign_stmt, if_stmt, while_stmt, repeat_stmt, ret_stmt, call_stmt, intrinsic_stmt } kind;
    union { struct { char* lhs; exp_node* rhs; } assign_ops;
        struct { exp_node* cond; struct list_* then_stmts; struct list_* else_stmts; } if_ops;
        struct { exp_node* cond; struct list_* body; struct list_* otherwise; } while_ops;
        struct { exp_node* cond; struct list_* body; } repeat_ops;
        struct { char* name; list * args; } call_ops;
        struct { char* name; list * args; } intrinsic_ops;
        exp_node* ret_exp;
    } data;
} stmt_node;

stmt_node* RetNode(exp_node* e);
stmt_node* IfNode(exp_node* cond, list * thenStmts, list * elseStmts);
stmt_node* WhileNode(exp_node* guard, list * body, list * otherwise);
stmt_node* RepeatNode(exp_node* n, list * body);
stmt_node* AssignNode(char * lhs, exp_node * rhs);
stmt_node* CallOpStmtNode(char * name, list * args);
stmt_node* IntrinsicStmtNode(char * name, list * args);

struct list_ {
    void* head;
    struct list_ * next;
};

list* ListCopy(list* lst);
list* ListAddFirst(void* hd, list* tl);
list* ListAddLast(void* hd, list* tl);

#endif
