// pgen.h

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define  NIL  		1		// mark
#define  MARK		2
#define  BMARK		3

#define  SP   		4		// atom
#define  TERM		5
#define  NONTERM	6
#define  STRING		7

#define  tkIDEN   	14		// token type
#define  tkNUMBER	15
#define  tkSTRING	16
#define  tkEOF		17
#define  tkERROR	18

#define  tkSTAR 	50		// token
#define  tkSLASH	51
#define  tkMINUS	52
#define  tkPLUS		53
#define  tkEQ		54
#define  tkEQEQ		55
#define  tkAND		56
#define  tkBAR		57
#define  tkCARET	58
#define  tkMOD		59
#define  tkNOT		60
#define  tkNE		61
#define  tkLT		62
#define  tkLE		63
#define  tkLTLT		64
#define  tkGT		65
#define  tkGE		66
#define  tkGTGT		67
#define  tkCOLON	68
#define  tkLPAREN	69
#define  tkRPAREN	70
#define  tkLBRACKET	71
#define  tkRBRACKET	72
#define  tkBB		73
#define  tkBE		74
#define  tkTO		75
#define  tkIF		76
#define  tkELSE		77
#define  tkWHILE	78
#define  tkFOR		79
#define  tkBREAK	80
#define  tkARRAY	81
#define  tkCASE		82
#define  tkENUM		83
#define  tkSYSCALL	84

#define streq(s1,s2)    strcmp(s1, s2) == 0

void seterror(char *mss);
int newatom(int type, int value);
int cons(int a, int ls);
int car(int a);
int cdr(int a);
int isatom(int x);
int append(int ls, int x);
void init_list(void);
int list(int a);
int grammar(void);
int ypop(void);
int lex(FILE *fi);
void initlex(void);
