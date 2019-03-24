#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "macroprotos.h"

/*
 * $Id $
 * $HeadURL $
 *
 */

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

/* use a token list to avoid scanner overhead when 
   repeatedly evaluating an expression */
   
typedef struct {
	int 			token;
	double			token_value;
	char			token_word[256];
	char			token_string[256];
	struct TokenObj	*next;
} TokenObj;

TokenObj *tokenlist = 0L;
TokenObj *token_list_ptr = 0L;
int use_token_list = 0;

/* Return 1 for a number (in double) or 2 for string (overwrite input) */
long int (*user_identifier_proc)(char *,double *) = 0L;

long int expression(void);
long int get_token(void);

void free_token_list()
{
	TokenObj	*tp;
	unsigned char	*temp;
	
	tp = tokenlist;	
	while (tp != 0L) {
		temp = (unsigned char *)tp;
		tp = (TokenObj *)tp->next;
		free(temp);
	}
	tokenlist = 0L;
	return;
}

int add_token_to_list()
{
	TokenObj *tp,*tt;
	
	tp = (TokenObj *)malloc(sizeof(TokenObj));
	if (tp == 0L) return(1);
	
	tt = tokenlist;
	if (tokenlist == 0L) {
		tokenlist = tp;
	} else {
		while (tt->next != 0L) {
			tt = (TokenObj *)tt->next;
		}
		tt->next = (struct TokenObj *)tp;
	}
/* fill it in */
	tp->next = 0L;
	tp->token = token;
	tp->token_value = token_value;
	strcpy(tp->token_string,token_string);
	strcpy(tp->token_word,word_string);
	return(0);
}

long int reset_tokenlist_ptr()
{
	if (use_token_list) {
		token_list_ptr = tokenlist;
		if (get_token()) return(-1);
	}
	return(0);
}

void set_user_identifier(long int (*proc)(char *,double *))
{
	user_identifier_proc = proc;
	return;
}

void init_scanner(char *str,long int tokenized)
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
	
/* handle the tokenized case */
	if (tokenlist != 0L) {
		free_token_list();
		token_list_ptr = 0L;
		use_token_list = 0;
	}
	
/* build a token list */
	if (tokenized) {
		while (get_token() == 0) {
			if (add_token_to_list()) break;
			if (token == EOF_CODE) break;
		}
		token_list_ptr = tokenlist;
		use_token_list = 1;
	}
	return;
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

long int valid_char_base(char ch,long int base,long int *num) 
{
	if (base == 8) {
		if ((ch >= '0') && (ch <= '7')) {
			*num = ch - '0';
			return(1);
		}
	} else if (base == 10) {
		if ((ch >= '0') && (ch <= '9')) {
			*num = ch - '0';
			return(1);
		}
	} else if (base == 2) {
		if ((ch >= '0') && (ch <= '1')) {
			*num = ch - '0';
			return(1);
		}
	} else if (base == 16) {
		if ((ch >= '0') && (ch <= '9')) {
			*num = ch - '0';
			return(1);
		}
		if ((ch >= 'a') && (ch <= 'f')) {
			*num = ch - 'a' + 10;
			return(1);
		}
		if ((ch >= 'A') && (ch <= 'F')) {
			*num = ch - 'A' + 10;
			return(1);
		}
	}
	return(0);
}

long int get_base_integer(long int base)
{
	double	value = 0.0;
	long int 	more,count,num;
	
	if (!valid_char_base(ch,base,&num)) {
		token = ERROR;
		return(1);
	}
	
	more = 1;
	count = 0;
	do {
		*tokenp = ch;
		tokenp++;
		
		value = value * base;
		value = value + num;
		
		get_char();
		if (!valid_char_base(ch,base,&num)) more = 0;
		count++;
		
	} while ((more) && (count <= MAX_DIGIT_COUNT));
	
	if (count > MAX_DIGIT_COUNT) {
		token = ERROR;
		return(1);
	}
	
	*tokenp = '\0';
	token = NUMBER;
	token_value = value;

	return(0);
}

void accumulate_value(double *valuep,char *err)
{
	double	value = *valuep;
	
	if (char_code[(short)ch] != DIGIT) {
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
	} while (char_code[(short)ch] == DIGIT);
	
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
	} else if (strcmp(word_string,"sgn") == 0) {
		token = SGN;
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
	} else if (strcmp(word_string,"atan2") == 0) {
		token = ATAN2;
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
	} else if (strcmp(word_string,"rand") == 0) {
		token = RAND;
		return(TRUE);
	} else if (strcmp(word_string,"stat") == 0) {
		token = STAT;
		return(TRUE);
	} else if (strcmp(word_string,"mstat") == 0) {
		token = MSTAT;
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
	while ((char_code[(short)ch] == LETTER) || 
	       (char_code[(short)ch] == DIGIT)) {
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
	int 		whole_count = 0;
	int		decimal_offset = 0;
	char		exp_sign = '+';
	int		exp = 0;
	double		nvalue = 0.0;
	double		evalue = 0.0;
	int		saw_dotdot = FALSE;
	
	digit_count = 0;
	count_error = FALSE;
	token = NO_TOKEN;
	
	if (ch != '.') {
		accumulate_value(&nvalue,"Invalid number.");
		if (token == ERROR) return(1);
		whole_count = digit_count;
	} 
	if (ch == '.') {
		if (get_char()) return(1);
		if (ch == '.') {
			saw_dotdot = TRUE;
			--bufferp;
		} else if (char_code[(short)ch] == DIGIT) {
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
		case '~': token = BNOT; if (get_char()) return(1); break;
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
		case '=':
			if (get_char()) return(1);
			if (ch == '=') {
				*tokenp = ch;
				tokenp++;
				token = EQUAL;
				if (get_char()) return(1);
			} else {
				token = ASSIGN;
			}
			break;
		case '<':
			if (get_char()) return(1);
			if (ch == '=') {
				*tokenp = ch;
				tokenp++;
				token = LE;
				if (get_char()) return(1);
			} else if (ch == '<') {
				*tokenp = ch;
				tokenp++;
				token = LSHIFT;
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
			} else if (ch == '>') {
				*tokenp = ch;
				tokenp++;
				token = RSHIFT;
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
				token = NOT;
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
				token = BAND;
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
				token = BOR;
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

	if (use_token_list) {
		if (token_list_ptr == 0L) return(1);
		token = token_list_ptr->token;
		token_value = token_list_ptr->token_value;
		if ((token == STRING) || (token == IDENTIFIER)) {
			strcpy(word_string,token_list_ptr->token_word);
			strcpy(token_string,token_list_ptr->token_string);
		}
		token_list_ptr = (TokenObj *)token_list_ptr->next;
		return(0);
	}
	
	if (skip_blanks()) return(1);
	tokenp = token_string;
	
	switch(char_code[(short)ch]) {
		case LETTER:
			if (get_word()) return(1);
			break;
		case DIGIT:
/* special check for 0xNNNN and 0NNNN hex and octal (0bNNNN for binary) constants */
			if (ch == '0') {
				if ((*bufferp == 'x') || (*bufferp == 'X')) {
					get_char();
					get_char();
					if (get_base_integer(16L)) return(1);
					return(0);
				} else if ((*bufferp == 'b') || (*bufferp == 'b')) {
					get_char();
					get_char();
					if (get_base_integer(2L)) return(1);
					return(0);
				} else if ((*bufferp >= '1') && (*bufferp <= '7')) {
					get_char();
					if (get_base_integer(8L)) return(1);
					return(0);
				}
			}
			if (get_number()) return(1);
			break;
		case QUOTE:
			if (get_string()) return(1);
			break;
		case EOF_CODE:
			token = EOF_CODE;
			break;
		default:
			if (ch == '.') {
				if (get_number()) return(1);
			} else {
				if (get_special()) return(1);
			}
			break;
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

long int factor();

long int factor2()
{
	double	mu;
	if (token == MINUS) {
		mu = -1.0;
		if (get_token()) return(1);
	} else {
		mu = 1.0;
        }
	if (factor()) return(1);
	*tos = (*tos) * mu;
	return(0);
}

long int factor()
{
	long int i,j;
	double	value;
	
	switch (token) {
		case NUMBER:
			if (push(token_value)) return(1);
			if (get_token()) return(1);
			break;
		case BNOT:
			if (get_token()) return(1);
			if (factor()) return(1);
			i = (*tos);
			*tos = (double)(~i);
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
                        if (*tos == 0.0) {
                                printf("Warning, ln of zero is undefined.\n");
                        } else {
                                *tos = log(*tos);
                        }
			break;
		case ABS:
			if (get_token()) return(1);
			if (factor()) return(1);
			*tos = fabs(*tos);
			break;
		case SGN:
			if (get_token()) return(1);
			if (factor()) return(1);
			*tos = (((*tos) < 0) ? -1 : 0);
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
                        if (*tos < -1) *tos = -1;
                        if (*tos > 1) *tos = 1;
			*tos = asin(*tos);
			break;
		case ACOS:
			if (get_token()) return(1);
			if (factor()) return(1);
                        if (*tos < -1) *tos = -1;
                        if (*tos > 1) *tos = 1;
			*tos = acos(*tos);
			break;
		case ATAN:
			if (get_token()) return(1);
			if (factor()) return(1);
			*tos = atan(*tos);
			break;
		case ATAN2:
			if (get_token()) return(1); /* atan2 */
			if (get_token()) return(1); /* '(' */
			if (factor2()) return(1);
			value = pop();
			if (get_token()) return(1); /* ',' */
			if (factor2()) return(1);
			if (get_token()) return(1); /* ')' */
			*tos = atan2(value,*tos);
			break;
		case INT:
			if (get_token()) return(1);
			if (factor()) return(1);
			*tos = floor(*tos);
			break;
		case RAND:
			if (get_token()) return(1);
			if (factor()) return(1);
			if ((*tos) != 0.0) srand((unsigned int)(*tos));
			*tos = ((double)rand())/((double)RAND_MAX);
			break;
		case STAT:
			{
			char label[256];
			int  which,dd[3];
                        /* stat(var,[0,1,2],-1,-1,-1) */
			if (get_token()) return(1); /* stat */
			if (get_token()) return(1); /* '(' */
                        if (token != IDENTIFIER) return(1);
			strcpy(label,word_string);
			if (get_token()) return(1); /* clear variable */
			if (get_token()) return(1); /* ',' */
			if (factor2()) return(1);    /* what */
			which = pop();
			if (get_token()) return(1); /* ',' */
			if (factor2()) return(1);    /* x */
			dd[0] = pop();
			if (get_token()) return(1); /* ',' */
			if (factor2()) return(1);    /* y */
			dd[1] = pop();
			if (get_token()) return(1); /* ',' */
			if (factor2()) return(1);    /* z */
			dd[2] = *tos;
			if (get_token()) return(1); /* ')' */
                        *tos=stat_function(label,NULL,which,dd[0],dd[1],dd[2]);
			}
			break;
		case MSTAT:
			{
			char label[256],mask[256];
			int  which,dd[3];
                        /* mstat(var,mask,[0,1,2],-1,-1,-1) */
			if (get_token()) return(1); /* mstat */
			if (get_token()) return(1); /* '(' */
                        if (token != IDENTIFIER) return(1);
			strcpy(label,word_string);
			if (get_token()) return(1); /* ',' */
			if (get_token()) return(1); /* variable */
                        if (token != IDENTIFIER) return(1);
			strcpy(mask,word_string);
			if (get_token()) return(1); /* clear variable */
			if (get_token()) return(1); /* ',' */
			if (factor2()) return(1);    /* what */
			which = pop();
			if (get_token()) return(1); /* ',' */
			if (factor2()) return(1);    /* x */
			dd[0] = pop();
			if (get_token()) return(1); /* ',' */
			if (factor2()) return(1);    /* y */
			dd[1] = pop();
			if (get_token()) return(1); /* ',' */
			if (factor2()) return(1);    /* z */
			dd[2] = *tos;
			if (get_token()) return(1); /* ')' */
                        *tos=stat_function(label,mask,which,dd[0],dd[1],dd[2]);
			}
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
		case IDENTIFIER:
			if (user_identifier_proc) {
				j=(*user_identifier_proc)(word_string,&value);
				if (j == 1) {
					token = NUMBER;
					token_value = value;
					if (push(token_value)) return(1);
					if (get_token()) return(1);
				} else {
					printf("Invalid expression.\n");
					return(1);
				}
			} else {
				if (get_token()) return(1);
				printf("Unknown identifier:%s\n",word_string);
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
	long int	iop_1,iop_2;
	
	if (factor()) return(1);
	
	while((token == STAR) || (token == SLASH) || (token == DIV) ||
		(token == MOD) || (token == BAND) || (token == LSHIFT) ||
		(token == RSHIFT) || (token == AND) || (token == POW)) {

		op = token;
		
		if (get_token()) return(1);
		if (factor()) return(1);
		
		op_2 = pop();
		op_1 = pop();
		iop_1 = op_1;
		iop_2 = op_2;
		
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
                                if ((op_1 < 0) && ((double)(iop_2) != op_2)) {
                                        printf("Warning, negative number raised to non-integer power.\n");
                                        if (push(0.0)) return(1);
                                } else {
                                        if (push( pow(op_1,op_2) )) return(1);
                                }
				break;
			case AND:
				if(push((op_1 != 0.0) && (op_2 != 0.0) ? 1.0 : 0.0)) return(1);
				break;
			case BAND:
				if(push((double)(iop_1 & iop_2))) return(1);
				break;
			case RSHIFT:
				if(push((double)(iop_1 >> iop_2))) return(1);
				break;
			case LSHIFT:
				if(push((double)(iop_1 << iop_2))) return(1);
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
	long int	iop_1,iop_2;
	
	if ((token == PLUS) || (token == MINUS)) {
		unary_op = token;
		if (get_token()) return(1);
	}
	
	if (term()) return(1);
	
	if (unary_op == MINUS) *tos = -(*tos);
	
	while((token == PLUS) || (token == MINUS) || (token == OR) 
		|| (token == BOR)) {
		op = token;
		
		if (get_token()) return(1);
		if (term()) return(1);
		
		op_2 = pop();
		op_1 = pop();
		iop_1 = op_1;
		iop_2 = op_2;
		
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
			case BOR:
				if(push((double)(iop_1 | iop_2))) return(1);
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

long int init_parser(char *exp,long int tokenize)
{
	init_scanner(exp,tokenize);
	if (get_token()) return(-1);
	return(0);
}

long int parse_math(char *out,double *value)
{
	tos = val_stack;

	*value = 0.0;

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
	if (expression()) {
		return(-1);
	} else {
		*value = pop();
	}
	sprintf(out,"%f",(*value));
	return(0);
}

void print_exp_help(char *prog,int which)
{
	fprintf(stderr, "        %s will parse mathematical expressions\n",prog);
	fprintf(stderr, "with parenthesis up to 30 levels deep.  Expressions\n");
	fprintf(stderr, "are evaluated as real numbers and then clipped to the\n");
	fprintf(stderr, "bounds of the output image type:0-255,0-65535.\n");
	fprintf(stderr, "Numbers can be expressed as integers, floating point\n");
	fprintf(stderr, "numbers or in scientific notation.  Binary, octal and\n");
	fprintf(stderr, "hexadecimal numbers can be entered by prefixing them\n");
	fprintf(stderr, "with '0b', '0' or '0x' respectively.\n");
	fprintf(stderr, "        Expressions can be logical or real\n");
	fprintf(stderr, "valued.  An value of 0 represents a logical FALSE and\n");
	fprintf(stderr, "any other value represents TRUE.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "	The following variables are supported:\n");
	switch(which) {
		case 1:  /* math */
	fprintf(stderr, "A-W	Pixel value from input image A-W\n");
	fprintf(stderr, "X,Y,Z	Pixel location within the sample\n");
	fprintf(stderr, "MAXX,MAXY,MAXZ  Dimensions of the samples\n");
	fprintf(stderr, "ISAMP  The index number of the sample in time\n");
	fprintf(stderr, "FSAMP  The time of the sample in seconds\n");
			break;
		case 2: /* stat */
	fprintf(stderr, "DX,DY,DZ Pixel location within the kernal\n");
	fprintf(stderr, "PA	Pixel value at the voxel in question\n");
	fprintf(stderr, "PB	Pixel value at the kernal center\n");
	fprintf(stderr, "KA	Kernal weight at the voxel in question\n");
	fprintf(stderr, "KB	Center kernal weight\n");
	fprintf(stderr, "KS	Sum of the kernal weights\n");
			break;
	}
	fprintf(stderr, "\n");
	fprintf(stderr, "        The following operators are supported:\n");
	fprintf(stderr, "()      Controls the order of evaluation\n");
	fprintf(stderr, "+       Addition\n");
	fprintf(stderr, "-       Subtraction\n");
	fprintf(stderr, "*       Multiplication\n");
	fprintf(stderr, "/       Division\n");
	fprintf(stderr, "%%,MOD   Modulus operation (remainder)\n");
	fprintf(stderr, "\\,DIV   Integer division\n");
	fprintf(stderr, "==      Logical equals\n");
	fprintf(stderr, "!=      Logical not equal\n");
	fprintf(stderr, ">       Logical greater than\n");
	fprintf(stderr, "<       Logical less than\n");
	fprintf(stderr, ">=      Logical greater than or equal\n");
	fprintf(stderr, "<=      Logical less than or equal\n");
	fprintf(stderr, "!,NOT() Logical not\n");
	fprintf(stderr, "&&,AND  Logical and\n");
	fprintf(stderr, "||,OR   Logical or\n");
	fprintf(stderr, "~       Bitwise not\n");
	fprintf(stderr, "&       Bitwise and\n");
	fprintf(stderr, "|       Bitwise or\n");
	fprintf(stderr, ">>      Bitwise shift right (divide)\n");
	fprintf(stderr, "<<      Bitwise shift left (multiply)\n");
	fprintf(stderr, "INT()   Integer truncation\n");
	fprintf(stderr, "ABS()   Absolute value\n");
	fprintf(stderr, "TAN()   Tangent (All trig functions use radians)\n");
	fprintf(stderr, "COS()   Cosine\n");
	fprintf(stderr, "SIN()   Sin\n");
	fprintf(stderr, "ATAN()  Arctangent\n");
	fprintf(stderr, "ATAN2() Two value arctangent (dy,dx)\n");
	fprintf(stderr, "ACOS()  Arccosine\n");
	fprintf(stderr, "ASIN()  Arcsine\n");
	fprintf(stderr, "LN()    Log base e\n");
	fprintf(stderr, "SGN()   Sign of arg (-1 for a < 0, +1 for a >= 0)\n");
	fprintf(stderr, "EXP()   Exponentiation base e\n");
	fprintf(stderr, "RAND()  Returns pseudo-random number 0.0-1.0\n");
	fprintf(stderr, "^,POW   Raise to a power (x^y = x to the yth power)\n");
	fprintf(stderr, "        Note: ^,POW have the same precedence as multiplication and division\n");
	fprintf(stderr, "STAT(A,wh,dx,dy,dz) Compute basic stats on variable A\n");
	fprintf(stderr, "        Note: wh:0=N,1=sum,2=sumsqu,3=min,4=max\n");
	fprintf(stderr, "        dx,dy,dz=-1 for full axis otherwise specific axis\n");
	fprintf(stderr, "MSTAT(A,M,wh,dx,dy,dz) Same as STAT() but mask by variable M\n");
	return;
}
