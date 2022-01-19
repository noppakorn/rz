// as21.h   16 Jan 2007


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define PRIVATE   static

#define NOP 0
#define LD 1		// op code number
#define ST 3
#define MOV 5
#define JMP 6
#define JAL 7
#define JT  8
#define JF  9

#define ADD 10
#define SUB 11
#define MUL 12
#define DIV 13
#define AND 14
#define OR 15
#define XOR 16
#define EQ 17
#define NE 18
#define LT 19
#define LE 20
#define GT 21
#define GE 22
#define SHL 23
#define SHR 24
#define NOT 25
#define MOD 26

#define RET 30		// xop, unique number
#define TRAP 31
#define PUSH 32
#define POP 33
#define INT 34
#define RETI 35
#define PUSHM 36
#define POPM 37
#define XCH 38

#define ABS 50		// addressing modes
#define IND 51
#define INX 52
#define IMM 53		// for two arg
#define REG 54
#define RR 55
#define RI 56
#define SP 57
#define TR 58       // trap
#define NA 59

#define SYM 60		// type of symbol
#define NUM 61
#define OP 62
#define DOTS 63
#define DOTC 64
#define DOTD 65
#define DOTE 66

#define MEM 70		// check arg, mem: ABS,IND,INX
#define MRI 71		// mov: IMM, REG
#define RRI	72		// RI, RR

#define UNDEF 0x80000000	// max neg int, denote undef in sym tab
#define	MXBUF	200			// max input line length
#define MXMEM	5000		// max no of token

#define eqs(a,b)  strcmp((a),(b)) == 0

typedef struct { char type; char mode; int ref; int line; } token;

// hash symbol table functions from hash.c
int searchsym(char *s);
int putsym(char *s);
//int getsym(char *s,int *type);
void putvalue(int idx, int value);
void error(char *s);
int getValue(int idx);
int getArg(int idx);
int getType(int idx);
void setsym(int idx, int value, int type, int arg);
