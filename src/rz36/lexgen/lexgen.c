
/* lexgen.c
	generate a recogniser for lexical analyser

		7 Apr 2001

  input file is a binding of tokens to their values (symbolic)

  /	tkSlash
  &&	tkAndAnd
  if	tkIF
  ...

  can generate a rudimentary tok() 	14 Apr 2001

  usage  lexgen < token.txt > token.c
  out	 token.h

  token.h  is "#define tkSlash  50 . . .   begin at 50
  token.c  is a function  int token() { . . . }
	return 1 if accept, 0 if reject
	assume the first char is already in CH
	the next char is pointed to by cp

*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>


#define		MAX	5000
#define		TMAX	50	/* max num of token */
#define		SMAX	500	/* max num of state node */

#define		NIL 	-1
#define 	QUOTE	34
#define		EPS	'$'	/* epsilon */
#define		NO	'!'

#define		nl 	printf("\n");

typedef char  word[32];
typedef struct { word str, name; } tkEntry;
typedef struct { char ch; int next, choice, prop; } snode;
typedef int np;		/* snode pointer */

char inbuf[MAX], *cp;
tkEntry tkProp[TMAX];
snode sG[SMAX];		/* state graph */
int sfree = 0;

#define		nextG(node)	sG[(node)].next
#define		choiceG(node)	sG[(node)].choice
#define		infoG(node)	sG[(node)].ch
#define		propG(node)	sG[(node)].prop


lgerr( char *mess ) {
  printf("%s",mess); nl;
  exit(-1);
}

/* function to deal with tkProp[] table */
static int currentPropNum = 0;
int getPropNum() { return currentPropNum; }
setPropNum(int k) { currentPropNum = k; }

/* build state graph */

np getanode( char c ) {
  np a;
  if (sfree == SMAX) lgerr("out of state node");
  a = sfree;
  sfree++;
  infoG(a) = c;
  nextG(a) = NIL;
  choiceG(a) = NIL;
  propG(a) = NIL;
  return a;
}

np buildG( char *s) {
  np node, node1, node2;
  node = getanode( *s );
  s++;
  node1 = node;
  while (*s != 0) {
    node2 = getanode( *s );
    nextG(node1) = node2;
    node1 = node2;
    s++;
  }
  propG(node1) = getPropNum();
  return node;
}

printG( np  g ) {
  int a;
  if( g == NIL ) { printf(")"); return;}
  a = propG(g);
  printf("( %c ",infoG(g));
  if( a != NIL ) printf(":%s ",tkProp[a].str);
  printG(nextG(g));
  printG(choiceG(g));
}

np makeE() {
  return( getanode(EPS) );
}

tryinsertE( np g ) {
  np a, b;
  if( g == NIL ) return;
  while( g != NIL ) {
    a = g;
    g = choiceG(g);
  }
  b = makeE();
  propG(b) = getPropNum();
  choiceG(a) = b;	/* insert as last choice */
}

extendNext( char *s, np g ) {
  np a,e;
  a = buildG(s);
  e = makeE();
  propG(e) = propG(g);
  choiceG(a) = e;
  nextG(g) = a;
}

insertChoice( np p1, np p2 ) { 	/* insert p1 into choice of p2 */
  np a;
  a = choiceG(p2);
  choiceG(p2) = p1;
  choiceG(p1) = a;
}

/* extend g with string s */
mergeG( char *s, np g ) {
  np a;
  a = g;
  while( g != NIL && infoG(g) != EPS) {	 /* check all choices */
    if( infoG(g) == *s ) {	 	 /* match */
      s++;
      if( *s == 0 ) {
	tryinsertE( nextG(g) );
	return;
      }
      if( nextG(g) == NIL ) {
	extendNext( s, g );
	return;
      }
      g = nextG(g);		/* continue matching */
    }else {
      a = g;
      g = choiceG(g);
    }
  }				/* end while, exhausive all choices */
  insertChoice( buildG(s), a);
}

mergeK(int k, np g ){  /* merge k-th tkstring to graph g */
  setPropNum(k);
  mergeG(tkProp[k].str,g);
}

np initG() { /* initialise the first tkstring to a graph */
  np a;
  setPropNum(0);
  a = buildG(tkProp[0].str);
  return a;
}


int readSpec() {		/* read the spec */
  int i;
  fread(inbuf, 1, MAX, stdin);
  i = 0;
  cp = strtok(inbuf, " \t\n");
  while( cp != NULL && i<TMAX) {
    strcpy(tkProp[i].str,cp);
    cp = strtok(NULL," \t\n");
    strcpy(tkProp[i].name,cp);
    cp = strtok(NULL," \t\n");
    i++;
  }
  return i;
}

writeDotH(int n) {
  int i,k;
  FILE *fp;
  fp = fopen("token.h","wt");
  k = 50;
  for(i=0;i<n; i++,k++ )
    fprintf(fp,"#define %s %d\n",tkProp[i].name,k);
  fclose(fp);
}

int isLetter( char c ) {
  if( c >= 'A' && c <= 'Z') return 1;
  if( c >= 'a' && c <= 'z') return 1;
  return 0;
}

static char line[80];

out( char *s) { printf("%s", s ); }

outCase( char c ) {
  sprintf(line,"case '%c': ",c);
  out(line);
}

outDefaultY( np g ) {		/* accept substring */
  int k;
  k = strlen(tkProp[ propG(g)].str);
  sprintf(line,"default: accept(%s,%d); }\n",tkProp[ propG(g)].name,k);
  out(line);
}

outDefaultN() {
  out("default: return 0; } /* reject */ \n");
}

outAccept( np g ) {
  int k;
  if( isLetter(tkProp[ propG(g) ].str[0]) ) {
    out("getC();\n");
    out(" if( isLetterOrDigit(CH) ) return 0;\n");
  }
  k = strlen(tkProp[ propG(g)].str);
  sprintf(line," accept(%s,%d); break;\n",tkProp[ propG(g) ].name,k);
  out(line);
}

gen( np g ) {
  np a;
  outCase( infoG(g) );
  if( nextG(g) == NIL ) outAccept( g );
  else {
    out("getC();\n");
    out("switch(CH) {\n");
    gen( nextG(g) );
    out("break;\n");
  }

  a = choiceG(g);
  if ( a != NIL ) {
    if( infoG(a) != EPS) gen(a);
    else outDefaultY(a);
  }else {
    outDefaultN();
  }
}


main()
{
  int i, n;
  np a;
  n = readSpec();
  writeDotH(n);

  a = initG();
  for( i=1; i<n; i++ ) mergeK(i,a);
/*
  printG(a);
*/
  out("/* automatically generated from lexgen */\n");
  printf("#include %ccompile.h%c\n",QUOTE,QUOTE);
  out("int token() {\n");
  out("switch(CH) {\n");
  gen( a );
  out("return 1;\n}\n");
}

/*====== debug ==========


  printf("i %d\n",i);
  n = i;
  for(i=0; i<n; i++)
    printf("%s %s\n",tkProp[i].str,tkProp[i].name);

printG( np g ) {
  int i;
  i = 1;
  while(sG[g].next != NIL) {
    printf("%c ",sG[g].ch);
    g = sG[g].next;
    i++;
  }
  nl;
  printf("i %d\n",i);
}

prNext( np g ) {
  if( g == NIL ) return;
  if( choiceG(g) == NIL )
    printf(" %c ", infoG(g) );
  else
    printG( g );
  prNext( nextG(g) );
}

printG2( np g ) {
  if( g == NIL ) return;
  printf("(");
  prNext(g);
  printf(")");
  printG( choiceG(g) );
}

gen( np g ) {
  out("\nint token() {\n");
  out("switch(CH) {\n");
  gen1( g );
  out("return 1;\n}\n");
}

dolexgen() {
  np a,b,c;
  a = initG();
  mergeK(1,a);
  tryinsertE(3,a);
  mergeG(2,a);
  mergeG(3,a);
  printG(a);
  nl;
  gen(a);
}


*/