#!/usr/bin/python3
import random
import string

maxDepth = 6
topLevelStmts = 10

exprs   = [ "+","-","*","|","&","term" ]
logical = [ "<", ">", "<=",  ">=", "==", "<>", "||"]
stmts =   [ "if", "printint" ]

vars=10

sDepth = 0
eDepth = 0

def genLogicalTrue():
    global eDepth
    eDepth += 1
    try:
        if eDepth == maxDepth:
            return genTerm()

        e = random.choice(logical)
        match e:
            case "term":
                return genTerm()
            case "=="|"<="|">=":
                e = genExpr()
                return e + "==" + e
            case "<>":
                e1 = genExpr()
                e2 = genExpr()
                return e1 + "<>" + e2
            case "<":
                e = genExpr()
                return e + "<(" + e + "|2000000)"
            case ">":
                e = genExpr()
                return "(" + e + "|2000000)>" + e
            case "||":
                e = genExpr()
                return e + "||1"
        return ""
    finally:
        eDepth -= 1

def genExpr():
    global eDepth
    eDepth += 1
    try:
        if eDepth == maxDepth:
            return genTerm()

        e = random.choice(exprs)
        match e:
            case "term":
                return genTerm()
            case "!":
                return "!" + genExpr()
            case _:
                return genExpr() + e + genExpr()
        return ""
    finally:
        eDepth -= 1

def genStmt():
    global sDepth
    sDepth += 1
    try:
        if sDepth == maxDepth:
            return "printint( " + genExpr() + ");"

        e = "if" #random.choice(stmts)
        match e:
            case "printint":
                return "printint( " + genExpr() + ");"
            case "return":
                return "return " + genExpr() + ";"
            case "if":
                if random.choice([True,False]):
                    return "if (" + genLogicalTrue() + ") {" + genStmt() + "}"
                else:
                    return "if (!" + genLogicalTrue() + ") {" + genStmt() + "}"
        return ""
    finally:
        sDepth -= 1

def genTerm():
    return str(random.randint(0,0xFF))

for i in range(vars):
    print("var t"+str(i)+" int := 0;")
    print("var f"+str(i)+" int := 0;")

for i in range(vars):
    print("f"+str(i)+" := !" + genLogicalTrue() + ";")
    print("t"+str(i)+" :=  " + genLogicalTrue() + ";")

for i in range(vars):
    print("if (f"+str(i)+") { printint(" + genExpr() + "); }")
    print("if (t"+str(i)+") { printint(" + genExpr() + "); }")

print("exit(0);")
