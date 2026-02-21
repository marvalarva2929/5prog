#pragma once
typedef struct Script Script;
typedef struct Entry Entry;
typedef struct Command Command;

#define null NULL
#include <stdio.h>
#include "labletable.h"
#include "argparse.h"

char * trim(char * totrim);

typedef enum CommandType {
	ADD,
	ADDI,
	SUB,
	SUBI,
	MUL,
	DIV,
	AND,
	OR,
	XOR,
	NOT,
	SHFTR,
	SHFTRI,
	SHFTL,
	SHFTLI,
	BR,
	BRR,
	BRNZ,
	CALL,
	RETURN,
	BRGT,
	PRIV,
	MOV,
	ADDF,
	SUBF,
	MULF,
	DIVF,
// MACROS
	IN,
	OUT,
	CLR,
	LD,
	PUSH,
	POP,
// data
	DATA,
	HALT,
} CommandType;

struct Command {
	char * args;
	int argl;
	CommandType type;
};

struct Entry {
	unsigned long long value;
	int address;
	int size;
	int type;
	
	int numArgs;
	char * str;
	char * lbl;
	Command cmd;
};

struct Script {
	Entry * entries;
	ltable * ltable;
	int numEntries;
	int byteSize;
};

typedef struct {
    const char *name;
	CommandType type;
	int cnt;
	int opcode;
	int arglenth;
	int hasLit;
} CmdMap;

static CmdMap cmdTable[] = {
    {"add", ADD, 1,   0x18, 3, 0}, 
	{"addi", ADDI, 1, 0x19, 2, 1},
    {"sub", SUB, 1,   0x1a, 3, 0}, 
	{"subi", SUBI, 1, 0x1b, 2, 1},
    {"mul", MUL, 1, 0x1c, 3, 0}, 
	{"div", DIV, 1, 0x1d, 3, 0},
    {"and", AND, 1, 0x0, 3, 0}, 
	{"or", OR, 1, 0x1, 3, 0}, 
	{"xor", XOR, 1, 0x2, 3, 0}, 
	{"not", NOT, 1, 0x3, 2, 0},
    {"shftr", SHFTR, 1, 0x4, 3, 0}, 
	{"shftri", SHFTRI, 1, 0x5, 2, 1},
    {"shftl", SHFTL, 1, 0x6, 3, 0},
	{"shftli", SHFTLI, 1, 0x7, 2, 1},
    {"br", BR, 1, 0x8, 1, 0},
	{"brr", BRR, 1, 0x9, 1, 1}, 
	{"brnz", BRNZ, 1, 0xb, 2, 0}, 
    {"call", CALL, 1, 0xc, 1, 0},
	{"return", RETURN, 1, 0xd, 0, 0},
	{"brgt", BRGT, 1, 0xe, 3, 0},
    {"priv", PRIV, 1, 0xf, 4, 1},
    {"mov", MOV, 1, 0x10, 3, 1},
    {"addf", ADDF, 1, 0x14, 3, 0}, 
	{"subf", SUBF, 1, 0x15, 3, 0},
    {"mulf", MULF, 1, 0x16, 3, 0}, 
	{"divf", DIVF, 1, 0x17, 3, 0},
    {"in", IN, 1, -1},
	{"out", OUT, 1, -1},
    {"clr", CLR, 1, -1}, 
	{"ld", LD, 12, -1},
    {"push", PUSH, 2, -1}, 
	{"pop", POP, 2, -1},
    {"data", DATA, 1, -1},
	{"halt", HALT, 1, -1}
};

Script * getScript(char * filename);
