#!/usr/bin/python3
import random
import string

maxDepth = 6
topLevelStmts = 10

exprs = [ "+","-","*","|","&","term" ]
stmts = [ "if", "printint" ]

sDepth = 0
eDepth = 0

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
            #return "return " + genExpr() + ";"

        e = "if" #random.choice(stmts)
        match e:
            case "printint":
                return "printint( " + genExpr() + ");"
            case "return":
                return "return " + genExpr() + ";"
            case "if":
                return "if (" + genExpr() + ") {" + genStmt() + "}"
            case "while":
                return "while (" + genExpr() + ") {" + genStmt() + "}"
            case "repeat":
                return "repeat (" + genExpr() + ") {" + genStmt() + "}"
        return ""
    finally:
        sDepth -= 1

def genTerm():
    return str(random.randint(0,0xFF))

for i in range(topLevelStmts):
    print(genStmt())

print("exit(0);")
