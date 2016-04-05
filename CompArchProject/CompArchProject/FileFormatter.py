with open('program.txt', 'w') as programFile, open('unFormatted.txt', 'r') as unFormattedFile:
    allLines = ""
    for line in unFormattedFile:
        line = line.replace(" ","")
        line = line.replace("\n","")
        allLines += line
    programFile.write(allLines)
        
