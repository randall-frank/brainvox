#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "macroprotos.h"

typedef struct macdef {
	char *name;
	char *value;
	struct macdef *next;
} macdef;

macdef	*root = 0L;

void search_macro(char *name,macdef **parent,macdef **child)
{
	*parent = 0L;
	*child = root;
	while (*child != 0L) {
		if (strcmp((*child)->name,name) == 0L) return;
		*parent = *child;
		*child = (*parent)->next;
	}
	*parent = 0L;
	*child = 0L;
	return;
}

void dump_macro(char *name)
{
	macdef *parent,*child;
	
	printf("--------------------\n");
	if (root == 0L) {
		printf("No macros defined\n");
		return;
	}
	if (name == 0L) {
		parent = root;
		while (parent != 0L) {
			if (parent->name != 0L) printf("Name:%s:",parent->name);
			if (parent->value != 0L) printf("Value:%s:",parent->value);
			printf("\n");
			parent = parent->next;
		}
	} else {
		search_macro(name,&parent,&child);
/* name not found */
		if (child == 0L) {
			printf("Macro:%s undefined.\n",name);
		} else {
			if (child->name != 0L) printf("Name:%s:",child->name);
			if (child->value != 0L) printf("Value:%s:",child->value);
			printf("\n");
		}
	}
	return;
}

void clear_macro(char *name)
{
	macdef *parent,*child;
	
	if (root == 0L) return;
	if (name == 0L) {
/* clear all */
		parent = root;
		while (parent != 0L) {
			if (parent->name != 0L) free(parent->name);
			if (parent->value != 0L) free(parent->value);
			child = parent;
			parent = parent->next;
			free(child);
		}
		root = 0L;
	} else {
		search_macro(name,&parent,&child);
/* name not found */
		if (child == 0L) return;
		if (child->name != 0L) free(child->name);
		if (child->value != 0L) free(child->value);
/* if child was not the root */
		if (parent != 0L) {
			parent->next = child->next;
		} else {
			root = child->next;
		}
		free(child);
	}
	return;
}

void set_macro(char *name,char *value)
{
	macdef *parent,*child;

	if (value == 0L) {
		clear_macro(name);
		return;
	}
/* find match */
	search_macro(name,&parent,&child);
	if (child == 0L) {
/* insert new at head of list */
		child = (macdef *)malloc(sizeof(macdef));
		if (child == 0L) return;
		child->next = root;
		child->name = malloc(strlen(name)+1);
		if (child->name == 0L) {
			free(child);
			return;
		}
		strcpy(child->name,name);
/* insert */
		root = child;
	} else {
/* free up any old value */
		if (child->value != 0L) free(child->value);
	}
	child->value = malloc(strlen(value)+1);
	if (child->value != 0L) strcpy(child->value,value);
	return;
}

void get_macro(char *name,char *value)
{
	macdef *parent,*child;
	
	if (root == 0L) return;
	search_macro(name,&parent,&child);
	if (child == 0L) return;
	strcpy(value,child->value);
	return;
}

void expand_macro(char *in,char *out)
{
	long int i,j,k;
	char t[200],s[200];
	double v;
	
	j = 0;
	i = 0;
	while (in[i] != '\0') {
		if ((in[i] == '$') && (in[i+1] == '(')) {
/* found $( */
			if ((in[i+2] == '$') && (in[i+3] == '(')) {
/* the special $($( case */
				out[j++] = '$';
				out[j++] = '(';
				i = i + 4;
			} else {
/* regular macro expansion */
				i = i + 2;
				k = 0;
				while ((in[i] != ')') && (in[i] != '\0')) {
					t[k++] = in[i++];
				}
				t[k++] = '\0';
				strcpy(s,"0");
				get_macro(t,s);
				k = 0;
				while (s[k] != '\0') out[j++] = s[k++];
				if (in[i] != '\0') i++;
			}
		} else if ((in[i] == '$') && (in[i+1] == '#') && 
			(in[i+2] == '(')) {
/* found $#( */
/* integer macro expansion */
			i = i + 3;
			k = 0;
			while ((in[i] != ')') && (in[i] != '\0')) {
				t[k++] = in[i++];
			}
			t[k++] = '\0';
			strcpy(s,"0");
			get_macro(t,s);
/* convert macro into integer form */
			sscanf(s,"%lf",&v);
			sprintf(s,"%ld",(long int)(v));
			k = 0;
			while (s[k] != '\0') out[j++] = s[k++];
			if (in[i] != '\0') i++;
		} else {
			out[j++] = in[i++];
		}
	}
	out[j++] = '\0';
	return;
}

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

#define LETTER 10
#define DIGIT 11
#define QUOTE 12
#define SPECIAL 13
#define ERROR 14

/* parser globals */
int		token;
short	*char_code = 0L;
char	token_string[256];
char	word_string[256];
char	*tokenp;
char	ch,*bufferp;
double	token_value;
double	val_stack[STACK_SIZE];
double	*tos;
int		digit_count, count_error;

long int expression();

void init_scanner(char *str)
{
	int i;
	
	if (char_code == 0L) {
		char_code = (short *)malloc(256*sizeof(short));
		for(i=0;i<256;i++) char_code[i] = SPECIAL;
		for(i='0';i<= '9';i++) char_code[i] = DIGIT;
		for(i='A';i<= 'Z';i++) char_code[i] = LETTER;
		for(i='a';i<= 'z';i++) char_code[i] = LETTER;
		char_code['\''] = QUOTE;
		char_code[0] = EOF_CODE;
	}
	bufferp = str;
	ch = ' ';
}

long int get_char()
{
	if (*bufferp == '\0') {
		ch = '\0';
		return(0);
	}
	
	ch = *bufferp;
	bufferp++;

	return(0);
}

void accumulate_value(double *valuep,char *err)
{
	double	value = *valuep;
	
	if (char_code[ch] != DIGIT) {
		printf("Error: %s\n",err);
		token = ERROR;
		return;
	}
	
	do {
		*tokenp = ch;
		tokenp++;
		
		if (++digit_count <= MAX_DIGIT_COUNT) {
			value = 10.0*value + (ch - '0');
		} else {
			count_error = TRUE;
		}
		
		get_char();
	} while (char_code[ch] == DIGIT);
	
	*valuep = value;
	
	return;
}

long int skip_blanks()
{
	while(ch == ' ') if (get_char()) return(1);
	return(0);
}

long int is_reserved_word()
{
	if (strcmp(word_string,"not") == 0) {
		token = NOT;
		return(TRUE);
	} else if (strcmp(word_string,"and") == 0) {
		token = AND;
		return(TRUE);
	} else if (strcmp(word_string,"or") == 0) {
		token = OR;
		return(TRUE);
	} else if (strcmp(word_string,"div") == 0) {
		token = DIV;
		return(TRUE);
	} else if (strcmp(word_string,"abs") == 0) {
		token = ABS;
		return(TRUE);
	} else if (strcmp(word_string,"ln") == 0) {
		token = LN;
		return(TRUE);
	} else if (strcmp(word_string,"exp") == 0) {
		token = EXP;
		return(TRUE);
	} else if (strcmp(word_string,"acos") == 0) {
		token = ACOS;
		return(TRUE);
	} else if (strcmp(word_string,"asin") == 0) {
		token = ASIN;
		return(TRUE);
	} else if (strcmp(word_string,"atan") == 0) {
		token = ATAN;
		return(TRUE);
	} else if (strcmp(word_string,"sin") == 0) {
		token = SIN;
		return(TRUE);
	} else if (strcmp(word_string,"cos") == 0) {
		token = COS;
		return(TRUE);
	} else if (strcmp(word_string,"tan") == 0) {
		token = TAN;
		return(TRUE);
	} else if (strcmp(word_string,"pow") == 0) {
		token = POW;
		return(TRUE);
	} else if (strcmp(word_string,"int") == 0) {
		token = INT;
		return(TRUE);
	} else if (strcmp(word_string,"mod") == 0) {
		token = MOD;
		return(TRUE);
	}
	return(FALSE);
}

void downshift_word()
{
	int 	offset = 'a' - 'A';
	char 	*wp = word_string;
	char	*tp = token_string;
	
	do {
		*wp = (*tp >= 'A') && (*tp <= 'Z') ? (*tp + offset) : *tp;
		wp++;
		tp++;
	} while (*tp != '\0');
	*wp = '\0';
	return;
}

long int get_word()
{
	while ((char_code[ch] == LETTER) || (char_code[ch] == DIGIT)) {
		*tokenp = ch;
		tokenp++;
		if (get_char()) return(1);
	}
	*tokenp = '\0';
	
	downshift_word();
	if (!is_reserved_word()) token = IDENTIFIER;
	
	return(0);
}

long int get_number()
{
	int 	whole_count = 0;
	int		decimal_offset = 0;
	char	exp_sign = '+';
	int		exp = 0;
	double	nvalue = 0.0;
	double	evalue = 0.0;
	int		saw_dotdot = FALSE;
	
	digit_count = 0;
	count_error = FALSE;
	token = NO_TOKEN;
	
	accumulate_value(&nvalue,"Invalid number.");
	if (token == ERROR) return(1);
	whole_count = digit_count;
	
	if (ch == '.') {
		if (get_char()) return(1);
		if (ch == '.') {
			saw_dotdot = TRUE;
			--bufferp;
		} else {
			*tokenp = '.';
			tokenp++;
			accumulate_value(&nvalue,"Invalid fraction.");
			if (token == ERROR) return(1);
			decimal_offset = whole_count - digit_count;
		}
	}
	
	if ((!saw_dotdot) && ((ch == 'e') || (ch == 'E'))) {
			*tokenp = ch;
			tokenp++;
			if (get_char()) return(1);
			if ((ch == '+') || (ch == '-')) {
				*tokenp = ch;
				tokenp++;
				exp_sign = ch;
				if (get_char()) return(1);
			}
			
			accumulate_value(&evalue,"Invalid exponent");
			if (token == ERROR) return(1);
			if (exp_sign == '-') evalue = -evalue;
	}
	
	if (count_error) {
		token = ERROR;
		return(1);
	}
	
	exp = evalue + decimal_offset;
	if (((exp + whole_count) < -MAX_EXPONENT) || 
		((exp + whole_count) > MAX_EXPONENT)) {
		token = ERROR;
		return(1);
	}
	
	if (exp != 0) nvalue *= pow(10.0,((double)exp));
	
	*tokenp = '\0';
	token = NUMBER;
	token_value = nvalue;
	
	return(0);
}

long int get_string()
{
	char 	*wp = word_string;

	*tokenp = '\'';
	tokenp++;
	if(get_char()) return(1);

	while (ch != '\0') {
		if (ch == '\'') {
			*tokenp = ch;
			tokenp++;
			if (get_char()) return(1);
			if (ch != '\'') break;
		}
		*tokenp = ch;
		*wp = ch;
		tokenp++;
		wp++;
		if (get_char()) return(1);
	}
	
	*tokenp = '\0';
	*wp = '\0';
	token = STRING;
	return(0);
}

long int get_special()
{
	*tokenp = ch;
	tokenp++;
	switch(ch) {
		case '^': token = POW; if (get_char()) return(1); break;
		case '~': token = NOT; if (get_char()) return(1); break;
		case '%': token = MOD; if (get_char()) return(1); break;
		case '*': token = STAR; if (get_char()) return(1); break;
		case '(': token = LPAREN; if (get_char()) return(1); break;
		case ')': token = RPAREN; if (get_char()) return(1); break;
		case '-': token = MINUS; if (get_char()) return(1); break;
		case '+': token = PLUS; if (get_char()) return(1); break;
		case '/': token = SLASH; if (get_char()) return(1); break;
		case ',': token = COMMA; if (get_char()) return(1); break;
		case ':': token = COLON; if (get_char()) return(1); break;
		case ';': token = SEMICOLON; if (get_char()) return(1); break;
		case '[': token = LBRACKET; if (get_char()) return(1); break;
		case ']': token = RBRACKET; if (get_char()) return(1); break;
		case '<':
			if (get_char()) return(1);
			if (ch == '=') {
				*tokenp = ch;
				tokenp++;
				token = LE;
				if (get_char()) return(1);
			} else {
				token = LT;
			}
			break;
		case '>':
			if (get_char()) return(1);
			if (ch == '=') {
				*tokenp = ch;
				tokenp++;
				token = GE;
				if (get_char()) return(1);
			} else {
				token = GT;
			}
			break;
		case '!':
			if (get_char()) return(1);
			if (ch == '=') {
				*tokenp = ch;
				tokenp++;
				token = NE;
				if (get_char()) return(1);
			} else {
				token = BANG;
			}
			break;
		case '&':
			if (get_char()) return(1);
			if (ch == '&') {
				*tokenp = ch;
				tokenp++;
				token = AND;
				if (get_char()) return(1);
			} else {
				token = AMPERSAND;
			}
			break;
		case '|':
			if (get_char()) return(1);
			if (ch == '|') {
				*tokenp = ch;
				tokenp++;
				token = OR;
				if (get_char()) return(1);
			} else {
				token = BAR;
			}
			break;
		case '.':
			if (get_char()) return(1);
			if (ch == '.') {
				*tokenp = ch;
				tokenp++;
				token = DOTDOT;
				if (get_char()) return(1);
			} else {
				token = PERIOD;
			}
			break;
		default :
			token = ERROR;
			if (get_char()) return(1);
			break;
	}
	
	*tokenp = '\0';
	return(0);
}

long int get_token()
{
	if (skip_blanks()) return(1);
	tokenp = token_string;
	
	switch(char_code[ch]) {
		case LETTER:
			if (get_word()) return(1);
			break;
		case DIGIT:
			if (get_number()) return(1);
			break;
		case QUOTE:
			if (get_string()) return(1);
			break;
		case EOF_CODE:
			token = EOF_CODE;
			break;
		default:
			if (get_special()) return(1);
	}
	return(0);
}

double  pop() 
{
	double t;
	
	if (tos < (&val_stack[0])) return(0.0);
	t = *tos;
	tos--;
	return(t);
}

long int push(double value)
{
	if (tos >= (&val_stack[STACK_SIZE])) {
		return(1);
	}
	tos++;
	*tos = value;
	return(0);
}


long int factor()
{
	switch (token) {
		case NUMBER:
			if (push(token_value)) return(1);
			if (get_token()) return(1);
			break;
		case NOT:
			if (get_token()) return(1);
			if (factor()) return(1);
			*tos = (*tos) == 0.0 ? 1.0 : 0.0;
			break;
		case EXP:
			if (get_token()) return(1);
			if (factor()) return(1);
			*tos = exp(*tos);
			break;
		case LN:
			if (get_token()) return(1);
			if (factor()) return(1);
			*tos = log(*tos);
			break;
		case ABS:
			if (get_token()) return(1);
			if (factor()) return(1);
			*tos = fabs(*tos);
			break;
		case SIN:
			if (get_token()) return(1);
			if (factor()) return(1);
			*tos = sin(*tos);
			break;
		case COS:
			if (get_token()) return(1);
			if (factor()) return(1);
			*tos = cos(*tos);
			break;
		case TAN:
			if (get_token()) return(1);
			if (factor()) return(1);
			*tos = tan(*tos);
			break;
		case ASIN:
			if (get_token()) return(1);
			if (factor()) return(1);
			*tos = asin(*tos);
			break;
		case ACOS:
			if (get_token()) return(1);
			if (factor()) return(1);
			*tos = acos(*tos);
			break;
		case ATAN:
			if (get_token()) return(1);
			if (factor()) return(1);
			*tos = atan(*tos);
			break;
		case INT:
			if (get_token()) return(1);
			if (factor()) return(1);
			*tos = floor(*tos);
			break;
		case LPAREN:
			if (get_token()) return(1);
			if (expression()) return(1);
			
			if (token == RPAREN) {
				if (get_token()) return(1);
			} else {
				printf("Missing right paren.\n");
				return(1);
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

long int term()
{
	int 	op;
	double	op_1,op_2;
	
	if (factor()) return(1);
	
	while((token == STAR) || (token == SLASH) || (token == DIV) || (token == MOD) ||
		(token == AND) || (token == POW)) {
		op = token;
		
		if (get_token()) return(1);
		if (factor()) return(1);
		
		op_2 = pop();
		op_1 = pop();
		
		switch(op) {
			case STAR:
				if (push(op_1 * op_2)) return(1);
				break;
			case SLASH:
				if (op_2 != 0.0) {
					if (push(op_1 / op_2)) return(1);
				} else {
					printf("Warning, divison by zero.\n");
					if (push(0.0)) return(1);
				}
				break;
			case DIV:
				if (op_2 != 0.0) {
					if (push( (double)( ((int)op_1) / ((int)op_2) ) )) return(1);
				} else {
					printf("Warning, divison by zero.\n");
					if (push(0.0)) return(1);
				}
				break;
			case MOD:
				if (op_2 != 0.0) {
					if (push( (double)( ((int)op_1) % ((int)op_2) ) )) return(1);
				} else {
					printf("Warning, divison by zero.\n");
					if (push(0.0)) return(1);
				}
				break;
			case POW:
				if (push( pow(op_1,op_2) )) return(1);
				break;
			case AND:
				if(push((op_1 != 0.0) && (op_2 != 0.0) ? 1.0 : 0.0)) return(1);
				break;
		}
	}
	return(0);
}

long int simple_expression()
{
	int 	op;
	int 	unary_op = PLUS;
	double	op_1,op_2;
	
	if ((token == PLUS) || (token == MINUS)) {
		unary_op = token;
		if (get_token()) return(1);
	}
	
	if (term()) return(1);
	
	if (unary_op == MINUS) *tos = -(*tos);
	
	while((token == PLUS) || (token == MINUS) || (token == OR)) {
		op = token;
		
		if (get_token()) return(1);
		if (term()) return(1);
		
		op_2 = pop();
		op_1 = pop();
		
		switch(op) {
			case PLUS:
				if(push(op_1 + op_2)) return(1);
				break;
			case MINUS:
				if(push(op_1 - op_2)) return(1);
				break;
			case OR:
				if(push((op_1 != 0.0) || (op_2 != 0.0) ? 1.0 : 0.0)) return(1);
				break;
		}
	}
	return(0);
}

long int expression()
{
	int		op;
	double	op_1,op_2;
	
	if(simple_expression()) return(1);
	
	if ((token == EQUAL) || (token == LT) || (token == GT) || (token == NE) ||
		(token == LE) || (token ==GE)) {
		op = token;
		if (get_token()) return(1);
		if (simple_expression()) return(1);
		op_2 = pop();
		op_1 = pop();
		
		switch(op) {
			case EQUAL:
				if(push(op_1 == op_2 ? 1.0 : 0.0)) return(1);
				break;
			case LT:
				if(push(op_1 < op_2 ? 1.0 : 0.0)) return(1);
				break;
			case GT:
				if(push(op_1 > op_2 ? 1.0 : 0.0)) return(1);
				break;
			case NE:
				if(push(op_1 != op_2 ? 1.0 : 0.0)) return(1);
				break;
			case LE:
				if(push(op_1 <= op_2 ? 1.0 : 0.0)) return(1);
				break;
			case GE:
				if(push(op_1 >= op_2 ? 1.0 : 0.0)) return(1);
				break;
		}
	}
	return(0);
}

long int init_parser(char *exp)
{
	init_scanner(exp);
	if (get_token()) return(-1);
	return(0);
}

long int parse_math(char *out,double *value)
{
	tos = val_stack;

	*value = 0.0;
	
	if (token == COMMA) if (get_token()) return(-1);
	if (token == STRING) { 
		strcpy(out,word_string);
		if (get_token()) return(-1);
		return(1);
	}
	if (token == IDENTIFIER) {
		strcpy(out,word_string);
		if (get_token()) return(-1);
		return(2);
	}
	if (expression()) {
		return(-1);
	} else {
		*value = pop();
	}
	sprintf(out,"%lf",(*value));
	return(0);
}
