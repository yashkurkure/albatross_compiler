#!/usr/bin/python3
import random
import string

maxDepth = 6
topLevelStmts = 10

exprs   = [ "+","-","*","|","&" ]
logical = [ "<", ">", "<=",  ">=", "==", "<>", "||"]
stmts =   [ "if", "printint" ]

funcs = {}
funcs[1] = []
funcs[2] = []
funcs[3] = []
funcs[4] = []

functions=30
calls=10

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

def genFunction():
    name = "fun" + str(random.randint(0,0xFFFF))
    n = random.randint(1,4)
    funcs[n].append(name)
    args = []
    adds = []
    for i in range(n):
        args.append("a"+str(i)+" int")
        adds.append("a"+str(i))

    op = random.choice(exprs)
    print("fun " + name + " int (" + ",".join(args) + ") { return "+ op.join(adds) + ";" + "}")

def randString():
    return (''.join(random.choices(string.ascii_letters + string.digits, k=random.randint(1,20))));

for i in range(functions):
    genFunction()

for i in range(calls):
    n = random.randint(1,4)
    args = []
    for i in range(n):
        args.append(genExpr())
    fun = random.choice(funcs[n])
    print("printint("+fun+"("+(",".join(args))+"));")

print("exit(0);")
