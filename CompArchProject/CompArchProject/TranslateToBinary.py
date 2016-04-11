OpcodeToBinary = {"add":0, "sub":1, "addi":2, "sll":3, "srl":4, "and":5, "nor":6, "or":7, "ori":8, "xor":9, "lw":10, "sw":11, "beq":12, "bltz":13, "j":14, "blt":15}
NameToRegister = {"$zero":0, "$a0":1, "$a1":2, "$v0":3, "$v1":4, "$v2":5, "$v3":6, "$t0":7, "$t1":8, "$t2":9, "$t3":10}

def TranslateAssembly(line):
    binaryLine = ""
    for instruction in OpcodeToBinary:
        if instruction == line[:3]:
            if line[:4] != "addi" and line[:4] != "bltz":
                binaryLine += str(bin(OpcodeToBinary[instruction]))[2:].zfill(4)
        
    print(binaryLine)
    return(binaryLine)

with open('rawAssembly.txt', 'r') as assemblyFile, open('program.txt', 'w') as programFile:
    for line in assemblyFile:
        programFile.write(TranslateAssembly(line))

