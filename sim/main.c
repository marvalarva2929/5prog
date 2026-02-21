#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "main.h"
#define MEM_SIZE 524288
#define ull unsigned long long
#define ll long long

unsigned char mem[MEM_SIZE];
ull r[32] = {0};
int pc = 0x2000;
int halt = 0;

void simErr() {
	fprintf(stderr, "Simulation error\n");
	exit(1);
}

int verifyAddress(ull add) {
	if (add < 0 || add >= MEM_SIZE) simErr();
	return add;
}

int verifyRegister(int r) {
	if (r < 0 || r > 31) simErr();
	return r;
}

void sign(int *imm) {
    if (*imm & 0x800) {  // Check if bit 11 is set
        *imm |= 0xFFFFF000;  // Sign-extend to 32 bits
    }
}

long long readMem(ull add, int size) { 
    verifyAddress(add);
    verifyAddress(add + size - 1);
    long long ret = 0;
    for (int i = 0; i < size; i++) {
		ret <<= 8;
        ret += (((long long)(mem[add+size-1-i])));
    }
    return ret;
}

void loadMem(ull add, long long v, int size) { 
    verifyAddress(add);
    verifyAddress(add + size - 1);
    for (int i = 0; i < size; i++) {
		//printf("%d 0x%x\n", i, (v >> (8 * i)) & 0xFF);
        mem[add + i] = (v >> (8 * i)) & 0xFF;
    }
}

double fcast(ull* l) {
	double db;
	memcpy(&db, l, sizeof(double));
	return db;
}

ull dcast(double *db) {
	ull l;
	memcpy(&l, db, sizeof(ull));
	return l;
}

void doAND(int rd, int rs, int rt, int imm) {
	r[rd] = r[rs] & r[rt];
}
void doOR(int rd, int rs, int rt, int imm) {
	r[rd] = r[rs] | r[rt];
}
void doXOR(int rd, int rs, int rt, int imm) {
	r[rd] = r[rs] ^ r[rt];
}

void doNOT(int rd, int rs, int rt, int imm) {
	r[rd] = ~r[rs];
}

void doSHFTR(int rd, int rs, int rt, int imm) {
	r[rd] = r[rs] >> r[rt];
}

void doSHFTRI(int rd, int rs, int rt, int imm) {
	r[rd] >>= imm;
}

void doSHFTL(int rd, int rs, int rt, int imm) {
	r[rd] = r[rs] << r[rt];
}

void doSHFTLI(int rd, int rs, int rt, int imm) {
	r[rd] <<= imm;
}

void doBR(int rd, int rs, int rt, int imm) {
	pc = verifyAddress(r[rd]);
}

void doBRR(int rd, int rs, int rt, int imm) {
	pc += r[rd];
}

void doBRR2(int rd, int rs, int rt, int imm) {
	sign(&imm);
	pc += imm;
}

void doBRNZ(int rd, int rs, int rt, int imm) {
	if (r[rs] != 0) pc = verifyAddress(r[rd]);
	else pc += 4;
}

void doCALL(int rd, int rs, int rt, int imm) {
	loadMem(r[31]-8, pc + 4, 8);
	pc = verifyAddress(r[rd]);
}

void doRETURN(int rd, int rs, int rt, int imm) {
	pc = readMem(r[31]-8, 8);
}

void doBRGT(int rd, int rs, int rt, int imm) {
	if (r[rs] > r[rt]) pc = verifyAddress(r[rd]);
	else pc += 4;
}

void doPRIV(int rd, int rs, int rt, int imm) {
	if (imm == 0x0) { // halt
		halt = 1;
	} else if (imm == 0x3 && r[rs] == 0) { // input
										   //
		char buf[100];
		if (!fgets(buf, sizeof(buf), stdin)) 
			simErr();

		char *endptr;
		errno = 0;
		ull value = strtoull(buf, &endptr, 10);

		// 1. Check for valid range
		if (buf[0] == '-' || errno == ERANGE) {
			simErr();
		}

		// 2. Check that entire string was valid number
		if (*endptr != '\n' && *endptr != '\0') {
			simErr();
		}

		r[rd] = value;
	} else if (imm == 0x4 && r[rd] == 1) {
		printf("%llu\n", r[rs]);
	} else if (imm == 0x4 && r[rd] == 3) {
		printf("%c", (char)r[rs]);
	} else {
		simErr();
	}
}

void doMOV(int rd, int rs, int rt, int imm) {
	sign(&imm);
	r[rd] = readMem(r[rs]+imm, 8);
}

void doMOV1(int rd, int rs, int rt, int imm) {
	sign(&imm);
	r[rd] = r[rs];
}

void doMOV2(int rd, int rs, int rt, int imm) {
	r[rd] >>= 12; r[rd] <<= 12;
	r[rd] += (0xFFF & imm);
}

void doMOV3(int rd, int rs, int rt, int imm) {
	sign(&imm);
	loadMem(r[rd]+imm, r[rs], 8);
}

void doADDF(int rd, int rs, int rt, int imm) {
	double sum = fcast(&r[rs]) + fcast(&r[rt]);
	r[rd] = dcast(&sum);
}

void doSUBF(int rd, int rs, int rt, int imm) {
	double diff = fcast(&r[rs]) - fcast(&r[rt]);
	r[rd] = dcast(&diff);
}

void doMULF(int rd, int rs, int rt, int imm) {
	double prod = fcast(&r[rs]) * fcast(&r[rt]);
	r[rd] = dcast(&prod);

}

void doDIVF(int rd, int rs, int rt, int imm) {
	double div = fcast(&r[rt]);
	if (div == 0.0) 
		simErr();

	double quo = fcast(&r[rs]) / div;
	r[rd] = dcast(&quo);

}

void doADD(int rd, int rs, int rt, int imm) {
	r[rd] = r[rs] + r[rt];
}

void doADDI(int rd, int rs, int rt, int imm) {
	r[rd] += imm;
}

void doSUB(int rd, int rs, int rt, int imm) {
	r[rd] = r[rs] - r[rt];
}

void doSUBI(int rd, int rs, int rt, int imm) {
	r[rd] -= imm;
}

void doMUL(int rd, int rs, int rt, int imm) {
	r[rd] = r[rs] * r[rt];
}

void doDIV(int rd, int rs, int rt, int imm) {
	if (r[rt] == 0) 	
		simErr();

	r[rd] = r[rs] / r[rt];
}

void parse(int i, int *opcode, int *rd, int *rs, int *rt, int *imm) {
	int j= i;
	*imm = i & 0xFFF; i >>= 12;


	*rt = i & 0x1F; i >>= 5;
	*rs = i & 0x1F; i >>= 5;
	*rd = i & 0x1F; i >>= 5;
	*opcode = i & 0x1F; i >>= 5;

	verifyRegister(*rt);
	verifyRegister(*rs);
	verifyRegister(*rd);
	//if (*rd == 0) printf("waa 0x%x\n", j), simErr();
}

CommandType getCmd(int opcode) {
	for (int i = 0; i < 30; i++)
		if (cmdTable[i].opcode == opcode) 
			return cmdTable[i].type;
	simErr();
	return -1;
}



int main(int argc, char * argv[]) {
	FILE * file;
	if (argc < 2 || (file = fopen(argv[1], "rb")) == NULL) {
		fprintf(stderr, "Invalid tinker filepath\n");
		exit(1);
	}

	ll header[5];
	int c = fread(header, sizeof(ull), 5, file);

	pc = header[1];
	int cnt = fread(mem+pc, 1, header[2], file); // read code
	cnt = fread(mem+header[3], 1, header[4], file); // read mem
													
	fclose(file);

	r[31] = MEM_SIZE;
	
	int i = 0;
	while (!halt) {
		// 1: extract bits for next command
		// 2: extract opcode, operands, and literals
		// 3: process command and update register values
		// 4: continue until halt
	
		// 1
		int read = readMem(pc, 4);
		int opcode, rd, rs, rt, imm;
		parse(read, &opcode, &rd, &rs, &rt, &imm);
		CommandType cmd = getCmd(opcode);
		//printf("%s 0x%x\n", cmdTable[cmd].name, read);

		//loadMem(0, 0x123456, 8);
		//int x = readMem(0, 8);
		//printf("0x%x\n", x);

		switch (cmd) {
			case AND   : doAND   (rd, rs, rt, imm); pc += 4; break;
			case OR    : doOR    (rd, rs, rt, imm); pc += 4; break;
			case XOR   : doXOR   (rd, rs, rt, imm); pc += 4; break;
			case NOT   : doNOT   (rd, rs, rt, imm); pc += 4; break;
			case SHFTR : doSHFTR (rd, rs, rt, imm); pc += 4; break;
			case SHFTRI: doSHFTRI(rd, rs, rt, imm); pc += 4; break;
			case SHFTL : doSHFTL (rd, rs, rt, imm); pc += 4; break;
			case SHFTLI: doSHFTLI(rd, rs, rt, imm); pc += 4; break;
			case BR    : doBR    (rd, rs, rt, imm); pc += 0; break;
			case BRR   : doBRR   (rd, rs, rt, imm); pc += 0; break;
			case BRR2  : doBRR2  (rd, rs, rt, imm); pc += 0; break;
			case BRNZ  : doBRNZ  (rd, rs, rt, imm); pc += 0; break;
			case CALL  : doCALL  (rd, rs, rt, imm); pc += 0; break;
			case RETURN: doRETURN(rd, rs, rt, imm); pc += 0; break;
			case BRGT  : doBRGT  (rd, rs, rt, imm); pc += 0; break;
			case PRIV  : doPRIV  (rd, rs, rt, imm); pc += 4; break;
			case MOV   : doMOV   (rd, rs, rt, imm); pc += 4; break;
			case MOV1  : doMOV1  (rd, rs, rt, imm); pc += 4; break;
			case MOV2  : doMOV2  (rd, rs, rt, imm); pc += 4; break;
			case MOV3  : doMOV3  (rd, rs, rt, imm); pc += 4; break;
			case ADDF  : doADDF  (rd, rs, rt, imm); pc += 4; break;
			case SUBF  : doSUBF  (rd, rs, rt, imm); pc += 4; break;
			case MULF  : doMULF  (rd, rs, rt, imm); pc += 4; break;
			case DIVF  : doDIVF  (rd, rs, rt, imm); pc += 4; break;
			case ADD   : doADD   (rd, rs, rt, imm); pc += 4; break;
			case ADDI  : doADDI  (rd, rs, rt, imm); pc += 4; break;
			case SUB   : doSUB   (rd, rs, rt, imm); pc += 4; break;
			case SUBI  : doSUBI  (rd, rs, rt, imm); pc += 4; break;
			case MUL   : doMUL   (rd, rs, rt, imm); pc += 4; break;
			case DIV   : doDIV   (rd, rs, rt, imm); pc += 4; break;
			default: 
			   simErr();
			   break;
		}

	}	
}
