/*  load.c

	for S2 cpu simulator
	5 December 2001
	modify from s1 sim 1997, 1998

	supporting function

	P. Chongstitvatana
	Department of Computer Engineering
	Chulalongkorn University

		add print, printc  to output buffer		31 Dec 2001
		redo for teaching interrupt programming  8 Nov 2016
		update and clean up s21					24 Jan 2017
*/

#include "s21.h"

int clock, ninst;
FILE *fp;
int ip;

extern int R[32], Ir, Pc, M[], RetAds;
extern int runflag, savePc, ninst, lastInt;

PRIVATE char ob[256];	// output buffer

void error(char *s){
	printf("error: %s at %d\n",s,Pc);
	exit(0);
}

PRIVATE int signx( int d ){		// sign bit 16 extended
	if( d & 0x010000 ) return d | 0xFFFE0000;
	return d;
}

// IR bits
int IRop(void)  { return((Ir >> 27) & 0x01F);} // bit 31..27
int IRr1(void)  { return((Ir >> 22) & 0x01F);} // bit 26..22
int IRads(void) { return( Ir & 0x003FFFFF  );} // bit 21..0
int IRr2(void)  { return((Ir >> 17) & 0x01F);} // bit 21..17
int IRr3(void)  { return((Ir >> 12) & 0x01F);} // bit 16..12
int IRxop(void) { return( Ir & 0x0FFF      );} // bit 11..0
int IRdisp(void){ return(signx(Ir & 0x01FFFF));} // bit 16..0


// -----load object code from file -----

// get one charactor skip newline and blank
PRIVATE char geta(void){
	char s[32];
	fscanf(fp,"%s",s);
	return s[0] ;
}

// get one integer
PRIVATE int geti(void){
	int c;
	fscanf(fp,"%d,",&c);
	return c;
}

// read one line
char *fgetline(void){
	static char buff[200],*cp,c;
	cp = buff;
	c = fgetc(fp);
	while(c != '\n'){
		*cp = c;
		cp++;
		c = fgetc(fp);
	}
	return buff;
}

void assemble(int ip, int a1, int a2, int a3, int a4){
	if(a1==1 || a1==3 || a1==5 || a1==7 ||   	// form L 22-bit
		a1==8 || a1==9 ){
		M[ip] = a1<<27 | a2<<22 | (a4&0x3FFFFF);
	}else if( a1==2 || a1==4 || ((10<=a1) && (a1<=25)) ){ // D 17-bit
		M[ip] = a1<<27 | a2<<22 | a3<<17 | (a4&0x1FFFF);
	}else if( (32<=a1) && (a1<=55) ){					  // X
		M[ip] = 0xF8000000 | a2<<22 | a3<<17 | a4<<12 | (a1-32);
	}else
		printf("error: op code undefine\n");
}

void disassem(void);

// maximum size 20000
void loadprogram(char *name){
	int i, cnt, a1, a2, a3, a4, a5;

	fp = fopen(name,"r");
	if(fp == NULL){
		printf("input file not found");
		exit(0);
	}
	ip = 0;
	for(cnt=0; cnt<20000; cnt++ ) {
		switch( geta() ) {
		case 'a': ip = geti(); break;			// set address
		case 'L':
			a1 = geti(); a2 = geti(); a3 = geti();
			M[ip] = a1<<27 | a2<<22 | (a3&0x3FFFFF);
			ip++;
			break;
		case 'D':
			a1 = geti(); a2 = geti(); a3 = geti(); a4 = geti();
			M[ip] = a1<<27 | a2<<22 | a3<<17 | (a4&0x1FFFF);
			ip++;
			break;
		case 'X':
			a1=geti(); a2=geti(); a3=geti(); a4=geti(); a5=geti();
			M[ip] = a1<<27 | a2<<22 | a3<<17 | a4<<12 | (a5&0x0FFF);
			ip++;
			break;
		case 'w': M[ip] = geti(); ip++; break;	// set constant data
		case 'e': cnt = 200001;	break;			// end
		default: error("incorrect object file");
		}
	}

	fclose(fp);
	printf("load program, last address %d\n",ip);
}

PRIVATE void pr3R(char *s){
	sprintf(ob,"%s r%d r%d r%d",s,IRr1(),IRr2(),IRr3());
}
PRIVATE void prI(char *s) {
	sprintf(ob,"%s r%d r%d #%d",s,IRr1(),IRr2(),IRdisp());
}
PRIVATE void prA(char *s) {
	sprintf(ob,"%s r%d %d",s,IRr1(),IRads());
}
PRIVATE void pr2R(char *s){
	sprintf(ob,"%s r%d r%d",s,IRr1(),IRr2());
}
PRIVATE void pr1R(char *s){
	sprintf(ob,"%s r%d",s,IRr1());
}

void disassem(void){
	switch( IRop() ) {
	case NOP: sprintf(ob,"nop"); break;
	case LDA: prA("ld"); break;
	case LDD: sprintf(ob,"ld r%d @%d r%d",IRr1(),IRdisp(),IRr2()); break;
	case STA: prA("st"); break;
	case STD: sprintf(ob,"st r%d @%d r%d",IRr1(),IRdisp(),IRr2()); break;
	case JMP: sprintf(ob,"jmp %d",IRads()); break;
	case JAL: prA("jal"); break;
	case JT:  prA("jt"); break;
	case JF:  prA("jf"); break;
	case MVI: sprintf(ob,"mov r%d #%d",IRr1(),signx2(IRads())); break;
	case ADDI:prI("add"); break;
	case SUBI:prI("sub"); break;
	case MULI:prI("mul"); break;
	case DIVI:prI("div"); break;
	case ANDI:prI("and"); break;
	case ORI: prI("or"); break;
	case XORI:prI("xor"); break;
	case EQI: prI("eq"); break;
	case NEI: prI("ne"); break;
	case LTI: prI("lt"); break;
	case LEI: prI("le"); break;
	case GTI: prI("gt"); break;
	case GEI: prI("ge"); break;
	case SHLI:prI("shl"); break;
	case SHRI:prI("shr"); break;
	case MODI:prI("mod"); break;
	case XOP:
		switch(IRxop()){
		case ADD: pr3R("add"); break;
		case SUB: pr3R("sub"); break;
		case MUL: pr3R("mul"); break;
		case DIV: pr3R("div"); break;
		case AND: pr3R("and"); break;
		case OR:  pr3R("or"); break;
		case XOR: pr3R("xor"); break;
		case EQ:  pr3R("eq"); break;
		case NE:  pr3R("ne"); break;
		case LT:  pr3R("lt"); break;
		case LE:  pr3R("le"); break;
		case GT:  pr3R("gt"); break;
		case GE:  pr3R("ge"); break;
		case SHL: pr3R("shl"); break;
		case SHR: pr3R("shr"); break;
		case MOD: pr3R("mod"); break;
		case MOV: pr2R("mov"); break;
		case PUSH:pr2R("push"); break;
		case POP: pr2R("pop"); break;
		case LDX: sprintf(ob,"ld r%d +r%d r%d",IRr1(),IRr2(),IRr3()); break;
		case STX: sprintf(ob,"st r%d +r%d r%d",IRr1(),IRr2(),IRr3()); break;
		case RET: pr1R("ret"); break;
		case TRAP:sprintf(ob,"trap r%d #%d",IRr1(),IRr2()); break;
		case NOT: pr2R("not"); break;
		case INT: sprintf(ob,"int #%d",IRr1()); break;
		case RETI:sprintf(ob,"reti"); break;
		case PUSHM: pr1R("pushm"); break;
		case POPM: pr1R("popm"); break;
		case XCH: pr1R("xch"); break;

		default: sprintf(ob,"unknown op code");
		}
		break;
	default: sprintf(ob,"unknown op code");
	}
}

void show(void){		// show some registers
	int i;

	printf("PC%4d ",savePc);
	disassem();
	for(i=strlen(ob); i<18; i++)	// pad blank to 18
		ob[i] = 32;
	ob[i] = 0;
	printf("%s",ob);
	for(i=0;i<10;i++)
		printf("r%d:%d ",i,R[i]);
	printf("\n\t\t\t ");
	for(i=27;i<32;i++)
		printf("r%d:%d ",i,R[i]);
	printf("\n");

}
