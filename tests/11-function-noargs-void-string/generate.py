#!/usr/bin/python3
import random
import string

maxDepth = 6
topLevelStmts = 10

exprs   = [ "+","-","*","|","&","term" ]
logical = [ "<", ">", "<=",  ">=", "==", "<>", "||"]
stmts =   [ "if", "printint" ]

voids = []
strings = []

vars=10
functions=10
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

def genStringFunction():
    name = "fun" + str(random.randint(0,0xFFFF))
    strings.append(name)
    if random.choice([True, False]):
        print("fun " + name + " string () { return s"+str(random.randint(0, vars-1)) + ";" + "}")
    else:
        print("fun " + name + " string () { return \""+ randString() + "\";" + "}")

def genVoidFunction():
    name = "fun" + str(random.randint(0,0xFFFF))
    voids.append(name)
    if random.choice([True, False]):
        print("fun " + name + " void () { printstring(s"+str(random.randint(0, vars-1)) + ");" + "}")
    else:
        print("fun " + name + " void () { printstring(\""+ randString() + "\");" + "}")

def randString():
    return (''.join(random.choices(string.ascii_letters + string.digits, k=random.randint(1,20))));

for i in range(vars):
    print("var s"+str(i)+" string := \"" + randString() + "\" ;")


for i in range(functions):
    genStringFunction()
    genVoidFunction()

for i in range(calls):
    print(random.choice(voids)+"();");
    print("printstring("+random.choice(strings)+"());");

print("exit(0);")
