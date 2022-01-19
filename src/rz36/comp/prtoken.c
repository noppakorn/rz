// prtoken.c  base code from token-s.txt of som-v5  26 Oct 2010

#include "compile.h"

extern char strbuf[];

// print-string of token
PRIVATE char tok_str[][8] = {
	"*", "/", "-", "+", "=", "==", "&", "&&", "|", "||",
	"!", "!=", "<", "<=", ">", ">=", ",", ";",
	"(", ")", "[", "]",	"{", "}",
	"if", "else", "while", "return", "print",
	"fun", "call", "do", "vec", "funx", "-", "!", "*", "&"
};
// print token
//void prtoken(int tk){
PRIVATE void prtoken(int tk){
    if( tk > tkADS )
        seterror("unknown token");
    else if( tk >= tkSTAR )
        printf("%s",tok_str[tk-tkSTAR]); // 0..32
	else {
		switch(tk){
		case tkIDEN: printf("%s", tokstring); break;
		case tkNUMBER: printf("%s", tokstring); break;
        case tkSTRING: printf("%c%s%c",34,tokstring,34); break;
		case tkEOF: printf("eof"); break;
		}
	}
}

// print atom
PRIVATE void pratom(int a){
	int type, ref;
	type = car(a);
	ref = cdr(a);
	switch(type){
	case SP: printf("NIL"); break;
	case OPER: prtoken(ref); break;
	case ADS: printf("$%d", ref); break;
	case NUM: printf("%d", ref); break;
	case GNAME: printf("%s",getName(ref)); break;
	case LNAME: printf("#%d", ref); break;
	case STRING: printf("%c%s%c",34,&strbuf[ref],34); break;
	}
}

void printList(int a){
    if( a == NIL ) return;
    if( isatom(a) ){
        pratom(a);
        printf(" ");
    }else{
		printf("(");
		while( a != NIL ){
			printList(car(a));
			a = cdr(a);
		}
		printf(")");
	}
}




