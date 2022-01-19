/* stmt.c
	P. Chongstitvatana		25 Aug 97

	start project RZ							3 Apr 2001
	incorporate dcl.c into stmt.c
	  and change to one pass					3 Jan 2002
	modify to untangle parsing parameters
	  in function call and use string stack		17 Oct 2010
	add pragma noframe							 9 Feb 2017
*/

#include "compile.h"

#define MAXYSTK  	2000
#define MAXZSTK  	100
#define MAXSTRBUF 	5000

int pv;
char strbuf[MAXSTRBUF];					//  string buffer
int freestr = 0;						//  index to free string

PRIVATE int noframe = 0;				//  pragma noframe flag
PRIVATE int DS = DSBASE;				// index to data segment
PRIVATE int ystack[MAXYSTK], ysp = 0;  	// 	parser stack
PRIVATE int zstack[MAXZSTK], zsp = 0;   //  operator stack
PRIVATE string strstk[MAXSTK];			//  string stack
PRIVATE int strstkp = 0;				//  string stack pointer

void expect( int tktype, char *mess ){
	if( tok != tktype ) seterror(mess);
}

void seterror( char *mess ){
	printf("line %d near '%s' : %s\n",line,tokstring,mess);
//  dumpSym2();
	exit(-1);
}

void warning( char *nm, char *mess){
	printf("Warning: %s %s\n",nm,mess);
}

// push string
void pusht(char *nm){
	strstkp++;
	if(strstkp > MAXSTK) seterror("string stack overflow");
	strcpy(strstk[strstkp],nm);
}

char *popt(void){
	if(strstkp <= 0) seterror("string stack underflow");
	return strstk[strstkp--];
}

PRIVATE void chktype(int type1, int type2) {
	if( type1 != type2 ) seterror("identifier type mismatch");
}

PRIVATE int installFunc(char *name, int pv){
	int a, found, type, ref, arg;
	a = installGlobal(name,tyFUNCTION,0,pv,&found);
	if( found )	{
		type = getType(a);
		chktype(type,tyFUNCTION);
//		if( arg != pv ) seterror("incorrect number of arguments");
	}
	return a;
}

/*  install global name  */
int putSym(char *name, int type, int ref, int arg){
	int a, found;
	a = installGlobal(name,type,ref,arg,&found);
	if(found) seterror("redefine global name");
	return a;
}

void putvec(char *name, int size){
	putSym(name,tyVECTOR,DS,size);
	DS += size;
}

void putvar(char *name){
	putSym(name,tySCALAR,DS,1);
	DS++;
}

void commit( int status ) {
	if ( status == 0 ) seterror("syntax");
}

// -------------- parser stack operator ----------

void ypush(int x){
	ysp++;
	if( ysp >= MAXYSTK )
		seterror("parser stack overflow");
	ystack[ysp] = x;
}

int ypop(void){
	int x;
	if( ysp <= 0 )
		seterror("parser stack underflow");
	x = ystack[ysp];
	ysp--;
	return x;
}

int ytos(void){
	return ystack[ysp];
}

// ----------- action routines ------------

void doLocal(void){
	int found;
	installLocal(tokstring,&found);
	pv++;
	if(found) seterror("duplicate formal parameters");
}

// [ -- idx ]
void setfun(char *name){
	int idx;
	idx = installFunc(name,pv);
	if( verbose )
		printf("%s\n", name);
	ypush(idx);
}

// [idx %ex -- ] store ref to fun in symtab
void dofun(void){
    int idx, a, e;
	e = ypop();
	idx = ypop();
	a = cons(newatom(GNAME,idx), list(e));
	if( noframe )
		setRef(idx,cons(newatom(OPER,tkFUNX),a));
	else
		setRef(idx,cons(newatom(OPER,tkFUN),a));
	clearLocal(idx);	// clear local symbol
	noframe = 0;		// clear noframe flag
}

//  [ -- %var ]
void dovar(char *name){
	int a,type;
	a = install(name);
	type = getType(a);
	switch(type){
	case tySCALAR: ypush(newatom(GNAME,a)); break;
	case tyVECTOR: ypush(newatom(GNAME,a)); break;
	case tyLOCAL: ypush(newatom(LNAME,getRef(a))); break;
	case tyFUNCTION: ypush(newatom(GNAME,a)); break;
	default:
		printf("%s\n",name);
		seterror("dovar: expect variable");
	}
}

// [%ex0 %ex -- %while]
void dowhile(void){
    int e;
	e = ypop();
	e = cons(ypop(), list(e));
	ypush(cons(newatom(OPER, tkWHILE), e));
}

// [%ex0 %ex -- %if] (if ex0 ex)
void doif(void){
	int e;
	e = ypop();
	e = cons(ypop(), list(e));
	ypush(cons(newatom(OPER, tkIF),e));
}

// [%ex0 ex1 ex2 -- %ifelse] (ifelse ex0 ex1 ex2)
void doifelse(void){
	int e1, e2, e;
	e2 = ypop();
	e1 = ypop();
	e = cons(ypop(), cons(e1, list(e2)));
	ypush(cons(newatom(OPER, tkELSE),e));
}
// [%ex -- %ret] (ret ex)
void doreturn(void){
	ypush(cons(newatom(OPER,tkRETURN),list(ypop())));
}

// [%ex -- %vec] (vec var ex)
void dovec(char *name){
    int e, v;
	e = ypop();
	dovar(name);	// ypush(v);
	v = ypop();
	ypush(cons(newatom(OPER,tkVEC), cons(v, list(e))));
}

// function call, n parameters
// [ MARK e1 .. en  -- %call]  (call name e1 .. en)
void docall(char *name){
    int a, ty, e, tmp;
	a = install(name);
	ty = getType(a);
	if( ty != tyFUNCTION )
		seterror("unknown function");
	e = NIL;
	while( ytos() != MARK )
		e = cons(ypop(),e);
	tmp = ypop();			// throw away MARK
	e = cons(newatom(GNAME, a), e);
	ypush(cons(newatom(OPER, tkCALL), e));
}

void donum(char *numstr){
	ypush(newatom(NUM,atoi(numstr)));
}

void dostring(void){
	int p;
	p = freestr;
	strcpy(&strbuf[p],tokstring);		// copy to strbuf[.]
	freestr += strlen(tokstring) + 1;
	ypush(newatom(STRING,p));
}

// [ MARK %e1 .. %en -- %print ]
void doprint(void){
	int e, t;
	e = NIL;
	while(ytos() != MARK)
		e = cons(ypop(),e);
	t = ypop();				// throw away MARK
	ypush(cons(newatom(OPER,tkPRINT),e));
}

// [ %var %ex -- %set ]  (= var ex)
void doset(void){
    int var, e;
	e = ypop();
	var = ypop();
	ypush(cons(newatom(OPER,tkEQ), cons(var, list(e))));
}

// [%ex -- %uop]
void douop(int op){
	ypush(cons(newatom(OPER, op), list(ypop())));
}

// block = tkBB, simplify block size 0 and 1
PRIVATE int makeblock(int a){
	if( a == NIL )
		return NIL; 		// block size 0 {} => NIL
	else if( cdr(a) == NIL )
		return car(a); 		// block size 1 {a} => a
	else
		return cons(newatom(OPER, tkDO), a);
}

// [MARK %e1 .. %en -- %block] (block e1 .. en)
void doblock(void){
    int e, t;
	e = NIL;
	while( ytos() != MARK )
		e = cons(ypop(), e);
	t = ypop();			// throw away MARK
	ypush(makeblock(e));
}

void dopragma(char *name){
//	printf("pragma %s\n",name);
	if( strcmp(name, "#noframe") == 0)
		noframe = 1;
	else
		seterror("unknown pragma");
}

// ------ bop operator stack -----------

void zpush(int op){
	zsp++;
	if( zsp >= MAXZSTK )
		seterror("operator stack overflow");
	zstack[zsp] = op;
}

int zpop(void){
	int x;
	if( zsp <= 0 )
		seterror("operator stack underflow");
	x = zstack[zsp];
	zsp--;
	return x;
}

int ztos(void){	return zstack[zsp];}

// precedence of binary operators low..high
// && ||    		(10)
// == != < <= > >=  (20)
// + -   			(30)
// * /   			(40)
// op: BOT * / - + = == & && | || ! != < <= > >=
PRIVATE int prec[] = {
	0,40,40,30,30,0,20,0,10,0,10,0,20,20,20,20,20};

PRIVATE int precedence(int op){	return prec[op-tkBOT]; }

// [%e1 %e2 -- %bop] (bop e1 e2)
PRIVATE void dobop(int op){
	int e1, e2;
	e2 = ypop();
	e1 = cons(ypop(),list(e2));
	ypush(cons(newatom(OPER,op),e1));
}
//  read "do-bop-precedence.txt" for details
void zop(int op){
	if( precedence(op) > precedence(ztos()) ){
		zpush(op);
	}else{			// do tos
		dobop(zpop());
		zpush(op);
	}
}
// unwind operator stack
void zclean(void){
	int e;
	while( ztos() != tkBOT )
		dobop(zpop());
	e = zpop();			// throw away BOT
}

// end


