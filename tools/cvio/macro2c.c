#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "macro2protos.h"

/*
 * $Id $
 * $HeadURL $
 *
 */

/* parser defines */
#define STACK_SIZE 32
#define MAX_DIGIT_COUNT 20
#define MAX_EXPONENT 37
#define MAX_INTEGER 32768

#define NO_TOKEN 99
#define PLUS 100
#define EQUAL 101
#define LT 102
#define GT 103
#define NE 104
#define GE 105
#define LE 106
#define MINUS 107
#define OR 108
#define AND 109
#define STAR 110
#define SLASH 111
#define DIV 112
#define MOD 113
#define LPAREN 114
#define RPAREN 115
#define NUMBER 116
#define NOT 117
#define EOF_CODE 118
#define IDENTIFIER 119
#define STRING 120
#define UPARROW 121
#define COMMA 122
#define SEMICOLON 123
#define COLON 124
#define LBRACKET 125
#define RBRACKET 126
#define DOTDOT 40
#define PERIOD 41
#define BANG 42
#define AMPERSAND 43
#define BAR 44
#define SIN 45
#define COS 46
#define POW 47
#define TAN 48
#define ACOS 49
#define ASIN 50
#define ATAN 51
#define ABS 52
#define LN 53
#define EXP 54
#define INT 55
#define BAND 56
#define BOR 57
#define BNOT 58
#define LSHIFT 59
#define RSHIFT 60
#define ASSIGN 61
#define RAND 62
#define SGN 63
#define ATAN2 64
#define STAT 65
#define MSTAT 66

#define LETTER 10
#define DIGIT 11
#define QUOTE 12
#define SPECIAL 13
#define ERROR 14

extern int     token;
extern double  token_value;
extern char    token_string[256];
extern char    word_string[256];

static char 	*C_out = 0L;
static long int	C_quiet;

extern long int (*user_identifier_proc)(char *,double *);

long int C_expression(void);
long int get_token(void);

long int strgetins()
{
	if (C_out == 0L) return(0);
	
	return(strlen(C_out));
}

void strins(char *t,long int loc)
{
	char		tstr[4096],*p;
	long int	i;

	strcpy(tstr,&(C_out[loc]));
	
	p = t;
	i = loc;
	while (*p) {
		C_out[i++] = *p++;
	}
	C_out[i] = '\0';

	strcat(C_out,tstr);

	return;
}

void strquiet(long int i) {
	C_quiet = i;
}

void strout(char *t)
{
	if (C_out == 0L) return;
	if (C_quiet) return;

	strcat(C_out,t);

	return;
}

long int C_factor();

long int C_factor2()
{
        if (token == MINUS) {
		strout("-");
                if (get_token()) return(1);
        }
        if (C_factor()) return(1);
        return(0);
}

long int C_factor()
{
	char	tstr[256];
	
	switch (token) {
		case NUMBER:
			sprintf(tstr,"%f",token_value);
			strout(tstr);
			if (get_token()) return(1);
			break;
		case BNOT:
			if (get_token()) return(1);
			strout("~(long int)(");
			if (C_factor()) return(1);
			strout(")");
			break;
		case NOT:
			if (get_token()) return(1);
			strout("!(long int)(");
			if (C_factor()) return(1);
			strout(")");
			break;
		case EXP:
			if (get_token()) return(1);
			strout("exp(");
			if (C_factor()) return(1);
			strout(")");
			break;
		case LN:
			if (get_token()) return(1);
			strout("log(");
			if (C_factor()) return(1);
			strout(")");
			break;
		case ABS:
			if (get_token()) return(1);
			strout("fabs(");
			if (C_factor()) return(1);
			strout(")");
			break;
                case SGN:
                        if (get_token()) return(1);
			strout("(((");
                        if (C_factor()) return(1);
			strout(") < 0) ? -1 : 0)");
                        break;
		case SIN:
			if (get_token()) return(1);
			strout("sin(");
			if (C_factor()) return(1);
			strout(")");
			break;
		case COS:
			if (get_token()) return(1);
			strout("cos(");
			if (C_factor()) return(1);
			strout(")");
			break;
		case TAN:
			if (get_token()) return(1);
			strout("tan(");
			if (C_factor()) return(1);
			strout(")");
			break;
		case ASIN:
			if (get_token()) return(1);
			strout("asin(");
			if (C_factor()) return(1);
			strout(")");
			break;
		case ACOS:
			if (get_token()) return(1);
			strout("acos(");
			if (C_factor()) return(1);
			strout(")");
			break;
		case ATAN:
			if (get_token()) return(1);
			strout("atan(");
			if (C_factor()) return(1);
			strout(")");
			break;
		case ATAN2:
			if (get_token()) return(1); /* atan2 */
			if (get_token()) return(1); /* '(' */
			strout("atan2(");
			if (C_factor2()) return(1);
			strout(",");
			if (get_token()) return(1); /* ',' */
			if (C_factor2()) return(1);
			if (get_token()) return(1); /* ')' */
			strout(")");
			break;
		case STAT:
			if (get_token()) return(1); /* stat */
			if (get_token()) return(1); /* '(' */
			strout("(*stat_func)(");
			strout("\""); strout(word_string); strout("\",");
			strout("NULL,");
			if (get_token()) return(1); /* clear variable */
			if (get_token()) return(1); /* ',' */
			if (C_factor2()) return(1);  /* which */
			strout(",");
			if (get_token()) return(1); /* ',' */
			if (C_factor2()) return(1);  /* dx */
			strout(",");
			if (get_token()) return(1); /* ',' */
			if (C_factor2()) return(1);  /* dy */
			strout(",");
			if (get_token()) return(1); /* ',' */
			if (C_factor2()) return(1);  /* dz */
			if (get_token()) return(1); /* ')' */
			strout(")");
			break;
		case MSTAT:
			if (get_token()) return(1); /* mstat */
			if (get_token()) return(1); /* '(' */
			strout("(*stat_func)(");
			strout("\""); strout(word_string); strout("\",");
			if (get_token()) return(1); /* ',' */
			if (get_token()) return(1); /* varname */
			strout("\""); strout(word_string); strout("\",");
			if (get_token()) return(1); /* clear variable */
			if (get_token()) return(1); /* ',' */
			if (C_factor2()) return(1);  /* which */
			strout(",");
			if (get_token()) return(1); /* ',' */
			if (C_factor2()) return(1);  /* dx */
			strout(",");
			if (get_token()) return(1); /* ',' */
			if (C_factor2()) return(1);  /* dy */
			strout(",");
			if (get_token()) return(1); /* ',' */
			if (C_factor2()) return(1);  /* dz */
			if (get_token()) return(1); /* ')' */
			strout(")");
			break;
		case INT:
			if (get_token()) return(1);
			strout("floor(");
			if (C_factor()) return(1);
			strout(")");
			break;
		case RAND:
			if (get_token()) return(1);
			strout("((double)rand())/((double)RAND_MAX)");
			strquiet(1);
			if (C_factor()) return(1);
			strquiet(0);
/*
			if ((*tos) != 0.0) srand((unsigned int)(*tos));
			*tos = ((double)rand())/((double)RAND_MAX);
*/
			break;
		case LPAREN:
			if (get_token()) return(1);
			strout("(");
			if (C_expression()) return(1);
			
			if (token == RPAREN) {
				if (get_token()) return(1);
				strout(")");
			} else {
				printf("Missing right paren.\n");
				return(1);
			}
			break;
		case IDENTIFIER:
			if (user_identifier_proc) {
				int j;
				double value;
				j=(*user_identifier_proc)(word_string,&value);
				if (j == 1) {
					sprintf(tstr,"%s",word_string);
					strout(tstr);
					if (get_token()) return(1);
				} else {
					printf("Invalid expression.\n");
				}
			} else {
				sprintf(tstr,"%s",word_string);
				strout(tstr);
				if (get_token()) return(1);
			}
			break;
		case EOF_CODE:
			return(1);
			break;
		default:
			printf("Invalid expression.\n");
			return(1);
			break;
	}
	return(0);
}

long int C_term()
{
	int 		op;
	long int	ins;
	
	ins=strgetins();
	
	if (C_factor()) return(1);
	
	while((token == STAR) || (token == SLASH) || (token == DIV) ||
		(token == MOD) || (token == BAND) || (token == LSHIFT) ||
		(token == RSHIFT) || (token == AND) || (token == POW)) {

		op = token;
		
		switch(op) {
			case STAR:
				strout(" * ");
				if (get_token()) return(1);
				if (C_factor()) return(1);
				break;
			case SLASH:
				strout(" / ");
				if (get_token()) return(1);
				if (C_factor()) return(1);
				break;
			case DIV:
				strins("(long int)(",ins);
				strout(") / (long int)(");
				if (get_token()) return(1);
				if (C_factor()) return(1);
				strout(")");
				break;
			case MOD:
				strins("(long int)(",ins);
				strout(") % (long int)(");
				if (get_token()) return(1);
				if (C_factor()) return(1);
				strout(")");
				break;
			case POW:
				strins("pow(",ins);
				strout(",");
				if (get_token()) return(1);
				if (C_factor()) return(1);
				strout(")");
				break;
			case AND:
				strins("(long int)(",ins);
				strout(") && (long int)(");
				if (get_token()) return(1);
				if (C_factor()) return(1);
				strout(")");
				break;
			case BAND:
				strins("(long int)(",ins);
				strout(") & (long int)(");
				if (get_token()) return(1);
				if (C_factor()) return(1);
				strout(")");
				break;
			case RSHIFT:
				strins("(long int)(",ins);
				strout(") >> (long int)(");
				if (get_token()) return(1);
				if (C_factor()) return(1);
				strout(")");
				break;
			case LSHIFT:
				strins("(long int)(",ins);
				strout(") << (long int)(");
				if (get_token()) return(1);
				if (C_factor()) return(1);
				strout(")");
				break;
		}
	}
	return(0);
}

long int C_simple_expression()
{
	int 		op;
	int 		unary_op = PLUS;
	long int	ins;
	
	ins=strgetins();

	strout("(");
	if ((token == PLUS) || (token == MINUS)) {
		unary_op = token;
		if (get_token()) return(1);
		if (unary_op == MINUS) {
			strout("-(");
		}
	}
	
	if (C_term()) return(1);

	if (unary_op == MINUS) {
		strout(")");
	}
	strout(")");
	
	while((token == PLUS) || (token == MINUS) || (token == OR) 
		|| (token == BOR)) {
		op = token;
		
		switch(op) {
			case PLUS:
				strout(" + (");
				if (get_token()) return(1);
				if (C_term()) return(1);
				strout(")");
				break;
			case MINUS:
				strout(" - (");
				if (get_token()) return(1);
				if (C_term()) return(1);
				strout(")");
				break;
			case OR:
				strout(" || (");
				if (get_token()) return(1);
				if (C_term()) return(1);
				strout(")");
				break;
			case BOR:
				strins("(long int)",ins);
				strout(" | (long int)(");
				if (get_token()) return(1);
				if (C_term()) return(1);
				strout(")");
				break;
		}
	}
	return(0);
}

long int C_expression()
{
	int	op;
/*
	(( (expr1) op (expr2) ) ? 1.0 : 0.0)
or	(( (expr1) [op space] ) [boolean space])
*/	
	strout("(((");
	if(C_simple_expression()) return(1);
	strout(")");
	
	if ((token == EQUAL) || (token == LT) || (token == GT) || 
		(token == NE) || (token == LE) || (token ==GE)) {
		op = token;
		
		switch(op) {
			case EQUAL:
				strout(" == (");
				if (get_token()) return(1);
				if (C_simple_expression()) return(1);
				strout(")) ? 1.0 : 0.0)");
				break;
			case LT:
				strout(" < (");
				if (get_token()) return(1);
				if (C_simple_expression()) return(1);
				strout(")) ? 1.0 : 0.0)");
				break;
			case GT:
				strout(" > (");
				if (get_token()) return(1);
				if (C_simple_expression()) return(1);
				strout(")) ? 1.0 : 0.0)");
				break;
			case NE:
				strout(" != (");
				if (get_token()) return(1);
				if (C_simple_expression()) return(1);
				strout(")) ? 1.0 : 0.0)");
				break;
			case LE:
				strout(" <= (");
				if (get_token()) return(1);
				if (C_simple_expression()) return(1);
				strout(")) ? 1.0 : 0.0)");
				break;
			case GE:
				strout(" >= (");
				if (get_token()) return(1);
				if (C_simple_expression()) return(1);
				strout(")) ? 1.0 : 0.0)");
				break;
		}
	} else {
		strout("))");
	}
	return(0);
}

long int C_parse_math(char *out)
{
	out[0] = '\0';
	C_out = out;
	C_quiet = 0;

/* skip one leading comma (if any) */
	if (token == COMMA) if (get_token()) return(-1);
#ifdef NEVER
/* identifiers returned as strings (if desired) */
	if (token == IDENTIFIER) { 
		strcpy(out,word_string);
		if (get_token()) return(-1);
		return(2);
	}
/* strings are returned as strings */
	if (token == STRING) { 
		strcpy(out,word_string);
		if (get_token()) return(-1);
		return(1);
	}
#endif
	if (C_expression()) {
		return(-1);
	} else {
		strout(";");
	}
	return(0);
}

#ifdef NEVER
int main(int argc,char **argv)
{
	char	st[256];
	char	out[4096];
	double	v;

	while(1) {
		printf("Enter string:");
		gets(st);

		init_parser(st,1);
		if (C_parse_math(out) == 0) {
			printf("Value=%s\n",out);
		}
		reset_tokenlist_ptr();
		if (parse_math(out,&v) == 0) {
			printf("Value=%lf\n",v);
		}

	}

	exit(0);
}
#endif
