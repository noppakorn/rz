/* as21.c  a simple assembler for s21

format  (case insensitive)

	.symbol                   define symbol
	symbol value
	 ...
	.code ads                 code segment at ads
	[:label] opcode operand
	 ...
	.data ads				  data segment at ads
	n n ...                   define words
	.end

operand
L-format: op r1 ads
D-format: op r1 r2 disp
X-format: op r1 r2 r3 xop

addressing mode

absolute: op r1 ads
indirect: op r1 @disp r2
index:    op r1 +r2 r3
register:  op r1 r2 r3
immediate: op r1 r2 #n

modify from cpu3 assembler A3  version 1.0  7th September 1999
fix bug : undef op, add listing 			2 Oct 1999
add extended instructions  					28 November 2001
modify from as2								16 Jan 2007

modify  to change order of operand in st	29 Jan 2016
so that it is similar to ld

st r1 ads
st r1 @d r2
st r1 +r2 r3

add interrupt and support instruction       27 Mar 2016
int, reti, savr, resr, savt, rest
clean up code for "grandfather s21" source  14 Jan 2017
add for MOS: pushm, popm, swap				 3 Feb 2017

Prabhas Chongstitvatana
*/

#include "as21.h"

char 	lbuf[MXBUF];				// copy of input
char 	ibuf[MXBUF], *cp = NULL;   	// input buffer, ch ptr
char	ob[MXBUF];					// output buffer
char	cbuf[MXBUF];				// output buffer for listing

static 	char sep[] = " \t\n";      	// separator char
char    *w;							// current input word
int 	lineno,loc;		   			// line num, current ads
FILE 	*fi, *fo, *fl;
token	mem[MXMEM];					// store tokens for pass2
int 	tp = 0;						// token index
int 	mark;						// index to current op token
int		pass;

// initial symbol in the symbol table

typedef struct { char name[8]; int value; int arg; } symtype;

symtype	initsym[] = {
	{"NOP",NOP,NA},{"LD",LD,MEM},{"ST",ST,MEM},{"MOV",MOV,MRI},
	{"JMP",JMP,SP},{"JAL",JAL,ABS},{"RET",RET,SP},
	{"JT",JT,ABS},{"JF",JF,ABS},
	{"ADD",ADD,RRI},{"SUB",SUB,RRI},{"MUL",MUL,RRI},{"DIV",DIV,RRI},
    {"MOD",MOD,RRI},
	{"AND",AND,RRI},{"OR",OR,RRI},{"XOR",XOR,RRI},
	{"EQ",EQ,RRI},{"NE",NE,RRI},{"LT",LT,RRI}, {"LE",LE,RRI},
	{"GT",GT,RRI},{"GE",GE,RRI},
	{"SHL",SHL,RRI},{"SHR",SHR,RRI}, {"NOT",NOT,REG},
	{"TRAP",TRAP,TR},{"PUSH",PUSH,REG},{"POP",POP,REG},
	{"INT",INT,SP},{"RETI",RETI,NA},
	{"PUSHM",PUSHM,SP},{"POPM",POPM,SP},{"XCH",XCH,SP},

	{"R0",0,0},{"R1",1,0},{"R2",2,0},{"R3",3,0},{"R4",4,0},
	{"R5",5,0},{"R6",6,0},{"R7",7,0},{"R8",8,0},{"R9",9,0},
	{"R10",10,0},{"R11",11,0},{"R12",12,0},{"R13",13,0},{"R14",14,0},
	{"R15",15,0},{"R16",16,0},{"R17",17,0},{"R18",18,0},{"R19",19,0},
	{"R20",20,0},{"R21",21,0},{"R22",22,0},{"R23",23,0},{"R24",24,0},
	{"R25",25,0},{"R26",26,0},{"R27",27,0},{"R28",28,0},{"R29",29,0},
	{"R30",30,0},{"R31",31,0},
	{"",0,0}
};

void error(char *s){
	if(pass == 1)
		printf("pass 1 line %d error: %s symbol %s\n%s",lineno,s,w,lbuf);
	else
		printf("pass 2 line %d error: %s\n",mem[tp].line,s);
	exit(0);
}

char* strupr(char* s) {
    char* tmp = s;
    for (;*tmp;++tmp) {
        *tmp = toupper((unsigned char) *tmp);
    }
    return s;
} 

// get one token from input
char *tok(void){
	if( cp != NULL ) cp = strtok(NULL,sep);
	while ( cp == NULL || eqs(cp,";") ){
		if( fgets(ibuf,MXBUF,fi) == NULL ) return NULL;
		strcpy(lbuf,ibuf);
		lineno++;
		cp = strtok(ibuf, sep);
	}
	cp = strupr(cp);   // convert to uppercase
	return cp;
}
// store token
void store(char type, char mode, int ref){
	if( tp >= MXMEM ) error("out of memory");
	mem[tp].type = type;
	mem[tp].mode = mode;
	mem[tp].ref = ref;
	mem[tp].line = lineno;
	tp++;
}

// check number
int isnumber_(char *s){
	int i = 0;
	if(s[0] == '-') i = 1;
	while( isdigit(s[i]) ) i++;
	if( s[i] == 0 ) return 1;
	return 0;
}

int getNum(void){
	w = tok();
	if(isnumber_(w))	return atoi(w);
	error("expect number");
	return 0;
}

// set mode of op token
void setop(char mode){
	mem[mark].mode = mode;
}

int valueof(char *s, int *type){
	*type = NUM;
	if( isnumber_(s) ) return atoi(s);
	*type = SYM;
	return putsym(s);
}

// read reg, mode RR
void readR(void){
	int v,type;
	w = tok();
	if(w[0]==':' || w[0]=='@' || w[0]=='+' || w[0]=='#')
		error("expect register or ads argument");
	v = valueof(w,&type);
	store(type,RR,v);
}

void storew(char *s, int mode){
	int v,type;
	v = valueof(s,&type);
	store(type,mode,v);
	setop(mode);
}
// read mem, set mode ABS IND INX
void readM(void){
	w = tok();
	switch( w[0] ) {
	case '@': storew(w+1,IND); readR(); break;
	case '+': storew(w+1,INX); readR(); break;
	default: storew(w,ABS); break;
	}
}
// read reg or imm set mode
void readRI(int imm, int r){
	w = tok();
	if( w[0]==':' || w[0]=='@' || w[0]=='+' )
		error("expect register or immediate argument");
	if( w[0] == '#') storew(w+1,imm);
	else storew(w,r);
}

// read imm
void readI(void){
	w =tok();
	if( w[0] == '#' ) storew(w+1,IMM);
	else  error("expect immediate argument");
}

// read and store ex. instruction in mode
void readOP(int mode){
	w = tok();
	storew(w,mode);
}
// read argument according to OP
void readcode(int op){
	int v, type;
	mark = tp;
	store(OP,NA,op);
	switch( op ) {
    case NOP:   		// zero arg
	case RETI: 	break;
	case LD:
	case ST: readR(); readM(); break;
	case JMP:			// one arg
	case RET: readR(); setop(SP); break;
	case JAL:			// two arg
	case JT:
	case JF: readR(); readR(); setop(ABS); break;
	case ADD:			// three arg
	case SUB:
	case MUL:
	case DIV:
	case MOD:
	case AND:
	case OR:
	case XOR:
	case EQ:
	case NE:
	case LT:
	case LE:
	case GT:
	case GE:
	case SHL:
	case SHR: readR(); readR(); readRI(RI,RR); break;
	case MOV: readR(); readRI(IMM,REG); break;
	case NOT:			// two regs
	case PUSH:
	case POP:  readR(); readR(); setop(REG); break;
	case TRAP: readR(); readI(); setop(TR); break;
	case INT:  readI(); setop(SP); break;
	case PUSHM:
	case POPM:
	case XCH: readR(); setop(SP); break;

	default: error("undefine op");
	}
}
// check arguments of current op
// type is the specified type, mode is the actual arg
void chkarg(int type){
	int mode, f;
	mode = mem[mark].mode;
	f = 0;
	switch(type){
	case MEM: f = (mode==ABS) || (mode==IND) || (mode==INX); break;
	case RRI: f = (mode==RR)  || (mode==RI); break;
	case MRI: f = (mode==IMM) || (mode==REG);  break;
	case ABS: f = mode == ABS; break;
	case RI:  f = mode == RI; break;
	case RR:  f = mode == RR; break;
	case SP:  f = mode == SP; break;
	case TR:  f = mode == TR; break;
	case REG: f = mode == REG; break;
	case NA:  f = mode == NA; break;
	}
	if( !f ) error("incorrect argument type");
}

void pass1(void){
	int idx, state;
	pass = 1;
	state = 'S';
	w = tok();
	while( !eqs(w,".END") ) {
		if( eqs(w,".SYMBOL")){
			state = 'S';
		}else if(eqs(w,".DATA")){
			loc = getNum();
			store(DOTD,NA,loc);
			state = 'D';
		}else if(eqs(w,".CODE")){
			loc = getNum();
			store(DOTC,NA,loc);
			state = 'C';
		}else
		switch(state){
		case 'S':
			idx = putsym(w);
			if(getValue(idx) != UNDEF)
				error("duplicate symbol");
			setsym(idx,getNum(),SYM,NA);
			break;
		case 'D':
			if( isnumber_(w) )
				store(NUM,NA,atoi(w));
			else
				store(SYM,NA,putsym(w));
			loc++;
			break;
		case 'C':
			if( w[0] == ':' ){	// insert symbol
				idx = putsym(w+1);
				if(getValue(idx) != UNDEF)
					error("duplicate label");
				setsym(idx,loc,SYM,NA);
			}else{
				idx = searchsym(w);
				if(idx < 0 || getType(idx) != OP)
					error("undefined op");
				readcode(getValue(idx));
				chkarg(getArg(idx));
				loc++;
			}
		} // switch
		w = tok();
	}  // while
	store(DOTE,NA,loc);
}

int isdot(char type){
	return type == DOTC || type == DOTD || type == DOTE;
}

/*  opcode encoding
LD   ld a 1, ld d 2, ld x 31 17
ST 	 st a 3, st d 4, st x 31 18
MOV  mvi 5, mov r 31 16
JMP	 jmp 6
JAL  jal 7
JT   jt 8
JF   jf 9

ADD	10 add i 10, add r 31 0
SUB	11 sub i 11, sub r 31 1
MUL	12 mul i 12, mul r 31 2
DIV	13 div i 13, div r 31 3
AND	14 and i 14, and r 31 4
OR  15 or  i 15, or  r 31 5
XOR 16 xor i 16, xor r 31 6
EQ  17 ...
NE  18
LT  19
LE  20
GT  21
GE  22
SHL 23
SHR 24
MOD 25 mod i 25, mod r 31 15

RET	  30   ret   31 19		ret r
TRAP  31   trap  31 20		trap r #n
PUSH  32   push  31 21		push r r
POP   33   pop   31 22		pop r r
NOT   34   not   31 23		not r r
INT   35   int   31 24		int #n
RETI  36   reti  31 25		reti
PUSHM 37   pushm 31 26      pushm sp
POPM  38   popm  31 27      popm sp
XCH   39   xch   31 28      xch r

*/

int rdtokval(void){
	int v;
	v = UNDEF;
	switch( mem[tp].type ){
	case NUM: v = mem[tp].ref; break;
	case SYM: v = getValue(mem[tp].ref);
	}
	if(v == UNDEF) error("undefined symbol");
	tp++;
	return v;
}

void readarg(int mode, int *a1, int *a2, int *a3){
	*a1 = rdtokval();	// read one arg
	switch( mode ){
	case SP: break;		// one arg
	case ABS:			// two arg
	case IMM:
	case REG:
	case TR: *a2 = rdtokval(); break;
	case INX:			// three arg
	case IND:
	case RI:
	case RR: *a2 = rdtokval(); *a3 = rdtokval(); break;
	default: error("unknow addressing mode");
	}
}

PRIVATE void prL(int op, int a1, int a2){
	sprintf(ob,"L %d %d %d",op,a1,a2);
}
PRIVATE void prD(int op, int a1, int a2, int a3){
	sprintf(ob,"D %d %d %d %d",op,a1,a2,a3);
}
PRIVATE void prX(int xop, int a1, int a2, int a3){
	sprintf(ob,"X 31 %d %d %d %d",a1,a2,a3,xop);
}

void gencode(void){
	int op, op2, mode, a1, a2, a3;
	mode = mem[tp].mode;
	op = mem[tp].ref;
	tp++;
	if( mode != NA)
		readarg(mode,&a1,&a2,&a3);
	switch( op ){
	case NOP: prL(0,0,0); break;
	case LD:
		switch( mode ){
		case ABS: prL(1,a1,a2); break;
		case IND: prD(2,a1,a3,a2); break;
		case INX: prX(17,a1,a2,a3); break;
		}
		break;
	case ST:
		switch( mode ){
		case ABS: prL(3,a1,a2); break;
		case IND: prD(4,a1,a3,a2); break;
		case INX: prX(18,a1,a2,a3); break;
		}
		break;
	case JMP: prL(6,0,a1); break;
	case JAL:
	case JT:
	case JF:  prL(op,a1,a2); break;
	case RET: prX(19,a1,0,0); break;
	case ADD:
	case SUB:
	case MUL:
	case DIV:
	case MOD:
	case AND:
	case OR:
	case XOR:
	case EQ:
	case NE:
	case LT:
	case LE:
	case GT:
	case GE:
	case SHL:
	case SHR:
		switch( mode ){
		case RI: prD(op,a1,a2,a3); break;
		case RR: prX(op-10,a1,a2,a3); break;
		}
		break;
	case MOV:
		switch( mode ){
		case IMM: prL(5,a1,a2); break;
		case REG: prX(16,a1,a2,0); break;
		}
		break;
	case TRAP:  prX(20,a1,a2,0); break;
	case PUSH:  prX(21,a1,a2,0); break;
	case POP:   prX(22,a1,a2,0); break;
	case NOT:   prX(23,a1,a2,0); break;
	case INT:   prX(24,a1,0,0);  break;
	case RETI:  prX(25,0,0,0);  break;
	case PUSHM: prX(26,a1,0,0); break;
	case POPM:  prX(27,a1,0,0); break;
	case XCH:   prX(28,a1,0,0); break;

	default: error("undefine op");
	}
}

void outobj(void){
	fprintf(fo,"%s\n",ob);
}

void outlst(void){
	int i, line;
	line = mem[tp-1].line;
	// get source until before current line
	while(lineno < (line-1)){
		fgets(lbuf,MXBUF,fi);
		fprintf(fl,"                        %s",lbuf);
		lineno++;
	}
	fgets(lbuf,MXBUF,fi);	// current line
	lineno++;
	sprintf(cbuf,"%4d %s",loc,ob);
	for(i=strlen(cbuf); i<24; i++)	// pad blank to 24
		cbuf[i] = 32;
	strcpy(cbuf+24,lbuf);
	fprintf(fl,"%s",cbuf);
}

void pass2(void){
	pass = 2;
	tp = 0;
	lineno = 0;
	rewind(fi);
	while( mem[tp].type != DOTE){
		switch(mem[tp].type){
		case DOTC:
			loc = mem[tp].ref;
			sprintf(ob,"a %d",loc);
			outobj();
			tp++;
			while(!isdot(mem[tp].type)){
				gencode();
				outobj();
				outlst();
				loc++;
			}
			break;
		case DOTD:
			loc = mem[tp].ref;
			sprintf(ob,"a %d",loc);
			outobj();
			tp++;
			while(!isdot(mem[tp].type)){
				sprintf(ob,"w %d",rdtokval());
				outobj();
				loc++;
			}
		}
	}
	sprintf(ob,"e");
	outobj();
	// list rest of the source
	while(fgets(lbuf,MXBUF,fi) != NULL)
		fprintf(fl,"                        %s",lbuf);
}

// put reserved words into symbol table
void initsymbol(void){
	int i;
	for(i=0; initsym[i].name[0] != 0; i++)
		setsym(putsym(initsym[i].name),
			initsym[i].value,OP,initsym[i].arg);
}

// make an obj file name from source
void makename( char *source, char *obj, char *lis ){
	int n;
	n = strcspn(source,".");
	strncpy(obj,source,n);
	strcpy(obj+n,".obj");
	strncpy(lis,source,n);
	strcpy(lis+n,".lis");
}

int main(int argc, char *argv[]){
	char fout[80], flis[80];

	if( argc < 2 ) {
		printf("usage : as21 inputfile\n");
		exit(0);
	}
	fi = fopen(argv[1],"r");
	if( fi == NULL ){
		printf("input file not found\n");
		exit(0);
	}
	makename(argv[1], fout, flis);
	initsymbol();
	lineno = 0;
	loc = 0;
	pass1();
	fl = fopen(flis,"w");
	fo = fopen(fout,"w");
	pass2();
	fclose(fi);
	fclose(fo);
	fclose(fl);
}

/*----------- for debugging


void testread(void){
	int cnt = 100;
	w = tok();
	while ( !eqs(w,".END") && (w != NULL) ){
		printf("%s ",w);
		w = tok();
		if(cnt-- < 0) break;
	}
}

char prmode( int mode){
	switch( mode ){
	case ABS: return 'A';
	case DISP:return '@';
	case INX:return '+';
	case IMM:return '%';
	case RI:return '#';
	case RR:return 'R';
	case SP:return 'S';
	case NA:return '?';
	case EL: return 'L';
	case ED: return 'D';
	case EX: return 'X';
	}
	return 0;
}

void prtoken(int type, int mode, int ref){
	switch( type ){
	case SYM: printf("sym %c %d\n",prmode(mode),ref); break;
	case NUM: printf("num %c %d\n",prmode(mode),ref); break;
	case OP: printf("op %c %d\n",prmode(mode),ref); break;
	case DOTA: printf(".a %d\n",ref); break;
	case DOTC: printf(".c %d\n",ref); break;
	case DOTW: printf(".w %d\n",ref); break;
	case DOTE: printf(".e\n"); break;
	}
}

void dumptoken(){
	int i;
	for(i=0;i<tp;i++)
		prtoken(mem[i].type,mem[i].mode,mem[i].ref);
}

--------------- */
