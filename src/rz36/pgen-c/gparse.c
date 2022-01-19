// gparse.c
//   parse the input grammar  30 Sept 2010
//   project  parser generator in C
//   base source from pgen-som (som 4.2a)

#include "pgen.h"

#define   PRIVATE  static

#define  MAXYSTK	500			// size of parser stack

extern FILE *FI;
extern char tokstring[];
PRIVATE int ystack[MAXYSTK], ysp = 0;
int tok = 0;
int header;

// ---- parser stack  operators -----

void ypush(int x){
	ysp++;
	if( ysp >= MAXYSTK ) seterror("parser stack overflow");
	ystack[ysp] = x;
}

int ypop(void){
	int x;
	if( ysp <= 0 ) seterror("parser stack underflow");
	x = ystack[ysp];
	ysp--;
	return x;
}

int ytos(void){ return ystack[ysp]; }

// -------------------------------

// create a new string
char *copystring(char *s1){
	char *s2;
	s2 = (char*)malloc(strlen(s1)+1);
	strcpy(s2, s1);
	return s2;
}

void doiden(void){
	int ty; char *s;
	s = copystring(tokstring);
	if( s[0] == 't' && s[1] == 'k' )
		ty = TERM;
	else if( streq(tokstring,"nil") )
		ty = NIL;
	else
		ty = NONTERM;
	ypush(newatom(ty, (int)s));     // how to deal with pointer to char?
}

void dostring(void){
	ypush(newatom(STRING, (int)copystring(tokstring)));
}

// [%mark %atom .. %atom -> %alt]
void doalt(void){
	int a, b;
	b = NIL;
	while( ytos() != MARK ){
		a = ypop();
		b = cons(a, b);
	}
	a = ypop();					// throw mark away
	ypush(b);
}

void pratom(int type, int val){
	switch(type){
		case NIL: printf("NIL"); break;
		case SP:  printf("SP"); break;
		case TERM:  printf("%c%s", 39, (char*)val); break;
		case NONTERM: printf("%s", (char*)val); break;			// how to print string?
		case STRING: printf("%c%s%c", 34, (char*)val, 34); break;
	}
}

void prlist(int a){
	if( a == NIL ) return;
	if( isatom(a) ){
		pratom( car(a), cdr(a) );
		printf(" ");
	}else {
		printf("(");
		while( a != NIL ){
			prlist( car(a) );
			a = cdr(a);
		}
		printf(")");
	}
}

// ------------------------------

void expect(int tk, char *mss){
	if( tok != tk ) seterror(mss);
}

void commit(int status){
	if( status == 0 ) seterror("syntax error");
}

void mylex(void){
	tok = lex(FI);			// what is syslex FI ?
//	prToken(tok);
}

//  lexgen grammar

// grammar -> 'string | rule | 'eof
// rule -> 'id rule2
// rule2 ->  '= es | '[ var
// var -> 'id var | '] '= es
// es -> e1 es | '| es | '%
// e1 -> 'id | 'string

int e1(void){
	switch(tok){
		case tkIDEN:
			doiden();
			mylex();
			return 1;
		case tkSTRING:
			dostring();
			mylex();
			return 1;
	}
	return 0;
}

int es(void){
	while( e1() ) ;
	switch(tok){
		case tkBAR:
			doalt();
			ypush(MARK);
			mylex();
			return es();
		case tkMOD:
			doalt();
			doalt();
			return 1;
	}
	return 0;
}

int var(void){
	while( tok == tkIDEN ){
		doiden();
		mylex();
	}
	if( tok == tkRBRACKET ){
		doalt();
		mylex();
		expect(tkEQ, "expect = ");
		ypush(MARK);
		mylex();
		return es();
	}
	return 0;
}

int rule2(void){
	switch(tok){
		case tkEQ:
			ypush(newatom(NIL, 0));
			ypush(MARK);
			mylex();
			return es();
		case tkLBRACKET:
			ypush(MARK);
			mylex();
			return var();
	}
	return 0;
}

int rule(void){
	if( tok == tkIDEN ){
		ypush(MARK);
		doiden();
		mylex();
		return rule2();
	}
	return 0;
}

int grammar(void){
	int a;
	mylex();
	ypush(MARK);
	while( tok != tkEOF ){
		if( tok == tkSTRING ){
			a = (int)copystring(tokstring);
			header = append(header, (newatom(STRING,a)));
		}else{
			commit(rule());
//			prlist(ytos());
//			printf("\n");
		}
		mylex();
	}
//	mylex();
	doalt();		// collect all rules
	return 1;
}

