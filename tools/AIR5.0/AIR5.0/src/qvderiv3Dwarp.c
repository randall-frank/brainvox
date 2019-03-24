/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified 5/16/01 */

/*
 * double qvderiv3Dwarp()
 *
 * This routine computes the first and second derivatives
 *  of the normalized cost function with respect to all
 *  external parameters.
 *
 * The matrix indices have been adjusted to allow for a more
 *  orderly relationship between the matrix and its elements
 *
 * Returns the sum of squares
 *
 * Note that derivatives with respect to external parameters are ignormed
 */

#include "AIR.h"

#define COORDS_INT 3


double AIR_qvderiv3Dwarp(const unsigned int parameters, double **es, const unsigned int samplefactor, AIR_Pixels ***pixel2, const struct AIR_Key_info *stats2, const AIR_Pixels threshold2, AIR_Pixels ***pixel5, const struct AIR_Key_info *stats5, const AIR_Pixels threshold5, double *dcf, double **ecf, double *dx, const double scale, unsigned int *error)

{

	unsigned long int counttotal=0;
	
	double cf=0.0;		/*intermediate value*/

	double x_max1=stats2->x_dim-1.0;
	double y_max1=stats2->y_dim-1.0;
	double z_max1=stats2->z_dim-1.0;

	const unsigned int x_dim2=stats5->x_dim;
	const unsigned int y_dim2=stats5->y_dim;
	const unsigned int z_dim2=stats5->z_dim;
	
	const signed int threshold2i=(signed int)threshold2;	/* avoid need to keep doing type conversions */

	unsigned int order=12;
	unsigned int coeffp=455;

	if(parameters<1365){
		order=11;
		coeffp=364;
	}
	if(parameters<1092){
		order=10;
		coeffp=286;
	}
	if(parameters<858){
		order=9;
		coeffp=220;
	}
	if(parameters<660){
		order=8;
		coeffp=165;
	}
	if(parameters<495){
		order=7;
		coeffp=120;
	}
	if(parameters<360){
		order=6;
		coeffp=84;
	}
	if(parameters<252){
		order=5;
		coeffp=56;
	}
	if(parameters<168){
		order=4;
		coeffp=35;
	}
	if(parameters<105){
		order=3;
		coeffp=20;
	}
	if(parameters<60){
		order=2;
		coeffp=10;
	}
	if(parameters<30){
		order=1;
		coeffp=4;
	}

	{
		double *dxt=dx;
		unsigned int t;
		
		for(t=0; t<coeffp; t++, dxt++){
			*dxt=0;
		}
	}
	{
		double *dcft=dcf;
		double *ecfts=*ecf;
		unsigned int t;
		
		for (t=0; t<=3*coeffp; t++, dcft++){
		
			unsigned int s;
			
			*dcft=0;
			
			for (s=0; s<=t; s++, ecfts++){
			
				*ecfts=0;
			}
		}
	}


	/*Examine pixels of standard file at samplefactor interval*/
	/*Note that it is assumed here that pixel5[z_dim][y_dim][x_dim] refers to the*/
	/* same pixel as *(pixel5[0][0]+z_dim*y_dim*x_dim), i.e. that all the pixels */
	/* are represented in a contiguous block of memory--see the routine*/
	/* "create_vol3.c" for an illustration of how this is assured*/

	/*ARRAY STRUCTURE ASSUMPTIONS MADE HERE*/
	{
		unsigned long int r;
		unsigned long int r_term=x_dim2*y_dim2*z_dim2;
		AIR_Pixels *rr;
		
		for (r=0,rr=**pixel5;r<r_term;r+=samplefactor,rr+=samplefactor){

			signed int n0,n1,n2,n3,n4,n5,n6,n7;
			double a,b,c,d,e,f;
			
			/*We don't yet verify that pixel5>threshold to allow for less biased function*/

			double pix3=(double)*rr;
			double spix3=pix3*scale;

			{
				/*Calculate coordinates (i,j,k) of pixel r in standard file*/
				
				unsigned int i,j,k;
				double i1,j1,k1;
				{
					unsigned long int tempplane=x_dim2*y_dim2;
					unsigned long int tempremainder=r%tempplane;
					
					k=(unsigned int)r/tempplane;
					j=(unsigned int)tempremainder/x_dim2;
					i=(unsigned int)tempremainder%x_dim2;
				}
				i1=(double)i;
				j1=(double)j;
				k1=(double)k;

					
				/*Calculate coordinates (x_i,y_i,z_i) of corresponding pixel in reslice file*/
				/*Skip further computations if pixel is out of bounds*/
				{
					double *dxt=dx;

					*dxt++=(double)1.0;		/* 0 */
					*dxt++=i1;			/* 1 */
					*dxt++=j1;			/* 2 */
					*dxt++=k1;			/* 3 */
					if(order>1){
						double i2=i1*i1;
						double j2=j1*j1;
						double k2=k1*k1;
						*dxt++=i2;		/* 4 */
						*dxt++=i1*j1;		/* 5 */
						*dxt++=j2;		/* 6 */
						*dxt++=i1*k1;		/* 7 */
						*dxt++=j1*k1;		/* 8 */
						*dxt++=k2;		/* 9 */

					if(order>2){
						double i3=i2*i1;
						double j3=j2*j1;
						double k3=k2*k1;
						*dxt++=i3;		/* 10 */
						*dxt++=i2*j1;		/* 11 */
						*dxt++=i1*j2;		/* 12 */
						*dxt++=j3;		/* 13 */
						*dxt++=i2*k1;		/* 14 */
						*dxt++=i1*j1*k1;		/* 15 */
						*dxt++=j2*k1;		/* 16 */
						*dxt++=i1*k2;		/* 17 */
						*dxt++=j1*k2;		/* 18 */
						*dxt++=k3;		/* 19 */

					if(order>3){
						double i4=i3*i1;
						double j4=j3*j1;
						double k4=k3*k1;
						*dxt++=i4;		/* 20 */
						*dxt++=i3*j1;		/* 21 */
						*dxt++=i2*j2;		/* 22 */
						*dxt++=i1*j3;		/* 23 */
						*dxt++=j4;		/* 24 */
						*dxt++=i3*k1;		/* 25 */
						*dxt++=i2*j1*k1;		/* 26 */
						*dxt++=i1*j2*k1;		/* 27 */
						*dxt++=j3*k1;		/* 28 */
						*dxt++=i2*k2;		/* 29 */
						*dxt++=i1*j1*k2;		/* 30 */
						*dxt++=j2*k2;		/* 31 */
						*dxt++=i1*k3;		/* 32 */
						*dxt++=j1*k3;		/* 33 */
						*dxt++=k4;		/* 34 */

					if(order>4){
						double i5=i4*i1;
						double j5=j4*j1;
						double k5=k4*k1;
						*dxt++=i5;		/* 35 */
						*dxt++=i4*j1;		/* 36 */
						*dxt++=i3*j2;		/* 37 */
						*dxt++=i2*j3;		/* 38 */
						*dxt++=i1*j4;		/* 39 */
						*dxt++=j5;		/* 40 */
						*dxt++=i4*k1;		/* 41 */
						*dxt++=i3*j1*k1;		/* 42 */
						*dxt++=i2*j2*k1;		/* 43 */
						*dxt++=i1*j3*k1;		/* 44 */
						*dxt++=j4*k1;		/* 45 */
						*dxt++=i3*k2;		/* 46 */
						*dxt++=i2*j1*k2;		/* 47 */
						*dxt++=i1*j2*k2;		/* 48 */
						*dxt++=j3*k2;		/* 49 */
						*dxt++=i2*k3;		/* 50 */
						*dxt++=i1*j1*k3;		/* 51 */
						*dxt++=j2*k3;		/* 52 */
						*dxt++=i1*k4;		/* 53 */
						*dxt++=j1*k4;		/* 54 */
						*dxt++=k5;		/* 55 */

					if(order>5){
						double i6=i5*i1;
						double j6=j5*j1;
						double k6=k5*k1;
						*dxt++=i6;		/* 56 */
						*dxt++=i5*j1;		/* 57 */
						*dxt++=i4*j2;		/* 58 */
						*dxt++=i3*j3;		/* 59 */
						*dxt++=i2*j4;		/* 60 */
						*dxt++=i1*j5;		/* 61 */
						*dxt++=j6;		/* 62 */
						*dxt++=i5*k1;		/* 63 */
						*dxt++=i4*j1*k1;		/* 64 */
						*dxt++=i3*j2*k1;		/* 65 */
						*dxt++=i2*j3*k1;		/* 66 */
						*dxt++=i1*j4*k1;		/* 67 */
						*dxt++=j5*k1;		/* 68 */
						*dxt++=i4*k2;		/* 69 */
						*dxt++=i3*j1*k2;		/* 70 */
						*dxt++=i2*j2*k2;		/* 71 */
						*dxt++=i1*j3*k2;		/* 72 */
						*dxt++=j4*k2;		/* 73 */
						*dxt++=i3*k3;		/* 74 */
						*dxt++=i2*j1*k3;		/* 75 */
						*dxt++=i1*j2*k3;		/* 76 */
						*dxt++=j3*k3;		/* 77 */
						*dxt++=i2*k4;		/* 78 */
						*dxt++=i1*j1*k4;		/* 79 */
						*dxt++=j2*k4;		/* 80 */
						*dxt++=i1*k5;		/* 81 */
						*dxt++=j1*k5;		/* 82 */
						*dxt++=k6;		/* 83 */

					if(order>6){
						double i7=i6*i1;
						double j7=j6*j1;
						double k7=k6*k1;
						*dxt++=i7;		/* 84 */
						*dxt++=i6*j1;		/* 85 */
						*dxt++=i5*j2;		/* 86 */
						*dxt++=i4*j3;		/* 87 */
						*dxt++=i3*j4;		/* 88 */
						*dxt++=i2*j5;		/* 89 */
						*dxt++=i1*j6;		/* 90 */
						*dxt++=j7;		/* 91 */
						*dxt++=i6*k1;		/* 92 */
						*dxt++=i5*j1*k1;		/* 93 */
						*dxt++=i4*j2*k1;		/* 94 */
						*dxt++=i3*j3*k1;		/* 95 */
						*dxt++=i2*j4*k1;		/* 96 */
						*dxt++=i1*j5*k1;		/* 97 */
						*dxt++=j6*k1;		/* 98 */
						*dxt++=i5*k2;		/* 99 */
						*dxt++=i4*j1*k2;		/* 100 */
						*dxt++=i3*j2*k2;		/* 101 */
						*dxt++=i2*j3*k2;		/* 102 */
						*dxt++=i1*j4*k2;		/* 103 */
						*dxt++=j5*k2;		/* 104 */
						*dxt++=i4*k3;		/* 105 */
						*dxt++=i3*j1*k3;		/* 106 */
						*dxt++=i2*j2*k3;		/* 107 */
						*dxt++=i1*j3*k3;		/* 108 */
						*dxt++=j4*k3;		/* 109 */
						*dxt++=i3*k4;		/* 110 */
						*dxt++=i2*j1*k4;		/* 111 */
						*dxt++=i1*j2*k4;		/* 112 */
						*dxt++=j3*k4;		/* 113 */
						*dxt++=i2*k5;		/* 114 */
						*dxt++=i1*j1*k5;		/* 115 */
						*dxt++=j2*k5;		/* 116 */
						*dxt++=i1*k6;		/* 117 */
						*dxt++=j1*k6;		/* 118 */
						*dxt++=k7;		/* 119 */

					if(order>7){
						double i8=i7*i1;
						double j8=j7*j1;
						double k8=k7*k1;
						*dxt++=i8;		/* 120 */
						*dxt++=i7*j1;		/* 121 */
						*dxt++=i6*j2;		/* 122 */
						*dxt++=i5*j3;		/* 123 */
						*dxt++=i4*j4;		/* 124 */
						*dxt++=i3*j5;		/* 125 */
						*dxt++=i2*j6;		/* 126 */
						*dxt++=i1*j7;		/* 127 */
						*dxt++=j8;		/* 128 */
						*dxt++=i7*k1;		/* 129 */
						*dxt++=i6*j1*k1;		/* 130 */
						*dxt++=i5*j2*k1;		/* 131 */
						*dxt++=i4*j3*k1;		/* 132 */
						*dxt++=i3*j4*k1;		/* 133 */
						*dxt++=i2*j5*k1;		/* 134 */
						*dxt++=i1*j6*k1;		/* 135 */
						*dxt++=j7*k1;		/* 136 */
						*dxt++=i6*k2;		/* 137 */
						*dxt++=i5*j1*k2;		/* 138 */
						*dxt++=i4*j2*k2;		/* 139 */
						*dxt++=i3*j3*k2;		/* 140 */
						*dxt++=i2*j4*k2;		/* 141 */
						*dxt++=i1*j5*k2;		/* 142 */
						*dxt++=j6*k2;		/* 143 */
						*dxt++=i5*k3;		/* 144 */
						*dxt++=i4*j1*k3;		/* 145 */
						*dxt++=i3*j2*k3;		/* 146 */
						*dxt++=i2*j3*k3;		/* 147 */
						*dxt++=i1*j4*k3;		/* 148 */
						*dxt++=j5*k3;		/* 149 */
						*dxt++=i4*k4;		/* 150 */
						*dxt++=i3*j1*k4;		/* 151 */
						*dxt++=i2*j2*k4;		/* 152 */
						*dxt++=i1*j3*k4;		/* 153 */
						*dxt++=j4*k4;		/* 154 */
						*dxt++=i3*k5;		/* 155 */
						*dxt++=i2*j1*k5;		/* 156 */
						*dxt++=i1*j2*k5;		/* 157 */
						*dxt++=j3*k5;		/* 158 */
						*dxt++=i2*k6;		/* 159 */
						*dxt++=i1*j1*k6;		/* 160 */
						*dxt++=j2*k6;		/* 161 */
						*dxt++=i1*k7;		/* 162 */
						*dxt++=j1*k7;		/* 163 */
						*dxt++=k8;		/* 164 */

					if(order>8){
						double i9=i8*i1;
						double j9=j8*j1;
						double k9=k8*k1;
						*dxt++=i9;		/* 165 */
						*dxt++=i8*j1;		/* 166 */
						*dxt++=i7*j2;		/* 167 */
						*dxt++=i6*j3;		/* 168 */
						*dxt++=i5*j4;		/* 169 */
						*dxt++=i4*j5;		/* 170 */
						*dxt++=i3*j6;		/* 171 */
						*dxt++=i2*j7;		/* 172 */
						*dxt++=i1*j8;		/* 173 */
						*dxt++=j9;		/* 174 */
						*dxt++=i8*k1;		/* 175 */
						*dxt++=i7*j1*k1;		/* 176 */
						*dxt++=i6*j2*k1;		/* 177 */
						*dxt++=i5*j3*k1;		/* 178 */
						*dxt++=i4*j4*k1;		/* 179 */
						*dxt++=i3*j5*k1;		/* 180 */
						*dxt++=i2*j6*k1;		/* 181 */
						*dxt++=i1*j7*k1;		/* 182 */
						*dxt++=j8*k1;		/* 183 */
						*dxt++=i7*k2;		/* 184 */
						*dxt++=i6*j1*k2;		/* 185 */
						*dxt++=i5*j2*k2;		/* 186 */
						*dxt++=i4*j3*k2;		/* 187 */
						*dxt++=i3*j4*k2;		/* 188 */
						*dxt++=i2*j5*k2;		/* 189 */
						*dxt++=i1*j6*k2;		/* 190 */
						*dxt++=j7*k2;		/* 191 */
						*dxt++=i6*k3;		/* 192 */
						*dxt++=i5*j1*k3;		/* 193 */
						*dxt++=i4*j2*k3;		/* 194 */
						*dxt++=i3*j3*k3;		/* 195 */
						*dxt++=i2*j4*k3;		/* 196 */
						*dxt++=i1*j5*k3;		/* 197 */
						*dxt++=j6*k3;		/* 198 */
						*dxt++=i5*k4;		/* 199 */
						*dxt++=i4*j1*k4;		/* 200 */
						*dxt++=i3*j2*k4;		/* 201 */
						*dxt++=i2*j3*k4;		/* 202 */
						*dxt++=i1*j4*k4;		/* 203 */
						*dxt++=j5*k4;		/* 204 */
						*dxt++=i4*k5;		/* 205 */
						*dxt++=i3*j1*k5;		/* 206 */
						*dxt++=i2*j2*k5;		/* 207 */
						*dxt++=i1*j3*k5;		/* 208 */
						*dxt++=j4*k5;		/* 209 */
						*dxt++=i3*k6;		/* 210 */
						*dxt++=i2*j1*k6;		/* 211 */
						*dxt++=i1*j2*k6;		/* 212 */
						*dxt++=j3*k6;		/* 213 */
						*dxt++=i2*k7;		/* 214 */
						*dxt++=i1*j1*k7;		/* 215 */
						*dxt++=j2*k7;		/* 216 */
						*dxt++=i1*k8;		/* 217 */
						*dxt++=j1*k8;		/* 218 */
						*dxt++=k9;		/* 219 */

					if(order>9){
						double i10=i9*i1;
						double j10=j9*j1;
						double k10=k9*k1;
						*dxt++=i10;		/* 220 */
						*dxt++=i9*j1;		/* 221 */
						*dxt++=i8*j2;		/* 222 */
						*dxt++=i7*j3;		/* 223 */
						*dxt++=i6*j4;		/* 224 */
						*dxt++=i5*j5;		/* 225 */
						*dxt++=i4*j6;		/* 226 */
						*dxt++=i3*j7;		/* 227 */
						*dxt++=i2*j8;		/* 228 */
						*dxt++=i1*j9;		/* 229 */
						*dxt++=j10;		/* 230 */
						*dxt++=i9*k1;		/* 231 */
						*dxt++=i8*j1*k1;		/* 232 */
						*dxt++=i7*j2*k1;		/* 233 */
						*dxt++=i6*j3*k1;		/* 234 */
						*dxt++=i5*j4*k1;		/* 235 */
						*dxt++=i4*j5*k1;		/* 236 */
						*dxt++=i3*j6*k1;		/* 237 */
						*dxt++=i2*j7*k1;		/* 238 */
						*dxt++=i1*j8*k1;		/* 239 */
						*dxt++=j9*k1;		/* 240 */
						*dxt++=i8*k2;		/* 241 */
						*dxt++=i7*j1*k2;		/* 242 */
						*dxt++=i6*j2*k2;		/* 243 */
						*dxt++=i5*j3*k2;		/* 244 */
						*dxt++=i4*j4*k2;		/* 245 */
						*dxt++=i3*j5*k2;		/* 246 */
						*dxt++=i2*j6*k2;		/* 247 */
						*dxt++=i1*j7*k2;		/* 248 */
						*dxt++=j8*k2;		/* 249 */
						*dxt++=i7*k3;		/* 250 */
						*dxt++=i6*j1*k3;		/* 251 */
						*dxt++=i5*j2*k3;		/* 252 */
						*dxt++=i4*j3*k3;		/* 253 */
						*dxt++=i3*j4*k3;		/* 254 */
						*dxt++=i2*j5*k3;		/* 255 */
						*dxt++=i1*j6*k3;		/* 256 */
						*dxt++=j7*k3;		/* 257 */
						*dxt++=i6*k4;		/* 258 */
						*dxt++=i5*j1*k4;		/* 259 */
						*dxt++=i4*j2*k4;		/* 260 */
						*dxt++=i3*j3*k4;		/* 261 */
						*dxt++=i2*j4*k4;		/* 262 */
						*dxt++=i1*j5*k4;		/* 263 */
						*dxt++=j6*k4;		/* 264 */
						*dxt++=i5*k5;		/* 265 */
						*dxt++=i4*j1*k5;		/* 266 */
						*dxt++=i3*j2*k5;		/* 267 */
						*dxt++=i2*j3*k5;		/* 268 */
						*dxt++=i1*j4*k5;		/* 269 */
						*dxt++=j5*k5;		/* 270 */
						*dxt++=i4*k6;		/* 271 */
						*dxt++=i3*j1*k6;		/* 272 */
						*dxt++=i2*j2*k6;		/* 273 */
						*dxt++=i1*j3*k6;		/* 274 */
						*dxt++=j4*k6;		/* 275 */
						*dxt++=i3*k7;		/* 276 */
						*dxt++=i2*j1*k7;		/* 277 */
						*dxt++=i1*j2*k7;		/* 278 */
						*dxt++=j3*k7;		/* 279 */
						*dxt++=i2*k8;		/* 280 */
						*dxt++=i1*j1*k8;		/* 281 */
						*dxt++=j2*k8;		/* 282 */
						*dxt++=i1*k9;		/* 283 */
						*dxt++=j1*k9;		/* 284 */
						*dxt++=k10;		/* 285 */

					if(order>10){
						double i11=i10*i1;
						double j11=j10*j1;
						double k11=k10*k1;
						*dxt++=i11;		/* 286 */
						*dxt++=i10*j1;		/* 287 */
						*dxt++=i9*j2;		/* 288 */
						*dxt++=i8*j3;		/* 289 */
						*dxt++=i7*j4;		/* 290 */
						*dxt++=i6*j5;		/* 291 */
						*dxt++=i5*j6;		/* 292 */
						*dxt++=i4*j7;		/* 293 */
						*dxt++=i3*j8;		/* 294 */
						*dxt++=i2*j9;		/* 295 */
						*dxt++=i1*j10;		/* 296 */
						*dxt++=j11;		/* 297 */
						*dxt++=i10*k1;		/* 298 */
						*dxt++=i9*j1*k1;		/* 299 */
						*dxt++=i8*j2*k1;		/* 300 */
						*dxt++=i7*j3*k1;		/* 301 */
						*dxt++=i6*j4*k1;		/* 302 */
						*dxt++=i5*j5*k1;		/* 303 */
						*dxt++=i4*j6*k1;		/* 304 */
						*dxt++=i3*j7*k1;		/* 305 */
						*dxt++=i2*j8*k1;		/* 306 */
						*dxt++=i1*j9*k1;		/* 307 */
						*dxt++=j10*k1;		/* 308 */
						*dxt++=i9*k2;		/* 309 */
						*dxt++=i8*j1*k2;		/* 310 */
						*dxt++=i7*j2*k2;		/* 311 */
						*dxt++=i6*j3*k2;		/* 312 */
						*dxt++=i5*j4*k2;		/* 313 */
						*dxt++=i4*j5*k2;		/* 314 */
						*dxt++=i3*j6*k2;		/* 315 */
						*dxt++=i2*j7*k2;		/* 316 */
						*dxt++=i1*j8*k2;		/* 317 */
						*dxt++=j9*k2;		/* 318 */
						*dxt++=i8*k3;		/* 319 */
						*dxt++=i7*j1*k3;		/* 320 */
						*dxt++=i6*j2*k3;		/* 321 */
						*dxt++=i5*j3*k3;		/* 322 */
						*dxt++=i4*j4*k3;		/* 323 */
						*dxt++=i3*j5*k3;		/* 324 */
						*dxt++=i2*j6*k3;		/* 325 */
						*dxt++=i1*j7*k3;		/* 326 */
						*dxt++=j8*k3;		/* 327 */
						*dxt++=i7*k4;		/* 328 */
						*dxt++=i6*j1*k4;		/* 329 */
						*dxt++=i5*j2*k4;		/* 330 */
						*dxt++=i4*j3*k4;		/* 331 */
						*dxt++=i3*j4*k4;		/* 332 */
						*dxt++=i2*j5*k4;		/* 333 */
						*dxt++=i1*j6*k4;		/* 334 */
						*dxt++=j7*k4;		/* 335 */
						*dxt++=i6*k5;		/* 336 */
						*dxt++=i5*j1*k5;		/* 337 */
						*dxt++=i4*j2*k5;		/* 338 */
						*dxt++=i3*j3*k5;		/* 339 */
						*dxt++=i2*j4*k5;		/* 340 */
						*dxt++=i1*j5*k5;		/* 341 */
						*dxt++=j6*k5;		/* 342 */
						*dxt++=i5*k6;		/* 343 */
						*dxt++=i4*j1*k6;		/* 344 */
						*dxt++=i3*j2*k6;		/* 345 */
						*dxt++=i2*j3*k6;		/* 346 */
						*dxt++=i1*j4*k6;		/* 347 */
						*dxt++=j5*k6;		/* 348 */
						*dxt++=i4*k7;		/* 349 */
						*dxt++=i3*j1*k7;		/* 350 */
						*dxt++=i2*j2*k7;		/* 351 */
						*dxt++=i1*j3*k7;		/* 352 */
						*dxt++=j4*k7;		/* 353 */
						*dxt++=i3*k8;		/* 354 */
						*dxt++=i2*j1*k8;		/* 355 */
						*dxt++=i1*j2*k8;		/* 356 */
						*dxt++=j3*k8;		/* 357 */
						*dxt++=i2*k9;		/* 358 */
						*dxt++=i1*j1*k9;		/* 359 */
						*dxt++=j2*k9;		/* 360 */
						*dxt++=i1*k10;		/* 361 */
						*dxt++=j1*k10;		/* 362 */
						*dxt++=k11;		/* 363 */

					if(order>11){
						double i12=i11*i1;
						double j12=j11*j1;
						double k12=k11*k1;
						*dxt++=i12;		/* 364 */
						*dxt++=i11*j1;		/* 365 */
						*dxt++=i10*j2;		/* 366 */
						*dxt++=i9*j3;		/* 367 */
						*dxt++=i8*j4;		/* 368 */
						*dxt++=i7*j5;		/* 369 */
						*dxt++=i6*j6;		/* 370 */
						*dxt++=i5*j7;		/* 371 */
						*dxt++=i4*j8;		/* 372 */
						*dxt++=i3*j9;		/* 373 */
						*dxt++=i2*j10;		/* 374 */
						*dxt++=i1*j11;		/* 375 */
						*dxt++=j12;		/* 376 */
						*dxt++=i11*k1;		/* 377 */
						*dxt++=i10*j1*k1;	/* 378 */
						*dxt++=i9*j2*k1;		/* 379 */
						*dxt++=i8*j3*k1;		/* 380 */
						*dxt++=i7*j4*k1;		/* 381 */
						*dxt++=i6*j5*k1;		/* 382 */
						*dxt++=i5*j6*k1;		/* 383 */
						*dxt++=i4*j7*k1;		/* 384 */
						*dxt++=i3*j8*k1;		/* 385 */
						*dxt++=i2*j9*k1;		/* 386 */
						*dxt++=i1*j10*k1;	/* 387 */
						*dxt++=j11*k1;		/* 388 */
						*dxt++=i10*k2;		/* 389 */
						*dxt++=i9*j1*k2;		/* 390 */
						*dxt++=i8*j2*k2;		/* 391 */
						*dxt++=i7*j3*k2;		/* 392 */
						*dxt++=i6*j4*k2;		/* 393 */
						*dxt++=i5*j5*k2;		/* 394 */
						*dxt++=i4*j6*k2;		/* 395 */
						*dxt++=i3*j7*k2;		/* 396 */
						*dxt++=i2*j8*k2;		/* 397 */
						*dxt++=i1*j9*k2;		/* 398 */
						*dxt++=j10*k2;		/* 399 */
						*dxt++=i9*k3;		/* 400 */
						*dxt++=i8*j1*k3;		/* 401 */
						*dxt++=i7*j2*k3;		/* 402 */
						*dxt++=i6*j3*k3;		/* 403 */
						*dxt++=i5*j4*k3;		/* 404 */
						*dxt++=i4*j5*k3;		/* 405 */
						*dxt++=i3*j6*k3;		/* 406 */
						*dxt++=i2*j7*k3;		/* 407 */
						*dxt++=i1*j8*k3;		/* 408 */
						*dxt++=j9*k3;		/* 409 */
						*dxt++=i8*k4;		/* 410 */
						*dxt++=i7*j1*k4;		/* 411 */
						*dxt++=i6*j2*k4;		/* 412 */
						*dxt++=i5*j3*k4;		/* 413 */
						*dxt++=i4*j4*k4;		/* 414 */
						*dxt++=i3*j5*k4;		/* 415 */
						*dxt++=i2*j6*k4;		/* 416 */
						*dxt++=i1*j7*k4;		/* 417 */
						*dxt++=j8*k4;		/* 418 */
						*dxt++=i7*k5;		/* 419 */
						*dxt++=i6*j1*k5;		/* 420 */
						*dxt++=i5*j2*k5;		/* 421 */
						*dxt++=i4*j3*k5;		/* 422 */
						*dxt++=i3*j4*k5;		/* 423 */
						*dxt++=i2*j5*k5;		/* 424 */
						*dxt++=i1*j6*k5;		/* 425 */
						*dxt++=j7*k5;		/* 426 */
						*dxt++=i6*k6;		/* 427 */
						*dxt++=i5*j1*k6;		/* 428 */
						*dxt++=i4*j2*k6;		/* 429 */
						*dxt++=i3*j3*k6;		/* 430 */
						*dxt++=i2*j4*k6;		/* 431 */
						*dxt++=i1*j5*k6;		/* 432 */
						*dxt++=j6*k6;		/* 433 */
						*dxt++=i5*k7;		/* 434 */
						*dxt++=i4*j1*k7;		/* 435 */
						*dxt++=i3*j2*k7;		/* 436 */
						*dxt++=i2*j3*k7;		/* 437 */
						*dxt++=i1*j4*k7;		/* 438 */
						*dxt++=j5*k7;		/* 439 */
						*dxt++=i4*k8;		/* 440 */
						*dxt++=i3*j1*k8;		/* 441 */
						*dxt++=i2*j2*k8;		/* 442 */
						*dxt++=i1*j3*k8;		/* 443 */
						*dxt++=j4*k8;		/* 444 */
						*dxt++=i3*k9;		/* 445 */
						*dxt++=i2*j1*k9;		/* 446 */
						*dxt++=i1*j2*k9;		/* 447 */
						*dxt++=j3*k9;		/* 448 */
						*dxt++=i2*k10;		/* 449 */
						*dxt++=i1*j1*k10;	/* 450 */
						*dxt++=j2*k10;		/* 451 */
						*dxt++=i1*k11;		/* 452 */
						*dxt++=j1*k11;		/* 453 */
						*dxt++=k12;		/* 454 */

					}	/* end if(order>11) */
					}	/* end if(order>10) */
					}	/* end if(order>9) */
					}	/* end if(order>8) */
					} 	/* end if(order>7) */
					}	/* end if(order>6) */
					}	/* end if(order>5) */
					}	/* end if(order>4) */
					}	/* end if(order>3) */
					}	/* end if(order>2) */
					}	/* end if(order>1) */
				}
				/* Find the x, y and z coordinate */
				{

					double x_i=0.0;
					double y_i=0.0;
					double z_i=0.0;
					
					{
						double *es0ii=es[0];
						double *es1ii=es[1];
						double *es2ii=es[2];
						double *dxii=dx;
						unsigned int ii;
						
						for(ii=0;ii<coeffp;ii++, es0ii++, es1ii++, es2ii++, dxii++){
							/* x_i+=e0[ii]*dx[ii]; */
							/* y_i+=e1[ii]*dx[ii]; */
							/* z_i+=e2[ii]*dx[ii]; */

							x_i+=*es0ii**dxii;
							y_i+=*es1ii**dxii;
							z_i+=*es2ii**dxii;
						}
					}
					if(x_i<0 || x_i>x_max1) continue;
					if(y_i<0 || y_i>y_max1) continue;
					if(z_i<0 || z_i>z_max1) continue;


					/*Get the coordinates of the 8 voxels surrounding the designated pixel*/
					/* in the reslice file*/
					{
						unsigned int x_up=(unsigned int)ceil(x_i);
						unsigned int x_down=(unsigned int)floor(x_i);
						unsigned int y_up=(unsigned int)ceil(y_i);
						unsigned int y_down=(unsigned int)floor(y_i);
						unsigned int z_up=(unsigned int)ceil(z_i);
						unsigned int z_down=(unsigned int)floor(z_i);

						if(x_up==x_down){
							if(x_up<(unsigned int)x_max1) x_up++;
							else x_down--;
						}
						a=x_i-x_down;
						d=x_up-x_i;

						if(y_up==y_down){
							if(y_up<(unsigned int)y_max1) y_up++;
							else y_down--;
						}
						b=y_i-y_down;
						e=y_up-y_i;
						
						if(z_up==z_down){
							if(z_up<(unsigned int)z_max1) z_up++;
							else z_down--;
						}
						c=z_i-z_down;
						f=z_up-z_i;


						/*Get the values of these 8 voxels*/

						n0=(int)pixel2[z_down][y_down][x_down];
						n1=(int)pixel2[z_down][y_down][x_up];
						n2=(int)pixel2[z_down][y_up][x_down];
						n3=(int)pixel2[z_down][y_up][x_up];
						n4=(int)pixel2[z_up][y_down][x_down];
						n5=(int)pixel2[z_up][y_down][x_up];
						n6=(int)pixel2[z_up][y_up][x_down];
						n7=(int)pixel2[z_up][y_up][x_up];
					}
				}
			}

			/*Verify that something is above threshold here*/
			if(*rr<=threshold5 && n0<=threshold2i && n1<=threshold2i && n2<=threshold2i && n3<=threshold2i && n4<=threshold2i && n5<=threshold2i && n6<=threshold2i && n7<=threshold2i) continue;

			{
				/*Calculate the trilinear interpolated voxel value*/

				double pix4=n0*d*e*f+n1*a*e*f+n2*d*b*f+n3*a*b*f+n4*d*e*c+n5*a*e*c+n6*d*b*c+n7*a*b*c;


				/*Some intermediate values needed to calculate derivatives efficiently*/

				double dxpix4=((e*f)*(n1-n0)+(b*f)*(n3-n2)+(c*e)*(n5-n4)+(b*c)*(n7-n6));
				double dypix4=((d*f)*(n2-n0)+(a*f)*(n3-n1)+(c*d)*(n6-n4)+(a*c)*(n7-n5));
				double dzpix4=((d*e)*(n4-n0)+(a*e)*(n5-n1)+(b*d)*(n6-n2)+(a*b)*(n7-n3));

				/*Calculate the square of the difference*/
				cf+=pix4*pix4-2.0*pix4*spix3+spix3*spix3;


				/*Calculate derivatives that are nonzero*/

				{
					double *dcft=dcf;
					unsigned int t;
					
					{
						double *dxt=dx;
						
						for(t=0; t<coeffp; t++, dcft++, dxt++){
						
							/* dcf[t]=2.0*(pix4-pix3*scale)*dxpix4*dx[t];*/
							*dcft+=2.0*(pix4-spix3)*dxpix4**dxt;
						}
					}
					{
						double *dyt=dx;
						
						for(; t<2*coeffp; t++, dcft++, dyt++){
						
							/* dcf[t]=2.0*(pix4-pix3*scale)*dypix4*dy[t-coeffp];*/
							*dcft+=2.0*(pix4-spix3)*dypix4**dyt;
						}
					}
					{
						double *dzt=dx;
						
						for(; t<3*coeffp; t++, dcft++, dzt++){
						
							/* dcf[t]=2.0*(pix4-pix3*scale)*dzpix4*dz[t-2*coeffp];*/
							*dcft+=2.0*(pix4-spix3)*dzpix4**dzt;
						}
					}
					*dcft+=-2.0*pix3*(pix4-spix3);	/* formerly dcf_scale */
				}
				{
					double *ecfts=*ecf;		/* Note assumption that ecf was allocated by tritrix2() */
					unsigned int t;
					{
						double *dxt=dx;
						
						for(t=0; t<coeffp; t++, dxt++){
						
							unsigned int s;
							
							{
								double *dxs=dx;
								
								for(s=0; s<=t; s++, ecfts++, dxs++){
								
									*ecfts+=2.0*dxpix4*dxpix4**dxt**dxs;
								}
							}
						}
					}
					{
						double *dyt=dx;
						
						for(; t<2*coeffp; t++, dyt++){
						
							unsigned int s;
							
							{
								double *dxs=dx;
								
								for(s=0; s<coeffp; s++, ecfts++, dxs++){
								
									*ecfts+=2.0**dyt**dxs*(dxpix4*dypix4);
								}
							}
							{
								double *dys=dx;
								
								for(; s<=t; s++, ecfts++, dys++){
								
									*ecfts+=2.0*dypix4*dypix4**dyt**dys;
								}
							}
						}
					}
					{
						double *dzt=dx;
						
						for(; t<3*coeffp; t++, dzt++){
						
							unsigned int s;
							
							{
								double *dxs=dx;
								
								for(s=0; s<coeffp; s++, ecfts++, dxs++){
								
									*ecfts+=2.0**dzt**dxs*(dxpix4*dzpix4);
								}
							}
							{
								double *dys=dx;
								
								for(s=coeffp; s<2*coeffp; s++, ecfts++, dys++){
								
									*ecfts+=2.0**dzt**dys*(dypix4*dzpix4);
								}
							}
							{
								double *dzs=dx;
								
								for(s=2*coeffp; s<=t; s++, ecfts++, dzs++){
								
									*ecfts+=2.0*dzpix4*dzpix4**dzt**dzs;
								}
							}
						}
					}
					{
						unsigned int s;
						
						{
							double *dxs=dx;
							
							for(s=0; s<coeffp; s++, ecfts++, dxs++){
							
								*ecfts+=-2.0*pix3*dxpix4**dxs;
							}
						}
						{
							double *dys=dx;
							
							for(; s<2*coeffp; s++, ecfts++, dys++){
							
								*ecfts+=-2.0*pix3*dypix4**dys;
							}
						}
						{
							double *dzs=dx;
							
							for(; s<3*coeffp; s++, ecfts++, dzs++){
							
								*ecfts+=-2.0*pix3*dzpix4**dzs;
							}
						}
						{
							*ecfts+=2.0*pix3*pix3;	/* formerly ecf_scale_scale */
						}
					}
				}
				counttotal++;
			}
		}
	}

	/*Calculate the weighted, normalized standard deviation and its derivatives*/

	if(counttotal==0){
		(*error)++;
		return 0.0;
	}

	/*Normalize by the number of voxels*/

	cf/=counttotal;

	{
		double *dcft=dcf;
		double *ecfts=*ecf;
		unsigned int t;
		
		for(t=0;t<=3*coeffp;t++, dcft++){
		
			*dcft/=counttotal;
			{
				unsigned int s;
				
				for(s=0;s<=t;s++, ecfts++){
				
					*ecfts/=counttotal;
				}
			}
		}
	}
	
	return cf;
}
