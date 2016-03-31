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

typedef enum BUFFER{
	FETCH_DECODE,
	DECODE_EXECUTE,
	EXECUTE_MEMORY,
	MEMORY_WRITEBACK
};

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