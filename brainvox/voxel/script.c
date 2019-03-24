#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifndef WIN32
#include <unistd.h>
#else
#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#endif
#include <math.h>
#include <sys/types.h>
#include "voxel.h"
#include "macroprotos.h"
#include "gl.h"
#include "device.h"
#include "device.h"
#include "script.h"

#ifdef CAP_NOUS
#define execute_command_ EXECUTE_COMMAND
#define continue_command_ CONTINUE_COMMAND
#endif

static void read_palette(char *fname,unsigned long *pal, int which);
static void find_goto_label(FILE *fp,char *label);

static char	outtemp[200] = "image_%%%.rgb";
static int	imgcount = 1,imgstep = 1;

#define PAL_LUT 0
#define PAL_OPAC 1

/* fileptr stack */
static	FILE		*(inputs[10]);
static	long int	inputptr = 0;
static  char  		*pending_cmd = NULL;

void execute_command_(char *inbuff, int len)
{
	while(inputptr != 0) {
               	fclose(inputs[inputptr]);
               	inputptr--;
	}
/* Fortran interface */
	pending_cmd = (char *)malloc(len+1);
        memcpy(pending_cmd,inbuff,len);
        pending_cmd[len] = '\0';
}

void continue_command_(vset *set,int *more, 
	int *ret, float *arr, char *name, int namelen)
{
	long int num,ival,i,j,ivals[9],dx,dy;
	unsigned char *img;
	char cmd[256],cmd2[256],p[9][200];
	char outcmd[300],*s;
	double	d[9];

/* interpret a command */
	*ret = SCR_CMD_NONE;
	*more = 0;
next:
	if (pending_cmd) {
		strcpy(cmd,pending_cmd);
		free(pending_cmd);
		pending_cmd = NULL;
	} else {
/* NEXT command (if any) */
		while(inputptr != 0) {
/* check for abort */
			if (getbutton(ESCKEY)) {
				while(inputptr != 0) {
       		                 	fclose(inputs[inputptr]);
       		                 	inputptr--;
				}
				printf("User abort.\n");
				inputptr = 0;
				return;
			}
/* get next line */
			if (fgets(cmd,256,inputs[inputptr])) goto top;
			fclose(inputs[inputptr]);
			inputptr--;
		}
		inputptr = 0;
		return;
	}
 
/* clean and expand */
top:
        s = strchr(cmd,'\n'); if (s) *s = '\0';
        s = strchr(cmd,'\r'); if (s) *s = '\0';
	expand_macro(cmd,cmd2);
        if (cmd2[0] == '#') {
                /* comment */
		printf("%s\n",cmd2);
		goto next;
        }
        else if (cmd2[0] == '!') {
                system(cmd2+1);
		goto next;
	}
	else if (strncasecmp(cmd2,"ECHO ",5) == 0L) {
		printf("%s\n",cmd2+5);
		goto next;
        }
	if (init_parser(cmd2,0,1)) goto next;
/* parse the command */
	i = 0;
	j = parse_math(p[i],&(d[i]));
	if (j != 2) goto next;
	i++;
	while ((j != -1) && (i<9)) {
		j = parse_math(p[i],&(d[i]));
		i++;
	}
	num = i - 1;
	strcpy(cmd,p[0]);

/* Switch on the command name */  
        if ((strcasecmp(cmd,"ROT") == 0) && (num == 4)) {
                vl_rotations_(d[1],d[2],d[3],set);
		*ret = SCR_CMD_ROT;
		arr[0] = set->rots[0];
		arr[1] = set->rots[1];
		arr[2] = set->rots[2];
        }
        else if ((strcasecmp(cmd,"DROT") == 0) && (num == 4)) {
                d[1] = set->rots[0] + d[1];
                d[2] = set->rots[1] + d[2];
                d[3] = set->rots[2] + d[3];
                vl_rotations_(d[1],d[2],d[3],set);
		*ret = SCR_CMD_ROT;
		arr[0] = set->rots[0];
		arr[1] = set->rots[1];
		arr[2] = set->rots[2];
        }  
        else if ((strcasecmp(cmd,"ARBROT") == 0) && (num == 5)) {
                vl_arb_rotate_(&(d[1]),&(d[2]),set);
		*ret = SCR_CMD_ROT;
		arr[0] = set->rots[0];
		arr[1] = set->rots[1];
		arr[2] = set->rots[2];
        }
#if 0
        else if ((strcasecmp(cmd,"TRANS") == 0) && (num == 4)) {
                vl_translations_(d[1],d[2],d[3],set);
        }
        else if ((strcasecmp(cmd,"DTRANS") == 0) && (num == 4)) {
                d[1] = set->trans[0] + d[1];
                d[2] = set->trans[1] + d[2];
                d[3] = set->trans[2] + d[3];
                vl_translations_(d[1],d[2],d[3],set);
        }
#endif
        else if ((strcasecmp(cmd,"ILUT") == 0) && (num == 2)) {
		*ret = SCR_CMD_PAL;
		arr[0] = 4;
		strcpy(name,p[1]);
        }
        else if ((strcasecmp(cmd,"IOPAC") == 0) && (num == 2)) {
		*ret = SCR_CMD_PAL;
		arr[0] = 5;
		strcpy(name,p[1]);
        }
        else if ((strcasecmp(cmd,"LUT") == 0) && (num == 2)) {
                read_palette(p[1],(unsigned long *)set->rlut,PAL_LUT);
		*ret = SCR_CMD_PAL;
		arr[0] = 0;
		strcpy(name,p[1]);
        }
        else if ((strcasecmp(cmd,"AUXLUT") == 0) && (num == 2)) {
                read_palette(p[1],(unsigned long *)set->llut,PAL_LUT);
		*ret = SCR_CMD_PAL;
		arr[0] = 1;
		strcpy(name,p[1]);
        }
        else if ((strcasecmp(cmd,"OPAC") == 0) && (num == 2)) {
                read_palette(p[1],(unsigned long *)set->rlut,PAL_OPAC);
		*ret = SCR_CMD_PAL;
		arr[0] = 2;
		strcpy(name,p[1]);
        }
        else if ((strcasecmp(cmd,"AUXOPAC") == 0) && (num == 2)) {
                read_palette(p[1],(unsigned long *)set->llut,PAL_OPAC);
		*ret = SCR_CMD_PAL;
		arr[0] = 3;
		strcpy(name,p[1]);
        }
#if 0
        else if ((strcasecmp(cmd,"QUALITY") == 0) && (num == 2)) {
                sscanf(p[1],"%d",&ival);
                if (ival < 1) ival = 1;
                vl_speed_(ival,set);
        }
#endif
        else if (strcasecmp(cmd,"HWRENDER") == 0) {
		*ret = SCR_CMD_HWRENDER;
                sscanf(p[1],"%f",arr+0);
        }
        else if (strcasecmp(cmd,"AUTORENDER") == 0) {
		*ret = SCR_CMD_AUTORENDER;
                sscanf(p[1],"%f",arr+0);
        }
        else if (strcasecmp(cmd,"TEXRENDER") == 0) {
		*ret = SCR_CMD_TEXRENDER;
                sscanf(p[1],"%f",arr+0);
        }
        else if (strcasecmp(cmd,"RENDER") == 0) {
		*ret = SCR_CMD_RENDER;
        }
        else if (strcasecmp(cmd,"SAVERGB") == 0) {
                if (num == 1) {
                        vl_name_changer(outtemp,imgcount,&i,p[1]);
                        imgcount = imgcount + imgstep;
                }
		*ret = SCR_CMD_SAVE;
		strcpy(name,p[1]);
        }
        else if (strcasecmp(cmd,"AVERAGE") == 0) {
                vl_filter_output_((long int *)set->image,set->imagex,
                                  set->imagey,VL_AVERAGE);
		*ret = SCR_CMD_FILTER;
        }
        else if (strcasecmp(cmd,"ZAVERAGE") == 0) {
                vl_filter_output_((long int *)set->zbuffer,set->imagex,
                                  set->imagey,VL_ZAVERAGE);
		*ret = SCR_CMD_FILTER;
        }
        else if (strcasecmp(cmd,"MEDIAN") == 0) {
                vl_filter_output_((long int *)set->image,set->imagex,
                                  set->imagey,VL_MEDIAN);
		*ret = SCR_CMD_FILTER;
        }
        else if (strcasecmp(cmd,"ZMEDIAN") == 0) {
                vl_filter_output_((long int *)set->zbuffer,set->imagex,
                                  set->imagey,VL_ZMEDIAN);
		*ret = SCR_CMD_FILTER;
        }
        else if ((strcasecmp(cmd,"LIGHT") == 0) && (num == 5)) {
                vl_postlight_((long int *)set->image,set->zbuffer,
			(long int *)set->image,set->imagex,set->imagey,&(d[1]));
		*ret = SCR_CMD_FILTER;
        }
        else if ((strcasecmp(cmd,"TEMPLATE") == 0) && (num == 2)) {
                strcpy(outtemp,p[1]);
        }
        else if ((strcasecmp(cmd,"COUNT") == 0) && (num == 2)) {
                sscanf(p[1],"%d",&imgcount);
        }
        else if ((strcasecmp(cmd,"STEP") == 0) && (num == 2)) {
                sscanf(p[1],"%d",&imgstep);
        }   
        else if ((strcasecmp(cmd,"LIMITS") == 0) && (num == 7)) {
                sscanf(p[1],"%d",&(ivals[0]));
                sscanf(p[2],"%d",&(ivals[1]));
                sscanf(p[3],"%d",&(ivals[2]));
                sscanf(p[4],"%d",&(ivals[3]));
                sscanf(p[5],"%d",&(ivals[4]));
                sscanf(p[6],"%d",&(ivals[5]));
                vl_limits_(ivals[0],ivals[1],ivals[2],
                        ivals[3],ivals[4],ivals[5],set);
        }
        else if ((strcasecmp(cmd,"LIGHTOFF") == 0) && (num == 1)) {
		*ret = SCR_CMD_LIGHTING;
		arr[0] = 0.0;
        }
        else if ((strcasecmp(cmd,"LIGHTON") == 0) && (num == 6)) {
		*ret = SCR_CMD_LIGHTING;
		arr[0] = 1.0;
		arr[1] = d[1];
		arr[2] = d[2];
		arr[3] = d[3];
		arr[4] = d[4];
		arr[5] = d[5];
        }
        else if ((strcasecmp(cmd,"COLOR") == 0) && (num == 4)) {
                sscanf(p[1],"%d",&(ivals[0]));
                sscanf(p[2],"%d",&(ivals[1]));
                sscanf(p[3],"%d",&(ivals[2]));
                ival = ivals[0] + (ivals[1] << 8) + (ivals[2] << 16);
		set->backpack = ival;
		*ret = SCR_CMD_COLOR;
        }
        else if ((strcasecmp(cmd,"REBOUND") == 0)) {
		*ret = SCR_CMD_REBOUND;
	}
        else if ((strcasecmp(cmd,"DATA") == 0)) {
/* volume number */
		int q,swap=0;
		ivals[2] = 1; /* default volume 0=main,1=texture */
		ivals[3] = 0; /* default interp */
		ivals[4] = 1; /* default bytesperpixel (1,2,4) */
		ivals[5] = 0; /* default inmin */
		ivals[6] = 255; /* default inmax */
		ivals[7] = 0; /* default outmax */
		ivals[8] = 255; /* default outmax */
		for(q=2;q<num;q++) sscanf(p[q],"%d",&(ivals[q]));
                if ((ivals[2] != 1) && (ivals[2] != 0)) {
                   printf("Invalid target volume\n");
	           *ret = SCR_CMD_NONE;
                }
                if (ivals[4] < 0) {
                   swap = 512;
                   ivals[4] = -ivals[4];
                }
                if ((ivals[4] != 1) && (ivals[4] != 2) && (ivals[4] != 4)) {
                   printf("Invalid voxel depth\n");
	           *ret = SCR_CMD_NONE;
                }
		if (num < 6) ivals[4] |= 256;
                ivals[4] |= swap;
		*ret = SCR_CMD_DATA;
		strcpy(name,p[1]);
		for(q=2;q<=8;q++) arr[q-2] = ivals[q];
        }
        else if (strcasecmp(cmd,"CD") == 0) {
                if (num >= 2) chdir(p[1]);
                printf("pwd: %s\n",getcwd(p[2],200));
        }
        else if (strcasecmp(cmd,"EXIT") == 0) {
                exit(0);
        }
        else if (strcasecmp(cmd,"HELP") == 0) {
                printf("Commands:\n");
                printf("ROT,X,Y,Z\n");
                printf("DROT,X,Y,Z\n");
                printf("ARBROT,ANG,dX,dY,dZ\n");
#if 0
                printf("TRANS,X,Y,Z\n");
                printf("DTRANS,X,Y,Z\n");
#endif
                printf("ILUT,filename\n");
                printf("LUT,filename\n");
                printf("AUXLUT,filename\n");
                printf("IOPAC,filename\n");
                printf("OPAC,filename\n");
                printf("AUXOPAC,filename\n");
#if 0
                printf("QUALITY,int\n");
#endif
                printf("HWRENDER,on\n");
                printf("AUTORENDER,on\n");
                printf("TEXRENDER,on\n");
                printf("RENDER\n");
                printf("AVERAGE\n");
                printf("ZAVERAGE\n");
                printf("MEDIAN\n");
                printf("ZMEDIAN\n");
                printf("SAVERGB [,filename]\n");
                printf("TEMPLATE,filename\n");
                printf("COLOR,R,G,B\n");
                printf("REBOUND\n");
                printf("DATA,filename[,which[,interp[,bpv[,inmin,inmax]]]]\n");
                printf("     which=target volume (0=voxel,1=texture) default:1\n");
                printf("     interp=interpolate (0=no,1=yes) default:0\n");
                printf("     bpv=bytes per voxel (1,2,4) (negative for byteswap) default:1\n");
                printf("     inmin=input volume min default:autoscale\n");
                printf("     inmax=input volume max default:autoscale\n");
                printf("COUNT,int\n");
                printf("STEP,int\n");
                printf("LIMITS,xs,xe,ys,ye,zs,ze\n");
                printf("LIGHT,dx,dy,dz,inten\n");
                printf("LIGHTON,dx,dy,dz,srcinten,iminten\n");
                printf("LIGHTOFF\n");
                printf("CHAIN,filename\n");
                printf("CD,filename\n");
                printf("DUMP[,macro]\n");
                printf("TEMPSET,macro,'template',value\n");
                printf("SET,macro,value\n");
                printf("UNSET[,macro]\n");
                printf("LABEL,name\n");
                printf("GOTO,label\n");
                printf("IF,expression,GOTO,label\n");
                printf("! system command\n");
                printf("END\n");
                printf("EXIT\n");
                printf("Expand a macro with: $(macro)\n");
                printf("Use single quotes for strings: 'string'\n");
        }
/* special macro commands */
        else if ((strcasecmp(cmd,"SET") == 0L) && (num == 3)) {
                set_macro(p[1],p[2]);
        } else if ((strcasecmp(cmd,"TEMPSET") == 0L) && (num == 4)) {
/* tempset,macro,'template',number */
                ival = d[3];
                vl_name_changer(p[2],ival,&i,p[3]);
                set_macro(p[1],p[3]);
        } else if (strcasecmp(cmd,"DUMP") == 0L) {
                if (num == 1) dump_macro(0L);
                if (num != 1) dump_macro(p[1]);
        } else if (strcasecmp(cmd,"UNSET") == 0L) {
                if (num == 1) {
                        set_macro(0L,0L);
                } else if (num == 2) {
                        set_macro(p[1],0L);
                }
	}

/* special branching commands */
        else if ((strcasecmp(cmd,"LABEL") == 0) && (num == 2)) {
		/* we skip the labels */
        }
        else if ((strcasecmp(cmd,"CHAIN") == 0) && (num == 2)) {
/* make the stack one deeper */
		if (inputptr < 9) {
			inputs[inputptr+1] = fopen(p[1],"r");
			if (inputs[inputptr+1] != 0L) inputptr++;
		}
        }
        else if (strcasecmp(cmd,"END") == 0) {
/* pop the stack */
                if (inputptr != 0) {
                        fclose(inputs[inputptr]);
                        inputptr--;
                }
        }
        else if ((strcasecmp(cmd,"IF") == 0L) && (num == 4)) {
                if ((strcasecmp(p[2],"GOTO") == 0L) && (inputptr != 0)) {
                        if (d[1] != 0.0) find_goto_label(inputs[inputptr],p[3]);                } else {
                        printf("Invalid IF-GOTO statement.\n");
                	if (inputptr != 0) {
              	 	        fclose(inputs[inputptr]);
               		        inputptr--;
               		}
                }
        }	
        else if ((strcasecmp(cmd,"GOTO") == 0) && (num == 2)) {
                if (inputptr != 0) {
                        find_goto_label(inputs[inputptr],p[1]);
                } else {
                        printf("Invalid GOTO statement.\n");
                	if (inputptr != 0) {
              	 	        fclose(inputs[inputptr]);
               		        inputptr--;
               		}
                }
        }
        else {
                printf("Unknown command: %s\n",cmd);
               	if (inputptr != 0) {
               	        fclose(inputs[inputptr]);
            	        inputptr--;
               	}
        }   
	if (inputptr) *more = 1;
	return;
}

static void read_palette(char *file,unsigned long *lut, int type)
{
    unsigned char ftab[768];
    long int i;
    unsigned long j;

    (void) vl_read_pal_(file,ftab);
    if (type == PAL_LUT) {
          for (i=0; i<256; i++ ) {
                j = 1L*ftab[i] + 256L*ftab[i+256] + 65536L*ftab[i+512];
                lut[i] = (lut[i] & 0xff000000) | j;
          };
    } else if (type == PAL_OPAC) {
          for (i=0; i<256; i++ ) {
                j = lut[i] & 0x00ffffff;
                lut[i] = j | (((long)ftab[i] << 24) & 0xff000000);
          };
    }
}

static void find_goto_label(FILE *fp,char *label)
{
        char            tmp[256],tmp2[256],*s;
        double          d;
        long int        j,i;

/* start at the top and search for 'label name' */
        fseek(fp,0L,0);
/* while there are lines in the file */
        while (fgets(tmp,256,fp) != 0L) {
/* clean and expand the line */
                s = strchr(tmp,'\r'); if (s) *s = '\0';
                s = strchr(tmp,'\n'); if (s) *s = '\0';
		if (tmp[0] == '#') continue;
		if (tmp[0] == '!') continue;
                expand_macro(tmp,tmp2);
                if (init_parser(tmp2,0,1) == 0L) {
                        j = parse_math(tmp,&d);
/* if we find a LABEL XXX statement */
                        if ((j == 2) && (strcasecmp(tmp,"label") == 0L)) {
                                j = parse_math(tmp,&d);
/* and the next value is a string */
                                if ((j == 1) || (j == 2)) {
/* and it matches, we return leaving the file pointer here */
                                        if (strcasecmp(tmp,label) == 0L) return;
                                }
                        }
                }
        }
        return;
}

