isLabel = False
with open('program.txt', 'w') as programFile, open('unFormatted.txt', 'r') as unFormattedFile:
    allLines = ""
    for line in unFormattedFile:
        line = line.replace(" ","")
        line = line.replace("\n","")
        allLines += line
    for i in allLines:
        if(i.isalpha()):
            isLabel = True
    programFile.write(allLines)
if(isLabel):
    print("ATTENTION: There are LABELS in the code that must")
    print("be manually translated!!")
else:
    print("Finished translating with no errors!")
        
