#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "macroprotos.h"

void find_goto_label(FILE *fp,char *label)
{
	char 		tmp[256],tmp2[256];
	double 		d;
	long int 	j,i;
	
/* start at the top and search for 'label name' */
	fseek(fp,0L,0);
/* while there are lines in the file */
	while (fgets(tmp,256,fp) != 0L) {
/* clean and expand the line */
		for(i=0;i<strlen(tmp);i++) if (tmp[i] == '\n') tmp[i] = '\0';
		expand_macro(tmp,tmp2);
		if (init_parser(tmp2) == 0L) {
			j = parse_math(tmp,&d);
/* if we find a LABEL XXX statement */
			if ((j == 2) && (strcmp(tmp,"label") == 0L)) {
				j = parse_math(tmp,&d);
/* and the next value is a string */
				if ((j == 1) || (j == 2)) {
/* and it matches, we return leaving the file pointer here */
					if (strcmp(tmp,label) == 0L) return;
				}
			}
		}
	}
	return;
}

void do_a_command(char *buff) 
{
	char 		cmd[256],cmd2[256],tmp[256],p[7][80];
	double 		d[7];
	long int 	j,i;
	
	/* stack of input sources (starts with stdin) */
	FILE		*(inputs[10]);
	long int	inputptr = 0;
	
	strcpy(cmd,buff);
top:
/* clean and expand the line */
	for(i=0;i<strlen(cmd);i++) if (cmd[i] == '\n') cmd[i] = '\0';
	expand_macro(cmd,cmd2);
	if (init_parser(cmd2)) return;
/* get the command */
	i = 0;
	j = parse_math(p[i],&(d[i]));
#ifdef DEBUG 
	printf("Parsing:\n");
	printf("j = %ld : i = %ld : str = %s : val = %lf\n",j,i,p[i],d[i]);
#endif
	if (j != 2) return;
	i++;
	while ((j != -1) && (i < 7)) {
		j = parse_math(p[i],&(d[i]));
#ifdef DEBUG 
		printf("j = %ld : i = %ld : str = %s : val = %lf\n",j,i,p[i],d[i]);
#endif
		i++;
	}
	i--;
	if (strcmp(p[0],"exit") == 0L) {
		exit(0);
		return;
	} else if ((strcmp(p[0],"set") == 0L) && (i > 2)) {
		set_macro(p[1],p[2]);
	} else if ((strcmp(p[0],"echo") == 0L) && (i > 1)) {
		printf("%s\n",strstr(cmd2,"echo")+5);
	} else if (strcmp(p[0],"dump") == 0L) {
		if (i == 1) dump_macro(0L);
		if (i != 1) dump_macro(p[1]);
	} else if (strcmp(p[0],"unset") == 0L) {
		if (i == 1) {
			set_macro(0L,0L);
		} else if (i == 2) {
			set_macro(p[1],0L);
		}
	} else if ((strcmp(p[0],"chain") == 0L) && (i == 2)) {
		if (inputptr < 9) {
			inputs[inputptr+1] = fopen(p[1],"r");
			if (inputs[inputptr+1] != 0L) inputptr++;
		}
	} else if ((strcmp(p[0],"if") == 0L) && (i == 4)) {
		if ((strcmp(p[2],"goto") == 0L) && (inputptr != 0)) {
			if (d[1] != 0.0) find_goto_label(inputs[inputptr],p[3]);
		} else {
			printf("Invalid IF () GOTO statement.\n");
		}
	} else if ((strcmp(p[0],"goto") == 0L) && (i == 2)) {
		if (inputptr != 0) {
			find_goto_label(inputs[inputptr],p[1]);
		} else {
			printf("Invalid GOTO statement.\n");
		}
	} else if ((strcmp(p[0],"end") == 0L) && (i == 1)) {
		if (inputptr != 0) {
			fclose(inputs[inputptr]);
			inputptr--;
		}
	}
/* next line */
	while (inputptr != 0) {
		if (fgets(cmd,256,inputs[inputptr]) != 0L) goto top;
		fclose(inputs[inputptr]);
		inputptr--;
	}
	return;
}

void main()
{
	int 	done;
	char	cmd[256];
	
	done = 0;	
	while (!done) {
		printf("Enter a command>");
		if (fgets(cmd,256,stdin) == 0L) break;
		do_a_command(cmd);
		printf("\n");
	}
}

