void set_macro(char *name,char *value);
void get_macro(char *name,char *value);
void expand_macro(char *in,char *out);
void clear_macro(char *name);
void dump_macro(char *name);
long int init_parser(char *exp);
long int parse_math(char *out,double *value);

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
