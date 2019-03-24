/*
 *
 * $Id $
 * $HeadURL $
 *
 * cvio_selfalign [options] stdfile infile outfile
 * options:
 *     threshold
 *     FWHM  X:Y
 *     Standard image number
 *     
 *     cost_function = least squares
 *
 *	Time limits
*	FWHM values (smoothing)
*	Fit vectors
*	Masks
*	Array sizes
 *	Initial conditions...
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "cvio_lib.h"
#include "../AIR5.0/AIR.h"
#include "air_interface.h"

#define MAX_PARAMS 16

int verbosity = 0;
int tstext = 0;

static void *alloc_sample_buffer(uint32_t stream,int32_t num,int32_t *sz);
void get_xyz(long int *xyz,char *str);

void info(char *app) 
{
	fprintf(stderr,"(%s) Usage: %s [options] stdfile infile outfile\n",
			__DATE__,app);
	fprintf(stderr,"Options:\n");
	fprintf(stderr,"      -v Verbose\n");
	fprintf(stderr,"      -s(st:del:end) Sampling steps (default: 81:3:1)\n");
	fprintf(stderr,"      -x(costfunc) Cost function (default: 2)\n");
	fprintf(stderr,"      -c(conv) Convergence (default: 0.00001)\n");
	fprintf(stderr,"      -m(model) Fit model (default: 23)\n");
	fprintf(stderr,"      -I(interp) Reslice Interpolation (default: 1)\n");
	fprintf(stderr,"      -p1(part1) Standard partitions (default: 1)\n");
	fprintf(stderr,"      -p2(part2) Reslice partitions (default: 1)\n");
	fprintf(stderr,"      -t1(threshold) Standard image threshold (default: 7000)\n");
	fprintf(stderr,"      -t2(threshold) Reslice image threshold (default: 7000)\n");
	fprintf(stderr,"      -i(index) Standard volume index (default: 1)\n");
	fprintf(stderr,"      -n(num) Number of slices in a volume (default: read file tag)\n");
	fprintf(stderr,"      -S(step) Process every step volume (default: 1)\n");
	fprintf(stderr,"      -F(fitout) Cvio file to save final cost func values (default: none)\n");
	fprintf(stderr,"      -f If specified, fitout will contain complete fit params (default: no)\n");
	fprintf(stderr,"      -d Don't create output files (because they already exist)\n");
	fprintf(stderr,"      -T Timestamp textual output\n");
#ifdef NEVER	
	fprintf(stderr,"      -e1(maskfile) Standard mask (default: none)\n");
	fprintf(stderr,"      -e2(maskfile) Reslice mask (default: none)\n");
#endif
	fprintf(stderr,"\nOption value details:\n");
	fprintf(stderr,"\nFitting model: \n");
	fprintf(stderr," 3-D models:\n");
	fprintf(stderr,"   6. rigid body 6 parameter model\n");
	fprintf(stderr,"   7. global rescale 7 parameter model\n");
	fprintf(stderr,"   9. traditional 9 parameter model (std must be on AC-PC line)\n");
	fprintf(stderr,"  12. affine 12 parameter model\n");
	fprintf(stderr,"  15. perspective 15 parameter model\n");
	fprintf(stderr," 2-D models (constrained to in-plane, no interpolation):\n");
	fprintf(stderr,"  23. 2-D rigid body 3 parameter model\n");
	fprintf(stderr,"  24. 2-D global rescale 4 parameter model\n");
	fprintf(stderr,"  25. 2-D affine/fixed determinant 5 parameter model\n");
	fprintf(stderr,"  26. 2-D affine 6 parameter model\n");
	fprintf(stderr,"  28. 2-D perspective 8 parameter model\n");
	fprintf(stderr,"\nCost function: \n");
	fprintf(stderr,"   1. standard deviation of ratio image\n");
	fprintf(stderr,"   2. least squares\n");
	fprintf(stderr,"   3. least squares with intensity rescaling\n");

	fprintf(stderr,"\nInterpolation function: \n");
	fprintf(stderr,"   0. nearest neighbor\n");
	fprintf(stderr,"   1. trilinear\n");
	fprintf(stderr,"   2. windowed sinc in original xy plane, linear along z\n");
	fprintf(stderr,"   3. windowed sinc in original xz plane. linear along y\n");
	fprintf(stderr,"   4. windowed sinc in original yz plane, linear along x\n");
	fprintf(stderr,"   5. 3D windowed sinc\n");
	fprintf(stderr,"   6. 3D windowed scanline sinc\n");
	fprintf(stderr,"   7. 3D unwindowed scanline sinc\n");
	fprintf(stderr,"  10. 3D scanline chirp-z\n");
	fprintf(stderr,"  11. scanline chirp-z in original xy plane, linear along z\n");
	fprintf(stderr,"  12. scanline chirp-z in original xz plane, linear along y\n");
	fprintf(stderr,"  13. scanline chirp-z in original yz plane, linear along x\n");


	exit(1);
}

void	exit_cvio_error(int32_t err) 
{
	char	tstr[256];
	int32_t	tlen = 256;

	cvio_get_error_message(err,tstr,&tlen);
	cvio_log(tstext,"Fatal error:%s\n",tstr);

	cvio_cleanup(0);
	exit(1);
}

static void *alloc_sample_buffer(uint32_t stream,int32_t num,int32_t *sz)
{
	int32_t	datatype,ndims,dims[3];
	int32_t	iSize;
	int32_t	err;

        err = cvio_datatype(stream,&datatype,&ndims,dims);
        if (err) return(NULL);

        iSize = (datatype & CVIO_TYP_SIZE_MASK)*dims[0]*dims[1]*dims[2]/8;
	if (sz) *sz = iSize;

	return(calloc(num,iSize));
}


int main(int argc, char **argv)
{
	int	first_img = 1, dontcreate=0;
	int	nslices = -1;
	char	*stdfile, *infile, *outfile;
	char	*outfitfile = NULL;
	long int sfac[3] = {81,3,1};
	AlignParams	p;
	double	fitparams[MAX_PARAMS+1];

	uint32_t	inf_s,out_s,std_s,outfit_s,maxsamp;
	int32_t	datatype,ndims,dims[3],err;
	int32_t	datatype2,ndims2,dims2[3];

	char	*in_fixed=NULL,*in_sample=NULL,*out_sample=NULL;

	float	runt,runn;

	int	i,done,iSize,icount;
	int32_t	Bsize;
	int	fitlength = 1;
	int	iStep = 1;

	align_param_init(&p);

        i = 1;
        while((i<argc) && (argv[i][0] == '-')) {
                switch(argv[i][1]) {
                        case 'v' : verbosity = strlen(argv[i])-1; break;
			case 't' : 
				   if (argv[i][2] == '1') {
					p.threshold1 = atoi(argv[i]+3);
				   }
				   if (argv[i][2] == '2') {
					p.threshold2 = atoi(argv[i]+3);
				   }
				   break;
			case 'i' : first_img = atoi(argv[i]+2); break;
			case 'F' : outfitfile = argv[i]+2; break;
		        case 'x' : p.costfxn = atoi(argv[i]+2); break;
		        case 'c' : p.precision = atof(argv[i]+2); break;
		        case 'm' : p.model = atoi(argv[i]+2); break;
		        case 'I' : p.interp = atoi(argv[i]+2); break;
		        case 's' : get_xyz(sfac,argv[i]+2); break;
			case 'S' : iStep = atoi(argv[i]+2); break;
			case 'n' : nslices = atoi(argv[i]+2); break;
			case 'f' : fitlength = MAX_PARAMS+1; break;
			case 'T' : tstext = 1; break;
#ifdef NEVER
/* eventually, the masking cases */
		        case 'e' : 
				   if (argv[i][2] == '1') {
				   }
				   if (argv[i][2] == '2') {
				   }
				   break;
#endif				   
		        case 'p' : 
				   if (argv[i][2] == '1') {
					   p.partitions1 = atoi(argv[i]+3); 
				   }
				   if (argv[i][2] == '2') {
					   p.partitions2 = atoi(argv[i]+3); 
				   }
				   break;
			case 'd' :
				dontcreate = 1;
				break;
			default: info(argv[0]); break;
		}
		i++;
	}
	if (argc-i != 3) info(argv[0]);

	p.verbosity = verbosity-1;
	if (p.verbosity < 0) p.verbosity = 0;
	/* move the values in */
	p.samplefactor = sfac[0];
	p.sffactor = sfac[1];
	p.samplefactor2 = sfac[2];

	/* get the filenames */
	stdfile = argv[i++];
	infile = argv[i++];
	outfile = argv[i++];

#ifdef NEVER
	if (fitlength > 1) {
		if (p.model > 20) {
			fitlength = p.model - 20 + 1;
		} else {
			fitlength = p.model + 1;
		}
	}
#endif

	/* init the cvio library */
	if (cvio_init()) exit(1);

	/* read the std file */
	err = cvio_open(stdfile,CVIO_ACCESS_READ, &std_s);
	if (err) exit_cvio_error(err);

	/* get its format and size */
	err = cvio_datatype(std_s,&datatype,&ndims,dims);
	if (err) exit_cvio_error(err);

	if (nslices == -1) {
		int blen = 100;
		char buffer[100];
		err = cvio_getattribute(std_s, "NUMBER_SLICES_ST", &buffer, &blen);
		if (err != CVIO_ERR_OK) {
			nslices = 1;
		} else {
			nslices = atoi(buffer);
		}
	}

	if (ndims < 2) {
		cvio_log(tstext,"Error:input file must have at least 2D samples\n");
		cvio_close(std_s);
		goto byebye;
	}
	if ((datatype & CVIO_TYP_SIZE_MASK) != 16) {
		cvio_log(tstext,"Error:input file must have 16bit samples\n");
		cvio_close(std_s);
		goto byebye;
	}

	/* allocate fixed buffer */
	in_fixed = alloc_sample_buffer(std_s,nslices,&Bsize);

	/* find the fixed image */
	first_img = nslices*(first_img-1)+1;
	if (first_img < 1) first_img = 1;
	i = 0;
	while(first_img || (i < nslices)) {
		int32_t sc = 1;
		CVIO_DT_TIME in_time;
		/* read the next sample */
		in_time = CVIO_TIME_NEXT;
		err = cvio_read_samples_blocking(std_s,&in_time,in_fixed+(i*Bsize),&sc,0,0);
		if (err == CVIO_ERR_OK) {
			if (first_img) first_img -= 1;
			if (!first_img) i++;
		} else {
			cvio_log(tstext,
			    "Error:unable to read standard image\n");
			cvio_close(std_s);
			if (dontcreate) {
				//open and close output stream so nobody blocks forever
				err = cvio_open(outfile,CVIO_ACCESS_APPEND,&out_s);
				if (err == CVIO_ERR_OK)
					cvio_close(out_s);
			}
			goto byebye;
		}
	}

	/* we now have the fixed image */
	cvio_close(std_s);

	/* read the input file */
	err = cvio_open(infile,CVIO_ACCESS_READ,&inf_s);
	if (err) exit_cvio_error(err);

	/* get its format and size */
	err = cvio_datatype(inf_s,&datatype2,&ndims2,dims2);
	if (err) exit_cvio_error(err);

        iSize = (datatype & CVIO_TYP_SIZE_MASK)*dims[0]*dims[1]*dims[2]/8;

	if (ndims2 != ndims) {
		cvio_log(tstext,"Error:input files must have the same dimensions\n");
		cvio_close(inf_s);
		goto byebye;
	}
	if ((datatype2 & CVIO_TYP_SIZE_MASK) != 16) {
		cvio_log(tstext,"Error:input file must have 16bit samples\n");
		cvio_close(inf_s);
		goto byebye;
	}
	if ((dims[0] != dims2[0]) || (dims[1] != dims2[1])) {
		cvio_log(tstext,"Error:input files must have the same dimensions\n");
		cvio_close(inf_s);
		goto byebye;
	}

	/* how big could the output become?? */
	err = cvio_max_length(inf_s,&maxsamp);
        if (err) exit_cvio_error(err);
					
	if (!dontcreate) {
	        /* create the destination file */
	        err = cvio_create(outfile,maxsamp,datatype,ndims,dims,0);
	        if (err) exit_cvio_error(err);
	}

        /* open the destination file */
        err = cvio_open(outfile,CVIO_ACCESS_APPEND,&out_s);
        if (err) exit_cvio_error(err);

	if (outfitfile) {
		int32_t fdims[3] = {1,1,1};

		fdims[0] = fitlength;
        	err = cvio_create(outfitfile,maxsamp,CVIO_TYP_FLOAT,
			1,fdims,0);
        	if (err) exit_cvio_error(err);

        	err = cvio_open(outfitfile,CVIO_ACCESS_APPEND,&outfit_s);
        	if (err) exit_cvio_error(err);
	}

	/* allocate sample buffers */
	in_sample = alloc_sample_buffer(inf_s,1,NULL);
	out_sample = alloc_sample_buffer(inf_s,1,NULL);

	/* get some stats */
	runt = 0.0;
	runn = 0.0;

	icount = 0;
	done = 0;
	while(!done) {
		int sc;
		CVIO_DT_TIME in_time;
		double fitval;
		CVIO_DT_TIME t;

		/* read the next sample */
		in_time = CVIO_TIME_NEXT;
		sc = 1;
		err = cvio_read_samples_blocking(inf_s,&in_time,in_sample,&sc,0,0);
		if (err == CVIO_ERR_OK) {

			i = icount/nslices;
			if ((i % iStep) == 0) {
#ifdef DEBUG
printf("Read image @ %d\n",in_time);
printf("%f %f -> %d\n",runn,runt,((int)runn % nslices));
for(i=128*64;i<128*64+64;i++) printf("%d ",in_fixed[i+(((int)runn % nslices)*Bsize)]);
printf("\n");
for(i=128*64;i<128*64+64;i++) printf("%d ",in_sample[i]);
printf("\n");
#endif
			    /* align */
			    t = cvio_current_time();
			    err=align_images(in_fixed+(((int)runn % nslices)*Bsize),
				in_sample,out_sample,(int *)dims,&p,&fitval,
				fitparams+1, 0);
			    t = cvio_current_time()-t;
			    runt += ((float)t)/((float)CVIO_TIME_TICKS_PER_SEC);
			    runn += 1;
			    fitparams[0] = fitval;
			    if (err) AIR_report_error(err);
			    if (verbosity) {
				cvio_log(tstext, "Fit function: ");
				for(i=0;i<fitlength;i++) 
					cvio_log(0, "%f ",fitparams[i]);
				cvio_log(0, "\n");
			    }
#ifdef DEBUG
printf("aligned...\n");
#endif
			    /* output the fixed slice */
			    err = cvio_add_samples(out_s,&in_time,out_sample,1);
			    if (err != CVIO_ERR_OK) done = 1;

			    if (outfitfile) {
				float t[MAX_PARAMS+1];
				for(i=0;i<MAX_PARAMS+1;i++) t[i]=fitparams[i];
				err = cvio_add_samples(outfit_s,&in_time,t,1);
				if (err != CVIO_ERR_OK) done = 1;
			    }
			}
			icount++;

		} else
			done = 1;
	}

	if (verbosity) {
		cvio_log(tstext, "Computed %f fits in %f sec (%f fps)\n",
				runn,runt,runn/runt);
	}

	/* close em up */
	err = cvio_close(inf_s);
	err = cvio_close(out_s);
	if (outfitfile) err = cvio_close(outfit_s);

byebye:
	/* free memory */
	if (in_fixed) free(in_fixed);
	if (in_sample) free(in_sample);
	if (out_sample) free(out_sample);

	/* cleanup and exit */
	cvio_cleanup(0);

	exit(0);
}
