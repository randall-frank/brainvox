/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified 5/26/01 */

/*
 * This program allows user interactive reslicing of a file.
 *
 * Pitch, roll, yaw, x-shift, y-shift, z-shift, x-scale, y-scale, and z-scale
 * can all be specified.
 *
 * The output can be an initialization file, a .air file, or a resliced file
 */


#include "AIRmain.h"

#define PARAMETERS 9

static void free_function(/*@null@*/ /*@out@*/ /*@only@*/ double ***de, /*@null@*/ /*@out@*/ /*@only@*/ double ****ee, /*@null@*/ /*@out@*/ /*@only@*/ AIR_Pixels ***datain)

{
	if(de) AIR_free_3(de);
	if(ee) AIR_free_4(ee);
	if(datain) AIR_free_vol3(datain);
}

static double radians(double angle)

{
	angle*=(6*fabs(asin(0.5)))/180;
	return angle;
}

static FILE *i_open(char *proposed)
{
	/*See whether file exists already*/
	{
		/* See if the proposed file can be saved */
		AIR_Error errcode=AIR_fprobw(proposed,FALSE);
		if(errcode==0){
			FILE *fp=fopen(proposed,"wb");
			if(fp) return fp;
			errcode=AIR_UNSPECIFIED_FILE_WRITE_ERROR;
		}
		else if(errcode==AIR_NO_PERMISSION_WRITE_ERROR){
			printf("File %s already exists. Overwrite it? (y/n):",proposed);
			{
				char answer[4];
				scanf("%s",answer);
				if(*answer=='y'){
					FILE *fp=fopen(proposed,"wb");
					if(fp) return fp;
					errcode=AIR_UNSPECIFIED_FILE_WRITE_ERROR;
				}
			}
		}
		if(errcode!=AIR_NO_PERMISSION_WRITE_ERROR){
			printf("Can't save file with name %s because of the following error\n",proposed);
			AIR_report_error(errcode);
		}
	}

	printf("\n\tGive up on opening a file? (y/n):");
	{
		char answer[4];

		scanf("%s",answer);
		if(*answer=='y') return NULL;
	}
	printf("\nAlternative [path/]file name:");
	scanf("%s",proposed);

	return i_open(proposed);
}

static int i_open_image(char *proposed)
{
	{
		/* See if the proposed file can be saved */
		AIR_Error errcode=AIR_save_probw(proposed,FALSE);
		if(errcode==0) return 0;
		else if(errcode==AIR_NO_PERMISSION_WRITE_ERROR){
			printf("File %s already exists. Overwrite it? (y/n):",proposed);
			{
				char answer[4];
				scanf("%s",answer);
				if(*answer=='y') return 0;
			}
		}
		printf("Can't save file with name %s because of the following error:\n",proposed);
		AIR_report_error(errcode);
	}
	printf("\n\tGive up on opening a file? (y/n):");
	{
		char answer[4];

		scanf("%s",answer);
		if(*answer=='y') return 1;
	}
	printf("\nAlternative [path/]file name (suffix not required):");
	scanf("%s",proposed);
	return i_open_image(proposed);
}

static int i_open_file(char *proposed)
{
	{
		/* See if the proposed file can be saved */
		AIR_Error errcode=AIR_fprobw(proposed,FALSE);
		if(errcode==0) return 0;
		else if(errcode==AIR_NO_PERMISSION_WRITE_ERROR){
			printf("File %s already exists. Overwrite it? (y/n):",proposed);
			{
				char answer[4];
				scanf("%s",answer);
				if(*answer=='y') return 0;
			}
		}
		printf("Can't save file with name %s because of the following error:\n",proposed);
		AIR_report_error(errcode);
	}
	printf("\n\tGive up on opening a file? (y/n):");
	{
		char answer[4];

		scanf("%s",answer);
		if(*answer=='y') return 1;
	}
	printf("\nAlternative [path/]file name (suffix not required):");
	scanf("%s",proposed);
	return i_open_file(proposed);
}
static int i_exist_image(char *proposed)
{
	{
		/* See if the proposed file can be loaded */
		AIR_Error errcode=AIR_load_probr(proposed,TRUE);
		if(errcode==0) return 0;

		printf("Can't load file %s because of the following error:\n",proposed);
		AIR_report_error(errcode);
	}
	printf("\n\tGive up on opening a file? (y/n):");
	{
		char answer[4];

		scanf("%s",answer);
		if(*answer=='y') return 1;
	}
	printf("\nAlternative [path/]file name (suffix not required):");
	scanf("%s",proposed);
	return i_exist_image(proposed);
}



int main(/*@unused@*/ int argc, char *argv[])

{
	double 		***de=NULL;
	double 		****ee=NULL;
	AIR_Pixels	***datain=NULL;
	AIR_Pixels	***dataout=NULL;

	double 		i[PARAMETERS];
	float		scale;
	char		answer[128];
	char		temp[128];
	struct AIR_Air16 	manual;
	struct AIR_Key_info	dimensions;
	double 		*e[4];

	double		pixel_size_s;
	double		pixel_size_r;
	

	/* Allocate memory */
	de=AIR_matrix3(PARAMETERS,4,4);
	ee=AIR_matrix4(PARAMETERS,PARAMETERS,4,4);

	if(!de ||!ee){
		printf("Memory allocation failure\n");
		free_function(de,ee,datain);
		AIR_report_error(AIR_MEMORY_ALLOCATION_ERROR);
		exit(EXIT_FAILURE);
	}

	e[0]=manual.e[0];
	e[1]=manual.e[1];
	e[2]=manual.e[2];
	e[3]=manual.e[3];


	/*Get realignment parameters*/
	printf("Shifts should be specified in units of cubified reslice file voxels\n");
	printf("x-axis shift in voxels:");
	scanf("%le",&i[3]);
	i[3]*=2;
	printf("y_axis shift in voxels:");
	scanf("%le",&i[4]);
	i[4]*=2;
	printf("z_axis shift in voxels:");
	scanf("%le",&i[5]);
	i[5]*=2;

	printf("roll (rotation around y-axis) in degrees:");
	scanf("%le",&i[1]);
	i[1]=radians(i[1]);
	printf("pitch (rotation around x-axis) in degrees:");
	scanf("%le",&i[0]);
	i[0]=radians(i[0]);
	printf("yaw (rotation around z_axis) in degrees:");
	scanf("%le",&i[2]);
	i[2]=radians(i[2]);

	printf("x scale-down factor:");
	scanf("%le",&i[6]);
	printf("y scale-down factor:");
	scanf("%le",&i[7]);
	printf("z scale-down factor:");
	scanf("%le",&i[8]);

	
	/*Save initialization file if desired*/
	printf("\nSave parameters as initialization file for automated registration? (y/n):");
	scanf("%s",answer);
	if(answer[0]=='y'){
		printf("\tName of initialization file to save:");
		scanf("%s",answer);
		{
			FILE *fp=i_open(answer);
			if(!fp){
				printf("\nExit? (y/n):");
				scanf("%s",answer);
				if(answer[0]=='y'){
					free_function(de,ee,datain);
					exit(EXIT_FAILURE);
				}
			}
			if(i[6]==1.0 && i[7]==1.0 && i[8]==1.0){
				if(fprintf(fp,"%e\n%e\n%e\n%e\n%e\n%e\n",i[0],i[1],i[2],i[3],i[4],i[5])<1){
					printf("Failed to save data into file %s\n",answer);
					free_function(de,ee,datain);
					if(fclose(fp)!=0){
						printf("Failed to close file %s\n",answer);
						AIR_report_error(AIR_CANT_CLOSE_WRITE_ERROR);
						exit(EXIT_FAILURE);
					}
					AIR_report_error(AIR_WRITE_INIT_FILE_ERROR);
					exit(EXIT_FAILURE);
				}
				printf("\tParameters saved in 6 parameter rigid body initialization file %s\n",answer);
			}
			else{
				if(fprintf(fp,"%e\n%e\n%e\n%e\n%e\n%e\n%e\n%e\n%e\n",i[0],i[1],i[2],i[3],i[4],i[5],i[6],i[7],i[8])<1){
					printf("Failed to save data into file %s\n",answer);
					free_function(de,ee,datain);
					if(fclose(fp)!=0){
						printf("Failed to close file %s\n",answer);
						AIR_report_error(AIR_CANT_CLOSE_WRITE_ERROR);
						exit(EXIT_FAILURE);
					}
					AIR_report_error(AIR_WRITE_INIT_FILE_ERROR);
					exit(EXIT_FAILURE);
				}						
				printf("\tParameters saved in 9 parameter initialization file %s\n",answer);
			}
			if(fclose(fp)!=0){
				printf("Failed to close file %s\n",answer);
				free_function(de,ee,datain);
				AIR_report_error(AIR_CANT_CLOSE_WRITE_ERROR);
				exit(EXIT_FAILURE);
			}
		}
		printf("\nExit? (y/n):");
		scanf("%s",answer);
		if(answer[0]=='y'){
			free_function(de,ee,datain);
			exit(0);
		}
	}
		
	printf("\n[path/]name of file to reslice (no spaces allowed):");
	scanf("%s",manual.r_file);
	AIR_clip_suffix(manual.r_file);
	strcat(manual.r_file,AIR_CONFIG_HDR_SUFF);
	{
		int error=i_exist_image(manual.r_file);
		if(error!=0){
			free_function(de,ee,datain);
			exit(EXIT_FAILURE);
		}
	}
	{
		struct AIR_Fptrs fps;
		{
			AIR_Error errcode;
			int flag[8];
			
			(void)AIR_open_header(manual.r_file,&fps,&manual.r,flag);
			if(fps.errcode!=0){
				errcode=fps.errcode;
				AIR_close_header(&fps);
				free_function(de,ee,datain);
				AIR_report_error(errcode);
				exit(EXIT_FAILURE);
			}
		}
		AIR_close_header(&fps);
		if(fps.errcode!=0){
			free_function(de,ee,datain);
			AIR_report_error(fps.errcode);
			exit(fps.errcode);
		}
	}
	manual.r_hash=1;

	printf("\nDefine target space by {e}xample file or {m}anually? (e/m):");
	scanf("%s",answer);
	if(answer[0]=='e'){
		printf("[path/]name of example file (no spaces allowed):");
		scanf("%s",manual.s_file);
		AIR_clip_suffix(manual.s_file);
		strcat(manual.s_file,AIR_CONFIG_HDR_SUFF);
		{
			int error=i_exist_image(manual.s_file);
			if(error!=0){
				free_function(de,ee,datain);
				exit(EXIT_FAILURE);
			}
		}
		{
			struct AIR_Fptrs fps;
			{
				AIR_Error errcode;
				int flag[8];
				
				(void)AIR_open_header(manual.s_file,&fps,&manual.s,flag);
				if(fps.errcode!=0){
					errcode=fps.errcode;
					AIR_close_header(&fps);
					free_function(de,ee,datain);
					AIR_report_error(errcode);
					exit(EXIT_FAILURE);
				}
			}
			AIR_close_header(&fps);
			if(fps.errcode!=0){
				free_function(de,ee,datain);
				AIR_report_error(fps.errcode);
				exit(fps.errcode);
			}
		}
		manual.s_hash=1;
	}
	else{
		strcpy(manual.s_file,"null");
		manual.s.bits=8*sizeof(AIR_Pixels);
		manual.s_hash=1;
		printf("\nThe file you want to reslice has voxels with\n");
		printf("\tx-dimension size of %e\n",manual.r.x_size);
		printf("\ty-dimension size of %e\n",manual.r.y_size);
		printf("\tz-dimension size of %e\n\n",manual.r.z_size);

		printf("What sizes do you want for voxels after reslicing?\n");
		printf("\tx-dimension size:");
		scanf("%le",&manual.s.x_size);
		printf("\ty-dimension size:");
		scanf("%le",&manual.s.y_size);
		printf("\tz_dimension size:");
		scanf("%le",&manual.s.z_size);

		printf("\nThe file you want to reslice has matrix dimensions of\n");
		printf("\tx-dimension %u\n",manual.r.x_dim);
		printf("\ty-dimension %u\n",manual.r.y_dim);
		printf("\tz-dimension %u\n\n",manual.r.z_dim);

		printf("What dimensions do you want for matrix after reslicing?\n");
		printf("\tx_dimension:");
		scanf("%u",&manual.s.x_dim);
		printf("\ty_dimension:");
		scanf("%u",&manual.s.y_dim);
		printf("\tz_dimension:");
		scanf("%u",&manual.s.z_dim);
	}
	AIR_uv3D9(i,e,de,ee,&manual.r,&manual.s,0);

	pixel_size_s=manual.s.x_size;
	if(manual.s.y_size<pixel_size_s) pixel_size_s=manual.s.y_size;
	if(manual.s.z_size<pixel_size_s) pixel_size_s=manual.s.z_size;

	pixel_size_r=manual.r.x_size;
	if(manual.r.y_size<pixel_size_r) pixel_size_r=manual.r.y_size;
	if(manual.r.z_size<pixel_size_r) pixel_size_r=manual.r.z_size;

	if(i[0]==0.0 && i[1]==0.0 && i[2]==0.0 && i[3]==0.0 && i[4]==0.0 && i[5]==0.0 && i[6]==1.0 && i[7]==1.0 && i[8]==1.0){

		int interped=0;

		if(manual.r.x_dim!=manual.s.x_dim || fabs(manual.r.x_size-manual.s.x_size)>AIR_CONFIG_PIX_SIZE_ERR){
			if(floor((manual.r.x_size/pixel_size_r)*(manual.r.x_dim-1)+1.0+AIR_d1mach(1))==manual.s.x_dim || floor((manual.s.x_size/pixel_size_s)*(manual.s.x_dim-1)+1.0+AIR_d1mach(1))==manual.r.x_dim){
				interped=1;
			}
			else{
				interped=-1;
			}
		}

		if(interped!=-1 && (manual.r.y_dim!=manual.s.y_dim || fabs(manual.r.y_size-manual.s.y_size)>AIR_CONFIG_PIX_SIZE_ERR)){
			if(floor((manual.r.y_size/pixel_size_r)*(manual.r.y_dim-1)+1.0+AIR_d1mach(1))==manual.s.y_dim || floor((manual.s.y_size/pixel_size_s)*(manual.s.y_dim-1)+1.0+AIR_d1mach(1))==manual.r.y_dim){
				interped=1;
			}
			else{
				interped=-1;
			}
		}

		if(interped!=-1 && (manual.r.z_dim!=manual.s.z_dim || fabs(manual.r.z_size-manual.s.z_size)>AIR_CONFIG_PIX_SIZE_ERR)){
			if(floor((manual.r.z_size/pixel_size_r)*(manual.r.z_dim-1)+1.0+AIR_d1mach(1))==manual.s.z_dim || floor((manual.s.z_size/pixel_size_s)*(manual.s.z_dim-1)+1.0+AIR_d1mach(1))==manual.r.z_dim){
				interped=1;
			}
			else{
				interped=-1;
			}
		}
		if(interped==1){
			printf("\nThe two spaces that you have defined meet criteria suggesting that you may view them as identical except for interpolation error.\n");
			printf("\nDo you want to treat them as spatially identical? (y/n):");
			scanf("%s",temp);
			if(temp[0]=='y'){
				e[3][0]=0;
				e[3][1]=0;
				e[3][2]=0;	/*AIR 2.0*/
				printf("\nParameters adjusted from center-center alignment to first_plane-first_plane alignment\n");
				printf("Reslicing will reflect spatial identity\n");
			}
		}
	}


	printf("\nSave a .air file based on the information provided? (y/n):");
	scanf("%s",answer);
	if(answer[0]=='y'){
		printf("\tFull name of .air file:");
		scanf("%s",answer);
		{
			int error=i_open_file(answer);
			if(error!=0){
				printf("\tExit? (y/n):");
				scanf("%s",answer);
				if(answer[0]=='y'){
					free_function(de,ee,datain);
					exit(EXIT_FAILURE);
				}
			}
		}
		if(strlen(argv[0])<(size_t)AIR_CONFIG_MAX_COMMENT_LENGTH){
			strcpy(manual.comment,argv[0]);
		}
		else{
			const char *shortname=AIR_file_from_path(argv[0]);
			strncpy(manual.comment,shortname,AIR_CONFIG_MAX_COMMENT_LENGTH-1);
			manual.comment[AIR_CONFIG_MAX_COMMENT_LENGTH-1]='\0';
		}
		
		if(AIR_write_air16(answer,TRUE,e,0,&manual)==0){
			printf("\t.air file saved as %s\n",answer);
		}
		else{
			printf("\tAttempt to save file %s failed\n",answer);
		}
		printf("\tExit? (will proceed to reslice file if answer is no) (y/n):");
		scanf("%s",answer);
		if(answer[0]=='y'){
			free_function(de,ee,datain);
			exit(0);
		}
	}

	printf("intensity scale-up factor:");
	scanf("%e",&scale);

	{
		AIR_Error errcode;

		datain=AIR_load(manual.r_file, &manual.r, 0, &errcode);
		if(!datain){
			free_function(de,ee,datain);
			AIR_report_error(errcode);
			exit(EXIT_FAILURE);
		}
	}
	dimensions.bits=8*sizeof(AIR_Pixels);
	dimensions.x_dim=manual.s.x_dim;
	dimensions.y_dim=manual.s.y_dim;
	dimensions.z_dim=manual.s.z_dim;
	dimensions.x_size=manual.s.x_size;
	dimensions.y_size=manual.s.y_size;
	dimensions.z_size=manual.s.z_size;
	
	if(fabs(dimensions.x_size-pixel_size_s)>AIR_CONFIG_PIX_SIZE_ERR || fabs(dimensions.y_size-pixel_size_s)>AIR_CONFIG_PIX_SIZE_ERR || fabs(dimensions.z_size-pixel_size_s)>AIR_CONFIG_PIX_SIZE_ERR){
		printf("\nOutput voxel dimensions will be %.4e x %.4e x %.4e\n",manual.s.x_size,manual.s.y_size,manual.s.z_size);
		printf("Would you prefer cubic voxels (%.4e x %.4e x %.4e)? (y/n):",pixel_size_s,pixel_size_s,pixel_size_s);
		scanf("%s",answer);
		if(answer[0]=='y'){
			if(fabs(dimensions.x_size-pixel_size_s)>AIR_CONFIG_PIX_SIZE_ERR){
				float xoom=dimensions.x_size/pixel_size_s;
				dimensions.x_dim=(dimensions.x_dim-1)*xoom+1;
				dimensions.x_size=pixel_size_s;
			}

			if(fabs(dimensions.y_size-pixel_size_s)>AIR_CONFIG_PIX_SIZE_ERR){
				float yoom=dimensions.y_size/pixel_size_s;
				dimensions.y_dim=(dimensions.y_dim-1)*yoom+1;
				dimensions.y_size=pixel_size_s;
			}

			if(fabs(dimensions.z_size-pixel_size_s)>AIR_CONFIG_PIX_SIZE_ERR){
				float zoom=dimensions.z_size/pixel_size_s;
				dimensions.z_dim=(dimensions.z_dim-1)*zoom+1;
				dimensions.z_size=pixel_size_s;
			}
		}
	}
		
	printf("\n[path/]name of output file (no spaces allowed):");

	scanf("%s",answer);

	{
		int error=i_open_image(answer);
		if(error!=0){
			printf("\tExit? (y/n):");
			scanf("%s",answer);
			if(answer[0]=='y'){
				free_function(de,ee,datain);
				exit(EXIT_FAILURE);
			}
		}
	}
	{
		AIR_Error errcode;
		
		dataout=AIR_r_affine_lin(datain,&manual.r,&dimensions,e,scale,&errcode);
		if(!dataout){
			free_function(de,ee,datain);
			AIR_report_error(errcode);
			exit(EXIT_FAILURE);
		}
	}
	{
		AIR_Error errcode=AIR_save(dataout,&dimensions,answer,TRUE,argv[0]);
		
		if(errcode!=0){
			free_function(de,ee,datain);
			AIR_report_error(errcode);
			exit(EXIT_FAILURE);
		}
	}
	printf("\nResliced file %s has been saved\n",answer);
	
	free_function(de,ee,datain);
	AIR_free_vol3(dataout);

	return 0;
} 
