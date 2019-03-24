void set_macro(char *name,char *value);
void get_macro(char *name,char *value);
void expand_macro(char *in,char *out);
void clear_macro(char *name);
void dump_macro(char *name);
long int init_parser(char *exp,long int tokenize, long int use_strs);
long int parse_math(char *out,double *value);
long int reset_tokenlist_ptr();
void set_user_identifier(long int (*proc)(char *,double *));

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
