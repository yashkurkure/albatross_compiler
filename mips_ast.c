#define _POSIX_C_SOURCE 200809L // enable strdup

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>
#include "ast.h"
#include "mips_ast.h"
#include "util.h"



# define DEBUG_TEST 0
# define debug_print(fmt, ...) \
            do { if (DEBUG_TEST) fprintf(out, fmt, __VA_ARGS__); } while (0)


static FILE * out;

list * labels;

// Push to register v0
static void push0();

// Push to register v1
static void push1();

// Push to register ra
static void pushra();

static void pushfp();

// Pop to register ra
static void popra();

// Pop to register v0
static void pop0();

// Pop to register v1
static void pop1();

static void emitInstruction(char * instruction, char * comment, ...);

static void emitLabel(char * label, char * comment, ...);

static char * generateFreshLabel();

void mips_astExpr(exp_node * e, S_table global_types, S_table function_rets, frame * f);

void intrinsic(char* name, list * args, S_table global_types, S_table function_rets, frame * f)
{
    printf("intrinsic\n");
    if(!strcmp("exit", name)) {
        printf("intrinsic: exit\n");
        // PUSH all arguments to the stack
        exp_node * ret = (exp_node *) args->head;
        mips_astExpr(ret, global_types, function_rets, f);
        // POP and pass them to a system call
        pop0();
        emitInstruction("move $a0, $v0", "EXIT pass return code");
        emitInstruction("li $v0, 17", "EXIT specify exit2 as the syscall");
        emitInstruction("syscall", "EXIT perform syscall");
        return;

    } else if (!strcmp("printint", name)) {
        printf("intrinsic: printint\n");
        // PUSH all arguments to the stack
        exp_node * intToPrint = (exp_node *) args->head;
        mips_astExpr(intToPrint, global_types, function_rets, f);
        // POP and pass them to a system call
        pop0();
        emitInstruction("move $a0, $v0", "PRINTINT pass int to print");
        emitInstruction("li $v0, 1", "PRINTINT specify print_int as the syscall");
        emitInstruction("syscall", "PRINTINT perform syscall");
        return;

    } else if (!strcmp("printstring", name)) {
        printf("intrinsic: printstring\n");
        // PUSH all arguments to the stack
        exp_node * init = (exp_node *) args->head;

        mips_astExpr(init, global_types, function_rets, f);
        // POP and pass them to a system call
        pop0();
        emitInstruction("move $a0, $v0", "PRINTSTRING pass string to print");
        emitInstruction("li $v0, 4", "PRINTSTRING specify print_string as the syscall");
        emitInstruction("syscall", "PRINTSTRING perform syscall");
        return;
    }

}

static char * generateFreshLabel() {
    static char buffer[1024];
    static int lastLabel = 0;

    snprintf(buffer, sizeof(buffer), "L%d", lastLabel);
    lastLabel += 1;
    return strdup(buffer);
}


void mips_astExpr(exp_node * e, S_table global_types, S_table function_rets, frame * f) {
    printf("mips_astExpr\n");
    UNUSED(global_types);
    UNUSED(function_rets);
    UNUSED(f);
    if(!e) return;
    switch(e->kind){
        case int_exp: {
            // PUSH the int constant to the stack
            emitInstruction("li $v0, %d","INT EXP: LOAD", e->data.ival);
            push0();
            break;
        }
        case string_exp: {
            break;
        }
        case binop_exp: {
            printf("\tbinop_exp\n");


            switch (e->data.bin_ops.op) {
                case plus_op: {
                    // Compile left hand side
                    mips_astExpr(e->data.bin_ops.e1, global_types, function_rets, f);

                    // Compile right hand side
                    mips_astExpr(e->data.bin_ops.e2, global_types, function_rets, f);

                    // POP rhs into register 1 ($v1)
                    pop1();

                    // POP lhs into register 0 ($v0)
                    pop0();
                    emitInstruction("add $v0, $v0, $v1", "BINOP ADD");
                    break;
                }
                case minus_op: {
                    // Compile left hand side
                    mips_astExpr(e->data.bin_ops.e1, global_types, function_rets, f);

                    // Compile right hand side
                    mips_astExpr(e->data.bin_ops.e2, global_types, function_rets, f);

                    // POP rhs into register 1 ($v1)
                    pop1();

                    // POP lhs into register 0 ($v0)
                    pop0();
                    emitInstruction("sub $v0, $v0, $v1", "BINOP SUB");
                    break;
                }
                case bor_op: {
                    // Compile left hand side
                    mips_astExpr(e->data.bin_ops.e1, global_types, function_rets, f);

                    // Compile right hand side
                    mips_astExpr(e->data.bin_ops.e2, global_types, function_rets, f);

                    // POP rhs into register 1 ($v1)
                    pop1();

                    // POP lhs into register 0 ($v0)
                    pop0();
                    emitInstruction("or $v0, $v0, $v1", "BINOP B-OR");
                    break;
                }
                case ban_op: {
                    // Compile left hand side
                    mips_astExpr(e->data.bin_ops.e1, global_types, function_rets, f);

                    // Compile right hand side
                    mips_astExpr(e->data.bin_ops.e2, global_types, function_rets, f);

                    // POP rhs into register 1 ($v1)
                    pop1();

                    // POP lhs into register 0 ($v0)
                    pop0();
                    emitInstruction("and $v0, $v0, $v1", "BINOP B-AND");
                    break;
                }
                case xor_op: {
                    // Compile left hand side
                    mips_astExpr(e->data.bin_ops.e1, global_types, function_rets, f);

                    // Compile right hand side
                    mips_astExpr(e->data.bin_ops.e2, global_types, function_rets, f);

                    // POP rhs into register 1 ($v1)
                    pop1();

                    // POP lhs into register 0 ($v0)
                    pop0();
                    emitInstruction("xor $v0, $v0, $v1", "BINOP B-XOR");
                    break;
                }
                case times_op: {
                    // Compile left hand side
                    mips_astExpr(e->data.bin_ops.e1, global_types, function_rets, f);

                    // Compile right hand side
                    mips_astExpr(e->data.bin_ops.e2, global_types, function_rets, f);

                    // POP rhs into register 1 ($v1)
                    pop1();

                    // POP lhs into register 0 ($v0)
                    pop0();
                    emitInstruction("mult $v0, $v1", "BINOP MULTIPLY");
                    emitInstruction("mflo $v0", "BINOP MULTIPLY - MOVE LO TO V0");
                    break;
                }
                case div_op: {
                    // Compile left hand side
                    mips_astExpr(e->data.bin_ops.e1, global_types, function_rets, f);

                    // Compile right hand side
                    mips_astExpr(e->data.bin_ops.e2, global_types, function_rets, f);

                    // POP rhs into register 1 ($v1)
                    pop1();

                    // POP lhs into register 0 ($v0)
                    pop0();
                    emitInstruction("div $v0, $v1", "BINOP DIVISION");
                    emitInstruction("mflo $v0", "BINOP DIVISION - MOVE LO(QUOTIENT) TO V0");
                    break;
                }
                case rem_op: {
                    // Compile left hand side
                    mips_astExpr(e->data.bin_ops.e1, global_types, function_rets, f);

                    // Compile right hand side
                    mips_astExpr(e->data.bin_ops.e2, global_types, function_rets, f);

                    // POP rhs into register 1 ($v1)
                    pop1();

                    // POP lhs into register 0 ($v0)
                    pop0();
                    emitInstruction("div $v0, $v1", "BINOP REMAINDER");
                    emitInstruction("mfhi $v0", "BINOP REMAINDER - MOVE HI(REMAINDER) TO V0");
                    break;
                }
                case lt_op: {
                    // Compile left hand side
                    mips_astExpr(e->data.bin_ops.e1, global_types, function_rets, f);

                    // Compile right hand side
                    mips_astExpr(e->data.bin_ops.e2, global_types, function_rets, f);

                    // POP rhs into register 1 ($v1)
                    pop1();

                    // POP lhs into register 0 ($v0)
                    pop0();
                    emitInstruction("slt $v0, $v0, $v1","BINOP LESS THAN");
                    break;
                }

                case le_op: {
                    // Compile left hand side
                    mips_astExpr(e->data.bin_ops.e1, global_types, function_rets, f);

                    // Compile right hand side
                    mips_astExpr(e->data.bin_ops.e2, global_types, function_rets, f);

                    // POP rhs into register 1 ($v1)
                    pop1();

                    // POP lhs into register 0 ($v0)
                    pop0();
                    emitInstruction("sle $v0, $v0, $v1","BINOP LESS THAN EQUAL");
                    break;
                }

                case gt_op: {
                    // Compile left hand side
                    mips_astExpr(e->data.bin_ops.e1, global_types, function_rets, f);

                    // Compile right hand side
                    mips_astExpr(e->data.bin_ops.e2, global_types, function_rets, f);

                    // POP rhs into register 1 ($v1)
                    pop1();

                    // POP lhs into register 0 ($v0)
                    pop0();
                    emitInstruction("sgt $v0, $v0, $v1","BINOP GREATER THAN");
                    break;
                }

                case ge_op: {
                    // Compile left hand side
                    mips_astExpr(e->data.bin_ops.e1, global_types, function_rets, f);

                    // Compile right hand side
                    mips_astExpr(e->data.bin_ops.e2, global_types, function_rets, f);

                    // POP rhs into register 1 ($v1)
                    pop1();

                    // POP lhs into register 0 ($v0)
                    pop0();
                    emitInstruction("sge $v0, $v0, $v1","BINOP GREATER THAN EQUAL");
                    break;
                }

                case eq_op: {
                    // Compile left hand side
                    mips_astExpr(e->data.bin_ops.e1, global_types, function_rets, f);

                    // Compile right hand side
                    mips_astExpr(e->data.bin_ops.e2, global_types, function_rets, f);

                    // POP rhs into register 1 ($v1)
                    pop1();

                    // POP lhs into register 0 ($v0)
                    pop0();
                    emitInstruction("seq $v0, $v0, $v1","BINOP EQUAL");
                    break;
                }

                case ne_op: {
                    // Compile left hand side
                    mips_astExpr(e->data.bin_ops.e1, global_types, function_rets, f);

                    // Compile right hand side
                    mips_astExpr(e->data.bin_ops.e2, global_types, function_rets, f);

                    // POP rhs into register 1 ($v1)
                    pop1();

                    // POP lhs into register 0 ($v0)
                    pop0();
                    emitInstruction("sne $v0, $v0, $v1","BINOP NOT EQUAL");
                    break;
                }

                case or_op: {

                    char * set1label = generateFreshLabel();
                    char * exitlabel = generateFreshLabel();

                    mips_astExpr(e->data.bin_ops.e1, global_types, function_rets, f);

                    // POP lhs into register 0 ($v0)
                    pop0();

                    // Branch to set1label if v1 is non zero
                    emitInstruction("bne $v0, $zero, %s", "OR: CHECK LHS NON ZERO", set1label);

                    // Compile right hand side
                    mips_astExpr(e->data.bin_ops.e2, global_types, function_rets, f);

                    // POP rhs into register 1 ($v1)
                    pop1();

                    // Branch to set1label if v0 is non zero
                    emitInstruction("bne $v1, $zero, %s", "OR: CHECK RHS NON ZERO", set1label);

                    // Set v0 as 0 - control reaches here if v0 and v1 are both zero
                    emitInstruction("li $v0, %d","OR: LHS AND RHS ARE ZERO, RESULT 0", 0);

                    // Jump to end of evaluation for OR
                    emitInstruction("j %s", "OR: JUMP TO COMPLETE EVALUATION OF OR", exitlabel);

                    // Set 1 label - sets result of OR to 1
                    emitLabel(set1label, "OR: JUMP HERE WHEN LHS OR RHS IS NON ZERO");

                    // Set v0 as 1 - control reaches here if v0 or v1 are non-zero above
                    emitInstruction("li $v0, %d","OR: LHS AND RHS ARE ZERO, RESULT 1", 1);

                    // Exit label - indicates the completion of evaluation of the OR operator
                    emitLabel(exitlabel, "OR: JUMP HERE WHEN EVALUATION IS COMPLETE");

                    free(set1label);
                    free(exitlabel);
                    break;
                }

                case and_op: {
                    printf("\t\tand_op\n");
                    char * set0label = generateFreshLabel();
                    char * exitlabel = generateFreshLabel();

                    mips_astExpr(e->data.bin_ops.e1, global_types, function_rets, f);

                    // POP lhs into register 0 ($v0)
                    pop0();

                    // Branch to set0label if v0 is zero
                    emitInstruction("beq $v0, $zero, %s", "AND: CHECK LHS ZERO", set0label);

                    // Compile right hand side
                    mips_astExpr(e->data.bin_ops.e2, global_types, function_rets, f);

                    // POP rhs into register 1 ($v1)
                    pop1();

                    // Branch to set1label if v0 is non zero
                    emitInstruction("beq $v1, $zero, %s", "AND: CHECK RHS ZERO", set0label);

                    // Set v0 as 0 - control reaches here if v0 and v1 are both zero
                    emitInstruction("li $v0, %d","AND: LHS AND RHS ARE NON_ZERO, RESULT 1", 1);

                    // Jump to end of evaluation for OR
                    emitInstruction("j %s", "AND: JUMP TO COMPLETE EVALUATION OF AND", exitlabel);

                    // Set 1 label - sets result of OR to 1
                    emitLabel(set0label, "AND: JUMP HERE WHEN LHS OR RHS IS ZERO");

                    // Set v0 as 1 - control reaches here if v0 or v1 are non-zero above
                    emitInstruction("li $v0, %d","AND: LHS AND RHS ARE ZERO, RESULT 1", 0);

                    // Exit label - indicates the completion of evaluation of the OR operator
                    emitLabel(exitlabel, "AND: JUMP HERE WHEN EVALUATION IS COMPLETE");

                    free(set0label);
                    free(exitlabel);
                    break;
                }
                default : {
                    assert(0); //Not supported
                }
            }
            push0();
            break;
        }
        case call_exp: {

            //          Rest of the stack   high memory
            //          -----------------
            // $fp ->   <prev fp>
            //          arg0
            //          ...
            //          argn
            //          <ret add>
            // $sp ->   UNUSED              low memory

            // push current $fp
            pushfp();

            //              Rest of the stack   high memory
            //              -----------------
            // $fp -> <1>   <prev fp>
            //              arg0
            //              ...
            //              argn
            //              <ret add>
            //              <1>
            // $sp ->       UNUSED              low memory


            // evaluate and push each argument
            list * args = e->data.call_ops.args;
            int num_args = 0;
            while(args != NULL) {
                exp_node * arg = (exp_node *) args->head;
                mips_astExpr(arg, global_types, function_rets, f);
                args = args->next;
                num_args++;
            }

            //              Rest of the stack   high memory
            //              -----------------
            // $fp -> <1>   <prev fp>
            //              arg0
            //              ...
            //              argn
            //              <ret add>
            //              <1>
            //              arg0
            //              ...
            //              argn
            // $sp ->       UNUSED              low memory

            // adjust $fp = $sp + N*4
            emitInstruction("move $fp, $sp","CALL EXP: ADJUST FRAME POINTER");
            emitInstruction("addiu $fp, $fp, %d", "CALL EXP: ADJUST FRAME POINTER", num_args*4);


            //              Rest of the stack   high memory
            //              -----------------
            //      <1>     <prev fp>
            //              arg0
            //              ...
            //              argn
            //              <ret add>
            // $fp ->       <1>
            //              arg0
            //              ...
            //              argn
            // $sp ->       UNUSED              low memory


            // call the function
            emitInstruction("jal %s", "CALL EXP: JUMP TO CALLEE", e->data.call_ops.name);
            break;
        }
        case unop_exp: {
            // Compile expression
            mips_astExpr(e->data.un_ops.e, global_types, function_rets, f);

            // POP result into register 0 ($v0)
            pop0();

            switch(e->data.un_ops.op) {
                case not_op: {
                    char * set0label = generateFreshLabel();
                    char * exitlabel = generateFreshLabel();

                    // Branch to set0label if v0 is non zero
                    emitInstruction("bne $v0, $zero, %s", "NOT: BRANCH ON EXPR NON-ZERO",
                                    set0label);

                    // Set v0 as 1 - control reaches here if v0 is zero
                    emitInstruction("li $v0, %d","NOT: SET RESULT AS 1, AS EXPR IS 0", 1);

                    // Jump to end of evaluation for NOT
                    emitInstruction("j %s", "NOT: JUMP TO COMPLETE EVALUATION OF NOT", exitlabel);

                    // Set 0 label - sets result of NOT to 0
                    emitLabel(set0label, "NOT: JUMP HERE WHEN EXPR IS NON ZERO");

                    // Set v0 as 0 - control reaches here if v0 is non-zero
                    emitInstruction("li $v0, %d","NOT: SET RESULT AS 0, AS EXPR IS NON ZERO", 0);

                    // Exit label - indicates the completion of evaluation of the NOT operator
                    emitLabel(exitlabel, "NOT: JUMP HERE WHEN EVALUATION IS COMPLETE");

                    free(set0label);
                    free(exitlabel);

                    break;
                }
                default:
                    assert(0); //Not Supported
            }
            push0();
            break;
        }
        case var_exp: {

            if(f==NULL){
                // Get the type of the variable
                ty_node * var_ty = S_look(global_types, S_Symbol(e->data.var_ops.name));

                switch (var_ty->kind) {

                    case int_ty: {
                        emitInstruction("lw $v0, %s","EXP: INT VAR", e->data.var_ops.name);
                        push0();
                        break;
                    }
                    case string_ty: {
                        emitInstruction("la $v0, %s", "EXP: STRING VAR", e->data.var_ops.name);
                        push0();
                        break;
                    }

                    default: {
                        assert(0); //Not Supported
                    }

                }
            } else {
                char * varname = e->data.var_ops.name;

                // Local variable access
                long index = (long) S_look(f->indexes, S_Symbol(varname));
                ty_node * var_ty = (ty_node *) S_look(f->args_locs_types, S_Symbol(varname));
                if(var_ty == NULL){
                    // Get the type of the variable
                    ty_node * var_ty = S_look(global_types, S_Symbol(e->data.var_ops.name));

                    switch (var_ty->kind) {

                        case int_ty: {
                            emitInstruction("lw $v0, %s","EXP: INT VAR", e->data.var_ops.name);
                            push0();
                            break;
                        }
                        case string_ty: {
                            emitInstruction("la $v0, %s", "EXP: STRING VAR", e->data.var_ops.name);
                            push0();
                            break;
                        }

                        default: {
                            assert(0); //Not Supported
                        }
                    }
                    break;
                }
                switch (var_ty->kind) {

                    case int_ty: {

                        // Variable at index i is located at $fp + (i+1)*4

                        //          Rest of the stack                   high memory
                        //          -----------------
                        //          <Caller's stack>
                        // $fp ->   <prev fp>               index
                        //          arg0                    [0]
                        //          ...                     [1]
                        //          argn                    .
                        //          loc0                    .
                        //          ...                     .
                        //          locm                    [i]
                        //          <ret add>
                        // $sp ->   UNUSED                              low memory

                        long fp_var_offset = -(index)*4;
                        emitInstruction("lw $v0, %ld($fp)", "VAR EXP: load value of local/arg",
                                        fp_var_offset);

                        // Push the variable's value to the stack
                        push0();
                        break;
                    }
                    case string_ty: {
                        // Variable at index i is located at $fp + (i+1)*4

                        //          Rest of the stack                   high memory
                        //          -----------------
                        //          <Caller's stack>
                        // $fp ->   <prev fp>               index
                        //          arg0                    [0]
                        //          ...                     [1]
                        //          argn                    .
                        //          loc0                    .
                        //          ...                     .
                        //          locm                    [i]
                        //          <ret add>
                        // $sp ->   UNUSED                              low memory

                        long fp_var_offset = -(index)*4;
                        emitInstruction("lw $v0, %ld($fp)", "VAR EXP: load value of local/arg",
                                        fp_var_offset);

                        // Push the variable's value to the stack
                        push0();
                        break;
                    }

                    default: {
                        assert(0); //Not Supported
                    }

                }

            }
            break;
        }
        case intrinsic_exp: {
            intrinsic(e->data.intrinsic_ops.name, e->data.intrinsic_ops.args, global_types,
                      function_rets, f);
            break;
        }
        default:
            assert(0);
    }
}

void mips_astStmts(list * l, S_table globals, S_table function_rets, frame * f);

void mips_astStmt(stmt_node * s, S_table global_types, S_table function_rets, frame * f) {
    printf("mips_astStmt\n");
    UNUSED(global_types);
    UNUSED(function_rets);
    UNUSED(f);
    if(!s) return;
    switch(s->kind){
        case assign_stmt: {

            if(f == NULL) {
                char * vname = s->data.assign_ops.lhs;
                ty_node * var_ty = (ty_node *) S_look(global_types, S_Symbol(vname));

                switch (var_ty->kind) {

                    case int_ty: {
                        exp_node * exp = s->data.assign_ops.rhs;
                        mips_astExpr(exp, global_types, function_rets, f);
                        pop0();
                        emitInstruction("sw $v0, %s", "Variable definition", vname);
                        break;
                    }

                    case string_ty: {
                        assert(0);
                    }

                    default: {
                        assert(0); //Unimplemented
                    }
                }

            } else {

                char * vname = s->data.assign_ops.lhs;
                // long index = (long) S_look(f->indexes, S_Symbol(vname));
                ty_node * var_ty = (ty_node *) S_look(f->args_locs_types, S_Symbol(vname));

                if(var_ty == NULL) {

                    var_ty = (ty_node *) S_look(global_types, S_Symbol(vname));
                    switch (var_ty->kind) {

                        case int_ty: {
                            exp_node * exp = s->data.assign_ops.rhs;
                            mips_astExpr(exp, global_types, function_rets, f);
                            pop0();
                            emitInstruction("sw $v0, %s", "Variable definition", vname);
                            break;
                        }

                        case string_ty: {
                            assert(0);
                        }

                        default: {
                            assert(0); //Unimplemented
                        }
                    }

                }
                break;
            }
            break;
        }
        case if_stmt: {
            exp_node * cond = s->data.if_ops.cond;
            list * then_stmts = s->data.if_ops.then_stmts;
            list * else_stmts = s->data.if_ops.else_stmts;

            // Evaluated expression would be stored in $v0
            mips_astExpr(cond, global_types, function_rets, f);

            // Pop evaluated expression from stack to $v0
            pop0();

            if(else_stmts == NULL){

                char * exitthenlbl = generateFreshLabel();
                emitInstruction("beq $v0, $zero, %s", "IF STMT: CONDITION 0 JUMP TO EXIT", exitthenlbl);
                mips_astStmts(then_stmts, global_types, function_rets, f);
                emitLabel(exitthenlbl,"IF STMT: EXIT IF STATEMENT");
                free(exitthenlbl);

            } else {
                char * exitlbl = generateFreshLabel();
                char * elselbl = generateFreshLabel();
                emitInstruction("beq $v0, $zero, %s", "IF STMT: CONDITION 0 JUMP TO EXIT", elselbl);
                mips_astStmts(then_stmts, global_types, function_rets, f);
                emitInstruction("j %s", "IF STMT: JUMP TO EXIT THEN CLAUSE", exitlbl);
                emitLabel(elselbl,"IF STMT: JMP HERE WHEN CONDITION FALSE");
                mips_astStmts(else_stmts, global_types, function_rets, f);
                emitLabel(exitlbl, "IF STMT: EXIT IF STATEMENT");
                free(exitlbl);
                free(elselbl);
            }

            break;
        }
        case while_stmt: {
            char * exitlbl = generateFreshLabel();
            char * startlbl = generateFreshLabel();
            emitLabel(startlbl, "WHILE STMT: start label");
            mips_astExpr(s->data.while_ops.cond, global_types, function_rets, f);
            pop0();
            emitInstruction("beq $v0, $zero, %s", "WHILE STMT: BRANCH ON CONDITION FALSE", exitlbl);
            mips_astStmts(s->data.while_ops.body, global_types, function_rets, f);
            emitInstruction("j %s", "WHILE STMT: GO TO CHECK CONDITION", startlbl);
            emitLabel(exitlbl, "WHILE STMT: exit label");
            free(exitlbl);
            free(startlbl);
            break;
        }
        case repeat_stmt: {
            assert(0);
        }
        case ret_stmt: {
            if(f == NULL) {
                assert(0); // Returning from top level is intrinsic
            }
            // push $v0
            exp_node * retexp = s->data.ret_exp;
            ty_node * rettyp = f->ret;
            switch (rettyp->kind) {

                case void_ty: {


                    // Save return address to register $ra
                    popra();

                    //          Rest of the stack   high memory
                    //          -----------------
                    //          <Caller's stack>
                    // $fp ->   <prev fp>
                    //          arg0
                    //          ...
                    //          argn
                    // $sp ->   UNUSED              low memory
                    //
                    // -----------------------------------------
                    // $ra = <ret add>

                    // Set $sp = $fp
                    emitInstruction("move $sp, $fp","RETURN: RESTORE STACK FOR RETURN");
                    emitInstruction("addi $sp, $sp, 4", "RETURN: RESTORE STACK FOR RETURN");

                    //          Rest of the stack   high memory
                    //          -----------------
                    //          <Caller's stack>
                    // $fp->    <prev fp>           <- $sp
                    //          arg0
                    //          ...
                    //          argn
                    //          <ret add>
                    //          UNUSED              low memory
                    //
                    // -----------------------------------------
                    // $ra = <ret add>

                    // Set $fp = <prev fp>
                    emitInstruction("lw $fp, 4($fp)","RETURN: RESTORE FRAME TO PREVIOUS FRAME");

                    //          Rest of the stack   high memory
                    //          -----------------
                    // $fp ->   <prev fp>
                    //          <Caller's stack>
                    //          <Caller's ret add>
                    //          [UNUSED]<prev fp>   <- $sp
                    //          [UNUSED]arg0
                    //          [UNUSED]...
                    //          [UNUSED]argn
                    //          [UNUSED]<ret add>
                    //          UNUSED              low memory
                    //
                    // -----------------------------------------
                    // $v0 = <value opf ret exp>
                    // $ra = <ret add>

                    //          Rest of the stack   high memory
                    //          -----------------
                    // $fp ->   <prev fp>
                    //          <Caller's stack>
                    //          <Caller's ret add>
                    //          <return value>
                    //          [UNUSED]<prev fp>   <- $sp
                    //          [UNUSED]arg0
                    //          [UNUSED]...
                    //          [UNUSED]argn
                    //          [UNUSED]<ret add>
                    //          UNUSED              low memory
                    //
                    // -----------------------------------------
                    // $v0 = <value opf ret exp>
                    // $ra = <ret add>

                    //Jump to return address [next instruction]
                    emitInstruction("jr $ra","RETURN: return to next instruction");
                    break;
                }

                case int_ty: {

                    //          Rest of the stack   high memory
                    //          -----------------
                    //          <Caller's stack>
                    // $fp ->   <prev fp>
                    //          arg0
                    //          ...
                    //          argn
                    //          <ret add>
                    // $sp ->   UNUSED              low memory

                    // Evaluate the return expression
                    mips_astExpr(retexp, global_types, function_rets, f);

                    //          Rest of the stack   high memory
                    //          -----------------
                    //          <Caller's stack>
                    // $fp ->   <prev fp>
                    //          arg0
                    //          ...
                    //          argn
                    //          <ret add>
                    //            <value of ret exp>
                    // $sp ->   UNUSED              low memory

                    // Save return value to register v0
                    pop0();

                    //          Rest of the stack   high memory
                    //          -----------------
                    //          <Caller's stack>
                    // $fp ->   <prev fp>
                    //          arg0
                    //          ...
                    //          argn
                    //          <ret add>
                    // $sp ->   UNUSED              low memory
                    //
                    // -----------------------------------------
                    //  $v0 = <value opf ret exp>

                    // Save return address to register $ra
                    popra();

                    //          Rest of the stack   high memory
                    //          -----------------
                    //          <Caller's stack>
                    // $fp ->   <prev fp>
                    //          arg0
                    //          ...
                    //          argn
                    // $sp ->   UNUSED              low memory
                    //
                    // -----------------------------------------
                    // $v0 = <value opf ret exp>
                    // $ra = <ret add>

                    // Set $sp = $fp
                    emitInstruction("move $sp, $fp","RETURN: RESTORE STACK FOR RETURN");
                    emitInstruction("addi $sp, $sp, 4", "RETURN: RESTORE STACK FOR RETURN");

                    //          Rest of the stack   high memory
                    //          -----------------
                    //          <Caller's stack>
                    // $fp->    <prev fp>           <- $sp
                    //          arg0
                    //          ...
                    //          argn
                    //          <ret add>
                    //          UNUSED              low memory
                    //
                    // -----------------------------------------
                    // $v0 = <value opf ret exp>
                    // $ra = <ret add>

                    // Set $fp = <prev fp>
                    emitInstruction("lw $fp, 4($fp)","RETURN: RESTORE FRAME TO PREVIOUS FRAME");

                    //          Rest of the stack   high memory
                    //          -----------------
                    // $fp ->   <prev fp>
                    //          <Caller's stack>
                    //          <Caller's ret add>
                    //          [UNUSED]<prev fp>   <- $sp
                    //          [UNUSED]arg0
                    //          [UNUSED]...
                    //          [UNUSED]argn
                    //          [UNUSED]<ret add>
                    //          UNUSED              low memory
                    //
                    // -----------------------------------------
                    // $v0 = <value opf ret exp>
                    // $ra = <ret add>

                    // Push return value to the top of the stack
                    push0();

                    //          Rest of the stack   high memory
                    //          -----------------
                    // $fp ->   <prev fp>
                    //          <Caller's stack>
                    //          <Caller's ret add>
                    //          <return value>
                    //          [UNUSED]<prev fp>   <- $sp
                    //          [UNUSED]arg0
                    //          [UNUSED]...
                    //          [UNUSED]argn
                    //          [UNUSED]<ret add>
                    //          UNUSED              low memory
                    //
                    // -----------------------------------------
                    // $v0 = <value opf ret exp>
                    // $ra = <ret add>

                    //Jump to return address [next instruction]
                    emitInstruction("jr $ra","RETURN: return to next instruction");
                    break;
                }

                case string_ty: {
                    //          Rest of the stack   high memory
                    //          -----------------
                    //          <Caller's stack>
                    // $fp ->   <prev fp>
                    //          arg0
                    //          ...
                    //          argn
                    //          <ret add>
                    // $sp ->   UNUSED              low memory

                    // Evaluate the return expression
                    mips_astExpr(retexp, global_types, function_rets, f);

                    //          Rest of the stack   high memory
                    //          -----------------
                    //          <Caller's stack>
                    // $fp ->   <prev fp>
                    //          arg0
                    //          ...
                    //          argn
                    //          <ret add>
                    //            <value of ret exp>
                    // $sp ->   UNUSED              low memory

                    // Save return value to register v0
                    pop0();

                    //          Rest of the stack   high memory
                    //          -----------------
                    //          <Caller's stack>
                    // $fp ->   <prev fp>
                    //          arg0
                    //          ...
                    //          argn
                    //          <ret add>
                    // $sp ->   UNUSED              low memory
                    //
                    // -----------------------------------------
                    //  $v0 = <value opf ret exp>

                    // Save return address to register $ra
                    popra();

                    //          Rest of the stack   high memory
                    //          -----------------
                    //          <Caller's stack>
                    // $fp ->   <prev fp>
                    //          arg0
                    //          ...
                    //          argn
                    // $sp ->   UNUSED              low memory
                    //
                    // -----------------------------------------
                    // $v0 = <value opf ret exp>
                    // $ra = <ret add>

                    // Set $sp = $fp
                    emitInstruction("move $sp, $fp","RETURN: RESTORE STACK FOR RETURN");
                    emitInstruction("addi $sp, $sp, 4", "RETURN: RESTORE STACK FOR RETURN");

                    //          Rest of the stack   high memory
                    //          -----------------
                    //          <Caller's stack>
                    // $fp->    <prev fp>           <- $sp
                    //          arg0
                    //          ...
                    //          argn
                    //          <ret add>
                    //          UNUSED              low memory
                    //
                    // -----------------------------------------
                    // $v0 = <value opf ret exp>
                    // $ra = <ret add>

                    // Set $fp = <prev fp>
                    emitInstruction("lw $fp, 4($fp)","RETURN: RESTORE FRAME TO PREVIOUS FRAME");

                    //          Rest of the stack   high memory
                    //          -----------------
                    // $fp ->   <prev fp>
                    //          <Caller's stack>
                    //          <Caller's ret add>
                    //          [UNUSED]<prev fp>   <- $sp
                    //          [UNUSED]arg0
                    //          [UNUSED]...
                    //          [UNUSED]argn
                    //          [UNUSED]<ret add>
                    //          UNUSED              low memory
                    //
                    // -----------------------------------------
                    // $v0 = <value opf ret exp>
                    // $ra = <ret add>

                    // Push return value to the top of the stack
                    push0();

                    //          Rest of the stack   high memory
                    //          -----------------
                    // $fp ->   <prev fp>
                    //          <Caller's stack>
                    //          <Caller's ret add>
                    //          <return value>
                    //          [UNUSED]<prev fp>   <- $sp
                    //          [UNUSED]arg0
                    //          [UNUSED]...
                    //          [UNUSED]argn
                    //          [UNUSED]<ret add>
                    //          UNUSED              low memory
                    //
                    // -----------------------------------------
                    // $v0 = <value opf ret exp>
                    // $ra = <ret add>

                    //Jump to return address [next instruction]
                    emitInstruction("jr $ra","RETURN: return to next instruction");
                    break;
                }

                default: {
                    assert(0); //Not Implemented
                }
            }
            break;
        }
        case call_stmt: {
            //          Rest of the stack   high memory
            //          -----------------
            // $fp ->   <prev fp>
            //          arg0
            //          ...
            //          argn
            //          <ret add>
            // $sp ->   UNUSED              low memory

            // push current $fp
            pushfp();

            //              Rest of the stack   high memory
            //              -----------------
            // $fp -> <1>   <prev fp>
            //              arg0
            //              ...
            //              argn
            //              <ret add>
            //              <1>
            // $sp ->       UNUSED              low memory


            // evaluate and push each argument
            list * args = s->data.call_ops.args;
            int num_args = 0;
            while(args != NULL) {
                exp_node * arg = (exp_node *) args->head;
                mips_astExpr(arg, global_types, function_rets, f);
                args = args->next;
                num_args++;
            }

            //              Rest of the stack   high memory
            //              -----------------
            // $fp -> <1>   <prev fp>
            //              arg0
            //              ...
            //              argn
            //              <ret add>
            //              <1>
            //              arg0
            //              ...
            //              argn
            // $sp ->       UNUSED              low memory

            // adjust $fp = $sp + N*4
            emitInstruction("move $fp, $sp","CALL EXP: ADJUST FRAME POINTER");
            emitInstruction("addiu $fp, $fp, %d", "CALL EXP: ADJUST FRAME POINTER", num_args*4);


            //              Rest of the stack   high memory
            //              -----------------
            //      <1>     <prev fp>
            //              arg0
            //              ...
            //              argn
            //              <ret add>
            // $fp ->       <1>
            //              arg0
            //              ...
            //              argn
            // $sp ->       UNUSED              low memory


            // call the function
            emitInstruction("jal %s", "CALL EXP: JUMP TO CALLEE", s->data.call_ops.name);
            break;
        }
        case intrinsic_stmt: {
            printf("mips_ast intrinsic stmt\n");
            intrinsic(s->data.intrinsic_ops.name, s->data.intrinsic_ops.args, global_types,
                      function_rets, f);
            break;
         }
        default:
            assert(0);
    }
}

void mips_astStmts(list * l, S_table globals_types, S_table function_rets, frame * f) {
    printf("mips_astStmts\n");
    if (l == NULL) return;
    mips_astStmt(l->head, globals_types, function_rets, f);
    mips_astStmts(l->next, globals_types, function_rets, f);
    printf("mips_astStmts END\n");
}

void mips_astVariable(vardec_node * node, S_table globals_types, S_table function_rets, frame * f) {
    debug_print("Variable defined: %s of type %s\n", node->name, typeToStr(node->type));

    if(f != NULL) {
        // Create room for local variables and initialize them
        exp_node * init = node->init;
        mips_astExpr(init, globals_types, function_rets, f);
        return;
    }

    switch(node->type->kind) {

        case int_ty: {
            emitInstruction("%s: .word 0", "INT VAR: DEFINITION", node->name);

            fprintf(out, ".text\n");
            mips_astExpr(node->init, globals_types, function_rets, f);
            pop0();
            emitInstruction("sw $v0, %s", "INT VAR: DEFINITION", node->name);
            break;
        }
        case string_ty: {

            char * init = node->init->data.sval;
            emitInstruction("%s: .asciiz %s", "STRING VAR: DEFINITION", node->name, init);
            fprintf(out, ".text\n");
            break;
        }
        default: {
            assert(0); // Not implemented
        }

    }
}

void mips_astVariables(list * l, S_table global_types, S_table function_rets, frame * f) {
    if (l == NULL) return;

    if(f == NULL) fprintf(out, ".data\n");
    mips_astVariable((vardec_node *)l->head, global_types, function_rets, f);
    mips_astVariables(l->next, global_types, function_rets, f);
}

void mips_astFunction(fundec_node * fundec, S_table global_types, S_table functions_rets, frame *f) {

    // TAG for the function
    emitInstruction("%s:", "FUNCTION DECLARATION", fundec->name);

    // Evaluate function variables
    mips_astVariables(fundec->locs, global_types, functions_rets, f);

    list * locs = fundec->locs;
    int num_locs = 0;
    while(locs!=NULL){
        num_locs++;
        locs = locs->next;
    }
    debug_print("FUNCTION DECLARATION OF %s, NUM locs %d", fundec->name, num_locs);

    // PUSH the return address to the stack
    pushra();

    debug_print("START OF FUNCTION BODY %s", fundec->name);

    mips_astStmts(fundec->stmts, global_types, functions_rets, f);

    debug_print("END OF FUNCTION BODY %s", fundec->name);
}

void mips_astFunctions(list * l, S_table global_types, S_table function_ret_types, S_table frames) {
    if (l == NULL) return;
    fundec_node * fundec = (fundec_node*) l->head;
    frame * f = S_look(frames, S_Symbol(fundec->name));
    mips_astFunction(fundec, global_types, function_ret_types, f);
    mips_astFunctions(l->next, global_types, function_ret_types, frames);
}

static void emitLabel(char * label, char * comment, ...) {
    static char buffer[1024];
    va_list argp;

    assert(comment != NULL && strcmp("",comment));
    assert(label != NULL);

    va_start(argp, comment);
    vsnprintf(buffer, sizeof(buffer), label, argp);
    va_end(argp);
    fprintf(out, "%s: # %s\n", buffer, comment);
    fflush(out);
}

static void emitInstruction(char * instruction, char * comment, ...) {
    static char buffer[1024];

    va_list argp;
    assert(comment != NULL && strcmp("",comment));
    assert(instruction != NULL);

    va_start(argp, comment);
    vsnprintf(buffer, sizeof(buffer), instruction, argp);
    va_end(argp);
    fprintf(out, "\t\t%s\t\t\t# %s\n", buffer, comment);
    fflush(out);
}

static void push0() {
    // Push register v0

    // $sp pointing to the next empty position in the stack
    // Copy $v0 to top of the stack
    // Adjust $sp to point to the next unused position

    //          Rest of the stack   high memory
    //          -----------------
    // $sp ->   UNUSED              low memory

    emitInstruction("sw $v0, ($sp)", "PUSH store $v0 on top of stack");
    //          Rest of the stack   high memory
    //          -----------------
    // $sp ->   $v0                 low memory


    //          Rest of the stack   high memory
    //          -----------------
    //          $v0                 low memory
    // $sp ->   UNUSED
    emitInstruction("subi $sp, $sp, 4", "PUSH adjust $sp to next unused position");
    return;
}

static void push1() {
    // Push register v1

    // $sp pointing to the next empty position in the stack
    // Copy $v0 to top of the stack
    // Adjust $sp to point to the next unused position

    //          Rest of the stack   high memory
    //          -----------------
    // $sp ->   UNUSED              low memory

    emitInstruction("sw $v1, ($sp)", "PUSH store $v1 on top of stack");
    //          Rest of the stack   high memory
    //          -----------------
    // $sp ->   $v1                 low memory


    //          Rest of the stack   high memory
    //          -----------------
    //          $v1                 low memory
    // $sp ->   UNUSED
    emitInstruction("subi $sp, $sp, 4", "PUSH adjust $sp to next unused position");
    return;
}

static void pushra() {

    // Push register ra

    // $sp pointing to the next empty position in the stack
    // Copy $ra to top of the stack
    // Adjust $sp to point to the next unused position

    //          Rest of the stack   high memory
    //          -----------------
    // $sp ->   UNUSED              low memory

    emitInstruction("sw $ra, ($sp)", "PUSH store $ra on top of stack");
    //          Rest of the stack   high memory
    //          -----------------
    // $sp ->   $ra                low memory


    //          Rest of the stack   high memory
    //          -----------------
    //          $ra                 low memory
    // $sp ->   UNUSED
    emitInstruction("subi $sp, $sp, 4", "PUSH adjust $sp to next unused position");
    return;

}

static void pushfp() {

    // Push register fp

    // $sp pointing to the next empty position in the stack
    // Copy $fo to top of the stack
    // Adjust $sp to point to the next unused position

    //          Rest of the stack   high memory
    //          -----------------
    // $sp ->   UNUSED              low memory

    emitInstruction("sw $fp, ($sp)", "PUSH store $fp on top of stack");
    //          Rest of the stack   high memory
    //          -----------------
    // $sp ->   $fp                low memory


    //          Rest of the stack   high memory
    //          -----------------
    //          $fp                low memory
    // $sp ->   UNUSED
    emitInstruction("subi $sp, $sp, 4", "PUSH adjust $sp to next unused position");
    return;

}

static void popra() {

    // Pop to register ra

    // Store the top of the stack on register ra

    //          Rest of the stack   high memory
    //          -----------------
    //          value
    // $sp ->   UNUSED              low memory

    emitInstruction("addi $sp, $sp, 4", "POP adjust $sp to previous position");

    // Store the top of the stack on register ra

    //          Rest of the stack   high memory
    //          -----------------
    // $sp ->   value

    emitInstruction("lw $ra, ($sp)","POP load top of stack into $ra");

    // Store the top of the stack on register ra

    //          Rest of the stack   high memory
    //          -----------------
    // $sp ->   UNUSED

    return;

}

static void pop0() {
    // Pop to register v0

    // Store the top of the stack on register v0

    //          Rest of the stack   high memory
    //          -----------------
    //          value
    // $sp ->   UNUSED              low memory

    emitInstruction("addi $sp, $sp, 4", "POP adjust $sp to previous position");

    // Store the top of the stack on register v0

    //          Rest of the stack   high memory
    //          -----------------
    // $sp ->   value

    emitInstruction("lw $v0, ($sp)","POP load top of stack into $v0");

    // Store the top of the stack on register v0

    //          Rest of the stack   high memory
    //          -----------------
    // $sp ->   UNUSED

    return;
}

static void pop1() {
    // Pop to register v0

    // Store the top of the stack on register v0

    //          Rest of the stack   high memory
    //          -----------------
    //          value
    // $sp ->   UNUSED              low memory
    emitInstruction("addi $sp, $sp, 4", "POP adjust $sp to previous position");

    // Store the top of the stack on register v0

    //          Rest of the stack   high memory
    //          -----------------
    // $sp ->   value
    emitInstruction("lw $v1, ($sp)","POP load top of stack into $v0");

    // Store the top of the stack on register v0

    //          Rest of the stack   high memory
    //          -----------------
    // $sp ->   UNUSED

    return;
}

void mips_ast(program * p, S_table global_types, S_table function_rets, S_table frames, FILE * o) {
    printf("mips_ast\n");
    out = o;

    UNUSED(generateFreshLabel);
    UNUSED(emitInstruction);
    UNUSED(emitLabel);
    UNUSED(push0);
    UNUSED(push1);
    UNUSED(pushra);
    UNUSED(pop0);
    UNUSED(pop1);

    mips_astVariables(p->variables, global_types, function_rets, NULL);
    mips_astStmts(p->statements, global_types, function_rets, NULL);

    fprintf(out, "\n");
    mips_astFunctions(p->functions, global_types, function_rets, frames);

}
