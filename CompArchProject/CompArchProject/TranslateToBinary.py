OpcodeToBinary = {"add":0, "sub":1, "addi":2, "sll":3, "srl":4, "and":5, "nor":6, "or":7, "ori":8, "xor":9, "lw":10, "sw":11, "beq":12, "bltz":13, "j":14, "blt":15}
NameToRegister = {"$zero":0, "$a0":1, "$a1":2, "$v0":3, "$v1":4, "$v2":5, "$v3":6, "$t0":7, "$t1":8, "$t2":9, "$t3":10}

def TranslateAssembly(line):
    binaryLine = ""
    opCode = ""
    rd = ""
    rs = ""
    rt = ""
    for instruction in OpcodeToBinary:
        if instruction == line[:3] or instruction == line[:4] or instruction == line[:1]:
            opCode = str(bin(OpcodeToBinary[instruction]))[2:].zfill(4)
            print(opCode)
    if line[:3] == "add" or line[:3] == "sub" or line[:3] == "sll" or line[:3] == "srl" or line[:3] == "and" or line[:3] == "nor" or line[:3] == "xor":
        print(line[4:7])
        print(str(bin(NameToRegister[line[4:7]]))[2:].zfill(4))
        rd = str(bin(NameToRegister[line[4:7]]))[2:].zfill(4)
        rs = str(bin(NameToRegister[line[9:12]]))[2:].zfill(4)
        rt = str(bin(NameToRegister[line[14:17]]))[2:].zfill(4)
    print("opcode: "+opCode)
    print("rd: "+rd)
    print("rs: "+rs)
    print("rt: "+rt)
    return(binaryLine)

with open('rawAssembly.txt', 'r') as assemblyFile, open('program.txt', 'w') as programFile:
    for line in assemblyFile:
        programFile.write(TranslateAssembly(line))

