
/* S2 cpu micro architecture simulator

   5 December 2001
   s21  18 Jan 2007

   add address trace (Chinese new year)		  10 Feb 2016
   add interrupt 							  27 Mar 2016
   redo for teaching interrupt programming    8 Nov 2016
   add io  									  19 Nov 2016
   update to "grandfather s21"	(Chinese NY)  28 Jan 2017
   add: readinputstring 					   1 Feb 2017

   P. Chongstitvatana
   Department of Computer Engineering
   Chulalongkorn University

*/

#include "s21.h"

extern int timer0range, timer1range;
extern int display;

int R[32], Ir, Pc, M[MAXMEM], RetAds;
int intflag, intmask[4], intrq[4], intnum;
int runflag, savePc, cpuflag;
int heapfree = HEAP;

//extern char *cp2;		// output buffer

int signx2( int d ){	// sign bit 21 extended
	if( d & 0x0200000 ) return d | 0xFFC00000;
	return d;
}

char buf[80];		// input buffer

void trap( int reg, int num ){	// special function
	int n, i, x, y;
	switch( num ) {
	case 0:  // stop
		runflag = 0;
		printf("stop, clock %d, execute %d instructions\n",clock,ninst);
		break;
	case 1: // print integer
		printf("%d ",R[reg]);
		break;
	case 2: // printc
		printf("%c",R[reg]);
		break;
	case 3: // printstring
		i = 0;
		x = M[STRSEG+i];
		while( x != 0 ){		// convert Rz string to c string
			buf[i] = (char)x;
			i++;
			x = M[STRSEG+i];
		}
		printf("%s",buf);
		break;
	case 4: // input (return string)
		scanf("%s",buf);
		i = 0;
		while( buf[i] != 0 ){		// copy to M[]
			M[STRSEG+i] = buf[i];
			i++;
		}
		M[STRSEG+i] = 0;		// terminate string
//		printf("%s\n",buf);
		R[RETV] = STRSEG;		// return pointer to string
		break;

//	case 13: timer0range = R[reg]; break; // set timer0
//	case 14: timer1range = R[reg]; break; // set timer1

	case 15: 			// disable int
		n = R[reg];
		intmask[n] = 0;
		break;
	case 16: 			// enable int
		n = R[reg];
		intmask[n] = 1;
		break;
	case 17: cpuflag = 0; break;	// put cpu to sleep
/*
	case 18:
		n = R[reg];				// port number
//		printf("read port %d\n",n);
		switch(n){
		case 10: R[RETV] = readsinport(); break; // sin analog
		case 11: R[RETV] = readdigport(); break; // square
		case 12: R[RETV] = myrandom(); break;	 // random num
		case 13: R[RETV] = clock; break;		 // reads clock
		}
		break;
*/
	case 19:			// malloc
		R[RETV] = heapfree;
		heapfree += R[reg];
		if( heapfree > MAXMEM )
			error("malloc: out of memory");
		break;
	}
}

void interrupt(int n){
	if( n > 3 )
		error("unknown interrupt");
//	printf("interrupt%d\n",n);
	intnum = n;
	RetAds = Pc;		// save pc
	Pc = M[INTVEC+n];
	cpuflag = 1; 		// wake up sleepy cpu
	intflag = 0;		// master disable interrupt
	intrq[n] = 0;		// clear int request
}

void run(void){		// execute one instruction
	int ads, d, r1, r2, r3, i;

	savePc = Pc;
	Ir = M[Pc];		// instruction fetch
	Pc++;
	ads = IRads();	// decode
	d = IRdisp();	// signx  17-bit
	r1 = IRr1();
	r2 = IRr2();
	r3 = IRr3();
	ninst++;
	switch( IRop() ) {
	case NOP:  break;
	case LDA:  R[r1] = M[ads]; break;
	case LDD:  R[r1] = M[R[r2]+d];  break;
	case STA:  M[ads] = R[r1];  break;
	case STD:  M[R[r2]+d] = R[r1];  break;
	case MVI:  R[r1] = signx2(ads); break;
	case JMP:  Pc = ads; break;
	case JAL:  R[r1] = Pc; Pc = ads; break;
	case JT: if( R[r1] != 0 ) Pc = ads; break;
	case JF: if( R[r1] == 0 ) Pc = ads; break;
	case ADDI: R[r1] = R[r2] + d; break;
	case SUBI: R[r1] = R[r2] - d; break;
	case MULI: R[r1] = R[r2] * d; break;
	case DIVI: R[r1] = R[r2] / d; break;
	case ANDI: R[r1] = R[r2] & d; break;
	case ORI:  R[r1] = R[r2] | d; break;
	case XORI: R[r1] = R[r2] ^ d; break;
	case EQI:  R[r1] = R[r2] == d; break;
	case NEI:  R[r1] = R[r2] != d; break;
	case LTI:  R[r1] = R[r2] <  d; break;
	case LEI:  R[r1] = R[r2] <= d; break;
	case GTI:  R[r1] = R[r2] >  d; break;
	case GEI:  R[r1] = R[r2] >= d; break;
	case SHLI: R[r1] = R[r2] << d; break;
	case SHRI: R[r1] = R[r2] >> d; break;
	case MODI: R[r1] = R[r2] %  d; break;
	case XOP:
		switch( IRxop() ){
		case ADD:  R[r1] = R[r2] + R[r3]; break;
		case SUB:  R[r1] = R[r2] - R[r3]; break;
		case MUL:  R[r1] = R[r2] * R[r3]; break;
		case DIV:  R[r1] = R[r2] / R[r3]; break;
		case AND:  R[r1] = R[r2] & R[r3]; break;
		case OR:   R[r1] = R[r2] | R[r3]; break;
		case XOR:  R[r1] = R[r2] ^ R[r3]; break;
		case EQ:   R[r1] = R[r2] == R[r3]; break;
		case NE:   R[r1] = R[r2] != R[r3]; break;
		case LT:   R[r1] = R[r2] <  R[r3]; break;
		case LE:   R[r1] = R[r2] <= R[r3]; break;
		case GT:   R[r1] = R[r2] >  R[r3]; break;
		case GE:   R[r1] = R[r2] >= R[r3]; break;
		case SHL:  R[r1] = R[r2] << R[r3]; break;
		case SHR:  R[r1] = R[r2] >> R[r3]; break;
		case MOD:  R[r1] = R[r2] %  R[r3]; break;
		case MOV:  R[r1] = R[r2]; break;
		case LDX:  R[r1] = M[R[r2] + R[r3]]; break;
		case STX:  M[R[r2] + R[r3]] = R[r1]; break;
		case RET:  Pc = R[r1]; break;
		case TRAP: trap(r1,r2); break;
		case PUSH:
			R[r1]++;			// increment stack pointer
			M[R[r1]] = R[r2];
			break;
		case POP:
			R[r2] = M[R[r1]];
			R[r1]--;			// decrement stack pointer
			break;
		case NOT:  R[r1] = (R[r2] == 0) ? ~0 : 0;  break;
		case INT:  interrupt(r1); break;
		case RETI:
			Pc = RetAds;
			intflag = 1;	// enable interrupt
			break;
		case PUSHM:				// push r0..15 to stack
			for(i = 0; i < 16; i++){
				R[r1]++;		// sp++
				M[R[r1]] = R[i];
			}
			break;
		case POPM:				// pop stack to r15..r0
			for(i = 15; i >= 0; i--){
				R[i] = M[R[r1]];
				R[r1]--;		// sp--
			}
			break;
		case XCH:				// exchange RetAds:R[r1]
			d = RetAds;
			RetAds = R[r1];
			R[r1] = d;
			break;

		default:
			error("undefine xop");
		}
		break;
	default:
		error("undefine op");
	}
//	if( (Ir & 0xFFC00FFF) == 0xFC000013 )	// trap 16, ei
//		 ;     // do not interrupt just after ei
//	else

}

void checkinterrupt(void){
	if( intflag ){
		if( intmask[0] && intrq[0] ) interrupt(0);
		else if( intmask[1] && intrq[1] ) interrupt(1);
		else if( intmask[2] && intrq[2] ) interrupt(2);
		else if( intmask[3] && intrq[3] ) interrupt(3);
	}
}

//  run one instruction and check interrupt
void runoneclock(void){
	clock++;
//	simdevices();
	if( runflag && cpuflag ){
		run();
		if( display ) show();
	}
	checkinterrupt();
}

int main(int argc, char *argv[]){
	if( argc < 2 ) {
		printf("usage : sim21i objfile\n");
		exit(0);
	}
	loadprogram(argv[1]);

	display = 1;
//	init_io();
	clock = 0;
	ninst = 0;
	Pc = 0;
	runflag = 1;
	cpuflag = 1;
	intflag = 1;	// master enable interrupt
	intmask[0] = 1;
	intmask[1] = 0;
	intmask[2] = 0;
	intmask[3] = 0;
	interp();

	printf("stop, clock %d, execute %d instructions\n",clock,ninst);
	return 0;
}

// ---------- example of timer to generate interrupt
/*

int timer;

void simdevices(void){
	timer++;
	timer %= 200;					// interval 200 inst
	if( timer == 0 ) intrq[0] = 1;
}
*/

