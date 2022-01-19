/* compiler.h
	P.Chongstitvatana
	19 Aug 97

	3 Apr 2001  start project RZ
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "token.h"

#define PRIVATE		static
//#define DEBUG

#define	TABLESIZE	500
#define LOCALSIZE   100
#define NAMELEN		32
#define STRLEN      80
#define MAXCS		5000
//#define MAXFUNC		500
#define MAXSTK		50
#define MAXMEM		5000

#define STRBASE		3000
#define DSBASE		1100

// type of token
#define	tkIDEN    		10
#define	tkNUMBER        11
#define	tkSTRING        12
#define	tkEOF           13
#define tkNIL		    14
#define	tkERROR         15
#define tkBREAK			16
#define tkPRAGMA		17

// type of symbol in symbol table
#define tyVECTOR       	3
#define tySCALAR       	4
#define tyFUNCTION     	6
#define tyLOCAL		    7
//#define tyNEW			8

// 	type of atom in parse tree
#define SP				0
#define OPER			1
#define NUM				2
#define GNAME			3
#define LNAME			4
#define STRING			6
#define ADS				7

#define NL_CHAR		10
#define EOF_CHAR	255

// marker
#define NIL				0
#define MARK			1

#define verbose         0

typedef unsigned char uchar;
typedef char string[STRLEN];

typedef struct {
  char name[NAMELEN];
  int  type;
  int  ref;
  int  arg;
  int  fs;		// frame size
} sym_entry;

extern  int tok;		/* current token type, string */
extern  char tokstring[];
extern  int line;		/* current position */
extern  uchar CH;
extern  char strbuf[];

/* stmt.c */
void seterror(char *mess);
void expect( int tktype, char *mess );
void setfun(char *name);
int putSym(char *name, int type, int ref, int arg);
void commit( int status );

/* lex.c */
void mylex(void);
//void lex(void);
//void initCharType(void);
int isDigit(uchar c);
int isLetterOrDigit(uchar c);
void accept(int v, int k);
void getC(void);
void readinfile(char *fname);

/* symtab.c */
int installLocal(char *name, int *found);
int installGlobal(char *name, int type, int ref, int arg, int *found);
int install(char *name);
void clearLocal(int idx);
//void getAt(int index, int *type, int *ref, int *arg);
void setRef(int index, int ref);
char *getName(int idx);
int getType(int idx);
int getRef(int idx);
int getArg(int idx);
int getFs(int idx);
void setType(int idx, int val);
void setArg(int idx, int val);
void setFs(int idx, int val);
int isdefine(char *name);

void doLocal(void);
//int installFunc(char *name, int pv);
void putvec(char *name, int size);
void putvar(char *name);

void pusht(char *nm);
char *popt(void);
void seterror( char *mess );

int newatom(int type, int value);
int list(int a);
int cons(int a, int ls);
int isatom(int x);
int car(int a);
int cdr(int a);
int item2(int a);
int item3(int a);

void ypush(int x);
int ypop(void);
int ytos(void);

void zop(int op);
void zclean(void);
void zpush(int op);

void dofun(void);
void printList(int a);
void dovar(char *name);
void doset(void);
void donum(char *numstr);
void doblock(void);
void dovec(char *name);
void docall(char *name);
void douop(int op);
void doreturn(void);
void doifelse(void);
void doif(void);
void dowhile(void);
void dostring(void);
void doprint(void);
//int eval(int e);
void dopragma(char *name);

// gencode.c
int genex(int x);
char *findName(int type, int ref);
void dumpSymTab(void);
void initlex(void);

