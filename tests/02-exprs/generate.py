#!/usr/bin/python3
import random
import string

maxDepth = 6
topLevelStmts = 10

exprs = [ "+","-","|","&","^","term" ]

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

def genTerm():
    return str(random.randint(0,0xFF))

for i in range(topLevelStmts):
    print("printint(" + genExpr() + ");")

print("return 0;")
