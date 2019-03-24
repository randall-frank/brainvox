/* protos */
double  probks(double alam);
void gser(double *gamser,double a,double x, double *gln);
void gcf(double *gammcf,double a,double x, double *gln);
double  betacf(double a,double b,double x);

double gammln(double xx)
{
        double  x,tmp,ser;
        static double cof[6] = { 76.18009173, -86.50532033,24.01409822,
                -1.231739516,0.120858003e-2,-0.536382e-5};

        long int j;

        x = xx-1.0;
        tmp = x + 5.5;
        tmp -= (x+0.5)*log(tmp);
        ser = 1.0;
        for(j=0;j<=5;j++) {
                x += 1.0;
                ser += (cof[j]/x);
        }
        return (-tmp+log(2.50662827465*ser));
}

void gser(double *gamser,double a,double x, double *gln)
{
        long int n;
        double  sum,del,ap;

        *gln = gammln(a);
        if (x <=0.0) {
                if (x < 0.0) {
                        printf("X < 0 in GSER\n");
                        return;
                }
                *gamser = 0.0;
                return;
        } else {
                ap = a;
                sum = 1.0/a;
                del = sum;
                for(n=1;n<=ITMAX;n++) {
                        ap += 1.0;
                        del *= (x/ap);
                        sum += del;
                        if (fabs(del) < fabs(sum)*EPS) {
                                del = sum*exp((a*log(x)-x)-(*gln));
                                *gamser = del;
                                return;
                        }
                }
                printf("a too large, ITMAX too small in GSER\n");
        }
}

void gcf(double *gammcf,double a,double x, double *gln)
{
        long int n;
        double gold=0.0,g,fac=1.0,b1=1.0;
        double b0=0.0,anf,ana,an,a1,a0=1.0;

        *gln = gammln(a);
        a1 = x;
        for(n=1;n<=ITMAX;n++) {
                an = (double)(n);
                ana = an - a;
                a0 = (a1+a0*ana)*fac;
                b0 = (b1+b0*ana)*fac;
                anf = an*fac;
                a1 = x*a0+anf*a1;
                b1 = x*b0+anf*b1;
                if (a1) {
                        fac = 1.0/a1;
                        g = b1*fac;
                        if (fabs((g-gold)/g) < EPS) {
                                fac = exp((a*log(x)-x)-(*gln))*g;
                                *gammcf = fac;
                                return;
                        }
                        gold = g;
                }
        }
        printf("a too large, ITMAX too small in GCF\n");
}

double  gammq(double a,double x)
{
        double gamser,gammcf,gln;

        if (x < 0.0 || a <= 0.0) {
                printf("Invalid GAMMAQ args\n");
                return(-1);
        }
        if (x < (a+1.0)) {
                gser(&gamser,a,x,&gln);
                return (1.0-gamser);
        } else {
                gcf(&gammcf,a,x,&gln);
                return (gammcf);
        }
}

double  gammp(double a,double x)
{
        double gamser,gammcf,gln;

        if (x < 0.0 || a <= 0.0) {
                printf("Invalid GAMMAP args\n");
                return(-1);
        }
        if (x < (a+1.0)) {
                gser(&gamser,a,x,&gln);
                return (gamser);
        } else {
                gcf(&gammcf,a,x,&gln);
                return (1.0-gammcf);
        }
}

double  betacf(double a,double b,double x)
{
        double  qap,qam,qab,em,tem,d;
        double  bz,bm=1.0,bp,bpp;
        double  az=1.0,am=1.0,ap,app,aold;
        long int m;

        qab = a+b;
        qap = a+1.0;
        qam = a-1.0;
        bz = 1.0-qab*x/qap;
        for(m=1;m<=ITMAX;m++) {
                em = (double)(m);
                tem = em+em;
                d =em*(b-em)*x/((qam+tem)*(a+tem));
                ap = az+d*am;
                bp = bz+d*bm;
                d = -(a+em)*(qab+em)*x/((qap+tem)*(a+tem));
                app = ap+d*az;
                bpp = bp+d*bz;
                aold = az;
                am = ap/bpp;
                bm = bp/bpp;
                az = app/bpp;
                bz = 1.0;
                if (fabs(az-aold) < (EPS*fabs(az))) return(az);
        }
        printf("a or b too large or ITMAX too small in BETACF\n");
        return(0.0);
}

double  betai(double a,double b,double x)
{
        double bt;

        if (x < 0.0 || x > 1.0) {
                printf("Invalid args in BETAI\n");
                return(-1);
        }
        if (x == 0.0 || x == 1.0) {
                bt = 0.0;
        } else {
                bt=exp(gammln(a+b)-gammln(a)-gammln(b)+a*log(x)+b*log(1.0-x));
        }
        if (x < (a+1.0)/(a+b+2.0)) {
                return(bt*betacf(a,b,x)/a);
        } else {
                return(1.0-bt*betacf(b,a,1.0-x)/b);
        }
}

void    chstwo(double *bins1,double *bins2,long int nbins,long int knstrn,
        double *df,double *chsq,double *prob)
{
        long int j;
        double  temp;

        *df = (nbins-1-knstrn);
        *chsq = 0.0;
        for(j=0;j<nbins;j++) {
                if ((bins1[j] == 0.0) && (bins2[j] == 0.0)) {
                        *df = (*df) - 1.0;
                } else {
                        temp = bins1[j] - bins2[j];
                        *chsq = (*chsq) + ((temp*temp)/(bins1[j]+bins2[j]));
                }
        }
        *prob = gammq(0.5*(*df),0.5*(*chsq));
        return;
}

void    avevar(double *data,long int n,double *ave,double *svar)
{
        long int j;
        double  s;

        *ave=(*svar)=0.0;
        for(j=0;j<n;j++) *ave += data[j];
        *ave /= (double)(n);
        for(j=0;j<n;j++) {
                s = data[j] - (*ave);
                *svar += (s*s);
        }
        *svar /= (double)(n-1);
        return;
}
void    ftest(double *data1,long int n1,double *data2,long int n2,
                double *f,double *prob)
{
        double  var1,var2,ave1,ave2,df1,df2;

        avevar(data1,n1,&ave1,&var1);
        avevar(data2,n2,&ave2,&var2);
        if (var1 > var2) {
                *f = (var1/var2);
                df1 = n1 - 1;
                df2 = n2 - 1;
        } else {
                *f = (var2/var1);
                df1 = n2 - 1;
                df2 = n1 - 1;
        }
        *prob = 2.0*betai(0.5*df2,0.5*df1,df2/(df2+df1*(*f)));
        if ((*prob) > 1.0) *prob = 2.0 - (*prob);
        return;
}

void    tutest(double *data1,long int n1,double *data2,long int n2,
                double *t,double *prob)
{
        double  var1,var2,ave1,ave2,df;
        double  d1,d2,d3;

        avevar(data1,n1,&ave1,&var1);
        avevar(data2,n2,&ave2,&var2);
        *t=(ave1-ave2)/sqrt((var1/n1)+(var2/n2));
        d1 = ((var1/n1)+(var2/n2)) * ((var1/n1)+(var2/n2));
        d2 = (var1/n1) * (var1/n1);
        d3 = (var2/n2) * (var2/n2);
        df = d1/( (d2/(n1-1)) + (d3/(n2-1)) );
        *prob =betai(0.5*df,0.5,df/(df+((*t)*(*t)) ) );
        return;
}
void    moment(double *data,long int n,double *ave,double *adev,double *sdev,
                double *svar,double *skew,double *curt)
{
        long int j;
        double  s,p;

        *ave=0.0;
        *adev=(*svar)=(*skew)=(*curt)=0.0;
        if (n <= 1) return;
        s = 0.0;
        for(j=0;j<n;j++) s += data[j];
        *ave = s/n;
        for(j=0;j<n;j++) {
                *adev += fabs(s=data[j]-(*ave));
                *svar += (p=s*s);
                *skew += (p *= s);
                *curt += (p *= s);
        }
        *adev /= n;
        *svar /= (n-1);
        *sdev = sqrt(*svar);
        if ((*svar) != 0.0) {
                *skew /= (n*(*svar)*(*sdev));
                *curt = (*curt)/(n*(*svar)*(*svar)) - 3.0;
        } else {
                printf("No skew/kurtosis when variance = 0\n");
        }
}
void    kstwo(double *data1,long int n1,double *data2,long int n2,
                double *d,double *prob)
{
        long int        j1=0,j2=0;
        double          en1,en2,fn1=0,fn2=0,dt,d1,d2;

        en1 = n1;
        en2 = n2;
        *d = 0.0;
        qsort(data1,n1,sizeof(double),dcheck);
        qsort(data2,n2,sizeof(double),dcheck);
        while ((j1 < n1) && (j2 < n2)) {
                if ((d1=data1[j1]) <= (d2=data2[j2])) {
                        j1++;
                        fn1 = j1/en1;
                }
                if (d2 <= d1) {
                        j2++;
                        fn2=j2/en2;
                }
                if ((dt=fabs(fn2-fn1)) > *d) *d = dt;
        }
        *prob = probks(sqrt(en1*en2/(en1+en2))*(*d));

        return;
}

#define EPS1 0.001
#define EPS2 1.0e-8
double probks(double alam)
{
        long int        j;
        double          a2,fac=2.0,sum=0,term,termbf=0;

        a2 = -2.0*alam*alam;
        for(j=1;j<=100;j++) {
                term = fac*exp(a2*j*j);
                sum += term;
                if ((fabs(term) <= EPS1*termbf) ||
                        (fabs(term) <= EPS2*sum)) return(sum);
                fac = -fac;
                termbf = fabs(term);
        }
        return(1.0);
}
void fit(double *x,double *y,long int n,double *a,double *b,double *chi)
{
        long int        i;
        double          ss,sx,sy,sxss,t,stt;

        *chi = -1;

        sx = 0.0;
        sy = 0.0;
        for(i=0;i<n;i++) {
                sx = sx + x[i];
                sy = sy + y[i];
        }
        ss = n;
        sxss = sx/ss;
        stt = 0;
        *b = 0;
        for(i=0;i<n;i++) {
                t = x[i] - sxss;
                stt = stt + (t*t);
                *b = (*b) + (t*y[i]);
        }
        if (stt == 0.0) stt = 0.000001;
        *b = (*b) / stt;

        *a = (sy-(sx*(*b)))/ss;
        *chi = 0;
        for(i=0;i<n;i++) {
                t = y[i] - ((*a) + ((*b)*x[i]));
                *chi = (*chi) + (t*t);
        }
        return;
}

void kurt(double *data,long int num,double *v,double *s,double *t)
{
        double  s1,s2,s3,s4;
        double  n,mean,var;
        double  g2;

        long int        i;

        s1 = 0; s2 = 0; s3 = 0; s4 = 0;
        n = num;
        for(i=0;i<num;i++) {
                s1 += (data[i]);
                s2 += (data[i]*data[i]);
                s3 += (data[i]*data[i]*data[i]);
                s4 += (data[i]*data[i]*data[i]*data[i]);
        }
        mean = s1/n;
        var = (s2 - mean*s1)/n;

        g2 = (s4
                -4.0*mean*s3+6.0*pow(mean,2.0)*s2
                -4.0*pow(mean,3.0)*s1
                +n*pow(mean,4.0))
                /(n*pow(var,2.0));

        *v = g2;
        *s = sqrt(24.0/n);
        *t = g2/sqrt(24.0*n*(n-1.0)*(n-1.0)/((n-3.0)*(n-2.0)*(n+3.0)*(n+5.0)));

}

void skew(double *data,long int num,double *v,double *s,double *t)
{
        double  s1,s2,s3;
        double  n,mean,var;
        double  g1;

        long int        i;

        s1 = 0; s2 = 0; s3 = 0;
        n = num;
        for(i=0;i<num;i++) {
                s1 += (data[i]);
                s2 += (data[i]*data[i]);
                s3 += (data[i]*data[i]*data[i]);
        }
        mean = s1/n;
        var = (s2 - mean*s1)/n;

        g1 = (s3
                - 3.0*mean*s2
                + 3.0*pow(mean,2.0)*s1
                - pow(mean,2.0)*s1)
                / (n*pow(var,1.5));

        *v = g1;
        *s = sqrt(6.0/n);
        *t = g1/sqrt(6.0*n*(n-1.0)/((n-2.0)*(n+1.0)*(n+3.0)));
}

