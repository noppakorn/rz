/* symtab.c
	P. Chongstitvatana

	use hash							 7 Feb 2017
*/


#include "compile.h"

#define  GSIZE     541		// global hash table size
#define  LSIZE 	   127		// local hash table size

#define  eqs(a,b)  (strcmp(a,b) == 0)

/*
==========  Interface =============

// search local and insert if not found, ret index or 0 (not found)
int installLocal(char *name, int *found)

// search global and insert if not found, ret index or 0 (not found)
int installGlobal(char *name, int type, int ref, int arg, int *found)

// search local and global if not found insert into local, ret index
int install(char *name)

void dumpSymTab()
====================================
*/

// symbols start from index 1, reserve index 0
PRIVATE sym_entry symtab[TABLESIZE+LOCALSIZE];
int nsym = 0;
int lv = 0;

PRIVATE int ghtable[GSIZE], lhtable[LSIZE];

// simple hash function
PRIVATE int hash( char *name ){
	int i, idx;
	idx = 0;
	for( i=0; name[i] != 0; i++ )
		idx += name[i];
	return idx;
}

PRIVATE int hsearch(char *name, int *table, int size){
	int i, h;

	i = hash(name) % size;
	h = table[i];
	while( h != 0 ){
		if( eqs(symtab[h].name, name) )
			return i;		//  found
		i++;				//  linear hash
		i %= size;
		h = table[i];
	}
	return i;				// not found, return empty slot
}

PRIVATE void putsymbol(int idx, char *name, int type, int ref, int arg){
	strcpy(symtab[idx].name, name);
	symtab[idx].type = type;
	symtab[idx].ref = ref;
	symtab[idx].arg = arg;
	symtab[idx].fs = 0;
}

// insert local symbol at the end of table
//   hindex is used to link back to lhtable[]
PRIVATE int enterLocal(char *name, int hindex){
	int a;
	lv++;
	if( lv >= LOCALSIZE ) seterror("local symbol table full");
	a = TABLESIZE + lv;
	putsymbol(a,name,tyLOCAL,lv,hindex);
	return a;
}

// insert global symbol at the end of table
PRIVATE int enterGlobal(char *name, int type, int ref, int arg){
	int a;
	nsym++;
	if( nsym >= TABLESIZE ) seterror("global symbol table full");
	a = nsym;
	putsymbol(a,name,type,ref,arg);
	return a;
}

// search local and insert if not found, ret index or 0 (not found)
//   use when want to check duplicate
int installLocal(char *name, int *found){
	int a;
	*found = 1;
	a = hsearch(name,lhtable,LSIZE);		// search local
 	if( lhtable[a] != 0 ) return lhtable[a];
	*found = 0;
	lhtable[a] = enterLocal(name,a);
	return lhtable[a];
}

// search global and insert if not found, ret index or 0 (not found)
//   use when want to check duplicate
int installGlobal(char *name, int type, int ref, int arg, int *found){
	int a;
	*found = 1;
	a = hsearch(name,ghtable,GSIZE);			// search global
	if( ghtable[a] != 0 ) return ghtable[a];	// ret index to symtab[]
	*found = 0;
	ghtable[a] = enterGlobal(name,type,ref,arg);
	return ghtable[a];
}

// search local and global if not found insert into local, ret index
int install(char *name){
	int a, b;
	a = hsearch(name,lhtable,LSIZE);
//	printf("sym enter %s at %d\n",name,a);
	if( lhtable[a] != 0 ) return lhtable[a];
	b = hsearch(name,ghtable,GSIZE);
	if( ghtable[b] != 0 ) return ghtable[b];
//	printf("enter local %s at %d\n",name,a);
	lhtable[a] = enterLocal(name,a);
	return lhtable[a];
}

// update fs of fun.idx and clear Local symbol
void clearLocal(int idx){
	int i, a;
	symtab[idx].fs = lv;
	for(i = TABLESIZE+1; i <= TABLESIZE + lv; i++){
//		printf("%s ",getName(i));
		a = symtab[i].arg;
		lhtable[a] = 0;			// clear lhtable entry
	}
	lv = 0;
}

// access functions

int getType(int idx){ return symtab[idx].type; }
int getRef(int idx){ return symtab[idx].ref; }
char *getName(int idx){ return symtab[idx].name; }
int getArg(int idx){ return symtab[idx].arg; }
int getFs(int idx){ return symtab[idx].fs; }

void setType(int idx, int val){ symtab[idx].type = val; }
void setRef(int idx, int val){ symtab[idx].ref = val; }
void setArg(int idx, int val){ symtab[idx].arg = val; }
void setFs(int idx, int val){ symtab[idx].fs = val; }

// search global for name with type,ref
char *findName(int type, int ref){
	int i;
	for(i=1; i<=nsym; i++) {
		if( symtab[i].type == type && symtab[i].ref == ref)
			return getName(i);
	}
	return NULL;
}

int getMainRef(void){
	int i;
	i = hsearch("main",ghtable,GSIZE);
	if( ghtable[i] != 0 ) return getRef(ghtable[i]);
	return 0;
}

// check if sym is defined in symbol table
int isdefine(char *sym){
	int h;
	h = hsearch(sym,ghtable,GSIZE);
	return ghtable[h] != 0;
}

// ========= for debugging =========

extern FILE *FO;

// output only global symbol
void dumpSymTab(void){
  int i;
  sym_entry *s;
//  printf("symbol table %d entries\n%s\n",nsym,
//  	"      name type ref arg");
  fprintf(FO,"%d\n",nsym);
  for( i=1; i<=nsym; i++ ) {
    s = &symtab[i];
    fprintf(FO,"%s %d %d %d %d\n",
      s->name,s->type,s->ref,s->arg,s->fs);
  }
}
/*
void dumpSym2(){
	int i;
	sym_entry *s;
	for(i=1;i<=nsym;i++){
		s = &symtab[i];
		printf("%d name %s type %d ref %d arg %d\n",
		  i, s->name, s->type, s->ref, s->arg);
	}
	for(i=TABLESIZE;i<=TABLESIZE+lv; i++){
		s = &symtab[i];
		printf("%d name %s type %d ref %d arg %d\n",
		  i-TABLESIZE, s->name, s->type, s->ref, s->arg);
	}
}
*/

