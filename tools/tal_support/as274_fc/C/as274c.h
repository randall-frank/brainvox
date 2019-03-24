int includ(int np, int nrbar, double w, 
           double *xrow, double yelem, double *d, double *rbar, 
           double *thetab, double *sserr);

int clear(int np, int nrbar, double *d, double *rbar,
          double *thetab, double *sserr);

int regcf(int np, int nrbar, double *d, double *rbar,
          double *thetab, double *tol, double *beta,
          int nreq);

int tolset(int np, int nrbar, double *d, double *rbar, double *tol);

int sing(int np, int nrbar, double *d, 
         double *rbar, double *thetab, double *sserr,
         double *tol, int *lindep);

int ss(int np, double *d, double *thetab, 
       double *sserr, double *rss);

int cov(int np, int nrbar, double *d, 
        double *rbar, int nreq, double *rinv, double *var, 
        double *covmat, int dimcov, double *sterr);

void inv(int np, int nrbar, double *rbar, int nreq, double *rinv);

int pcorr(int np, int nrbar, double *d, 
          double *rbar, double *thetab, double *sserr, int in, 
          double *cormat, int dimc, double *ycorr);

void cor(int np, double *d, double *rbar, 
         double *thetab, double *sserr, double *work,
         double *cormat, double *ycorr);

int vmove(int np, int nrbar, int *vorder, 
          double *d, double *rbar, double *thetab, double *rss, 
          int from, int to, double *tol);

int reordr(int np, int nrbar, int *vorder, 
	double *d, double *rbar, double *thetab, double *rss, 
           double *tol, int *list, int n, int pos1);

int hdiag(double *xrow, int np, int nrbar, double *d,
          double *rbar, double *tol, int nreq, double *hii);
