/* hash.c
   symbol table routines	26 July 98

   int hash(char *s)
   int putsym(char *s)
   void setsym(int idx, int value, int type, int arg)
   int searchsym(char *s)
   int getValue(int idx)
   int getType(int idx)
   int getArg(int idx)

   for as21		22 Jan 2007
*/

#include "as21.h"

#define TSIZE 7919	// must be next prime double num. of sym.

struct{
  char name[32];
  int value;
  int type;			// 1 op, 2 sym
  int arg;			// chk arg type: MEM, MOV, RRI
} htable[TSIZE];	// symbol table
int nsym = 0;		// number of symbol in hash table

PRIVATE int hash( char *name ){	// simple hash func.
  int i, idx;
  for( idx=0, i=0; name[i] != 0; i++ )
    idx += name[i];
  return idx % TSIZE;
}

void setsym(int idx, int value, int type, int arg){
	htable[idx].value = value;
	htable[idx].type = type;
	htable[idx].arg = arg;
}
// search and insert name, return index
int putsym( char *name){
	int idx;

	idx = hash(name);
	while( strcmp(htable[idx].name, name) != 0 ) {
		if(*htable[idx].name == 0 ) break;
		idx = (idx + 1) % TSIZE;
	}
	if( *htable[idx].name != 0 ) return idx; // found
	// insert sym as UNDEF
	nsym++;
	if( nsym > TSIZE/2 )
		error("too many symbols, increase table size\n");
	strcpy(htable[idx].name, name);
	htable[idx].type = SYM;
	htable[idx].value = UNDEF;
	return idx;
}
/*
int getsym( char *name, int *type)	// return -1 if not found
{
  int idx;
  idx = hash(name);
  while( strcmp(htable[idx].name, name) != 0 ) {
    if(*htable[idx].name == 0 ) return -1;
    idx = (idx + 1) % TSIZE;
  }
  *type = htable[idx].type;
  return htable[idx].value;
}
*/
void dumpsymbol(void)     // for debugging purpose
{
  int i;
  for(i=0; i<TSIZE; i++)
    if( isalpha(htable[i].name[0]) )
      printf("%s %d\n",htable[i].name, htable[i].value);
}
// return index, -1 if not found
int searchsym(char *name){
  int idx;
  idx = hash(name);
  while( strcmp(htable[idx].name, name) != 0 ) {
    if(*htable[idx].name == 0 ) return -1;
    idx = (idx + 1) % TSIZE;
  }
  return idx;
}

int getValue(int idx){ return htable[idx].value;}
int getType(int idx){ return htable[idx].type; }
int getArg(int idx){ return htable[idx].arg; }


