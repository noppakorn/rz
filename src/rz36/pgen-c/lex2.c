// experiment with lex in vm
// incremental read without hard limit on input file size
// 28 Aug 2009
// recode from token-s.txt, it is based on fgets which is simpler

//#include "compile.h"
#include "pgen.h"

#define MAXBUF      1000        // input buffer size
#define eol         10
#define eof         -1
#define isSpace(c)  (int)spaceQ[((int)(c))&255]
#define isHex(c)    (int)hexQ[((int)(c))&255]
#define isAlNum(c)  (int)alnumQ[((int)(c))&255]

//#define tokstring_ads	1		// system area in vm
//#define tokvalue_ads	102
//#define tokcol_ads		103
//#define line_ads		104

extern FILE *FO, *FI;			// output, input file

char spaceQ[256] = {
0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,0};

char hexQ[256] = {
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

// 0..9 A..F a..f, start at 48..102
int hexval[] = {
0,1,2,3,4,5,6,7,8,9,0,0,0,0,0,0,0,10,11,12,13,14,15,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
10,11,12,13,14,15};

char alnumQ[256] = {
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,0,0,0,0,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

char buf[MAXBUF+5];	            // input buffer, one line
int TP = 0;                     // current token pointer
int line = 0;
char tokstring[100];			// current token
int tokvalue;

void initlex(void){
	TP = 0;
	line = 0;
	buf[0] = 0;
}

// leave TP after the end of the current token
// return token value (tok) and tokstring[.]
// base on token-s.txt > lex1
int lex(FILE *fi){
	char *t, *p, c, d;
	int pos, tok, v, tokcol;
	char mess[80];
	// skip blank, if input is empty, read a line
//	line = M[line_ads];
	pos = TP;
	while( isSpace(buf[pos]))
		pos++;
	// while isEmpty or isComment, read and skip space
	while(buf[pos] == 0 || (buf[pos] == '/' && buf[pos+1] == '/')){
		p = fgets(buf,MAXBUF,fi);	    // read one line
		if(fi == stdin)
			buf[strlen(buf)] = eof;		// pad EOF at end
		else if( p == NULL )
			buf[0] = eof;
		line++;
		pos = 0;
		while( isSpace(buf[pos]) )		// 0 is not space!
			pos++;
	}
	c = buf[pos];				// current char
	d = buf[pos+1];				// lookahead
	tokcol = pos;
	pos++;						// pos is now at d

	// collect string
	tokstring[0] = 0;			// clear tokstring
	t = tokstring;
	switch( c ){
	case eof:
		buf[pos] = 0;			// force read next time
		tok = tkEOF;
		break;
	// num and hex
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
		v = c - 48;
		while( d >= '0' && d <= '9' ){
			v = v*10 + d - 48;
			pos++;
			d = buf[pos];
		}
		tokvalue = v;
		tok = tkNUMBER;
		break;
	case '#':
		v = 0;
		if( isHex(d) ){					// first digit
			v = hexval[d - 48];
			if( v > 7 ) v = v - 16;		// sign extension
			pos++;
			d = buf[pos];
		}else{
			sprintf(mess,
				"lex at line %d col %d, expect number",
				line,pos);
			seterror(mess);
		}
		while( isHex(d) ){
			v = v*16 + hexval[d - 48];
			pos++;
			d = buf[pos];
		}
		tokvalue = v;
		tok = tkNUMBER;
		break;
	// single
	case '+': tok = tkPLUS; break;
	case '-': tok = tkMINUS; break;
	case '*': tok = tkSTAR; break;
	case '/': tok = tkSLASH; break;
	case '&': tok = tkAND; break;
	case '|': tok = tkBAR; break;
	case '^': tok = tkCARET; break;
	case '%': tok = tkMOD; break;
	case '(': tok = tkLPAREN; break;
	case ')': tok = tkRPAREN; break;
	case '{': tok = tkBB; break;
	case '}': tok = tkBE; break;
	case '[': tok = tkLBRACKET; break;
	case ']': tok = tkRBRACKET; break;
	case ':': tok = tkCOLON; break;
	// double
	case '!':
		if( d == '='){
			pos++;
			tok = tkNE;
		}else
			tok = tkNOT;
		break;
	case '=':
		if( d == '='){
			pos++;
			tok = tkEQEQ;
		}else
			tok = tkEQ;
		break;
	case '<':
		if( d == '<'){
			pos++;
			tok = tkLTLT;
		}else if( d == '='){
			pos++;
			tok = tkLE;
		}else
			tok = tkLT;
		break;
	case '>':
		if( d == '>'){
			pos++;
			tok = tkGTGT;
		}else if( d == '='){
			pos++;
			tok = tkGE;
		}else
			tok = tkGT;
		break;
	case '"':
		while( d != '"' && d != 0 ){
			*t = d;
			t++;
			pos++;
			d = buf[pos];
		}
		if( d == 0 ){
			sprintf(mess,
			"lex at line %d col %d, expect \" at the end of string",
			line,pos);
			seterror(mess);
		}
		pos++;					// skip "
		*t = 0;
//		strpack(tokstring_ads,tokstring);
		tok = tkSTRING;
		break;
	default:
		*t = c;
		t++;
		while( isAlNum(d) ){
			*t = d;
			t++;
			pos++;
			d = buf[pos];
		}
		*t = 0;					   // terminate
//		strpack(tokstring_ads,tokstring);
		tok = tkIDEN;
		break;
	}
	TP = pos;
//	M[tokcol_ads] = tokcol;
//	M[line_ads] = line;
//	M[tokvalue_ads] = v;
	return tok;
}

char tokStr[][4] = {
	"*", "/", "-", "+", "=", "==", "&", "|",
	"^", "%", "!", "!=", "<", "<=", "<<", ">", ">=", ">>",
	":", "(", ")", "[", "]", "{", "}"
};

void prToken(int tk){
	switch(tk){
	case tkIDEN: printf("%s ",tokstring); break;
	case tkSTRING: printf("%c%s%c ",34,tokstring,34); break;
	case tkNUMBER: printf("%d ",tokvalue); break;
	default:
		if (tk >= 50 && tk <= 74){
			printf("%s ",tokStr[tk-50]);
		}
	}
}
/*
void testlex(void){
	int i, oldline;
	int tok;
	FI = fopen("bubble.txt","r");
	TP = 0; line = 0; buf[0] = 0;
	tok = lex();
	while( tok != tkEOF ){
		if( oldline != line ){
			printf("\n");
			for(i=1; i<tokcol; i++) printf(" ");
		}
		prToken(tok);
		oldline = line;
		tok = lex();
	}
	fclose(FI);
	printf("\nline %d\n",line);
}

void initlex(void){
	int i;
	for(i=0; i<255; i++) spaceQ[i] = 1;
	for(i=33; i<127; i++) spaceQ[i] = 0;
	spaceQ[255] = 0;            // eof
	TP = 0;
	printf("spaceQ[] = {\n");
	for(i=0; i<255; i++){
		printf("%d,",spaceQ[i]);
		if( (i % 40) == 0) printf("\n");
	}
}

void initlex(void){
	int i;
	for(i=0; i<255; i++) hexQ[i] = 0;
	for(i=48; i<=57; i++) hexQ[i] = 1;
	for(i=65; i<=70; i++) hexQ[i] = 1;
	for(i=97; i<=102; i++) hexQ[i] = 1;
	printf("hexQ[] = {\n");
	for(i=0; i<255; i++){
		printf("%d,",hexQ[i]);
		if( (i % 40) == 0) printf("\n");
	}
}

void initlex(void){
	int i;
	for(i=0; i<255; i++) alnumQ[i] = 0;
	for(i='0'; i<='9'; i++) alnumQ[i] = 1;
	for(i='A'; i<='Z'; i++) alnumQ[i] = 1;
	for(i='a'; i<='z'; i++) alnumQ[i] = 1;
	alnumQ['_'] = 1;
	printf("alnumQ[] = {\n");
	for(i=0; i<=255; i++){
		printf("%d,",alnumQ[i]);
		if( (i % 40) == 0) printf("\n");
	}
}

void initlex(void){
	int i;
	for(i=48; i<=102; i++) hexval[i] = 0;
	for(i='0'; i<='9'; i++) hexval[i] = i-48;
	for(i='A'; i<='F'; i++) hexval[i] = i-55;
	for(i='a'; i<='f'; i++) hexval[i] = i-87;
	printf("hexval[] = {\n");
	for(i=48; i<=102; i++){
		printf("%d,",hexval[i]);
		if( (i % 30) == 0) printf("\n");
	}
}
*/
// end

