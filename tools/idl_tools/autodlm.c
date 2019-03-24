#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define MAX_SIGS        200
#define MAX_PARAMS      20

typedef struct  {
        int     type;
        int     is_out;
        int     passbyref;
        int     ndims;
        char    dims[8][80];
} idlarg;

typedef struct  {
        char    name[64];
        char    idlname[64];
        int     is_fun;
        int     nargs;
        idlarg  args[MAX_PARAMS];
} signature;

/*
 * Input line format:
 * FUN|PRO libname idlname arg ...
 *
 * Where arg has the format:
 * {o}{*}VOID|BYTE|SHORT|USHORT|LONG|ULONG|FLOAT|DOUBLE{[{len},...]}
 * len must be a C expression for the length of an array (if it should
 * be checked).  The C variable "nX" is the value of the parameter "X"
 * expressed as a long int.  o=output  *=pass by reference
 */

static  signature       list[MAX_SIGS];

int main(int argc, char **argv)
{
        char    tmp[256];
        FILE    *in = NULL,*out = NULL;
        int     iNum,i,j,iCount;
        signature       *sig;
        time_t  build_time;

/* cmdline check */
        if (argc != 3) {
                fprintf(stderr,"Usage: %s infile
outbasename\n",argv[0]);
                exit(1);
        }

/* get the input file */
        in = fopen(argv[1],"r");
        if (!in) {
                fprintf(stderr,"Unable to read the file: %s\n",argv[1]);
                goto err_out;
        }

/* for each line in the input file */
        iNum = 0;
        while (fgets(tmp,256,in)) {
                char            *p,*q,*tok;
                char            parsed[MAX_PARAMS][100];

/* parse the string */
                p = tmp;
                iCount = 0;
                while((tok = strtok(p,"\t ")) && (iCount < MAX_PARAMS))
{
                        if (tok[0] == '#') break;
                        strcpy(parsed[iCount],tok);
                        iCount++;
                        p = NULL;
                }

/* create a signature */
                sig = &(list[iNum]);
                if (strcmp(parsed[0],"PRO")==0) {
                        sig->is_fun = 0;
                } else if (strcmp(parsed[0],"FUN")==0) {
                        sig->is_fun = 1;
                } else {
                        continue;
                }
                strcpy(sig->name,parsed[1]);
                p = parsed[2];
                while(*p) {
                        p[0] = (char)toupper((int)p[0]);
                        p++;
                }
                strcpy(sig->idlname,parsed[2]);

/* get the args */
                sig->nargs = iCount - 3;
                for (i=0;i<sig->nargs;i++) {
                        p = parsed[i+3];
                        sig->args[i].is_out = 0;
                        if (p[0] == 'o') {
                                sig->args[i].is_out = 1;
                                p++;
                        }
                        sig->args[i].passbyref = 0;
                        if (p[0] == 'v') {
                                sig->args[i].passbyref = 1;
                                p++;
                        }
                        sig->args[i].type = 0;
                        if (strstr(p,"VOID")) {
                                sig->args[i].type = 0;
                        } else if (strstr(p,"BYTE")) {
                                sig->args[i].type = 1;
                        } else if (strstr(p,"SHORT")) {
                                sig->args[i].type = 2;
                        } else if (strstr(p,"USHORT")) {
                                sig->args[i].type = 3;
                        } else if (strstr(p,"LONG")) {
                                sig->args[i].type = 4;
                        } else if (strstr(p,"ULONG")) {
                                sig->args[i].type = 5;
                        } else if (strstr(p,"FLOAT")) {
                                sig->args[i].type = 6;
                        } else if (strstr(p,"DOUBLE")) {
                                sig->args[i].type = 7;
                        } else {
                                fprintf(stderr,"Unknown type: %s, LONG
used.\n",
                                        p);
                                sig->args[i].type = 4;
                        }
                        p = strstr(p,"[");
                        if (p) {
                                j = 0;
                                p++;
                                while(1) {
                                        q = strstr(p,",");
                                        if (q) {
                                                *q = '\0';
                                                strcpy(sig->args[i].dims[j],p);
                                                j++;
                                        } else {
                                                q = strstr(p,"]");
                                                if (q) *q = '\0';
                                                strcpy(sig->args[i].dims[j],p);
                                                j++;
                                                break;
                                        }
                                        p = q + 1;
                                }
                                sig->args[i].ndims = j;
                        } else {
                                sig->args[i].ndims = 0;
                        }
                }
/* sanity check */
                if (sig->is_fun) {
                        if (!sig->args[0].is_out && sig->nargs) {
                                fprintf(stderr,"First FUN arg must be
out.\n");
                                continue;
                        }
                } else {
                        if (sig->args[0].type && sig->nargs) {
                                fprintf(stderr,"First PRO arg must be
void.\n");
                                continue;
                        }
                }

/* next routine */
                iNum++;
                if (iNum >= MAX_SIGS) break;
        }
        fclose(in);

        if (!iNum) exit(0);

/* the .c file */
        strcpy(tmp,argv[2]);  strcat(tmp,".c");
        out = fopen(tmp,"w");
        if (!out) {
                fprintf(stderr,"Unable to open the file: %s\n",tmp);
                goto err_out;
        }
        fprintf(out,"/* computer generated DLM from: %s */\n",argv[1]);
        fprintf(out,"#include <stdio.h>\n#include \"export.h\"\n\n");
        fprintf(out,"#define ARRLEN(arr)
(sizeof(arr)/sizeof(arr[0]))\n\n");
        fprintf(out,"static IDL_MSG_DEF msg_arr[] = { \n");
        fprintf(out,"#define M_ERROR 0 \n");
        fprintf(out,"\t{ \"M_ERROR\", \"%%NError: %%s.\"},\n");
        fprintf(out,"};\n\nstatic IDL_MSG_BLOCK msg_block;\n\n");

        sig = list;
        for(i=0;i<iNum;i++) {
                if (sig->is_fun) {
                        fprintf(out,"static IDL_CDECL void intf_%s(int
argc, IDL_VPTR *argv, char *argk)\n",sig->name);
                } else {
                        fprintf(out,"static IDL_CDECL IDL_VPTR
intf_%s(int argc, IDL_VPTR *argv, char *argk)\n",sig->name);
                }
                fprintf(out,"{\n");

                if (sig->is_fun) {
                        fprintf(out,"\treturn(IDL_GettmpLong(-1));\n");
                        fprintf(out,"}\n");
                } else {
                        fprintf(out,"\treturn;\n");
                        fprintf(out,"}\n");
                }
                sig++;
                fprintf(out,"\n");
        }

        fprintf(out,"static IDL_SYSFUN_DEF function_addr[] = {\n");
        sig = list;
        for(i=0;i<iNum;i++) {
                if (sig->is_fun) {
                        fprintf(out,"\t{intf_%s,\"%s\", %d, %d,
0},\n",sig->name,
                                sig->idlname,sig->nargs-1,sig->nargs-1);
                }
                sig++;
        }
        fprintf(out,"};\n\n");

        fprintf(out,"static IDL_SYSFUN_DEF procedure_addr[] = {\n");
        sig = list;
        for(i=0;i<iNum;i++) {
                if (!sig->is_fun) {
                        fprintf(out,"\t{(IDL_FUN_RET)intf_%s,\"%s\", %d,
%d, 0},\n",
                                sig->name,sig->idlname,sig->nargs-1,
                                sig->nargs-1);
                }
                sig++;
        }
        fprintf(out,"};\n\n");

        fprintf(out,"int IDL_Load(void)\n {\n");
        fprintf(out,"\tif (!(msg_block = IDL_MessageDefineBlock(\"%s\",
\n\t\tARRLEN(msg_arr),msg_arr))) return IDL_FALSE;\n\n",argv[2]);
        fprintf(out,"\tif (!IDL_AddSystemRoutine(function_addr, TRUE,
\n\t\tARRLEN(function_addr))) return IDL_FALSE;\n\n");
        fprintf(out,"\tif (!IDL_AddSystemRoutine(procedure_addr, FALSE,
\n\t\tARRLEN(function_addr))) return IDL_FALSE;\n\n");
        fprintf(out,"\treturn(IDL_TRUE);\n");
        fprintf(out,"}\n");
        fclose(out);

/* the .DLM file */
        strcpy(tmp,argv[2]);  strcat(tmp,".dlm");
        out = fopen(tmp,"w");
        if (!out) {
                fprintf(stderr,"Unable to open the file: %s\n",tmp);
                goto err_out;
        }
        fprintf(out,"MODULE %s\n",argv[2]);
        fprintf(out,"DESCRIPTION DLM for module %s\n",argv[2]);
        fprintf(out,"VERSION 1.0\n");
        fprintf(out,"SOURCE Anonymous\n");
        build_time = time(NULL);
        fprintf(out,"BUILD_DATE %s",ctime(&build_time));
        sig = list;
        for(i=0;i<iNum;i++) {
                if (sig->is_fun) {
                        fprintf(out,"FUNCTION\t");
                } else {
                        fprintf(out,"PROCEDURE\t");
                }
                fprintf(out,"%s\t%d\t%d\n",sig->idlname,
                        sig->nargs-1,sig->nargs-1);
                sig++;
        }
        fclose(out);

        exit(0);

/* error exit */
err_out:
        if (in) fclose(in);
        if (out) fclose(out);

        exit(1);
}