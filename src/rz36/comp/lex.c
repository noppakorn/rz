/* lex.c   lexical analyzer   for R1
	15 Oct 97
	binary search reserved word  		2 Sept 97
		start project RZ				3 Apr 2001
	change to accept token()			15 Apr 2001
	add lineat[] and getsrc() to track source code line
										20 December 2001
	use indent as {} and \n as ;        28 Aug 2011
	add pragma  #noframe				 9 Feb 2017
*/

#include "compile.h"

#define MAXFIN	10000	/* max input file size */
#define MAXLINE  1000	/* max no. of source line */

#define	cBLANK		0	// character type
#define	cLETTER     1
#define	cDIGIT      2
#define cQUOTE		3
#define cSPECIAL	4
#define cHASH		5

#define LEX_LEX		11
#define LEX_NEW		12
#define LEX_EOF		13
#define LEX_BACK	14
#define LEX_OLD		15

extern int token(void);			/* in token.c */

int tok;		     			/* current token type, string */
char tokstring[256];
int line = 1;					/* current position */
uchar CH;       				/* current char */

PRIVATE char inbuf[MAXFIN];		/* input file buffer */
PRIVATE char *cp, *ieof, *cp1;
PRIVATE int col = 0;					// current column
PRIVATE int tokcol = 1;					// current token column
PRIVATE int colstk[100], colstkp = 0;	// column stack, pointer
PRIVATE int lexstate = LEX_LEX;

// character type table, create by initCharType()
PRIVATE char chartype[] = {
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,4,3,5,4,4,4,4,4,4,4,4,4,4,4,4,2,2,2,2,2,2,2,2,2,2,4,4,
  4,4,4,4,4,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,4,4,4,4,1,4,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4
};

PRIVATE void pushcol(void){
    colstkp++;
    colstk[colstkp] = tokcol;
}

PRIVATE void popcol(void){
    colstkp--;
}

PRIVATE int topcol(void){
    return colstk[colstkp];
}

void getC(void){
	if ( cp >= ieof ) {
		CH = EOF_CHAR; cp = ieof;
	}else{
		CH  = *cp++;
		col++;
	}
}

PRIVATE void doline(void){   // store cp to the current line
	line++; col = 0;
}

PRIVATE void skipblank(void){
	for(;;) {
		while( chartype[CH] == cBLANK ) {
			if ( CH == NL_CHAR) doline();
			getC();
		}
		// check comment, *cp lookahead one char
        if( CH != '/' || *cp != '/' ) break;
		while( CH != NL_CHAR && CH != EOF_CHAR) getC();
	}
}

PRIVATE void copyToken(char* a, char* b) {
	int len;
	len = b - a;
	if( len <= 0 ) {
		printf("lex error token length = 0\n");
		exit(-1);
	}
	if( len > 255 ) {
		printf("error token too long > 256 char\n");
		exit(-1);
	}
	strncpy(tokstring,a,len);
	tokstring[len] = 0;
}

/* ------------- public ------------------ */

int isDigit(uchar c){ return (chartype[c] == cDIGIT); }

int isLetterOrDigit(uchar c){
	return  (chartype[c] == cLETTER || chartype[c] == cDIGIT);
}

void accept(int v, int k){
	tok = v;
	cp = cp1 + k;
	getC();
}

// tokcol is correct for the first token on a new line
void mylex2(void){
	skipblank();
	tokcol = col;
	if( CH == EOF_CHAR ) {tok = tkEOF; return; }
	cp1 = cp - 1;
	if( token() ) return;

	// recogniser reject, reset char pointer
	cp = cp1;
	getC();
	switch( chartype[CH] ) {
	case cLETTER :
		while( isLetterOrDigit(CH) ) getC();
		copyToken(cp1,cp-1);
		tok = tkIDEN;
		break;
	case cDIGIT :
		while( isDigit(CH) ) getC();
		copyToken(cp1,cp - 1);
		tok = tkNUMBER;
		break;
	case cQUOTE :
		do {
			getC();
			if ( CH == NL_CHAR ) doline();
		}  while( CH != '"' && CH != EOF_CHAR);
		copyToken(cp1+1,cp-1);
		getC();
		tok = tkSTRING;
		break;
	case cHASH :
		getC();
		while( isLetterOrDigit(CH) ) getC();
		copyToken(cp1,cp-1);
		tok = tkPRAGMA;
		break;
	default : tok = tkERROR;
	}
//    prtoken(tok);
}

// start at N
// assume one tok and tokcol in stack
// state  do    condition      action       next
// L      lex   eof match      out eof      L
// L      lex   eof not match  out } pop    L
// L      lex   oldline        out tok      L
// L      lex   nl =           out ;        N    same block
// L      lex   nl >           out { push   N    open new block
// L      lex   nl <           out ;    	B    close new block
// B                           out } pop    C
// C            <              out } pop    C    out } until match
// C            =              out tok      L
// N                           out tok      L

void mylex(void){
    int oldline;
    static int oldtok;

    switch(lexstate){
    case LEX_LEX:
        oldline = line;
        mylex2();
        // order of decision is important
        if( tok == tkEOF ){
            tok = tkSEMI;
            lexstate = LEX_EOF;
        }else if( line != oldline ){	// new line
            oldtok = tok;
            if( tokcol == topcol() ){		// out ;
                tok = tkSEMI;
                lexstate = LEX_NEW;
            }else if( tokcol > topcol() ){	// out {
                tok = tkLBRACE;
                pushcol();
                lexstate = LEX_NEW;
            }else{				// tokcol < top, out ; }
                tok = tkSEMI;
                lexstate = LEX_BACK;
            }
        }						// old line, out tok
        break;
    case LEX_NEW:
        tok = oldtok;			// out tok
        lexstate = LEX_LEX;
        break;
    case LEX_BACK:
        if( tokcol < topcol() ){
            tok = tkRBRACE;		// out } until match
            popcol();
        }else{
            tok = oldtok;
            lexstate = LEX_LEX;
        }
        break;
    case LEX_EOF:
        if( colstkp > 1 ){		// out } until match
            tok = tkRBRACE;
            popcol();
        }else
            tok = tkEOF;
        break;
    }
}

// to initialise and prime lex
void initlex(void){
    mylex2();
    pushcol();
}

void readinfile(char *fname) {
	FILE *fp;
	int n;
	fp = fopen(fname,"rt");
	if( fp == NULL ) {
		printf("cannot open : %s\n",fname);
		exit(-1);
	}
	n = fread(inbuf,1,MAXFIN,fp);
	fclose(fp);
	cp = inbuf;
	ieof = cp + n;
	getC();				// start lex
}

/*
PRIVATE char srcbuf[500];

char *getsrc(int line){	// return source line
	char *c;
	int n;
	c = lineat[line];
	if( c == NULL ) return NULL;
	n = strcspn(c,"\n");
	strncpy(srcbuf,c,n);
	srcbuf[n] = 0;
	return srcbuf;
}

void initCharType()
{
	int i;
	for(i=0; i<256; i++) chartype[i] = cBLANK;
	for(i=33; i<128; i++ ) chartype[i] = cSPECIAL;
	for(i='0'; i<='9'; i++ ) chartype[i] = cDIGIT;
	for(i='A'; i<='Z'; i++ ) chartype[i] = cLETTER;
	for(i='a'; i<='z'; i++ ) chartype[i] = cLETTER;
	chartype['_'] = cLETTER;
	chartype['"'] = cQUOTE;
	chartype['#'] = cHASH;
	chartype[EOF_CHAR] = cSPECIAL;
}

void printchartype(){
	int i;
	for(i=0; i<256; i++){
		printf("%d,",chartype[i]);
		if(i%30 == 29) printf("\n");
	}
}
*/

