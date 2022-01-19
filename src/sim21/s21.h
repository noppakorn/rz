// s21.h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define PRIVATE		static

#define NOP 0
#define LDA 1		// op code number
#define LDD 2
#define STA 3
#define STD 4
#define MVI 5
#define JMP 6
#define JAL 7
#define JT 8
#define JF 9
#define ADDI 10
#define SUBI 11
#define MULI 12
#define DIVI 13
#define ANDI 14
#define ORI 15
#define XORI 16
#define EQI 17
#define NEI 18
#define LTI 19
#define LEI 20
#define GTI 21
#define GEI 22
#define SHLI 23
#define SHRI 24
#define MODI 25
#define XOP 31

#define ADD 0		// xop
#define SUB 1
#define MUL 2
#define DIV 3
#define AND 4
#define OR 5
#define XOR 6
#define EQ 7
#define NE 8
#define LT 9
#define LE 10
#define GT 11
#define GE 12
#define SHL 13
#define SHR 14
#define MOD 15
#define MOV 16
#define LDX 17
#define STX 18
#define RET 19
#define TRAP 20
#define PUSH 21
#define POP 22
#define NOT 23
#define INT 24
#define RETI 25
#define PUSHM 26
#define POPM 27
#define XCH 28

#define MAXMEM		20000		// memory max
#define NIL			-1
#define DEL			100			// interrupt interval (inst)
#define INTVEC		1000		// int vec location
#define MAXSIM		10000		// max sim clock
#define STRSEG		9000		// string segment
#define HEAP		10000		// heap for malloc

#define RETV		28			// return value register
#define SPTR		29			// stack pointer

#define eqs(a,b)  (strcmp(a,b)==0)

// IR bits
int IRop(void); 	// bit 31..27
int IRr1(void); 	// bit 26..22
int IRads(void);	// bit 21..0
int IRr2(void); 	// bit 21..17
int IRr3(void); 	// bit 16..12
int IRxop(void);	// bit 11..0
int IRdisp(void);	// bit 16..0

extern int clock, ninst;

void error(char *s);
void interp(void);
void loadprogram(char *s);
void show(void);
void trap(int reg, int num);
int signx2( int d );

void dumpreg(void);
void runoneclock(void);
