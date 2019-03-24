#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "globals.h"
#include "protos.h"
#include "vl_polygon.h"
#include <math.h>

#include <sys/types.h>
#include <sys/time.h>

#include "macroprotos.h"

void    loadset(char *fname);
void 	find_goto_label(FILE *fp,char *label);

vset	set;
evset	set_ext;

char	dataset[80];
char	outtemp[80];
int	imgcount,imgstep;
unsigned char *undo_buffer;
long int d_histo[256];
long int light_flag;

#define PAL_LUT 0
#define PAL_OPAC 1

#define GRAB_UNDO if (undo_buffer != 0L) memcpy(undo_buffer,set.image,set.imagex*set.imagey*(sizeof(int)+sizeof(short)))

void cmdline_init()
{
	long int dx,dy;
	unsigned char *img;
	long int i,lut[256];

	strcpy(dataset,"");
	strcpy(outtemp,"image_%%%.rgb");
	imgcount = 1;
	imgstep = 1;
	light_flag = 0;
        vl_init_(&set);
	dx = 400;
	dy = 400;
	img = malloc((sizeof(int)+sizeof(short))*dx*dy);
	undo_buffer = malloc((sizeof(int)+sizeof(short))*dx*dy);
        vl_image_(dx,dy,img,(short *)(img+(sizeof(int)*dx*dy)),&set);
	buildimage();

/* allocate 2d image buffer */
	imagex2d = 256;
	imagey2d = 256;
	image2d_data = malloc(imagex2d*imagey2d);
	getset_2d_vals(1);
	buildimage2d();

/* init lookup tables */
	for(i=0;i<256;i++) lut[i] = (i) | (i << 8) | (i << 16) | (i << 24);
	i = 0;

        vl_lookups_(lut,lut,i,&set);

/* init the 2D image lut as well */
	for(i=0;i<256;i++) i2dlut[i] = lut[i];

	changelut(1);

	update_status();
}

void do_a_command(buff)
char *buff;
{
	long int num,ival,i,j,ivals[7],dx,dy;
	unsigned char *img;
	char cmd[256],cmd2[256],p[7][80];
	double	d[7];

        struct timeval before,after;
        struct timezone zone;
        float time1,time2;

/* structures to hold a polygon data description */
	vl_object *v_objects;
	vl_vertex *v_verts;
	vl_light *v_lights;
	vl_material *v_materials;
	long int n_objects,n_verts,n_lights,n_materials;

/* fileptr stack */
	FILE		*(inputs[10]);
	long int	inputptr = 0;

	strcpy(cmd,buff);
/* interpret a command */
top:
/* clean and expand */
	for(i=0;i<strlen(cmd);i++) if (cmd[i] == '\n') cmd[i] = '\0';
	expand_macro(cmd,cmd2);
	if (init_parser(cmd2)) return;
        if (cmd2[0] == '#') {
                /* comment */
		goto next;
        }
        else if (cmd2[0] == '!') {
                system(cmd2+1);
		goto next;
	}
	else if (strncasecmp(cmd2,"ECHO ",5) == 0L) {
		goto next;
        }
/* parse the command */
	i = 0;
	j = parse_math(p[i],&(d[i]));
	if (j != 2) return;
	i++;
	while ((j != -1) && (i<7)) {
		j = parse_math(p[i],&(d[i]));
		i++;
	}
	num = i - 1;
	strcpy(cmd,p[0]);

/* WORKING */  
        if ((strcasecmp(cmd,"ROT") == 0) && (num == 4)) {
                vl_rotations_(d[1],d[2],d[3],&set);
        }
        else if ((strcasecmp(cmd,"DROT") == 0) && (num == 4)) {
                d[1] = set.rots[0] + d[1];
                d[2] = set.rots[1] + d[2];
                d[3] = set.rots[2] + d[3];
                vl_rotations_(d[1],d[2],d[3],&set);
        }  
        else if ((strcasecmp(cmd,"ARBROT") == 0) && (num == 5)) {
                vl_arb_rotate_(&(d[1]),&(d[2]),&set);
        }
        else if ((strcasecmp(cmd,"TRANS") == 0) && (num == 4)) {
                vl_translations_(d[1],d[2],d[3],&set);
        }
        else if ((strcasecmp(cmd,"DTRANS") == 0) && (num == 4)) {
                d[1] = set.trans[0] + d[1];
                d[2] = set.trans[1] + d[2];
                d[3] = set.trans[2] + d[3];
                vl_translations_(d[1],d[2],d[3],&set);
        }
        else if ((strcasecmp(cmd,"RLUT") == 0) && (num == 2)) {
                read_palette(p[1],set.rlut,PAL_LUT);
/*                vl_lookups_(rlut,llut,bcolor,&set); */
        }
        else if ((strcasecmp(cmd,"LLUT") == 0) && (num == 2)) {
                read_palette(p[1],set.llut,PAL_LUT);
/*                vl_lookups_(rlut,llut,bcolor,&set); */
        }
        else if ((strcasecmp(cmd,"ROPAC") == 0) && (num == 2)) {
                read_palette(p[1],set.rlut,PAL_OPAC);
/*                vl_lookups_(rlut,llut,bcolor,&set); */
        }
        else if ((strcasecmp(cmd,"LOPAC") == 0) && (num == 2)) {
                read_palette(p[1],set.llut,PAL_OPAC);
/*                vl_lookups_(rlut,llut,bcolor,&set); */
        }
        else if ((strcasecmp(cmd,"QUALITY") == 0) && (num == 2)) {
                sscanf(p[1],"%d",&ival);
                if (ival < 1) ival = 1;
                vl_speed_(ival,&set);
        }
        else if (strcasecmp(cmd,"BACKGROUND") == 0) {
                /* not useful in the cmdline Unix fblast */
        }
        else if (strcasecmp(cmd,"FOREGROUND") == 0) {
                /* not useful in the cmdline Unix fblast */
        }     
        else if ((strcasecmp(cmd,"UNDO") == 0) && (num == 1)) {
                if (undo_buffer != 0L) {
			memcpy(set.image,undo_buffer,set.imagex*set.imagey*
                                (sizeof(int )+sizeof(short)));
			buildimage();
                }
        }
        else if ((strcasecmp(cmd,"UNDO") == 0) && (num == 2)) {
                if ((strcasecmp(p[1],"ON") == 0) && (undo_buffer == 0)) {
                        undo_buffer = malloc(set.imagex*set.imagey*(sizeof(int )+sizeof(short)));
                } else if ((strcasecmp(p[1],"OFF") == 0)
                                && (undo_buffer != 0)) {
                        free(undo_buffer);
                        undo_buffer = 0;
                } else if ((strcasecmp(p[1],"IMAGE") == 0)
                                && (undo_buffer != 0)) {
			memcpy(set.image,undo_buffer,
				set.imagex*set.imagey*(sizeof(int)));
			buildimage();
                } else if ((strcasecmp(p[1],"ZBUFFER") == 0)
                                && (undo_buffer != 0)) {
			memcpy(set.zbuffer,
			undo_buffer+(set.imagex*set.imagey*sizeof(int)),
			set.imagex*set.imagey*(sizeof(short)));
                }
        }
        else if ((strcasecmp(cmd,"RENDER") == 0) ||
                (strcasecmp(cmd,"BGRENDER") == 0)) {
                GRAB_UNDO;
                printf("Rendering...\n");
                (void) gettimeofday(&before,&zone);
                vl_render_((VL_BACKGROUND | light_flag),0,&i,&set);
                (void) gettimeofday(&after,&zone);
                printf("Renderer returned:%d\n",i);
                time1 = ((float)before.tv_usec/1000000.0);
                time2 = ((float)after.tv_usec/1000000.0);
                time2 = time2 + (float)(after.tv_sec - before.tv_sec);
                i = (set.d[0])*(set.d[1])*(set.d[2]);
                printf("%d voxels in %f seconds = %f voxels per sec\n",
                         i,time2-time1, ((float)(i))/(time2-time1));
		buildimage();
        }
        else if (strcasecmp(cmd,"NCRENDER") == 0) {
                GRAB_UNDO;
                printf("Rendering...\n");
                (void) gettimeofday(&before,&zone);
                vl_render_((VL_BACKGROUND | VL_RENDEROVER | light_flag),
			0,&i,&set);
                (void) gettimeofday(&after,&zone);
                printf("Renderer returned:%d\n",i);
                time1 = ((float)before.tv_usec/1000000.0);
                time2 = ((float)after.tv_usec/1000000.0);
                time2 = time2 + (float)(after.tv_sec - before.tv_sec);
                i = (set.d[0])*(set.d[1])*(set.d[2]);
                printf("%d voxels in %f seconds = %f voxels per sec\n",
                         i,time2-time1, ((float)(i))/(time2-time1));
		buildimage();
        }
        else if ((strcasecmp(cmd,"POLYRENDER") == 0) && (num == 2)) {
                GRAB_UNDO;
                vl_polygon_read_(p[1],&v_objects,&v_verts,&v_lights,
                        &v_materials,&n_objects,&n_verts,&n_lights,
                        &n_materials,&i);
                printf("Read:%d objects, %d vertex, %d lights, %d materials\n"
			,n_objects,n_verts,n_lights,n_materials);
                if (i == 0) {
                	printf("Rendering...\n");
                	vl_polygon_render_(&set,v_objects,v_verts,v_lights,
                        	v_materials,0L);
                }
                vl_polygon_free_(&v_objects,&v_verts,&v_lights,
                        &v_materials);
		buildimage();
        }
        else if (strcasecmp(cmd,"SAVERGB") == 0) {
                if (num == 1) {
                        vl_name_changer(outtemp,imgcount,&i,p[1]);
                        imgcount = imgcount + imgstep;
                }
                printf("Saving RGB color image %s\n",p[1]);
                svimg_(p[1],set.image,set.imagex,set.imagey,3,
                        1,0,0,set.imagex,set.imagey,&i);
                if (i == -1 ) {
                        fprintf(stderr,"Unable to save %s\n",p[1]);
                }
        }
        else if (strcasecmp(cmd,"SAVEGS") == 0) {
                if (num == 1) {
                        vl_name_changer(outtemp,imgcount,&i,p[1]);
                        imgcount = imgcount + imgstep;
                }
                printf("Saving grayscale image %s\n",p[1]);
                svimg_(p[1],set.image,set.imagex,set.imagey,1,
                        1,0,0,set.imagex,set.imagey,&i);
                if (i == -1 ) {
                        fprintf(stderr,"Unable to save %s\n",p[1]);
                }
        }
        else if (strcasecmp(cmd,"AVERAGE") == 0) {
                GRAB_UNDO;
                vl_filter_output_((long int *)set.image,set.imagex,set.imagey,VL_AVERAGE);
		buildimage();
        }
        else if (strcasecmp(cmd,"ZAVERAGE") == 0) {
                GRAB_UNDO;
               vl_filter_output_((long int *)set.zbuffer,set.imagex,set.imagey,VL_ZAVERAGE);
        }
        else if (strcasecmp(cmd,"MEDIAN") == 0) {
                GRAB_UNDO;
                vl_filter_output_((long int *)set.image,set.imagex,set.imagey,VL_MEDIAN);
		buildimage();
        }
        else if (strcasecmp(cmd,"ZMEDIAN") == 0) {
                GRAB_UNDO;
                vl_filter_output_((long int *)set.zbuffer,set.imagex,set.imagey,VL_ZMEDIAN);
        }
        else if ((strcasecmp(cmd,"LIGHT") == 0) && (num == 5)) {
                GRAB_UNDO;
                vl_postlight_((long int *)set.image,set.zbuffer,
			(long int *)set.image,set.imagex,set.imagey,&(d[1]));
		buildimage();
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
        else if ((strcasecmp(cmd,"IMAGE") == 0) && (num == 3)) {
                sscanf(p[1],"%d",&dx);
                sscanf(p[2],"%d",&dy);
                if ((dx * dy <= 0) ||
                         (dx * dy > 1024*1024)) {
                        dx = 400;
                        dy = 400;
                }
                free(set.image);
                img = malloc(dx*dy*(sizeof(int )+sizeof(short)));
                if (img == NULL) {
                        fprintf(stderr,"Unable to allocate image memory.\n");
                        exit(1);
                }
                (void) vl_image_(dx,dy,img,(short*)(img+(sizeof(int )*dx*dy)),&set);
                if (undo_buffer != 0L) {
                        free(undo_buffer);
                        undo_buffer=malloc(dx*dy*(sizeof(int )+sizeof(short)));
                }
		buildimage();
        }
        else if ((strcasecmp(cmd,"LIMITS") == 0) && (num == 7)) {
                sscanf(p[1],"%d",&(ivals[0]));
                sscanf(p[2],"%d",&(ivals[1]));
                sscanf(p[3],"%d",&(ivals[2]));
                sscanf(p[3],"%d",&(ivals[3]));
                sscanf(p[3],"%d",&(ivals[4]));
                sscanf(p[3],"%d",&(ivals[5]));
                vl_limits_(ivals[0],ivals[1],ivals[2],
                        ivals[3],ivals[4],ivals[5],&set);
        }
        else if ((strcasecmp(cmd,"LIGHTOFF") == 0) && (num == 1)) {
		light_flag = 0;
        }
        else if ((strcasecmp(cmd,"LIGHTON") == 0) && (num == 6)) {
		light_flag = VL_RENDERLIGHT;
		vl_set_light_param_(VL_ILIGHT_DX,&(d[1]));
		vl_set_light_param_(VL_ILIGHT_DY,&(d[2]));
		vl_set_light_param_(VL_ILIGHT_DZ,&(d[3]));
		vl_set_light_param_(VL_ILIGHT_INTEN,&(d[4]));
		vl_set_light_param_(VL_ILIGHT_SRCINTEN,&(d[5]));
        }
        else if ((strcasecmp(cmd,"PLANEOFF") == 0) && (num == 1)) {
                ival = 0;
                vl_plane_(ival,&(d[1]),&(d[4]),&set);
        }
        else if ((strcasecmp(cmd,"PLANEON") == 0) && (num == 7)) {
                ival = 1;
                vl_plane_(ival,&(d[1]),&(d[4]),&set);
        }
        else if ((strcasecmp(cmd,"COLOR") == 0) && (num == 4)) {
                sscanf(p[1],"%d",&(ivals[0]));
                sscanf(p[2],"%d",&(ivals[1]));
                sscanf(p[3],"%d",&(ivals[2]));
                ival = ivals[0] + (ivals[1] << 8) + (ivals[2] << 16);
		set.backpack = ival;
/*                vl_lookups_(rlut,llut,bcolor,&set); */
        }
        else if (strcasecmp(cmd,"CLEAR") == 0) {
		set.backpack = 0;
/*                vl_lookups_(rlut,llut,bcolor,&set); */
        }
        else if (strcasecmp(cmd,"ICLEAR") == 0) {
		vl_polygon_render_(&set,NULL,NULL,NULL,NULL,VL_P_ICLEAR);
		buildimage();
	}
        else if (strcasecmp(cmd,"ZCLEAR") == 0) {
		vl_polygon_render_(&set,NULL,NULL,NULL,NULL,VL_P_ZCLEAR);
	}
        else if ((strcasecmp(cmd,"DATA") == 0) && (num == 2)) {
                loadset(p[1]);
                strcpy(dataset,p[1]);
        }
        else if ((strcasecmp(cmd,"PRINT") == 0) && (num == 1)) {
                printf("Dataset:%s\n",dataset);
                printf("Output template:%s Cnt:%d Stp:%d\n",
                        outtemp,imgcount,imgstep);
                printf("Data limits x:%d,%d y:%d,%d z:%d,%d\n",
			set.start[0],set.end[0],
			set.start[1],set.end[1],
			set.start[2],set.end[2]);
                printf("Image X:%d Y:%d\n",set.imagex,set.imagey);
                printf("Back color:%d %d %d\n",(set.backpack & 0xff),
			(set.backpack & 0xff00) >> 8,
			(set.backpack & 0xff0000) >> 16);
                printf("Rotation:%f %f %f\n",
			set.rots[0],set.rots[1],set.rots[2]);
                printf("Translation:%f %f %f\n",
                        set.trans[0],set.trans[1],set.trans[2]);
                printf("Speed:%d\n",set.speed);
                if (set.plane == 0) {
                        printf("Cutting plane is OFF\n");
                } else {
                        printf("Cutting plane is ON\n");
                    printf("Plane point:%f %f %f\n",set.planept[0],
				set.planept[1],set.planept[2]);
                    printf("Plane normal:%f %f %f\n",set.planevec[0],
				set.planevec[1],set.planevec[2]);
                }
        }
        else if (strcasecmp(cmd,"CD") == 0) {
                if (num >= 2) chdir(p[1]);
                printf("pwd: %s\n",getcwd(p[2],80));
        }
        else if (strcasecmp(cmd,"EXIT") == 0) {
                exit(0);
        }
        else if ((cmd[0] == '?') || (strcasecmp(cmd,"HELP") == 0)) {
                printf("Commands:\n");
                printf("ROT X Y Z\n");
                printf("DROT X Y Z\n");
                printf("ARBROT ANG dX dY dZ\n");
                printf("TRANS X Y Z\n");
                printf("DTRANS X Y Z\n");
                printf("RLUT filename\n");
                printf("LLUT filename\n");
                printf("ROPAC filename\n");
                printf("LOPAC filename\n");
                printf("QUALITY int\n");
                printf("RENDER or BGRENDER\n");
                printf("NCRENDER\n");
                printf("POLYRENDER filename\n");
                printf("BACKGROUND\n");
                printf("FOREGROUND\n");
                printf("AVERAGE\n");
                printf("ZAVERAGE\n");
                printf("MEDIAN\n");
                printf("ZMEDIAN\n");
                printf("SAVERGB [filename]\n");
                printf("SAVEGS [filename]\n");
                printf("TEMPLATE filename\n");
                printf("COLOR R G B\n");
                printf("CLEAR\n");
                printf("ICLEAR\n");
                printf("ZCLEAR\n");
                printf("DATA filename\n");
                printf("COUNT int\n");
                printf("STEP int\n");
                printf("LIMITS xs xe ys ye zs ze\n");
                printf("IMAGE dx dy\n");
                printf("LIGHT dx dy dz inten\n");
                printf("LIGHTON dx dy dz iminten srcinten\n");
                printf("LIGHTOFF\n");
                printf("PLANEON px py pz nx ny nz\n");
                printf("PLANEOFF\n");
                printf("CHAIN filename\n");
                printf("UNDO [ON]|[OFF]|[IMAGE]|[ZBUFFER]\n");
                printf("PRINT\n");
                printf("CD filename\n");
                printf("! system command\n");
                printf("EXIT\n");
        }

/* special macro commands */
        else if ((strcasecmp(cmd,"SET") == 0L) && (num > 2)) {
                set_macro(p[1],p[2]);
/* should be handled elsewhere 
        } else if ((strcasecmp(cmd,"ECHO") == 0L) && (num > 1)) {
                printf("%s\n",strstr(cmd2,"echo")+5);
*/
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
                        printf("Invalid IF () GOTO statement.\n");
                }
        }	
        else if ((strcasecmp(cmd,"GOTO") == 0) && (num == 2)) {
                if (inputptr != 0) {
                        find_goto_label(inputs[inputptr],p[1]);
                } else {
                        printf("Invalid GOTO statement.\n");
                }
        }
        else {
                printf("huh?? %s\n",buff);
        }   
	update_status();

next:
/* NEXT command (if any) */
	while(inputptr != 0) {
/* check for abort */
		if (vl_rcallback(-1L,&set)) {
			while(inputptr != 0) {
                        	fclose(inputs[inputptr]);
                        	inputptr--;
			}
			return;
		}
/* get next line */
		if (fgets(cmd,256,inputs[inputptr]) != 0L) goto top;
		fclose(inputs[inputptr]);
		inputptr--;
	}
	return;
}

void    loadset(char *fname)
{
        long int start,end,step,dx,dy,interp;
        float ipixel,islice,squf;
        char tname[256],text[256],units[256];
        long int count,i,head,j,k;
        unsigned char *data;
        unsigned long int csum;

        if (tloaddefs(fname,&ipixel,&islice,&start,&end,&step,&dx,&dy,&head,tname,units)
		== -1) {
                fprintf(stderr,"Unable to read file:%s\n",fname);
                return;
        }

/* free old memory if needed */
        if (set.data != 0) {
                free((set.data));
        }

/* count the images */
        count =0;
        for(i=start; i<=end; i = i + step) count=count+1;
/* get the squeeze factor */
        squf = 10.0;
        interp = -1;
        while ((squf > 1.00) && (interp < 20)) {
                interp = interp + 1;
                squf = (((float)count)*islice)/
                        ((float)(count+ (interp*(count-1)))*ipixel);
        }
        printf("Interp = %d  squf = %f \n",interp,squf);

/* handle interpolations */
        count = count + (interp * (count-1));

/* allocate memory for the data x*y*count */
        (void) printf("Allocating %d bytes of memory.\n",count*dx*dy);
        data = malloc((count*dx*dy));
        if (data == NULL) {
                (void) printf("Unable to allocate image memory.\n");
                exit(1);
        }

/* read the images */
        (void) vl_read_images_h_(tname,dx,dy,start,end,step,interp,data,head);

/* set the dataset & squeeze factor */
        (void) vl_data_(dx,dy,count,data,&set);
        (void) vl_image_stats_(dx,dy,start,end,step,interp,data,d_histo,&csum);
        csum = csum & 0x0000ffff;
        csum = csum | 0x00ff0000;

	if (program_mode == DATASET_LOCKED) {
        	(void) protect3_(csum,&i);
       	 	if (i == 0) {
                	printf("Program not validated to use this dataset.\n");
                	free(data);
                	data = 0L;
                	(void) vl_data_(dx,dy,count,data,&set);

        	}
	}
        printf("Dataset ID: %d\n",csum);

        (void) vl_setsqu_(&squf,&set);

/* grab the extended information */
	set_ext.ipixel = ipixel;
	set_ext.islice = islice;
	set_ext.interp = interp;
	strcpy(set_ext.label,units);

/* resize output image if needed */	
	i = set.imagex;
	j = set.imagey;
	k = sqrt((double)((dx*dx)+(dy*dy)+(count*count)))+5;
	csum = 0;
	if (i<k) {
		i = k;
		csum = 1;
	}
	if (j<k) {
		j = k;
		csum = 1;
	}
	if (csum) {
		printf("Output image resized to %ld %ld\n",i,j);
		sprintf(text,"image %ld %ld",i,j);
		do_a_command(text);
	}

/* resize 2D image to match input */
	free(image2d_data);
	imagex2d = sqrt((double)((dx*dx)+(dy*dy)));
	imagey2d = imagex2d;
	image2d_data = malloc(imagex2d*imagey2d);
	getset_2d_vals(1);
	getset_2d_vals(3);
	buildimage2d();

        return;
}

void find_goto_label(FILE *fp,char *label)
{
        char            tmp[256],tmp2[256];
        double          d;
        long int        j,i;

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

