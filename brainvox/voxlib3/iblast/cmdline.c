/*
======================================================================
Copyright (C) 1994. All Rights Reserved. 
Image Analysis Facility, University of Iowa

THIS SOFTWARE IS PROPERTY OF:
Image Analysis Facility
The University of Iowa, Iowa City, Iowa

-- OVERVIEW --------------

MODULE NAME: iblast
FILE NAME: cmdline.c
DATE:  
AUTHOR(S): 

========================================================================
PROJECT: Voxlib
STATUS: Development
PLATFORM: Unix  
LANGUAGE: C
DESCRIPTION: Command line voxlib excercising utility.
DEPENDENCIES: Voxlib
LIMITATIONS: K&R (few prototypes)
NOTES: Reads VoxBlast ".fb" dataset definition files.



-- REVISION HISTORY --------------
DATE:
REASON:
REQUESTED BY:
AUTHOR:

-- GENERAL --------------



 ======================================================================
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/time.h>

#include "iblast.h"

void	loadset(char *fname,long int *lims);

#define GRAB_UNDO if (undo_buffer != 0L) for(i=0;i<set.imagex*set.imagey*(sizeof(int)+sizeof(short));i++) undo_buffer[i] = image[i]

/* Polygon objects */
#include "vl_polygon.h"
vl_object *v_objects;
vl_vertex *v_verts;
vl_light *v_lights;
vl_material *v_materials;
long int n_objects,n_verts,n_lights,n_materials;

void	cmdline()
{
	char		buff[256],cmd[80];
	long int 	num,imgcount,i,imgstep;
	long int	lims[6];
	unsigned char 	*image,*undo_buffer;
	char		p1[80],p2[80],p3[80],p4[80],p5[80],p6[80];
	char		otemp[80],dataf[80],lutf2[80],opacityf2[80];
	double 		rot[3],trans[3],t,pl[6];
	long int	col[3],plane;
        double 		norm[4];

/* timing variables */
        struct timeval before,after;
        struct timezone zone;
        float time1,time2;

/* lookup tables */
	long int	rlut[256],llut[256];

/* stack of input sources (starts with stdin) */
	FILE		*(inputs[10]);
	int		inputptr = 0;

	inputs[0] = stdin;

/* initialize the volume status */
	speed = 1;
	plane = 0;
	for (i=0; i < 3; i++ ) {
		rot[i] = 0.0;
		trans[i] = 0.0;
		lims[0+i] = 0;
		lims[3+i] = 0;
	}

	bcolor = 0;
	imgcount = 1;
	imgstep = 1;
	imagex = 400;
	imagey = 400;
	image = malloc(imagex*imagey*(sizeof(int)+sizeof(short)));
	if (image == NULL) {
		fprintf(stderr,"Unable to allocate image memory.\n");
		exit(1);
	}
	undo_buffer = malloc(imagex*imagey*(sizeof(int)+sizeof(short)));

	strcpy(otemp,"image_%%%.rgb");
	dataf[0] = '\0';
	strcpy(lutf2,lutf);
	strcpy(opacityf2,opacityf);

/* init the vset */
	vl_init_(&set);

/* setup the image buffers */
        (void) vl_image_(imagex,imagey,image,
                (short *)(image+(sizeof(int)*imagex*imagey)),&set);

/* set the palettes and baclground color */
	sread_palette(lutf,opacityf,rlut);
	sread_palette(lutf2,opacityf2,llut);
	vl_lookups_(rlut,llut,bcolor,&set);

	printf("\n\nIblast ver 2.0 interactive.\n");
	printf("\npwd: %s\n",getcwd(p2,80));
	printf("Output template: %s\n",otemp);

/* simple command line interpreter */
	while (1) {
		printf("Iblast> ");
		if (fgets(buff,256,inputs[inputptr]) == NULL) {
			fclose(inputs[inputptr]);
			inputptr = inputptr - 1;
			if (inputptr < 0) exit(0);
		/* fake a comment, this allows I/O switch on next loop */
			strcpy(buff,"#\n");
		}
		if (inputs[inputptr] != stdin) printf("%s",buff);
		strcpy(cmd,"#");
		num = sscanf(buff,"%s %s %s %s %s %s %s",cmd,p1,p2,p3,p4,p5,p6);
/* parse the commands */
		if ((strcasecmp(cmd,"ROT") == 0) && (num == 4)) {
			sscanf(p1,"%lf",&(rot[0]));
			sscanf(p2,"%lf",&(rot[1]));
			sscanf(p3,"%lf",&(rot[2]));
			vl_rotations_(rot[0],rot[1],rot[2],&set);
		}
		else if ((strcasecmp(cmd,"POLYRENDER") == 0) && (num == 2)) {
			vl_polygon_read_(p1,&v_objects,&v_verts,&v_lights,
				&v_materials,&n_objects,&n_verts,&n_lights,
				&n_materials,&i);
	printf("Read:%d objects, %d vertex, %d lights, %d materials\n",
		n_objects,n_verts,n_lights,n_materials);
			if (i == 0) {
			printf("Rendering...\n");
			vl_polygon_render_(&set,v_objects,v_verts,v_lights,
				v_materials,0L);
			}
			vl_polygon_free_(&v_objects,&v_verts,&v_lights,
				&v_materials);
		}
		else if ((strcasecmp(cmd,"DROT") == 0) && (num == 4)) {
			sscanf(p1,"%lf",&t);
			rot[0] = rot[0] + t;
			sscanf(p2,"%lf",&t);
			rot[1] = rot[1] + t;
			sscanf(p3,"%lf",&t);
			rot[2] = rot[2] + t;
			vl_rotations_(rot[0],rot[1],rot[2],&set);
		}
		else if ((strcasecmp(cmd,"ARBROT") == 0) && (num == 5)) {
			sscanf(p1,"%lf",&(norm[0]));
			sscanf(p2,"%lf",&(norm[1]));
			sscanf(p3,"%lf",&(norm[2]));
			sscanf(p4,"%lf",&(norm[3]));
			vl_arb_rotate_(&(norm[0]),&(norm[1]),&set);
			rot[0] = set.rots[0];
			rot[1] = set.rots[1];
			rot[2] = set.rots[2];
		}
		else if ((strcasecmp(cmd,"TRANS") == 0) && (num == 4)) {
			sscanf(p1,"%lf",&(trans[0]));
			sscanf(p2,"%lf",&(trans[1]));
			sscanf(p3,"%lf",&(trans[2]));
			vl_translations_(trans[0],trans[1],trans[2],&set);
		}
		else if ((strcasecmp(cmd,"DTRANS") == 0) && (num == 4)) {
			sscanf(p1,"%lf",&t);
			trans[0] = trans[0] + t;
			sscanf(p2,"%lf",&t);
			trans[1] = trans[1] + t;
			sscanf(p3,"%lf",&t);
			trans[2] = trans[2] + t;
			vl_translations_(trans[0],trans[1],trans[2],&set);
		}
		else if ((strcasecmp(cmd,"RLUT") == 0) && (num == 2)) {
			strcpy(lutf,p1);	
			sread_palette(lutf,opacityf,rlut);
			vl_lookups_(rlut,llut,bcolor,&set);
		}
		else if ((strcasecmp(cmd,"LLUT") == 0) && (num == 2)) {
			strcpy(lutf2,p1);
			sread_palette(lutf2,opacityf2,llut);
			vl_lookups_(rlut,llut,bcolor,&set);
		}
		else if ((strcasecmp(cmd,"ROPAC") == 0) && (num == 2)) {
			strcpy(opacityf,p1);
			sread_palette(lutf,opacityf,rlut);
			vl_lookups_(rlut,llut,bcolor,&set);
		}
		else if ((strcasecmp(cmd,"LOPAC") == 0) && (num == 2)) {
			strcpy(opacityf2,p1);
			sread_palette(lutf2,opacityf2,llut);
			vl_lookups_(rlut,llut,bcolor,&set);
		}
		else if ((strcasecmp(cmd,"QUALITY") == 0) && (num == 2)) {
			sscanf(p1,"%d",&speed);
			if (speed < 1) speed = 1;
			if (speed > 5) speed = 5;
			vl_speed_(speed,&set);
		}
		else if (strcasecmp(cmd,"BACKGROUND") == 0) {
			/* not useful in the cmdline Unix fblast */
		}
		else if (strcasecmp(cmd,"FOREGROUND") == 0) {
			/* not useful in the cmdline Unix fblast */
		}
		else if ((strcasecmp(cmd,"UNDO") == 0) && (num == 1)) {
			if (undo_buffer != 0L) {
				for(i=0;i<set.imagex*set.imagey*
					(sizeof(int)+sizeof(short));i++) {
					image[i] = undo_buffer[i];
				}
			}
		}
		else if ((strcasecmp(cmd,"UNDO") == 0) && (num == 2)) {
			if ((strcasecmp(p1,"ON") == 0) && (undo_buffer == 0)) {
				undo_buffer = malloc(imagex*imagey*
					(sizeof(int)+sizeof(short)));
			} else if ((strcasecmp(p1,"OFF") == 0) 
					&& (undo_buffer != 0)) {
				free(undo_buffer);
				undo_buffer = 0;
			}
		}
        	else if (strcasecmp(cmd,"LRENDER") == 0) {
                	GRAB_UNDO;
               	 	printf("Rendering...\n");
                	(void) gettimeofday(&before,&zone);
                	vl_render_((VL_BACKGROUND | VL_RENDERLIGHT),0,&i,&set);
                	(void) gettimeofday(&after,&zone);
                	printf("Renderer returned:%d\n",i);
                	time1 = ((float)before.tv_usec/1000000.0);
                	time2 = ((float)after.tv_usec/1000000.0);
                	time2 = time2 + (float)(after.tv_sec - before.tv_sec);
                	i = (set.d[0])*(set.d[1])*(set.d[2]);
                	printf("%d voxels in %f seconds = %f voxels per sec\n",
                      	   i,time2-time1, ((float)(i))/(time2-time1));
        	}
        	else if (strcasecmp(cmd,"NCRENDER") == 0) {
                	GRAB_UNDO;
               	 	printf("Rendering...\n");
                	(void) gettimeofday(&before,&zone);
                	vl_render_((VL_BACKGROUND | VL_RENDEROVER),0,&i,&set);
                	(void) gettimeofday(&after,&zone);
                	printf("Renderer returned:%d\n",i);
                	time1 = ((float)before.tv_usec/1000000.0);
                	time2 = ((float)after.tv_usec/1000000.0);
                	time2 = time2 + (float)(after.tv_sec - before.tv_sec);
                	i = (set.d[0])*(set.d[1])*(set.d[2]);
                	printf("%d voxels in %f seconds = %f voxels per sec\n",
                      	   i,time2-time1, ((float)(i))/(time2-time1));
        	}
		else if ((strcasecmp(cmd,"RENDER") == 0) ||
			(strcasecmp(cmd,"BGRENDER") == 0)) {
			GRAB_UNDO;
			printf("Rendering...\n");
                        (void) gettimeofday(&before,&zone);
			vl_render_(VL_BACKGROUND,0,&i,&set);
                        (void) gettimeofday(&after,&zone);
			printf("Renderer returned:%d\n",i);
        	        time1 = ((float)before.tv_usec/1000000.0);
        	        time2 = ((float)after.tv_usec/1000000.0);
        	        time2 = time2 + (float)(after.tv_sec - before.tv_sec);
	                i = (set.d[0])*(set.d[1])*(set.d[2]);
	                printf("%d voxels in %f seconds = %f voxels per sec\n",
       		                 i,time2-time1, ((float)(i))/(time2-time1));
		}
		else if (strcasecmp(cmd,"SAVERGB") == 0) {
			if (num == 1) {
				vl_name_changer(otemp,imgcount,&i,p1);
				imgcount = imgcount + imgstep;
			}
			printf("Saving RGB color image %s\n",p1);
			svimg_(p1,set.image,set.imagex,set.imagey,3,
				1,0,0,set.imagex,set.imagey,&i);
			if (i == -1 ) {
				fprintf(stderr,"Unable to save %s\n",p1);
			}
		}
		else if (strcasecmp(cmd,"SAVEGS") == 0) {
			if (num == 1) {
				vl_name_changer(otemp,imgcount,&i,p1);
				imgcount = imgcount + imgstep;
			}
			printf("Saving grayscale image %s\n",p1);
			svimg_(p1,set.image,set.imagex,set.imagey,1,
				1,0,0,set.imagex,set.imagey,&i);
			if (i == -1 ) {
				fprintf(stderr,"Unable to save %s\n",p1);
			}
		}
		else if (strcasecmp(cmd,"AVERAGE") == 0) {
			GRAB_UNDO;
			vl_filter_output_((long *)image,imagex,imagey,
				VL_AVERAGE);
		}
		else if (strcasecmp(cmd,"ZAVERAGE") == 0) {
			GRAB_UNDO;
		        vl_filter_output_((long *)set.zbuffer,imagex,imagey,
				VL_ZAVERAGE);
		}
		else if (strcasecmp(cmd,"MEDIAN") == 0) {
			GRAB_UNDO;
			vl_filter_output_((long *)image,imagex,imagey,
				VL_MEDIAN);
		}
		else if (strcasecmp(cmd,"ZMEDIAN") == 0) {
			GRAB_UNDO;
			vl_filter_output_((long *)set.zbuffer,imagex,imagey,
				VL_ZMEDIAN);
		}
		else if ((strcasecmp(cmd,"LIGHT") == 0) && (num == 5)) {
			GRAB_UNDO;
			sscanf(p1,"%lf",&(norm[0]));
			sscanf(p2,"%lf",&(norm[1]));
			sscanf(p3,"%lf",&(norm[2]));
			sscanf(p4,"%lf",&(norm[3]));
        		vl_postlight_((long *)set.image,set.zbuffer,
				(long *)set.image,imagex,imagey,norm);
		}
		else if ((strcasecmp(cmd,"TEMPLATE") == 0) && (num == 2)) {
			strcpy(otemp,p1);
		}
		else if ((strcasecmp(cmd,"COUNT") == 0) && (num == 2)) {
			sscanf(p1,"%d",&imgcount);
		}
		else if ((strcasecmp(cmd,"STEP") == 0) && (num == 2)) {
			sscanf(p1,"%d",&imgstep);
		}
		else if ((strcasecmp(cmd,"CHAIN") == 0) && (num == 2)) {
			inputs[inputptr+1] = fopen(p1,"r");
			if (inputs[inputptr+1] == NULL) {
				fprintf(stderr,"Unable to open file:%s\n",p1);
			} else {
				if (inputptr >= 9) {
				   fprintf(stderr,"CHAIN stack too deep\n");
				} else {
					inputptr = inputptr + 1;
				}
			}
		}
		else if ((strcasecmp(cmd,"IMAGE") == 0) && (num == 3)) {
			sscanf(p1,"%d",&imagex);
			sscanf(p2,"%d",&imagey);
			if ((imagex * imagey <= 0) ||
				 (imagex * imagey > 1024*1024)) {
				imagex = 400;
				imagey = 400;
			}
			free(image);
			image = malloc(imagex*imagey*
				(sizeof(int)+sizeof(short)));
			if (image == NULL) {
			fprintf(stderr,"Unable to allocate image memory.\n");
				exit(1);
			}
        		(void) vl_image_(imagex,imagey,image,
                		(short *)(image+(sizeof(int)*imagex*imagey)),
				&set);
			if (undo_buffer != 0L) {
				free(undo_buffer);
				undo_buffer = malloc(imagex*imagey*
					(sizeof(int)+sizeof(short)));
			}
		}
		else if ((strcasecmp(cmd,"LIMITS") == 0) && (num == 7)) {
			sscanf(p1,"%d",&(lims[0]));
			sscanf(p2,"%d",&(lims[1]));
			sscanf(p3,"%d",&(lims[2]));
			sscanf(p4,"%d",&(lims[3]));
			sscanf(p5,"%d",&(lims[4]));
			sscanf(p6,"%d",&(lims[5]));
			vl_limits_(lims[0],lims[1],lims[2],
				lims[3],lims[4],lims[5],&set);
		}
		else if ((strcasecmp(cmd,"PLANEOFF") == 0) && (num == 1)) {
			plane = 0;
			vl_plane_(plane,&(pl[0]),&(pl[3]),&set);
		}
		else if ((strcasecmp(cmd,"PLANEON") == 0) && (num == 7)) {
			plane = 1;
			sscanf(p1,"%lf",&(pl[0]));
			sscanf(p2,"%lf",&(pl[1]));
			sscanf(p3,"%lf",&(pl[2]));
			sscanf(p4,"%lf",&(pl[3]));
			sscanf(p5,"%lf",&(pl[4]));
			sscanf(p6,"%lf",&(pl[5]));
			vl_plane_(plane,&(pl[0]),&(pl[3]),&set);
		}
		else if ((strcasecmp(cmd,"COLOR") == 0) && (num == 4)) {
			sscanf(p1,"%d",&(col[0]));
			sscanf(p2,"%d",&(col[1]));
			sscanf(p3,"%d",&(col[2]));
			bcolor = col[0] + (col[1] << 8) + (col[2] << 16);
			vl_lookups_(rlut,llut,bcolor,&set);
		}
		else if (strcasecmp(cmd,"CLEAR") == 0) {
			bcolor = 0;
			vl_lookups_(rlut,llut,bcolor,&set);
		}
        	else if (strcasecmp(cmd,"ICLEAR") == 0) {
                	vl_polygon_render_(&set,NULL,NULL,NULL,NULL,
				VL_P_ICLEAR);
        	}
        	else if (strcasecmp(cmd,"ZCLEAR") == 0) {
                	vl_polygon_render_(&set,NULL,NULL,NULL,NULL,
				VL_P_ZCLEAR);
        	}
		else if ((strcasecmp(cmd,"DATA") == 0) && (num == 2)) {
			loadset(p1,lims);
			strcpy(dataf,p1);
		}
		else if ((strcasecmp(cmd,"PRINT") == 0) && (num == 1)) {
			printf("Dataset:%s\n",dataf);
			printf("Output template:%s Cnt:%d Stp:%d\n",
				otemp,imgcount,imgstep);
			printf("Data limits x:%d,%d y:%d,%d z:%d,%d\n",lims[0],
				lims[1],lims[2],lims[3],lims[4],lims[5]);
			printf("Image X:%d Y:%d\n",imagex,imagey);
			printf("Back color:%d %d %d\n",col[0],col[1],col[2]);
			printf("Rotation:%f %f %f\n",rot[0],rot[1],rot[2]);
			printf("Translation:%f %f %f\n",
				trans[0],trans[1],trans[2]);
			printf("Speed:%d\n",speed);
			printf("RLUT:%s\n",lutf);
			printf("ROpacity:%s\n",opacityf);
			printf("LLUT:%s\n",lutf2);
			printf("LOpacity:%s\n",opacityf2);
			if (plane == 0) {
				printf("Cutting plane is OFF\n");
			} else {
				printf("Cutting plane is ON\n");
			    printf("Plane point:%f %f %f\n",pl[0],pl[1],pl[2]);
			    printf("Plane normal:%f %f %f\n",pl[3],pl[4],pl[5]);
			}
		}
		else if (strcasecmp(cmd,"CD") == 0) {
			if (num >= 2) chdir(p1);
			printf("pwd: %s\n",getcwd(p2,80));
		}
		else if (strcasecmp(cmd,"EXIT") == 0) {
			exit(0);
		}
		else if (cmd[0] == '#') {
			/* comment */
		}
		else if (cmd[0] == '!') {
			system(strstr(buff,"!")+1);
		}
		else if ((cmd[0] == '?') || (strcasecmp(cmd,"HELP") == 0)) {
			printf("Commands:\n");
			printf("ROT X Y Z\n");
			printf("DROT X Y Z\n");
			printf("TRANS X Y Z\n");
			printf("DTRANS X Y Z\n");
			printf("RLUT filename\n");
			printf("LLUT filename\n");
			printf("ROPAC filename\n");
			printf("LOPAC filename\n");
			printf("QUALITY int\n");
			printf("RENDER or BGRENDER\n");
			printf("LRENDER\n");
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
			printf("DATA filename\n");
			printf("COUNT int\n");
			printf("STEP int\n");
			printf("LIMITS xs xe ys ye zs ze\n");
			printf("IMAGE dx dy\n");
			printf("LIGHT dx dy dz inten\n");
			printf("PLANEON px py pz nx ny nz\n");
			printf("PLANEOFF\n");
			printf("CHAIN filename\n");
			printf("UNDO [ON]|[OFF]\n");
			printf("PRINT\n");
			printf("CD filename\n");
			printf("! system command\n");
			printf("EXIT\n");
		}
		else {
			fprintf(stderr,"huh?? %s",buff);
		}
	}
}

/* read a dataset specified via a Voxblast .fb file */

void	loadset(char *fname,long int *lims)
{
	long int start,end,step,dx,dy,interp;
	float ipixel,islice,squf;
	char tname[256];
	long int count,i;
	unsigned char *data;
	unsigned long hist[256];

	if (tloaddefs(fname,&ipixel,&islice,&start,&end,&step,&dx,&dy,
		tname)== -1) {
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
    	(void) vl_read_images_(tname,dx,dy,start,end,step,interp,data);

/* set the dataset & squeeze factor */
        (void) vl_data_(dx,dy,count,data,&set);

/* report back the limits */
	lims[0] = 0;
	lims[1] = dx-1;
	lims[2] = 0;
	lims[3] = dy-1;
	lims[4] = 0;
	lims[5] = count-1;

        (void) vl_setsqu_(&squf,&set); 

	return;
}
