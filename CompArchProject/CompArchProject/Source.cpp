#include <iostream>
#include <fstream>
#include <string>
#include <typeinfo>
using namespace std;

#define INSTRUCTION_LEN 16
#define VERBOSE_OUTPUT true

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
	BLT,
	_NOP,
	NONE_OPCODE	//Used instead of 'NULL' because 'NULL' is interpreted as an int
};

typedef enum INSTRUCTION_TYPE{
	R_TYPE,
	I_TYPE,
	J_TYPE,
	NONE_TYPE	//Used instead of 'NULL' because 'NULL' is interpreted as an int
};

typedef enum REGISTER_ENUM{
	zero_reg,
	a0_reg,
	a1_reg,
	v0_reg,
	v1_reg,
	v2_reg,
	v3_reg,
	t0_reg,
	t1_reg,
	t2_reg,
	t3_reg
};
/*
typedef enum BUFFER{
	FETCH_DECODE,
	DECODE_EXECUTE,
	EXECUTE_MEMORY,
	MEMORY_WRITEBACK
};
*/
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
void PrintRegisters();
void RegisterInit(INSTRUCTION*);
void ExecuteProgram(int[]);
void PrintFullInstruction(INSTRUCTION*);
bool CheckForDataHazard(INSTRUCTION, INSTRUCTION, INSTRUCTION, INSTRUCTION, bool);

string RegisterNumberToName(int);

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
	while(programFile.get() != -1){
		instructionCount++;
	}
	programFile.close();

	//Dynamically allocate memory for array
	instructionMemory = new int[instructionCount+1];
	//Re-open file

	programFile.open("program.txt", ios::in);
	while(!programFile.eof()){
		programFile.read(ptr, INSTRUCTION_LEN);
		instructionMemory[i] = strtol(ptr, &nptr, 2);
		i++;
	}
	instructionMemory[i+1] = NULL;
	RegisterInit(&testInstruction);
	ExecuteProgram(instructionMemory);
	PrintRegisters();
	system("PAUSE");
	return(0);
}

void ExecuteProgram(int instructionMemory[]){
	int PC = 0;
	int CLK = 0;
	bool lastInstructionFetched = false;
	bool DataHazard = false;
	int continueLoop = 0;
	INSTRUCTION FETCH_DECODE;
	INSTRUCTION DECODE_EXECUTE;
	INSTRUCTION EXECUTE_MEMORY;
	INSTRUCTION MEMORY_WRITEBACK;
	INSTRUCTION	NOP;
	INSTRUCTION TEMP;
	NOP.func = _NOP;
	NOP.type = NONE_TYPE;
	NOP.rawFunction = NULL;
	do{
		if(CLK == 0){
			//Don't ever need to worry about data hazards here
			InstructionFetch(instructionMemory, PC, &FETCH_DECODE);
			//system("PAUSE");
		}else if(CLK == 1){
			//Don't *think* we ever need to worry about data hazards here
			DECODE_EXECUTE = FETCH_DECODE;

			InstructionFetch(instructionMemory, PC, &FETCH_DECODE);
			InstructionDecode(&DECODE_EXECUTE);
			//system("PAUSE");
		}else if(CLK == 2){
			//I don't think we can have a data hazard here because there can't be enough instructions in the pipeline yet
			EXECUTE_MEMORY = DECODE_EXECUTE;
			DECODE_EXECUTE = FETCH_DECODE;

			InstructionFetch(instructionMemory, PC, &FETCH_DECODE);
			InstructionDecode(&DECODE_EXECUTE);
			InstructionExecute(&EXECUTE_MEMORY);
			//system("PAUSE");
		}else if(CLK >= 3){
			MEMORY_WRITEBACK = EXECUTE_MEMORY;
			if(CheckForDataHazard(FETCH_DECODE, DECODE_EXECUTE, EXECUTE_MEMORY, MEMORY_WRITEBACK, false)){
				cout<<"*******"<<endl;
				PrintFullInstruction(&EXECUTE_MEMORY);
				PrintFullInstruction(&DECODE_EXECUTE);
				TEMP = DECODE_EXECUTE;	//Store the instruction in a temp
				EXECUTE_MEMORY = NOP;
				system("PAUSE");
			}else{
				EXECUTE_MEMORY = DECODE_EXECUTE;
			}
			DECODE_EXECUTE = FETCH_DECODE;

			InstructionFetch(instructionMemory, PC, &FETCH_DECODE);
			InstructionDecode(&DECODE_EXECUTE);
			InstructionExecute(&EXECUTE_MEMORY);
			InstructionMem(&MEMORY_WRITEBACK);
			InstructionWriteBack(&MEMORY_WRITEBACK);
			//system("PAUSE");
		}
		CLK++;
		if(instructionMemory[PC+1] == NULL){	//Without forwarding, this passes when all instructions have been fetched. Then we need 3 more cycles to clear the rest of the instructions
			lastInstructionFetched = true;
			continueLoop++;
		}else{
			PC++;
		}
	}while(continueLoop != 3);
}
void InstructionFetch(int instructionMemory[], int PC, INSTRUCTION *currInstruction){
	try{
		currInstruction->rawFunction = instructionMemory[PC];
	}catch(out_of_range){
		currInstruction->rawFunction = NULL;
	}
	if(VERBOSE_OUTPUT){
		cout<<"----------FETCH----------"<<endl;
		cout<<"Instruction fetched: "<<currInstruction->rawFunction<<endl;
		cout<<"-------------------------\n\n"<<endl;
	}
}

void InstructionDecode(INSTRUCTION *currInstruction){
	//Get the opcode--always the first 4 bits
	//But it just so happens that our ENUM perfectly matches the 
	//opcode number..I THINK
	if(VERBOSE_OUTPUT){
		cout<<"----------DECODE----------"<<endl;
		cout<<"Instruction: "<<currInstruction->rawFunction<<endl;
	}
	if(currInstruction->rawFunction != NULL){
		int temp;
		temp = ((currInstruction->rawFunction) & 0xF000)>>0x0C;	//This line is ONLY included to make things more readable. It can be removed later (by placing it on the line below)
		currInstruction->func = static_cast<OPCODE>(temp);
		if(VERBOSE_OUTPUT)
			cout<<"Function: "<<currInstruction->func<<endl;
		//Need to figure out if the instruction is R-, I-, or J-type
		if(currInstruction->func == ADD || (currInstruction->func == SUB || (currInstruction->func == SLL || (currInstruction->func == SRL || (currInstruction->func == AND | (currInstruction->func == NOR || (currInstruction->func == OR || (currInstruction->func == XOR)))))))){
			//Its an R-type
			currInstruction->rs.number = ((currInstruction->rawFunction) & 0x0F00)>>8;
			currInstruction->rt.number = ((currInstruction->rawFunction) & 0x00F0)>>4;
			currInstruction->rd.number = (currInstruction->rawFunction) & 0x000F;
			currInstruction->type = R_TYPE;
			if(VERBOSE_OUTPUT){
				cout<<"R-type instruction"<<endl;
				cout<<"rs: "<<RegisterNumberToName(currInstruction->rs.number)<<"("<<currInstruction->rs.number<<")"<<endl;
				cout<<"rt: "<<RegisterNumberToName(currInstruction->rt.number)<<"("<<currInstruction->rt.number<<")"<<endl;
				cout<<"rd: "<<RegisterNumberToName(currInstruction->rd.number)<<"("<<currInstruction->rd.number<<")"<<endl;
			}
		}
		else if(currInstruction->func == ADDI || (currInstruction->func == ORI || (currInstruction->func == LW || (currInstruction->func == SW || (currInstruction->func == BEQ || (currInstruction->func == BLTZ || (currInstruction->func == BLT))))))){
			//Its an I-type
			currInstruction->rs.number = ((currInstruction->rawFunction) & 0x0F00)>>8;
			currInstruction->rt.number = ((currInstruction->rawFunction) & 0x00F0)>>4;
			currInstruction->immediate = (currInstruction->rawFunction) & 0x000F;
			currInstruction->type = I_TYPE;
			if(VERBOSE_OUTPUT){
				cout<<"I-type instruction"<<endl;
				cout<<"rs: "<<RegisterNumberToName(currInstruction->rs.number)<<"("<<currInstruction->rs.number<<")"<<endl;
				cout<<"rt: "<<RegisterNumberToName(currInstruction->rt.number)<<"("<<currInstruction->rt.number<<")"<<endl;
				cout<<"Immediate: "<<currInstruction->immediate<<endl;
			}
		}
		else if(currInstruction->func == J){
			//Its an J-type
			currInstruction->immediate = (currInstruction->rawFunction) & 0x03FF;
			currInstruction->type = J_TYPE;
			if(VERBOSE_OUTPUT){
				cout<<"J-type instruction"<<endl;
				cout<<"Immediate: "<<currInstruction->immediate<<endl;
			}
		}
	}else{
		currInstruction->func = NONE_OPCODE;
	}
	if(VERBOSE_OUTPUT)
		cout<<"--------------------------\n\n"<<endl;
}

void InstructionExecute(INSTRUCTION *currInstruction){
	if(VERBOSE_OUTPUT){
		cout<<"----------EXECUTE----------"<<endl;
		cout<<"Instruction: "<<currInstruction->rawFunction<<endl;
	}
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
		case NONE_TYPE:
			currInstruction->tempMem = NULL;
			break;
	}
}

void InstructionExecute_R(INSTRUCTION *currInstruction){
	switch(currInstruction->func){
		case ADD:
			if(VERBOSE_OUTPUT)
			cout<<"ADD";
			currInstruction->tempMem = RegisterArray[currInstruction->rs.number].value + RegisterArray[currInstruction->rt.number].value;
			break;
		case SUB:
			if(VERBOSE_OUTPUT)
			cout<<"SUB";
			currInstruction->tempMem = RegisterArray[currInstruction->rs.number].value - RegisterArray[currInstruction->rt.number].value;
			break;
		case SLL:
			if(VERBOSE_OUTPUT)
			cout<<"SLL";
			currInstruction->tempMem = RegisterArray[currInstruction->rs.number].value << RegisterArray[currInstruction->rt.number].value;
			break;
		case SRL:
			if(VERBOSE_OUTPUT)
			cout<<"SRL";
			currInstruction->tempMem	= RegisterArray[currInstruction->rs.number].value >> RegisterArray[currInstruction->rt.number].value;
			break;
		case AND:
			if(VERBOSE_OUTPUT)
			cout<<"AND";
			currInstruction->tempMem = RegisterArray[currInstruction->rs.number].value & RegisterArray[currInstruction->rt.number].value;
			break;
		case NOR:
			if(VERBOSE_OUTPUT)
			cout<<"NOR";
			currInstruction->tempMem = ~(RegisterArray[currInstruction->rs.number].value | RegisterArray[currInstruction->rt.number].value);
			break;
		case OR:
			if(VERBOSE_OUTPUT)
			cout<<"OR";
			currInstruction->tempMem = RegisterArray[currInstruction->rs.number].value | RegisterArray[currInstruction->rt.number].value;
			break;
		case XOR:
			if(VERBOSE_OUTPUT)
			cout<<"XOR";
			currInstruction->tempMem = RegisterArray[currInstruction->rs.number].value ^ RegisterArray[currInstruction->rt.number].value;
			break;
	}
	if(VERBOSE_OUTPUT){
		cout<<" $"<<RegisterNumberToName(currInstruction->rd.number)<<"("<<RegisterArray[currInstruction->rd.number].value<<")"<<", $"<<RegisterNumberToName(currInstruction->rs.number)<<"("<<RegisterArray[currInstruction->rs.number].value<<")"<<", $"<<RegisterNumberToName(currInstruction->rt.number)<<"("<<RegisterArray[currInstruction->rt.number].value<<")"<<endl;
		cout<<"---------------------------\n\n"<<endl;
	}
}

void InstructionExecute_I(INSTRUCTION *currInstruction){
	switch(currInstruction->func){
		case ADDI:
			if(VERBOSE_OUTPUT)
			cout<<"ADDI";
			currInstruction->tempMem = RegisterArray[currInstruction->rs.number].value + currInstruction->immediate;
			break;
		case ORI:
			if(VERBOSE_OUTPUT)
				cout<<"ORI";
			currInstruction->tempMem = RegisterArray[currInstruction->rs.number].value | currInstruction->immediate;
			break;
		case LW:
			if(VERBOSE_OUTPUT)
				cout<<"LW";
			break;
		case SW:
			if(VERBOSE_OUTPUT)
				cout<<"SW";
			break;
		case BEQ:
			if(VERBOSE_OUTPUT)
				cout<<"BEQ";
			if(RegisterArray[currInstruction->rs.number].value == RegisterArray[currInstruction->rt.number].value){
				//need to change PC here
			}
			break;
		case BLTZ:
			if(VERBOSE_OUTPUT)
				cout<<"BLTZ";
			if(RegisterArray[currInstruction->rs.number].value < zero.value){
				//need to change PC here
			}
			break;
		case BLT:
			if(VERBOSE_OUTPUT)
				cout<<"BLT";
			if(RegisterArray[currInstruction->rs.number].value < RegisterArray[currInstruction->rt.number].value){
				//need to change PC here
			}
			break;
	}
	if(VERBOSE_OUTPUT)
		cout<<" $"<<RegisterNumberToName(currInstruction->rt.number)<<"("<<RegisterArray[currInstruction->rt.number].value<<")"<<", $"<<RegisterNumberToName(currInstruction->rs.number)<<"("<<RegisterArray[currInstruction->rs.number].value<<")"<<", "<<currInstruction->immediate<<endl;
		cout<<"---------------------------\n\n"<<endl;
}

void InstructionExecute_J(INSTRUCTION *currInstruction){
	//This will only be a jump instruction
	//need to change PC here
}

void InstructionMem(INSTRUCTION *currInstruction){
	if(VERBOSE_OUTPUT){
		cout<<"----------MEM----------"<<endl;
		cout<<"Instruction: "<<currInstruction->rawFunction<<endl;
	}
	if(currInstruction->type == R_TYPE || (currInstruction->type == I_TYPE || (currInstruction->type == NONE_TYPE))){
		if(VERBOSE_OUTPUT)
			cout<<"Not used in this instruction"<<endl;
	}
	if(VERBOSE_OUTPUT)
		cout<<"-----------------------\n\n"<<endl;
}

void InstructionWriteBack(INSTRUCTION *currInstruction){
	if(VERBOSE_OUTPUT){
		cout<<"----------WRITE BACK----------"<<endl;
		cout<<"Instruction: "<<currInstruction->rawFunction<<endl;
	}
	if(currInstruction->type == R_TYPE ){
		currInstruction->rd.value = currInstruction->tempMem;	//<--I don't know if this line is necessary anymore
		RegisterArray[currInstruction->rd.number].value = currInstruction->tempMem;
		if(VERBOSE_OUTPUT)
			cout<<"Rd"<<"($"<<RegisterNumberToName(currInstruction->rd.number)<<"): "<<RegisterArray[currInstruction->rd.number].value<<endl;
	}else if(currInstruction->type == I_TYPE){
		currInstruction->rt.value = currInstruction->tempMem;
		RegisterArray[currInstruction->rt.number].value = currInstruction->tempMem;
		if(VERBOSE_OUTPUT)
			cout<<"Rt"<<"($"<<RegisterNumberToName(currInstruction->rt.number)<<"): "<<RegisterArray[currInstruction->rt.number].value<<endl;
	}else if(currInstruction->type == NONE_TYPE){
		if(VERBOSE_OUTPUT)
			cout<<"NOP Instruction"<<endl;
	}
	if(VERBOSE_OUTPUT)
		cout<<"------------------------------\n\n"<<endl;
	
}

bool CheckForDataHazard(INSTRUCTION FETCH_DECODE, INSTRUCTION DECODE_EXECUTE, INSTRUCTION EXECUTE_MEMORY, INSTRUCTION MEMORY_WRITEBACK, bool FORWARDING){
	if(EXECUTE_MEMORY.rd.value == DECODE_EXECUTE.rs.value || (EXECUTE_MEMORY.rd.value == DECODE_EXECUTE.rt.value)){		//The rd of previous instruction is the rt/rs of the next instruction
		if(FORWARDING){
		}else{
			//Insert a NOP
			return(true);
		}
	}else{
		return(false);
	}
}

void RegisterInit(INSTRUCTION *currInstruction){
	
	RegisterArray[zero_reg].number = 0x00;
	RegisterArray[zero_reg].value = 0x00;

	RegisterArray[a0_reg].number = 0x01;
	RegisterArray[a0_reg].value = 0x00;

	RegisterArray[a1_reg].number = 0x02;
	RegisterArray[a1_reg].value = 0x00;

	RegisterArray[v0_reg].number = 0x03;
	RegisterArray[v0_reg].value = 0x00;

	RegisterArray[v1_reg].number = 0x04;
	RegisterArray[v1_reg].value = 0x00;

	RegisterArray[v2_reg].number = 0x05;
	RegisterArray[v2_reg].value = 0x00;

	RegisterArray[v3_reg].number = 0x06;
	RegisterArray[v3_reg].value = 0x00;

	RegisterArray[t0_reg].number = 0x07;
	RegisterArray[t0_reg].value = 0x00;

	RegisterArray[t1_reg].number = 0x08;
	RegisterArray[t1_reg].value = 0x00;

	RegisterArray[t2_reg].number = 0x09;
	RegisterArray[t2_reg].value = 0x00;

	RegisterArray[t3_reg].number = 0x0A;
	RegisterArray[t3_reg].value = 0x00;
}

//So Dr Sherif was saying that if you try to write to the zero register, nothing happens, it just doesn't write to the zero register. To emulate this behavior, I think we should just check
//the destination register in the WB stage, and if the destination register is the zero register, we just ignore the instruction and carry on. Thoughts? 

void PrintRegisters(){
	cout<<"Zero"<<endl;
	cout<<"Address: "<<RegisterArray[zero_reg].number<<endl;
	cout<<"Value: "<<RegisterArray[zero_reg].value<<"\n"<<endl;
	cout<<"a0"<<endl;
	cout<<"Address: "<<RegisterArray[a0_reg].number<<endl;
	cout<<"Value: "<<RegisterArray[a0_reg].value<<"\n"<<endl;
	cout<<"a1"<<endl;
	cout<<"Address: "<<RegisterArray[a1_reg].number<<endl;
	cout<<"Value: "<<RegisterArray[a1_reg].value<<"\n"<<endl;
	cout<<"v0"<<endl;
	cout<<"Address: "<<RegisterArray[v0_reg].number<<endl;
	cout<<"Value: "<<RegisterArray[v0_reg].value<<"\n"<<endl;
	cout<<"v1"<<endl;
	cout<<"Address: "<<RegisterArray[v1_reg].number<<endl;
	cout<<"Value: "<<RegisterArray[v1_reg].value<<"\n"<<endl;
	cout<<"v2"<<endl;
	cout<<"Address: "<<RegisterArray[v2_reg].number<<endl;
	cout<<"Value: "<<RegisterArray[v2_reg].value<<"\n"<<endl;
	cout<<"v3"<<endl;
	cout<<"Address: "<<RegisterArray[v3_reg].number<<endl;
	cout<<"Value: "<<RegisterArray[v3_reg].value<<"\n"<<endl;
	cout<<"t0"<<endl;
	cout<<"Address: "<<RegisterArray[t0_reg].number<<endl;
	cout<<"Value: "<<RegisterArray[t0_reg].value<<"\n"<<endl;
	cout<<"t1"<<endl;
	cout<<"Address: "<<RegisterArray[t1_reg].number<<endl;
	cout<<"Value: "<<RegisterArray[t1_reg].value<<"\n"<<endl;
	cout<<"t2"<<endl;
	cout<<"Address: "<<RegisterArray[t2_reg].number<<endl;
	cout<<"Value: "<<RegisterArray[t2_reg].value<<"\n"<<endl;
	cout<<"t3"<<endl;
	cout<<"Address: "<<RegisterArray[t3_reg].number<<endl;
	cout<<"Value: "<<RegisterArray[t3_reg].value<<"\n"<<endl;
	
}

string RegisterNumberToName(int regNum){
	switch(regNum){
	case 0:
		return("zero");
		break;
	case 1:
		return("a0");
		break;
	case 2:
		return("a1");
		break;
	case 3:
		return("v0");
		break;
	case 4:
		return("v1");
		break;
	case 5:
		return("v2");
		break;
	case 6:
		return("v3");
		break;
	case 7:
		return("t0");
		break;
	case 8:
		return("t1");
		break;
	case 9:
		return("t2");
		break;
	case 10:
		return("t3");
		break;
	default:
		return("Invalid register number!");
		break;
	}
}

void PrintFullInstruction(INSTRUCTION *currInstruction){
	switch(currInstruction->type){
	case R_TYPE:
		switch(currInstruction->func){
		case ADD:
			cout<<"ADD";
			break;
		case SUB:
			cout<<"SUB";
			break;
		case SLL:
			cout<<"SLL";
			break;
		case SRL:
			cout<<"SRL";
			break;
		case AND:
			cout<<"AND";
			break;
		case NOR:
			cout<<"NOR";
			break;
		case OR:
			cout<<"OR";
			break;
		case XOR:
			cout<<"XOR";
			break;
		}
		cout<<" $"<<RegisterNumberToName(currInstruction->rt.number)<<"("<<RegisterArray[currInstruction->rt.number].value<<")"<<", $"<<RegisterNumberToName(currInstruction->rs.number)<<"("<<RegisterArray[currInstruction->rs.number].value<<")"<<", "<<currInstruction->immediate<<endl;
		break;
	case I_TYPE:
		switch(currInstruction->func){
		case ADDI:
			cout<<"ADDI";
			break;
		case ORI:
			cout<<"ORI";
			break;
		case LW:
			cout<<"LW";
			break;
		case SW:
			cout<<"SW";
			break;
		case BEQ:
			cout<<"BEQ";
			break;
		case BLTZ:
			cout<<"BLTZ";
			break;
		case BLT:
			cout<<"BLT";
			break;
		}
		cout<<" $"<<RegisterNumberToName(currInstruction->rt.number)<<"("<<RegisterArray[currInstruction->rt.number].value<<")"<<", $"<<RegisterNumberToName(currInstruction->rs.number)<<"("<<RegisterArray[currInstruction->rs.number].value<<")"<<", "<<currInstruction->immediate<<endl;
	}	
}