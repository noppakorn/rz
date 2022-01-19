/* automatically generated from lexgen */
#include "compile.h"
int token() {
    switch(CH) {
    case '*':
        accept(tkSTAR,1);
        break;
    case '/':
        accept(tkSLASH,1);
        break;
    case '-':
        accept(tkMINUS,1);
        break;
    case '+':
        accept(tkPLUS,1);
        break;
    case '=':
        getC();
        switch(CH) {
        case '=':
            accept(tkEQEQ,2);
            break;
        default:
            accept(tkEQ,1);
        }
        break;
    case '&':
        getC();
        switch(CH) {
        case '&':
            accept(tkANDAND,2);
            break;
        default:
            accept(tkAND,1);
        }
        break;
    case '|':
        getC();
        switch(CH) {
        case '|':
            accept(tkOROR,2);
            break;
        default:
            accept(tkOR,1);
        }
        break;
    case '!':
        getC();
        switch(CH) {
        case '=':
            accept(tkNE,2);
            break;
        default:
            accept(tkNOT,1);
        }
        break;
    case '<':
        getC();
        switch(CH) {
        case '=':
            accept(tkLE,2);
            break;
        default:
            accept(tkLT,1);
        }
        break;
    case '>':
        getC();
        switch(CH) {
        case '=':
            accept(tkGE,2);
            break;
        default:
            accept(tkGT,1);
        }
        break;
    case ',':
        accept(tkCOMMA,1);
        break;
    case ';':
        accept(tkSEMI,1);
        break;
    case '(':
        accept(tkLPAREN,1);
        break;
    case ')':
        accept(tkRPAREN,1);
        break;
    case '[':
        accept(tkLBRACKET,1);
        break;
    case ']':
        accept(tkRBRACKET,1);
        break;
    case '{':
        accept(tkLBRACE,1);
        break;
    case '}':
        accept(tkRBRACE,1);
        break;
    case 'i':
        getC();
        switch(CH) {
        case 'f':
            getC();
            if( isLetterOrDigit(CH) ) return 0;
            accept(tkIF,2);
            break;
        default:
            return 0;
        } /* reject */
        break;
    case 'e':
        getC();
        switch(CH) {
        case 'l':
            getC();
            switch(CH) {
            case 's':
                getC();
                switch(CH) {
                case 'e':
                    getC();
                    if( isLetterOrDigit(CH) ) return 0;
                    accept(tkELSE,4);
                    break;
                default:
                    return 0;
                } /* reject */
                break;
            default:
                return 0;
            } /* reject */
            break;
        default:
            return 0;
        } /* reject */
        break;
    case 'w':
        getC();
        switch(CH) {
        case 'h':
            getC();
            switch(CH) {
            case 'i':
                getC();
                switch(CH) {
                case 'l':
                    getC();
                    switch(CH) {
                    case 'e':
                        getC();
                        if( isLetterOrDigit(CH) ) return 0;
                        accept(tkWHILE,5);
                        break;
                    default:
                        return 0;
                    } /* reject */
                    break;
                default:
                    return 0;
                } /* reject */
                break;
            default:
                return 0;
            } /* reject */
            break;
        default:
            return 0;
        } /* reject */
        break;
    case 'r':
        getC();
        switch(CH) {
        case 'e':
            getC();
            switch(CH) {
            case 't':
                getC();
                switch(CH) {
                case 'u':
                    getC();
                    switch(CH) {
                    case 'r':
                        getC();
                        switch(CH) {
                        case 'n':
                            getC();
                            if( isLetterOrDigit(CH) ) return 0;
                            accept(tkRETURN,6);
                            break;
                        default:
                            return 0;
                        } /* reject */
                        break;
                    default:
                        return 0;
                    } /* reject */
                    break;
                default:
                    return 0;
                } /* reject */
                break;
            default:
                return 0;
            } /* reject */
            break;
        default:
            return 0;
        } /* reject */
        break;
    case 'p':
        getC();
        switch(CH) {
        case 'r':
            getC();
            switch(CH) {
            case 'i':
                getC();
                switch(CH) {
                case 'n':
                    getC();
                    switch(CH) {
                    case 't':
                        getC();
                        if( isLetterOrDigit(CH) ) return 0;
                        accept(tkPRINT,5);
                        break;
                    default:
                        return 0;
                    } /* reject */
                    break;
                default:
                    return 0;
                } /* reject */
                break;
            default:
                return 0;
            } /* reject */
            break;
        default:
            return 0;
        } /* reject */
        break;
    default:
        return 0;
    } /* reject */
    return 1;
}
