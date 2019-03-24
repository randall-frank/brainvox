
/* Prototypes from svd.f:
      SUBROUTINE SSVDC(X, LDX, N, P, S, E, U, LDU, V, LDV, WORK, JOB,  
     + INFO)
      INTEGER LDX, N, P, LDU, LDV, JOB, INFO
      REAL X(LDX,1), S(1), E(1), U(LDU,1), V(LDV,1), WORK(1)
*/

void ssvdc_(float *x,long int *ldx,long int *n,long int *p,float *s,float *e,
    	float *u,long int *ldu,float *v,long int *ldv,float *work,
	long int *job,long int *info);

