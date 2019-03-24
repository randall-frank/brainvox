typedef struct {
    int K;                      /* number of regressors */
    int nrbar;                  /* k*(k-1)/2 */
    int T;                      /* obs count */
    double *d, *rbar, *thetab;
    double sserr;
} ls_cookie_t;
    /* This is the magic cookie which gets shipped around often
       among these routines */

void ls_clear_cookie(ls_cookie_t *C)
/* Based on ALGORITHM AS274.2  APPL. STATIST. (1992) VOL.41, NO. 2
   Sets arrays to zero prior to calling AS75.1 */
{
    int i;

    for (i=0; i<C->K; ++i) {
        C->d[i] = 0;
        C->thetab[i] = 0;
    }
    for (i=0; i<C->nrbar; ++i) C->rbar[i] = 0;
    C->T = C->sserr = 0;
}

int ls_mk_cookie(int k, ls_cookie_t *C)
{
    C->K = k;
    C->nrbar = k*(k-1)/2;
    if (NULL == (C->d = (double *) malloc(k*sizeof(double))))
      return 1;
    if (NULL == (C->rbar = (double *) malloc(sizeof(double)*C->nrbar)))
      return 1;
    if (NULL == (C->thetab = (double *) malloc(k*sizeof(double))))
      return 1;
    ls_clear_cookie(C);
    return 0;
}

void ls_free_cookie(ls_cookie_t C)
{
    free(C.d); free(C.rbar); free(C.thetab);
}

