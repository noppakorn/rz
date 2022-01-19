// main.c
//   project  parser generator in C
//   base source from pgen-som (som 4.2a)     30 Sept 2010

#include "pgen.h"

extern int header;
int verbose, lexflag, lhs, loopflag, nilflag;
FILE *FI;

void initPgen(void){
	verbose = 1;
	lexflag = 0;
	lhs = 0;
	loopflag = 0;
	nilflag = 0;
}
/*
void warning(char *nm, char *mss){
	if( verbose ){
		printf("Warning: ");
		if( nm ) printf("%s ", nm);
		printf("%s\n", mss);
	}
}
*/
// -------------------

int lenlist(int m){
	int k;
	k = 0;
	while( m != NIL ){
		k++;
		m = cdr(m);
	}
	return k;
}

int last(int m){
	while( cdr(m) != NIL )
		m = cdr(m);
	return car(m);
}

int atomeq(int a1, int a2){
	return car(a1) == car(a2) && streq((char*)cdr(a1), (char*)cdr(a2));
}
/*
// reverse m1 to m2
int reverselist(int m1){
	int m2;
	m2 = NIL;
	while( m1 != NIL ){
		m2 = cons(car(m1), m2);
		m1 = cdr(m1);
	}
	return m2;
}
*/
void trylex(void){
	if( lexflag ){
		printf("mylex();\n");
		lexflag = 0;
	}
}

// for the first match item
void genone1(int a){
	int ty, val;
	ty = car(a);
	val = cdr(a);
	switch(ty){
		case NIL: nilflag = 1; break;
		case TERM:
			trylex();
//			if( loopflag )
//				printf("while( tok == ");
//			else
				printf("if( tok == ");
			printf("%s ){\n", (char*)val);
			lexflag = 1;
			break;
		case NONTERM:
			trylex();
//			if( loopflag )
//				printf("while( ");
//			else
				printf("if( ");
			printf("%s() ){\n", (char*)val);
			break;
		case STRING:
			printf("%s\n",(char*)val);
			trylex();
			break;
	}
}

// for one match item
void genone(int a){
	int ty, val;
	ty = car(a);
	val = cdr(a);
	switch(ty){
		case NIL: nilflag = 1; break;
		case TERM:
			trylex();
			printf("expect(%s, ", (char*)val);
			printf("%cmissing %s%c);\n",34,(char*)val,34);
			lexflag = 1;
			break;
		case NONTERM:
			trylex();
			printf("commit(%s());\n", (char*)val);
			break;
		case STRING:
			printf("%s\n",(char*)val);
			trylex();
	}
}

// for each match in an alternative
void genalt2(int e, int level, int len){
	if( e == NIL ){
		if( nilflag == 0 ){
			trylex();
			printf("return 1; }\n");
		}
	}else if( loopflag && (level == len) ){		// don't do the last one
		printf("goto loop;}\n");
	}else{
		if( level == 1 )
			genone1(car(e));		// first match
		else
			genone(car(e));			// the rest
		genalt2(cdr(e), level+1, len);
	}
}

// for each alternative
void genalt(int e){
	int e2;
	if( e != NIL ){
		e2 = car(e);
		if(atomeq(lhs, last(e2)))
			loopflag = 1;
		else
			loopflag = 0;
		genalt2(e2, 1, lenlist(e2));
		genalt(cdr(e));
	}
}

void gencase(int e){
	if(e != NIL){
		genone(car(e));
		gencase(cdr(e));
	}
}

void genonecase(int a){
	printf("case %s:\n",(char*)cdr(car(a)));
	lexflag = 1;
	gencase(cdr(a));
	trylex();
	printf("return 1;\n");
}

void genmulti(int e){
	int k, i, a;
	printf("switch(tok){\n");
	k = lenlist(e);
	for(i=1; i<k; i++){
		genonecase(car(e));
		e = cdr(e);
	}
	// the last one
	if( car(car(car(e))) == TERM ) 		// is tkXX
		genonecase(car(e));
	else{
		printf("default: \n");
		genalt2(car(e), 1, lenlist(car(e)));
//		printf("}\n");					// close  last alt
	}
	printf("}\n");				// close switch()
}

// check if there is a recursive rule
int chkRecursive(int e){
	if( e == NIL )
		return 0;
	else if( atomeq(lhs, last(car(e))) )
		return 1;
	else
		return chkRecursive(cdr(e));
}

// to be multi, no recursion, alt > 2,
// all alts except last must be tkXX
int ismulti(int e){
   int k, i, e2, f, ty;
	if( loopflag )
		return 0;
	k = lenlist(e);
	if( k < 3 )
		return 0;
	f = 1;
	for(i=1; i<k && f; i++){
		ty = car(car(car(e)));		// type of first match
		if( ty != TERM )
			f = 0;
		else
			e = cdr(e);
	}
	return f;
}

// list of local variable
void prnames(int e){
	if( e != NIL ){
		printf("int %s;\n",(char*)cdr(car(e)));
		prnames(cdr(e));
	}
}

void endrule(void){
	trylex();
	if( nilflag )
		printf("return 1;\n");
	else
		printf("return 0;\n");
	nilflag = 0;
}

// for each rule
void genarule(int a){
	int b, nm;
	lhs = car(a);
	printf("int %s(void)",(char*)cdr(lhs));
	nm = car(cdr(a));
	printf("{\n");
	if( car(nm) != NIL ){
//		printf("{\n");
		prnames(nm);
	}
//	printf("{\n");
	b = cdr(cdr(a));
	loopflag = chkRecursive(b);
	if( loopflag ) printf("loop:\n");
	if( ismulti(b) )
		genmulti(b);
	else
		genalt(b);
	endrule();
	printf("}\n");
}

void gen(int e){
	if( e != NIL ){
		genarule(car(e));
		gen(cdr(e));
	}
}

void prheader(int h){
	if( h != NIL ){
		printf("%s\n",(char*)cdr(car(h)));
		prheader(cdr(h));
	}
}

// gen forward fun def
void genforward(int e){
	int e2;
	if( e != NIL ){
		e2 = car(e);			// a rule
		printf("int %s(void);\n",(char*)cdr(car(e2)));
		genforward(cdr(e));
	}
}

void parse(char *fn){
	FI = fopen(fn,"r");
	grammar();
	fclose(FI);
}

int main(void){
	int e;
	initPgen();
	init_list();
	initlex();
	header = list(newatom(SP,0));
	parse("rz-grammar9.txt");
	e = ypop();
	prheader(cdr(header));
	printf("\n");
//	genforward(e);
	gen(e);
	return 1;
}

// end


