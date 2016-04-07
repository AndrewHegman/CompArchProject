BinaryToOpcode = {0:"add", 1:"sub", 2:"addi", 3:"sll", 4:"srl", 5:"and", 6:"nor", 7:"or", 8:"ori", 9:"xor", 10:"lw", 11:"sw", 12:"beq", 13:"bltz", 14:"j", 15:"blt"}
RegisterToName = {0:"$zero", 1:"$a0", 2:"$a1", 3:"$v0", 4:"$v1", 5:"$v2", 6:"$v3", 7:"$t0", 8:"$t1", 9:"$t2", 10:"$t3"}

def TranslateBinary(line):
    if(int(line[:4],2) == 0 or int(line[:4],2) == 1 or int(line[:4],2) == 3 or int(line[:4],2) == 4 or int(line[:4],2) == 5 or int(line[:4],2) == 6 or int(line[:4],2) == 7 or int(line[:4],2) == 9):
        print(BinaryToOpcode[int(line[:4],2)]+" "+RegisterToName[int(line[5:9],2)]+", "+RegisterToName[int(line[10:14],2)]+", "+RegisterToName[int(line[15:19],2)])
    elif(int(line[:4],2) == 14):
        print("j "+line[5:])
    elif(int(line[:4],2) == 12 or int(line[:4],2) == 13 or int(line[:4],2) == 15):
        print(BinaryToOpcode[int(line[:4],2)]+" "+RegisterToName[int(line[5:9],2)]+", "+RegisterToName[int(line[10:14],2)]+", "+line[15:])      
    else:
        print(BinaryToOpcode[int(line[:4],2)]+" "+RegisterToName[int(line[10:14], 2)]+", "+RegisterToName[int(line[5:9])]+", "+str(int(line[15:19],2)))




with open('unFormatted.txt', 'r') as programFile:
    for line in programFile:
        TranslateBinary(line)
            
