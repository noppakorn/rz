// list.c
//  project pgen-c   29 Sept 2010
//  base source on list-s.txt  from som v4.1  9 Aug 2008

#include "pgen.h"

#define PRIVATE static

#define  CELLPTR 10			// min pointer to a cell
#define  MAXCELL 10000

PRIVATE int cell[MAXCELL];
PRIVATE int freecell, endcell;

void seterror(char *mess){ printf("%s\n",mess); }

void init_list(void){
	freecell = CELLPTR;
	endcell = MAXCELL - 2;
}

void setcar(int a, int value){  cell[a] = value; }
void setcdr(int a, int value){  cell[a+1] = value; }

int car(int a){ return  cell[a]; }
int cdr(int a){ return  cell[a+1]; }

int item2(int x){ return car(cdr(x)); }
int item3(int x){ return car(cdr(cdr(x))); }

int newcell(void){
	int a;
	a = freecell;
	freecell += 2;
	if (freecell >= endcell) seterror("out of memory cell");
	else{
		setcar(a,NIL);
		setcdr(a,NIL);
	}
	return a;
}

int islist(int x){ return car(x) >= CELLPTR; }
int isatom(int x){ return car(x) < CELLPTR; }

int newatom(int type, int value){
	int a;
	a = newcell();
	setcar(a,type);
	setcdr(a,value);
	return a;
}

int list(int a){
	int b;
	b = newcell();
	setcar(b,a);
	return b;
}

int cons(int a, int ls){
	int b;
	b = newcell();
	setcdr(b,ls);
	setcar(b,a);
	return b;
}

int append(int ls, int x){
	int a,b;
	if( x != NIL ){
		a = ls;
		b = cdr(a);
		while( b != NIL ){
			a = b;
			b = cdr(a);
		}
		setcdr(a,list(x));
	}
	return ls;
}

// cons2 x,y = {NIL, atom, list}
int cons2(int x, int y){
	int z;
	if( x == NIL )
		return y;
	if( y == NIL )
		return list(x);
	// it is the same whether x is atom of list
	// a new cell is required to build a dot-pair
	// only y must be inspected, if it is not a list
	// a new dot-pair to make y a list is needed
	if( isatom(y) )
		z = newatom(y,NIL);
	else
		z = y;
	return newatom(x,z);
}

// clone a copy of list t
int copylist(int t){
	if( t == NIL )
		return NIL;
	if( isatom(t) )
		return newatom(car(t), cdr(t));
	else
		return cons2(copylist(car(t)), copylist(cdr(t)));
}

void countcell(void){
	printf("+freecell = %d\n", (MAXCELL - freecell)/2);
}

// data structure of type lis
// is an one-dimension vector with element 0 storing its size
// total space of vector is size+1
void addlis(int *ls, int x){
	int n;
	n = ls[0] + 1;
	ls[n] = x;
	ls[0] = n;
}

void clearlis(int *ls){ ls[0] = 0; }
int sizeoflis(int *ls){ return ls[0]; }

int countlis(int *ls){
	int i, n;
	n = 0;
	for(i=1; i <= sizeoflis(ls); i++)
		if( ls[i] != 0 ) n++;
	return n;
}
