#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "voxel.h"
#include "vl_util.h"
#include "vl_polygon.h"
#include "vl_putil.h"


#if defined(WIN32) || defined(_Windows) 
#define DIR_SEPARATOR_CHAR '\\'
#else
#define DIR_SEPARATOR_CHAR '/'
#endif

/*
	Revisions:

	12 May 94 rjf
		Modified to match the new Ks(RGB)Kn form
		for material specularity.  Currently, a single Ks is
		read and written to Ks(R) Ks(G) Ks(B).

	4 APR 95 rac
		modified to read new types and to use floats rather than doubles
		for storage
*/

/* ASCII type encoding XX.YY.oo */
/* two chars:EN,CL,ZC,IC,PO,LI,ME,TE */
/* one VLCHAR:FL,SM,TE,VC */
/* optional chars:w,z */

void vl_str_to_ptype(VLCHAR *str,VLINT32 *type)
{
	VLINT32 i;

	*type = VL_P_END;
	if ((str[0] == 'E') && (str[1] == 'N') && (str[2] == '.')) {
		*type = VL_P_END;
	} else if ((str[0] == 'C') && (str[1] == 'L') && (str[2] == '.')) {
		*type = VL_P_CLEAR;
	} else if ((str[0] == 'Z') && (str[1] == 'C') && (str[2] == '.')) {
		*type = VL_P_ZCLEAR;
	} else if ((str[0] == 'I') && (str[1] == 'C') && (str[2] == '.')) {
		*type = VL_P_ICLEAR;
	} else if ((str[0] == 'P') && (str[1] == 'O') && (str[2] == '.')) {
		*type = VL_P_POLY;
	} else if ((str[0] == 'L') && (str[1] == 'I') && (str[2] == '.')) {
		*type = VL_P_LINE;
	} else if ((str[0] == 'M') && (str[1] == 'E') && (str[2] == '.')) {
		*type = VL_P_MESH;
	} else if ((str[0] == 'T') && (str[1] == 'E') && (str[2] == '.')) {
		*type = VL_P_TEXT;
	} else if ((str[0] == 'T') && (str[1] == 'A') && (str[2] == '.')) {
		*type = VL_P_TATTRIB;
	} else if ((str[0] == 'P') && (str[1] == 'T') && (str[2] == '.')) {
		*type = VL_P_POINT;
	}
	if ((str[3] == 'F') && (str[4] == 'L') && (str[5] == '.')) {
		*type = *type + VL_P_FLAT;
	} else if ((str[3] == 'S') && (str[4] == 'M') && (str[5] == '.')) {
		*type = *type + VL_P_SMOOTH;
	} else if ((str[3] == 'T') && (str[4] == 'E') && (str[5] == '.')) {
		*type = *type + VL_P_TEXTURED;
	} else if ((str[3] == 'V') && (str[4] == 'C') && (str[5] == '.')) {
		*type = *type + VL_P_VCOLORED;
	} else if ((str[3] == 'V') && (str[4] == 'S') && (str[5] == '.')) {
		*type = *type + VL_P_VSHADED;
	}
	i = 6;
	while (str[i] != '\0') {
		if (str[i] == 'w') {
			*type = *type + VL_P_NOZWRITE;
		} else if (str[i] == 'z') {
			*type = *type + VL_P_NOZCOMP;
		}
		i++;
	}
	return;
}

void vl_ptype_to_str(VLINT32 type,VLCHAR *str)
{
	str[0] = '\0';
	switch (type & VL_P_TYPEMASK) {
		case VL_P_END:
			strcat(str,"EN.");
			break;
		case VL_P_CLEAR:
			strcat(str,"CL.");
			break;
		case VL_P_ZCLEAR:
			strcat(str,"ZC.");
			break;
		case VL_P_ICLEAR:
			strcat(str,"IC.");
			break;
		case VL_P_POLY:
			strcat(str,"PO.");
			break;
		case VL_P_LINE:
			strcat(str,"LI.");
			break;
		case VL_P_MESH:
			strcat(str,"ME.");
			break;
		case VL_P_TEXT:
			strcat(str,"TE.");
			break;
		case VL_P_TATTRIB:
			strcat(str,"TA.");
			break;
		case VL_P_POINT:
			strcat(str,"PT.");
			break;
	}
	
	switch (type & VL_P_MODEMASK) {
		case VL_P_FLAT:
			strcat(str,"FL.");
			break;
		case VL_P_SMOOTH:
			strcat(str,"SM.");
			break;
		case VL_P_TEXTURED:
			strcat(str,"TE.");
			break;
		case VL_P_VCOLORED:
			strcat(str,"VC.");
			break;
		case VL_P_VSHADED:
			strcat(str,"VS." );
			break;
	}
	
	if (type & VL_P_NOZWRITE) strcat(str,"w");
	if (type & VL_P_NOZCOMP) strcat(str,"z");
	
	return;
}

void vl_polygon_read_(VLCHAR *filename,vl_object **objects,vl_vertex **verts,
        vl_light **lights,vl_material **materials,VLINT32 *n_objects,
        VLINT32 *n_verts,VLINT32 *n_lights,VLINT32 *n_materials,
        VLINT32 *err)
{
	FILE 	*fp,*fb;
	VLCHAR	text[256],token[256],ttype[256],*tt,fbin[256];
	VLCHAR    LineOfText[2048];
	VLFLOAT64  loc[3], norm[3], color[3];
	VLINT32	n,i,j, nScanned;
	
/* assume the best */
	*err = 0;
	*objects = 0L;	
	*verts = 0L;	
	*lights = 0L;	
	*materials = 0L;
	*n_objects = 0;
	*n_verts = 0;
	*n_lights = 0;
	*n_materials = 0;
/* open the file */
	fp = fopen(filename,"rt");
	if (fp == 0L) {
		vl_puts("Unable to open polygon data file.\n");
		*err = 1;
		return;
	}
	/* read lines to EOF */
        while (fgets(text,256,fp) != 0L) {
/* parse the line into item and length */
#ifdef DEBUG
		printf("TEXT: %s\n",text);
#endif
         if (sscanf(text,"%s %ld",token,&n) != 2) {
			strcpy(token,"NOTHING");
		}
#ifdef DEBUG
		printf("TOKEN:N %s %ld\n",token,n);
#endif
                if (strcmp(token,"OBJECT") == 0) {
			*n_objects = n;
			*objects = (vl_object *)MALLOC(n*sizeof(vl_object));
			if (*objects == 0L) goto noram;
			for(i=0;i<n;i++) {
				fgets(LineOfText,2048, fp );
				sscanf(LineOfText,"%s %ld %ld %ld %ld %ld",
					ttype,
					&((*objects)[i].material),
					&((*objects)[i].light),
					&((*objects)[i].vindex[0]),
					&((*objects)[i].vindex[1]),
					&((*objects)[i].vindex[2]));
				vl_str_to_ptype(ttype,&((*objects)[i].type));
				if (((*objects)[i].type & VL_P_TYPEMASK) == VL_P_TEXT) {
					fgets(text,256,fp);
					fgets(text,256,fp);
					tt = (VLCHAR *)MALLOC(256);
					(*objects)[i].vindex[1] = (VLINT32)tt;
					if (tt != 0L) {
						tt[0] = '\0';
						strcat(tt,text);
						if (tt[strlen(tt)-1] == '\n') tt[strlen(tt)-1] = '\0';
					}
#ifdef DEBUG
					printf("String read:%s\n",tt);
#endif
				}
#ifdef DEBUG
				printf("OBJECT %ld %ld %ld %ld %ld %ld\n",
					((*objects)[i].type),
					((*objects)[i].material),
					((*objects)[i].light),
					((*objects)[i].vindex[0]),
					((*objects)[i].vindex[1]),
					((*objects)[i].vindex[2]));
#endif
			}
                } else if (strcmp(token,"BINOBJECT") == 0) {
			*n_objects = n;
			*objects = (vl_object *)MALLOC(n*sizeof(vl_object));
			if (*objects == 0L) goto noram;
/* build filename */
			fgets(text,256,fp);
			tt = strchr(text,'"')+1;
			if ((VLINT32)tt == 1L) {
				vl_puts("Syntax error in .poly INCLUDE\n");
				goto errout;
			}
			strcpy(ttype,tt);
			tt = strchr(ttype,'"');
			if ((VLINT32)tt == 0L) {
				vl_puts("Syntax error in .poly INCLUDE\n");
				goto errout;
			}
			*tt = '\0';
			strcpy(fbin,filename);
			tt = strrchr(fbin,DIR_SEPARATOR_CHAR)+1;
			if ((VLINT32)tt != 1L) {
				*tt = '\0';
				strcat(fbin,ttype);
			} else {
				strcpy(fbin,ttype);
			}
			fb = fopen(fbin,"rb");
			if (fb == 0L) {
				sprintf(text,"Unable to open include file:%s\n",fbin);
				vl_puts(text);
				goto errout;
			}
/* read objects */
			for(i=0;i<n;i++) {
				fread(&((*objects)[i]),sizeof(vl_object),1,fb);
			}
/* read text strings */
			for(i=0;i<n;i++) {
			if (((*objects)[i].type & VL_P_TYPEMASK) == VL_P_TEXT){
				fread(&j,sizeof(VLINT32),1,fb);
				tt = (VLCHAR *)MALLOC(j);
				fread(tt,1,j,fb);
				(*objects)[i].vindex[1] = (VLINT32)tt;
			}
			}
			fclose(fb);
		} else if (strcmp(token,"VERTEX") == 0) {
			*n_verts = n;
			*verts = (vl_vertex *)MALLOC(n*sizeof(vl_vertex));
			if (*verts == 0L) goto noram;
			for(i=0;i<n;i++) {
				
				fgets(LineOfText,2048, fp );
				nScanned = sscanf(LineOfText,"%lf %lf %lf %lf %lf %lf %lf %lf %lf",
					&(loc[0]),&(loc[1]),&(loc[2]),
					&norm[0], &norm[1], &norm[2],
					&color[0], &color[1], &color[2]);
				(*verts)[i].loc[0] = loc[0];
				(*verts)[i].loc[1] = loc[1];
				(*verts)[i].loc[2] = loc[2];
				(*verts)[i].norm[0]= norm[0];
				(*verts)[i].norm[1]= norm[1];
				(*verts)[i].norm[2]= norm[2];
				(*verts)[i].color[0]= norm[0] * 255;
				(*verts)[i].color[1]= norm[1] * 255;
				(*verts)[i].color[2]= norm[2] * 255;
				/* this is a kludge to still read old files */
				if ( nScanned == 9 ) {
					(*verts)[i].color[0]= color[0] * 255;
					(*verts)[i].color[1]= color[1] * 255;
					(*verts)[i].color[2]= color[2] * 255; 	
				}
#ifdef DEBUG
				printf("VERTEX %f %f %f %f %f %f\n",
					((*verts)[i].loc[0]),
					((*verts)[i].loc[1]),
					((*verts)[i].loc[2]),
					((*verts)[i].norm[0]),
					((*verts)[i].norm[1]),
					((*verts)[i].norm[2]));
#endif
			}
		} else if (strcmp(token,"BINVERTEX") == 0) {
			*n_verts = n;
			*verts = (vl_vertex *)MALLOC(n*sizeof(vl_vertex));
			if (*verts == 0L) goto noram;
/* build filename */
			fgets(text,256,fp);
			tt = strchr(text,'"')+1;
			if ((VLINT32)tt == 1L) {
				vl_puts("Syntax error in .poly INCLUDE\n");
				goto errout;
			}
			strcpy(ttype,tt);
			tt = strchr(ttype,'"');
			if ((VLINT32)tt == 0L) {
				vl_puts("Syntax error in .poly INCLUDE\n");
				goto errout;
			}
			*tt = '\0';
			strcpy(fbin,filename);
			tt = strrchr(fbin,DIR_SEPARATOR_CHAR)+1;
			if ((VLINT32)tt != 1L) {
				*tt = '\0';
				strcat(fbin,ttype);
			} else {
				strcpy(fbin,ttype);
			}
			fb = fopen(fbin,"rb");
			if (fb == 0L) {
				sprintf(text,"Unable to open include file:%s\n",fbin);
				vl_puts(text);
				goto errout;
			}
			for(i=0;i<n;i++) {
				fread(&((*verts)[i]),sizeof(vl_vertex),1,fb);
			}
			fclose(fb);

		} else if (strcmp(token,"LIGHT") == 0) {
			*n_lights = n;
			*lights = (vl_light *)MALLOC(n*sizeof(vl_light));
			if (*lights == 0L) goto noram;
			for(i=0;i<n;i++) {
				fgets(LineOfText,2048, fp );
				sscanf(LineOfText,"%lf %lf %lf %lf %lf",
					&((*lights)[i].ambient),
					&((*lights)[i].diffuse),
					&((*lights)[i].norm[0]),
					&((*lights)[i].norm[1]),
					&((*lights)[i].norm[2]));
#ifdef DEBUG
				printf("LIGHT %lf %lf %lf %lf %lf\n",
					((*lights)[i].ambient),
					((*lights)[i].diffuse),
					((*lights)[i].norm[0]),
					((*lights)[i].norm[1]),
					((*lights)[i].norm[2]));
#endif
			}
		} else if (strcmp(token,"MATERIAL") == 0) {
			*n_materials = n;
			*materials = (vl_material *)MALLOC(n*
				sizeof(vl_material));
			if (*materials == 0L) goto noram;
			for(i=0;i<n;i++) {
				fgets(LineOfText,2048, fp ); 
				sscanf(LineOfText,"%lf %lf %lf %lf %lf %lf %lf %lf %lf",
					&((*materials)[i].diffuse[0]),
					&((*materials)[i].diffuse[1]),
					&((*materials)[i].diffuse[2]),
					&((*materials)[i].ambient[0]),
					&((*materials)[i].ambient[1]),
					&((*materials)[i].ambient[2]),
					&((*materials)[i].specular[0]),
					&((*materials)[i].specular[3]),
					&((*materials)[i].alpha));
					(*materials)[i].specular[1] =
						(*materials)[i].specular[0];
					(*materials)[i].specular[2] =
						(*materials)[i].specular[0];

						(*materials)[i].emissive[0]= 0;
						(*materials)[i].emissive[1]= 0;
						(*materials)[i].emissive[2]= 0;



#ifdef DEBUG
				printf("MATERIAL %lf %lf %lf %lf %lf %lf %lf %lf %lf\n",
					((*materials)[i].diffuse[0]),
					((*materials)[i].diffuse[1]),
					((*materials)[i].diffuse[2]),
					((*materials)[i].ambient[0]),
					((*materials)[i].ambient[1]),
					((*materials)[i].ambient[2]),
					((*materials)[i].specular[0]),
					((*materials)[i].specular[3]),
					((*materials)[i].alpha));
#endif
			}
		} else if (strcmp(token,"NOTHING") == 0) {
			/* swallow the line */
		} else {
			vl_puts("Bad format in polygon data file.\n");
	                *err = 1;
			break;
		}
	}

	fclose(fp);
	return;
	
/* out of memory error: report,FREE up anything allocated,close the file and error out*/
noram:
	vl_puts("Not enough memory to load polygon file.\n");
errout:
	vl_polygon_free_(objects,verts,lights,materials);
	*n_objects = 0;
	*n_verts = 0;
	*n_lights = 0;
	*n_materials = 0;
	*err = 1;
	fclose(fp);
	return;
}

void vl_polygon_write_(VLCHAR *filename,vl_object *objects,vl_vertex *verts,
        vl_light *lights,vl_material *materials,VLINT32 n_objects,
        VLINT32 n_verts,VLINT32 n_lights,VLINT32 n_materials,
        VLINT32 *err)
{
	FILE 	*fp;
	VLINT32 i;
	VLCHAR 	ttype[256];
	
/* assume the best */
	*err = 0;
/* open the file */
	fp = fopen(filename,"wt");
	if (fp == 0L) {
		vl_puts("Unable to open polygon data file.\n");
		*err = 1;
		return;
	}
	fprintf(fp,"OBJECT %ld\n",n_objects);
	for(i=0;i<n_objects;i++) {
		vl_ptype_to_str(objects[i].type,ttype);
		fprintf(fp,"%s %ld %ld %ld %ld %ld\n",
			ttype,
			objects[i].material,
			objects[i].light,
			objects[i].vindex[0],
			objects[i].vindex[1],
			objects[i].vindex[2]);
			if (((objects)[i].type & VL_P_TYPEMASK) == VL_P_TEXT) {
				fprintf(fp,"%s\n",(VLCHAR *)((objects)[i].vindex[1]));
			}
	}
	fprintf(fp,"VERTEX %ld\n",n_verts);
	for(i=0;i<n_verts;i++) {
		if (fprintf(fp,"%lf %lf %lf %lf %lf %lf\n",
			(VLFLOAT64 ) verts[i].loc[0],
			(VLFLOAT64 ) verts[i].loc[1],
			(VLFLOAT64 ) verts[i].loc[2],
			(VLFLOAT64 ) verts[i].norm[0],
			(VLFLOAT64 ) verts[i].norm[1],
			(VLFLOAT64 ) verts[i].norm[2]) == EOF){
				vl_puts("Disk error: Out of space");
            	*err = 2;
                goto end;
		}
		/*
		fprintf(fp,"%lf %lf %lf %lf %lf %lf %lf %lf %lf\n",
			(VLFLOAT64) verts[i].loc[0],
			(VLFLOAT64) verts[i].loc[1],
			(VLFLOAT64) verts[i].loc[2],
			(VLFLOAT64) verts[i].norm[0],
			(VLFLOAT64) verts[i].norm[1],
			(VLFLOAT64) verts[i].norm[2],
			((VLFLOAT64) verts[i].color[0])/255.0,
			((VLFLOAT64) verts[i].color[0])/255.0,
			((VLFLOAT64) verts[i].color[0])/255.0 );
		*/
	}
	fprintf(fp,"LIGHT %ld\n",n_lights);
	for(i=0;i<n_lights;i++) {
		fprintf(fp,"%lf %lf %lf %lf %lf\n",
			lights[i].ambient,
			lights[i].diffuse,
			lights[i].norm[0],
			lights[i].norm[1],
			lights[i].norm[2]);
	}
	fprintf(fp,"MATERIAL %ld\n",n_materials);
	for(i=0;i<n_materials;i++) {
		fprintf(fp,"%lf %lf %lf %lf %lf %lf %lf %lf %lf\n",
			materials[i].diffuse[0],
			materials[i].diffuse[1],
			materials[i].diffuse[2],
			materials[i].ambient[0],
			materials[i].ambient[1],
			materials[i].ambient[2],
			materials[i].specular[0],
			materials[i].specular[3],
			materials[i].alpha);
	}
end:
	fclose(fp);
	return;
}

void vl_polygon_free_(vl_object **objects,vl_vertex **verts,vl_light **lights,
	vl_material **materials)
{
	VLINT32 i;
	i = 0;

	if (*objects != 0L) {
/* FREE up TEXT objects */
		while (((*objects)[i].type  & VL_P_TYPEMASK) != VL_P_END) {
			if ((((*objects)[i].type & VL_P_TYPEMASK) == VL_P_TEXT) 
				&& ((*objects)[i].vindex[1] != 0L)) {
				FREE((VLCHAR *)((*objects)[i].vindex[1]));
			}
			i++;
		}
	}
	if (*objects != 0L) FREE(*objects);
	if (*verts != 0L) FREE(*verts);
	if (*lights != 0L) FREE(*lights);
	if (*materials != 0L) FREE(*materials);

	*objects = 0L;	
	*verts = 0L;	
	*lights = 0L;	
	*materials = 0L;	

	return;
}

void vl_polygon_write_bin_(VLCHAR *filename,vl_object *objects,vl_vertex *verts,
        vl_light *lights,vl_material *materials,VLINT32 n_objects,
        VLINT32 n_verts,VLINT32 n_lights,VLINT32 n_materials,
        VLINT32 *err)
{
	FILE 	*fp,*fb;
	VLINT32 i,j;
	VLCHAR 	ttype[256],fbin[256],*t1;
	
/* assume the best */
	*err = 0;
/* open the file */
	fp = fopen(filename,"wb");
	if (fp == 0L) {
		vl_puts("Unable to open polygon data file.\n");
		*err = 1;
		return;
	}
	fprintf(fp,"BINOBJECT %ld\n",n_objects);
/* cook-up the aux filename */
	strcpy(fbin,filename);
	strcat(fbin,".bo");
	t1 = strrchr(fbin,DIR_SEPARATOR_CHAR)+1;
	if ((VLINT32)t1 == 1L) t1 = fbin;
	fprintf(fp,"INCLUDE \"%s\"\n",t1);
	fb = fopen(fbin,"wb");
	if (fb ==0L) {
		vl_puts("Unable to open polygon data file.\n");
		fclose(fp);
		*err = 1;
		return;
	}
/* first write the raw objects */
	for(i=0;i<n_objects;i++) {
		fwrite(&(objects[i]),sizeof(vl_object),1,fb);
	}
/* now write the text strings (if any) */
	for(i=0;i<n_objects;i++) {
		if (((objects)[i].type & VL_P_TYPEMASK) == VL_P_TEXT) {
			j = strlen((VLCHAR *)((objects)[i].vindex[1]));
			j++; /* trailing null */
			fwrite(&j,sizeof(VLINT32),1,fb);
			fwrite((VLCHAR *)((objects)[i].vindex[1]),1,j,fb);
		}
	}
	fclose(fb);
	fprintf(fp,"BINVERTEX %ld\n",n_verts);
/* cook-up the aux filename */
	strcpy(fbin,filename);
	strcat(fbin,".bv");
	t1 = strrchr(fbin,DIR_SEPARATOR_CHAR)+1;
	if ((VLINT32)t1 == 1L) t1 = fbin;
	fprintf(fp,"INCLUDE \"%s\"\n",t1);
	fb = fopen(fbin,"wb");
	if (fb ==0L) {
		vl_puts("Unable to open polygon data file.\n");
		fclose(fp);
		*err = 1;
		return;
	}
/* first write the raw objects */
	for(i=0;i<n_verts;i++) {
		fwrite(&(verts[i]),sizeof(vl_vertex),1,fb);
	}
	fclose(fb);
	fprintf(fp,"LIGHT %ld\n",n_lights);
	for(i=0;i<n_lights;i++) {
		fprintf(fp,"%lf %lf %lf %lf %lf\n",
			lights[i].ambient,
			lights[i].diffuse,
			lights[i].norm[0],
			lights[i].norm[1],
			lights[i].norm[2]);
	}
	fprintf(fp,"MATERIAL %ld\n",n_materials);
	for(i=0;i<n_materials;i++) {
		fprintf(fp,"%lf %lf %lf %lf %lf %lf %lf %lf %lf\n",
			materials[i].diffuse[0],
			materials[i].diffuse[1],
			materials[i].diffuse[2],
			materials[i].ambient[0],
			materials[i].ambient[1],
			materials[i].ambient[2],
			materials[i].specular[0],
			materials[i].specular[3],
			materials[i].alpha);
	}
	fclose(fp);
	return;
}
