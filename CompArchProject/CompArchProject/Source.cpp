#include <iostream>
#include <fstream>
#include <string>

using namespace std;

typedef enum OPCODE{
	ADD,
	SUB,
	ADDI,
	SLL,
	SRL,
	NOR,
	OR,
	ORI,
	XOR,
	LW,
	SW,
	BEQ,
	BLTZ,
	J,
	BLT
};

typedef enum INSTRUCTION_TYPE{
	R,
	I,
	J,
};

typedef enum BUFFER{
	FETCH_DECODE,
	DECODE_EXECUTE,
	EXECUTE_MEMORY,
	MEMORY_WRITEBACK
};

struct INSTRUCTION{
	int rd;
	int rs;
	int rt;
	int tempMem;			//This will be used to store the values after the EXE stage and before the WB stages. So lets say you want to do an ADD instruction; the sum of RS and RT will be stored in this register until the WB stage
	int immediate;
	string rawFunction;		//This is the line of code straight out of the text file
	OPCODE func;			//This will be the actual function of the line of code. I *think* this is assigned in the InstructionDecode stage
	INSTRUCTION_TYPE type;
	//We can either add a bunch of entries here for the different
	//control signals
	//OR we could handle all of that "on the fly"--like we won't use
	//actual control signals, the program will just do what it needs
	//to do. I don't know if Dr. Sherif will be happy with the second
	//idea thoug...
};
void InstructionFetch(string[], int, INSTRUCTION);
void InstructionDecode();
/*
//This is the logic I came up with, but IDK where it should go..
switch(OpCode){
case ADD:
	break;
case SUB:
	break;
case ADDI:
	break;
case SLL:
	break;
case SRL:
	break;
case NOR:
	break;
case OR:
	break;
case ORI:
	break;
case LW:
	break;
case SW:
	break;
case BEQ:
	break;
case BLTZ:
	break;
case J:
	break;
case BLT:
	break;
default:
}
*/


int main(void){
	int instructionCount = 0;
	int i = 0;
	string* instructionMemory;
	string line;
	ifstream programFile;
	programFile.open("name.txt", ios::in);

	//This finds the number of lines in the file
	while(!programFile.eof()){
		getline(programFile,line);
		instructionCount++;
	}
	programFile.close();

	//Dynamically allocate memory for array
	instructionMemory = new string[instructionCount];

	//Re-open file

	programFile.open("name.txt", ios::in);
	while(!programFile.eof()){
		getline(programFile, instructionMemory[i]);
		i++;
	}
	system("PAUSE");
	return(0);
}


void InstructionFetch(string instructionMemory, int PC, INSTRUCTION currInstruction){
	currInstruction.rawFunction = instructionMemory[PC];
}

void InstructionDecode(INSTRUCTION currInstruction){
	//Get the opcode--always the first 4 bits
	//But it just so happens that our ENUM perfectly matches the 
	//opcode number..I THINK
	int temp;
	temp = stoi(currInstruction.rawFunction) & 0xF000;	//This line is ONLY included to make things more readable. It can be removed later (by placing it on the line below)
	currInstruction.func = static_cast<OPCODE>(temp);

	//Need to figure out if the instruction is R-, I-, or J-type
	if(currInstruction.func == ADD || (currInstruction.func == SUB || (currInstruction.func == SLL || (currInstruction.func == SRL || (currInstruction.func == NOR || (currInstruction.func == OR || (currInstruction.func == XOR))))))){
		//Its an R-type
		currInstruction.rs = stoi(currInstruction.rawFunction) & 0x0F00;
		currInstruction.rt = stoi(currInstruction.rawFunction) & 0x00F0;
		currInstruction.rd = stoi(currInstruction.rawFunction) & 0x000F;
		currInstruction.type = R;
	}
	else if(currInstruction.func == ADDI || (currInstruction.func == ORI || (currInstruction.func == LW || (currInstruction.func == SW || (currInstruction.func == BEQ || (currInstruction.func == BLTZ || (currInstruction.func == BLT))))))){
		//Its an I-type
		currInstruction.rs = stoi(currInstruction.rawFunction) & 0x0F00;
		currInstruction.rt = stoi(currInstruction.rawFunction) & 0x0F00;
		currInstruction.immediate = stoi(currInstruction.rawFunction) & 0x000F;
		currInstruction.type = I;
	}
	else if(currInstruction.func == J){
		//Its an J-type
		currInstruction.immediate = stoi(currInstruction.rawFunction) & 0x03FF;
		currInstruction.type = J;
	}
}