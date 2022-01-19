
// list.c   base code from list-s.txt  of som-v5   26 Oct 2010

#include "compile.h"

#define	CELLPTR 10			// min pointer to a cell
#define MAXCELL 100000

#define setcar(a,value)	 cell[a] = (value)
#define setcdr(a,value)  cell[(a)+1] = (value)

PRIVATE int cell[MAXCELL];		// storage of list structure
PRIVATE int freecell = CELLPTR;
PRIVATE int endcell = MAXCELL - 2;

int car(int a){ return cell[a]; }
int cdr(int a){ return cell[a+1]; }

int item2(int x){ return car(cdr(x)); }
int item3(int x){ return car(cdr(cdr(x))); }

PRIVATE int newcell(int hd, int tl){
    int a;
	a = freecell;
	freecell = freecell + 2;
	if( freecell >= endcell )
		seterror("out of memory cell");
	else{
		setcar(a, hd);
		setcdr(a, tl);
	}
	return a;
}

int islist(int x){ return car(x) >= CELLPTR; }
int isatom(int x){ return car(x) < CELLPTR; }

int newatom(int type, int value){
    return newcell(type,value);
}

int list(int a){
    return newcell(a, NIL);
}

int cons(int a, int ls){
    return newcell(a,ls);
}

int append(int ls, int x){
    int a;
	if( x != NIL ){
		a = ls;
		while( cdr(a) != NIL )
			a = cdr(a);
		setcdr(a, list(x));
	}
	return ls;
}

// end


