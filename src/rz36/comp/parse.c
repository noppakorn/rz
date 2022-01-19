// parser generated from rz-grammar 17 Oct 2010
#include <compile.h>
#include <parse.h>
extern int pv;

int pass(void){
loop:
	if( dcl() ){
		goto loop;
	}
	if( tok == tkEOF ){
		mylex();
		return 1;
	}
	return 0;
}
int dcl(void){
	if( tok == tkIDEN ){
		pusht(tokstring);
		mylex();
		commit(dcl2());
		return 1;
	}
	if( tok == tkPRAGMA ){
		dopragma(tokstring);
		mylex();
		expect(tkSEMI, "missing tkSEMI");
		mylex();
		return 1;
	}
	return 0;
}
int dcl2(void){
	int n;
	switch(tok){
	case tkLPAREN:
		pv=0;
		mylex();
		commit(formal());
		expect(tkRPAREN, "missing tkRPAREN");
		setfun(popt());
		mylex();
		commit(stmt());
		dofun();
		return 1;
	case tkLBRACKET:
		mylex();
		expect(tkNUMBER, "missing tkNUMBER");
		n=atoi(tokstring);
		mylex();
		expect(tkRBRACKET, "missing tkRBRACKET");
		putvec(popt(),n);
		mylex();
		commit(dcl3());
		return 1;
	case tkCOMMA:
		putvar(popt());
		mylex();
		return 1;
	case tkSEMI:
		putvar(popt());
		mylex();
		return 1;
	}
	return 0;
}
int dcl3(void){
	if( tok == tkCOMMA ){
		mylex();
		return 1;
	}
	if( tok == tkSEMI ){
		mylex();
		return 1;
	}
	return 0;
}
int formal(void){
	if( tok == tkIDEN ){
		doLocal();
		mylex();
		commit(formals());
		return 1;
	}
	return 1;
}
int formals(void){
	if( tok == tkCOMMA ){
		mylex();
		commit(formal());
		return 1;
	}
	return 1;
}
int stmt(void){
	if( block() ){
		return 1;
	}
	if( stmt1() ){
		return 1;
	}
	return 0;
}
int stmts(void){
loop:
	if( stmt1() ){
		goto loop;
	}
	doblock();
	return 1;
}
int block(void){
	if( tok == tkLBRACE ){
		ypush(MARK);
		mylex();
		commit(stmts());
		expect(tkRBRACE, "missing tkRBRACE");
		mylex();
		return 1;
	}
	return 0;
}
int stmt1(void){
	switch(tok){
	case tkSEMI:
		ypush(NIL);
		mylex();
		return 1;
	case tkIF:
		mylex();
		expect(tkLPAREN, "missing tkLPAREN");
		mylex();
		commit(expr());
		expect(tkRPAREN, "missing tkRPAREN");
		mylex();
		commit(stmt());
		commit(elsest());
		return 1;
	case tkWHILE:
		mylex();
		expect(tkLPAREN, "missing tkLPAREN");
		mylex();
		commit(expr());
		expect(tkRPAREN, "missing tkRPAREN");
		mylex();
		commit(stmt());
		dowhile();
		return 1;
	case tkRETURN:
		mylex();
		commit(returnst());
		return 1;
	case tkPRINT:
		mylex();
		expect(tkLPAREN, "missing tkLPAREN");
		ypush(MARK);
		mylex();
		commit(prlist());
		expect(tkRPAREN, "missing tkRPAREN");
		mylex();
		expect(tkSEMI, "missing tkSEMI");
		doprint();
		mylex();
		return 1;
	case tkSTAR:
		mylex();
		expect(tkIDEN, "missing tkIDEN");
		dovar(tokstring);
		douop(tkDEREF);
		mylex();
		expect(tkEQ, "missing tkEQ");
		mylex();
		commit(expr());
		expect(tkSEMI, "missing tkSEMI");
		doset();
		mylex();
		return 1;
	case tkIDEN:
		pusht(tokstring);
		mylex();
		commit(stmt2());
		return 1;
	}
	return 0;
}
int elsest(void){
	if( tok == tkELSE ){
		mylex();
		commit(stmt());
		doifelse();
		return 1;
	}
	doif();
	return 1;
}
int returnst(void){
	if( tok == tkSEMI ){
		ypush(NIL);
		doreturn();
		mylex();
		return 1;
	}
	if( expr() ){
		expect(tkSEMI, "missing tkSEMI");
		doreturn();
		mylex();
		return 1;
	}
	return 0;
}
int prlist(void){
	if( tok == tkSTRING ){
		dostring();
		mylex();
		commit(prlists());
		return 1;
	}
	if( expr() ){
		commit(prlists());
		return 1;
	}
	return 0;
}
int prlists(void){
	if( tok == tkCOMMA ){
		mylex();
		commit(prlist());
		return 1;
	}
	return 1;
}
int stmt2(void){
	switch(tok){
	case tkEQ:
		dovar(popt());
		mylex();
		commit(expr());
		expect(tkSEMI, "missing tkSEMI");
		doset();
		mylex();
		return 1;
	case tkLBRACKET:
		mylex();
		commit(expr());
		expect(tkRBRACKET, "missing tkRBRACKET");
		mylex();
		expect(tkEQ, "missing tkEQ");
		dovec(popt());
		mylex();
		commit(expr());
		expect(tkSEMI, "missing tkSEMI");
		doset();
		mylex();
		return 1;
	case tkLPAREN:
		ypush(MARK);
		mylex();
		commit(param());
		expect(tkRPAREN, "missing tkRPAREN");
		mylex();
		expect(tkSEMI, "missing tkSEMI");
		docall(popt());
		mylex();
		return 1;
	}
	return 0;
}
int param(void){
	if( expr() ){
		commit(params());
		return 1;
	}
	return 1;
}
int params(void){
	if( tok == tkCOMMA ){
		mylex();
		commit(param());
		return 1;
	}
	return 1;
}
int expr(void){
	if( term() ){
		zpush(tkBOT);
		commit(exprs());
		return 1;
	}
	return 0;
}
int exprs(void){
loop:
	if( bop() ){
		commit(term());
		goto loop;
	}
	zclean();
	return 1;
}
int bop(void){
	switch(tok){
	case tkOROR:
		zop(tok);
		mylex();
		return 1;
	case tkANDAND:
		zop(tok);
		mylex();
		return 1;
	case tkLT:
		zop(tok);
		mylex();
		return 1;
	case tkLE:
		zop(tok);
		mylex();
		return 1;
	case tkEQEQ:
		zop(tok);
		mylex();
		return 1;
	case tkNE:
		zop(tok);
		mylex();
		return 1;
	case tkGE:
		zop(tok);
		mylex();
		return 1;
	case tkGT:
		zop(tok);
		mylex();
		return 1;
	case tkPLUS:
		zop(tok);
		mylex();
		return 1;
	case tkMINUS:
		zop(tok);
		mylex();
		return 1;
	case tkSTAR:
		zop(tok);
		mylex();
		return 1;
	case tkSLASH:
		zop(tok);
		mylex();
		return 1;
	}
	return 0;
}
int term(void){
	switch(tok){
	case tkMINUS:
		mylex();
		commit(term1());
		douop(tkMINUS);
		return 1;
	case tkNOT:
		mylex();
		commit(term1());
		douop(tkUNOT);
		return 1;
	case tkSTAR:
		mylex();
		commit(term1());
		douop(tkDEREF);
		return 1;
	case tkAND:
		mylex();
		expect(tkIDEN, "missing tkIDEN");
		pusht(tokstring);
		mylex();
		commit(index());
		douop(tkADS);
		return 1;
	default:
		if( term1() ){
			return 1;
		}
	}
	return 0;
}
int term1(void){
	switch(tok){
	case tkIDEN:
		pusht(tokstring);
		mylex();
		commit(mod());
		return 1;
	case tkNUMBER:
		donum(tokstring);
		mylex();
		return 1;
	case tkSTRING:
		dostring();
		mylex();
		return 1;
	case tkLPAREN:
		mylex();
		commit(expr());
		expect(tkRPAREN, "missing tkRPAREN");
		mylex();
		return 1;
	}
	return 0;
}
int mod(void){
	if( tok == tkLPAREN ){
		ypush(MARK);
		mylex();
		commit(param());
		expect(tkRPAREN, "missing tkRPAREN");
		docall(popt());
		mylex();
		return 1;
	}
	if( index() ){
		return 1;
	}
	return 0;
}
int index(void){
	if( tok == tkLBRACKET ){
		mylex();
		commit(expr());
		expect(tkRBRACKET, "missing tkRBRACKET");
		dovec(popt());
		mylex();
		return 1;
	}
	dovar(popt());
	return 1;
}
