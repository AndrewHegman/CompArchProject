#include <iostream>
#include <fstream>
#include <string>
#include <typeinfo>
using namespace std;

#define INSTRUCTION_LEN 16
typedef enum OPCODE{
	ADD,
	SUB,
	ADDI,
	SLL,
	SRL,
	AND,
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
	R_TYPE,
	I_TYPE,
	J_TYPE,
};

typedef enum BUFFER{
	FETCH_DECODE,
	DECODE_EXECUTE,
	EXECUTE_MEMORY,
	MEMORY_WRITEBACK
};

struct REGISTER{
	int number;		//this is the memory position
	int value;
};

struct INSTRUCTION{
	REGISTER rd;
	REGISTER rs;
	REGISTER rt;
	int tempMem;			//This will be used to store the values after the EXE stage and before the WB stages. So lets say you want to do an ADD instruction; the sum of RS and RT will be stored in this register until the WB stage
	int immediate;
	int rawFunction;		//This is the line of code straight out of the text file
	OPCODE func;			//This will be the actual function of the line of code. I *think* this is assigned in the InstructionDecode stage
	INSTRUCTION_TYPE type;
	//We can either add a bunch of entries here for the different
	//control signals
	//OR we could handle all of that "on the fly"--like we won't use
	//actual control signals, the program will just do what it needs
	//to do. I don't know if Dr. Sherif will be happy with the second
	//idea thoug...
};

void InstructionFetch(int[], int, INSTRUCTION*);
void InstructionDecode(INSTRUCTION*);
void InstructionExecute(INSTRUCTION*);
void InstructionExecute_R(INSTRUCTION*);
void InstructionExecute_I(INSTRUCTION*);
void InstructionExecute_J(INSTRUCTION*);
void InstructionMem(INSTRUCTION*);
void InstructionWriteBack(INSTRUCTION*);

void RegisterInit(INSTRUCTION*);

REGISTER zero;
REGISTER a0;
REGISTER a1;
REGISTER v0;
REGISTER v1;
REGISTER v2;
REGISTER v3;
REGISTER t0;
REGISTER t1;
REGISTER t2;
REGISTER t3;
REGISTER RegisterArray[11] = {zero, a0, a1, v0, v1, v2, v3, t0, t1, t2, t3};
INSTRUCTION testInstruction;



int main(void){
	int instructionCount = 0;
	int i = 0;
	int* instructionMemory;
	char* nptr = NULL;
	char ptr[16];
	
	
	string line;
	ifstream programFile;
	programFile.open("program.txt", ios::in);

	//This finds the number of lines in the file
	while(!programFile.eof()){
		getline(programFile,line);
		instructionCount++;
	}
	programFile.close();

	//Dynamically allocate memory for array
	instructionMemory = new int[instructionCount];
	//Re-open file

	programFile.open("program.txt", ios::in);
	while(!programFile.eof()){
		programFile.read(ptr, INSTRUCTION_LEN);
		//cout<<strtol(ptr, &nptr, 2)<<endl;
		instructionMemory[i] = strtol(ptr, &nptr, 2);
		i++;
	}
	for(i=0; i<instructionCount; i++){
		cout<<instructionMemory[i]<<endl;;
	}
	RegisterInit(&testInstruction);
	/*
	cout<<"Rs: "<<testInstruction.rs.number<<endl;
	cout<<"Rs value: "<<testInstruction.rs.value<<endl;
	cout<<"Rt: "<<testInstruction.rt.number<<endl;
	cout<<"Rt value: "<<testInstruction.rt.value<<endl;
	cout<<"Rd: "<<testInstruction.rd.number<<endl;
	cout<<"Rd value: "<<testInstruction.rd.value<<endl;
	system("PAUSE");
	*/
	InstructionFetch(instructionMemory, 0, &testInstruction);
	//cout<<"RawFunction: "<<testInstruction.rawFunction<<endl;
	InstructionDecode(&testInstruction);
	InstructionExecute(&testInstruction);
	InstructionWriteBack(&testInstruction);
	cout<<"Function: "<<testInstruction.func<<endl;
	cout<<"Rs: "<<testInstruction.rs.number<<endl;
	cout<<"Rs value: "<<testInstruction.rs.value<<endl;
	cout<<"Rt: "<<testInstruction.rt.number<<endl;
	cout<<"Rt value: "<<testInstruction.rt.value<<endl;
	cout<<"Rd: "<<testInstruction.rd.number<<endl;
	cout<<"Rd value: "<<RegisterArray[7].value<<endl;
	system("PAUSE");
	return(0);
}


void InstructionFetch(int instructionMemory[], int PC, INSTRUCTION *currInstruction){
	//cout<<"test: "<<instructionMemory[PC]<<endl;
	currInstruction->rawFunction = instructionMemory[PC];
	//cout<<"test2: "<<currInstruction->rawFunction<<endl;
}

void InstructionDecode(INSTRUCTION *currInstruction){
	//Get the opcode--always the first 4 bits
	//But it just so happens that our ENUM perfectly matches the 
	//opcode number..I THINK
	int temp;
	temp = (currInstruction->rawFunction) & 0xF000;	//This line is ONLY included to make things more readable. It can be removed later (by placing it on the line below)
	currInstruction->func = static_cast<OPCODE>(temp);

	//Need to figure out if the instruction is R-, I-, or J-type
	if(currInstruction->func == ADD || (currInstruction->func == SUB || (currInstruction->func == SLL || (currInstruction->func == SRL || (currInstruction->func == NOR || (currInstruction->func == OR || (currInstruction->func == XOR))))))){
		//Its an R-type
		currInstruction->rs.number = ((currInstruction->rawFunction) & 0x0F00)>>8;
		cout<<"*"<<currInstruction->rs.number<<endl;
		currInstruction->rt.number = ((currInstruction->rawFunction) & 0x00F0)>>4;
		currInstruction->rd.number = (currInstruction->rawFunction) & 0x000F;
		currInstruction->type = R_TYPE;
	}
	else if(currInstruction->func == ADDI || (currInstruction->func == ORI || (currInstruction->func == LW || (currInstruction->func == SW || (currInstruction->func == BEQ || (currInstruction->func == BLTZ || (currInstruction->func == BLT))))))){
		//Its an I-type
		currInstruction->rs.number = (currInstruction->rawFunction) & 0x0F00;
		currInstruction->rt.number = (currInstruction->rawFunction) & 0x0F00;
		currInstruction->immediate = (currInstruction->rawFunction) & 0x000F;
		currInstruction->type = I_TYPE;
	}
	else if(currInstruction->func == J){
		//Its an J-type
		currInstruction->immediate = (currInstruction->rawFunction) & 0x03FF;
		currInstruction->type = J_TYPE;
	}
}

void InstructionExecute(INSTRUCTION *currInstruction){
	switch(currInstruction->type){
		case R_TYPE:
			InstructionExecute_R(currInstruction);
			break;
		case I_TYPE:
			InstructionExecute_I(currInstruction);
			break;
		case J_TYPE:
			InstructionExecute_J(currInstruction);
			break;
	}
}

void InstructionExecute_R(INSTRUCTION *currInstruction){
	switch(currInstruction->func){
		case ADD:
			currInstruction->tempMem = RegisterArray[currInstruction->rs.number].value + RegisterArray[currInstruction->rt.number].value;
			//cout<<RegisterArray[currInstruction->rs.number].value<<endl;
			//cout<<RegisterArray[currInstruction->rt.number].value<<endl;
			//cout<<"**"<<currInstruction->tempMem<<endl;
			break;
		case SUB:
			currInstruction->tempMem = RegisterArray[currInstruction->rs.number].value - RegisterArray[currInstruction->rt.number].value;
			break;
		case SLL:
			currInstruction->tempMem = RegisterArray[currInstruction->rs.number].value << RegisterArray[currInstruction->rt.number].value;
			break;
		case SRL:
			currInstruction->tempMem	= RegisterArray[currInstruction->rs.number].value >> RegisterArray[currInstruction->rt.number].value;
			break;
		case AND:
			currInstruction->tempMem = RegisterArray[currInstruction->rs.number].value & RegisterArray[currInstruction->rt.number].value;
			break;
		case NOR:
			currInstruction->tempMem = ~(RegisterArray[currInstruction->rs.number].value | RegisterArray[currInstruction->rt.number].value;
			break;
		case OR:
			currInstruction->tempMem = RegisterArray[currInstruction->rs.number].value | RegisterArray[currInstruction->rt.number].value;
			break;
		case XOR:
			currInstruction->tempMem = RegisterArray[currInstruction->rs.number].value ^ RegisterArray[currInstruction->rt.number].value;
			break;
	}
}

void InstructionExecute_I(INSTRUCTION *currInstruction){
	switch(currInstruction->func){
		case ADDI:
			currInstruction->tempMem = RegisterArray[currInstruction->rs.number].value + currInstruction->immediate;
			break;
		case ORI:
			currInstruction->tempMem = RegisterArray[currInstruction->rs.number].value - currInstruction->immediate;
			break;
		case LW:
			
			break;
		case SW:
			
			break;
		case BEQ:
			if(RegisterArray[currInstruction->rs.number].value == RegisterArray[currInstruction->rt.number].value){
				//need to change PC here
			}
			break;
		case BLTZ:
			if(RegisterArray[currInstruction->rs.number].value < zero.value){
				//need to change PC here
			}
			break;
		case BLT:
			if(RegisterArray[currInstruction->rs.number].value < RegisterArray[currInstruction->rt.number].value){
				//need to change PC here
			}
			break;
	}
}

void InstructionExecute_J(INSTRUCTION *currInstruction){
	//This will only be a jump instruction
	//need to change PC here
}

void InstructionMem(INSTRUCTION *currInstruction){
	if(currInstruction->type == R_TYPE || (currInstruction->type == I_TYPE)){
		//do nothing
	}
}

void InstructionWriteBack(INSTRUCTION *currInstruction){
	if(currInstruction->type == R_TYPE || (currInstruction->type == I_TYPE)){
		currInstruction->rd.value = currInstruction->tempMem;	//<--I don't know if this line is necessary anymore
		RegisterArray[currInstruction->rd.number].value = currInstruction->tempMem;
	}
}

void RegisterInit(INSTRUCTION *currInstruction){
	
	RegisterArray[0].number = 0x00;
	RegisterArray[0].value = 0x00;

	RegisterArray[1].number = 0x01;
	RegisterArray[1].value = 0x00;

	RegisterArray[2].number = 0x02;
	RegisterArray[2].value = 0x00;

	RegisterArray[3].number = 0x03;
	RegisterArray[3].value = 0x00;

	RegisterArray[4].number = 0x04;
	RegisterArray[4].value = 0x00;

	RegisterArray[5].number = 0x05;
	RegisterArray[5].value = 0x00;

	RegisterArray[6].number = 0x06;
	RegisterArray[6].value = 0x00;

	RegisterArray[7].number = 0x07;
	RegisterArray[7].value = 0x00;

	RegisterArray[8].number = 0x08;
	RegisterArray[8].value = 0x01;

	RegisterArray[9].number = 0x09;
	RegisterArray[9].value = 0x02;

	RegisterArray[10].number = 0x0A;
	RegisterArray[10].value = 0x00;
}
//So Dr Sherif was saying that if you try to write to the zero register, nothing happens, it just doesn't write to the zero register. To emulate this behavior, I think we should just check
//the destination register in the WB stage, and if the destination register is the zero register, we just ignore the instruction and carry on. Thoughts? 