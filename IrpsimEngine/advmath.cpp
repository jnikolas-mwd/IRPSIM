// advmath.cpp : implementation file
//
// Copyright © 1998-2015 Casey McSpadden   
//		mailto:casey@crossriver.com
//		http://www.crossriver.com/
//

// ==========================================================================  
// HISTORY:	
// ==========================================================================
//			1.00	09 March 2015	- Initial re-write and release.
// ==========================================================================
//
/////////////////////////////////////////////////////////////////////////////

#include "advmath.h"
#include "random.h"

#include <math.h>

#define MAXIT 100
#define EPS 3.0e-7
#define FPMIN 1.0e-30

static double betacf(double a, double b, double x);
static void gcf(double *gammcf, double a, double x, double *gln);
static void gser(double *gamser, double a, double x, double *gln);

double exp10(double r)
{
	return pow(10,r);
}

double dmin(double r1,double r2)
{
	return (r1 < r2) ? r1 : r2;
}

double dmax(double r1,double r2)
{
	return (r1 > r2) ? r1 : r2;
}

double pythag(double a, double b)
{
	double absa,absb;
	absa=fabs(a);
	absb=fabs(b);
	if (absa > absb) return absa*sqrt(1.0+cmsqr(absb/absa));
	else return (absb == 0.0 ? 0.0 : absb*sqrt(1.0+cmsqr(absa/absb)));
}

double ibeta(double a, double b, double x)
{
	double bt;

	if (x < 0.0 || x > 1.0 || a <= 0.0 || b <= 0.0) return 0;
	if (x == 0.0 || x == 1.0) bt=0.0;
	else
		bt=exp(lngamma(a+b)-lngamma(a)-lngamma(b)+a*log(x)+b*log(1.0-x));
	if (x < (a+1.0)/(a+b+2.0))
		return bt*betacf(a,b,x)/a;
	else
		return 1.0-bt*betacf(b,a,1.0-x)/b;
}

double invibeta(double a, double b, double p)
{
	if (p<0.0 || p>1.0 || a <= 0.0 || b <= 0.0) return 0;
	double y = invnormal(1-p);
	double l = (y*y-3)/6;
	double h = 2/(1/(2*a-1)+1/(2*b-1));
	double w = (y*sqrt(h+l)/h) - (1/(2*b-1)-1/(2*a-1))*(l+0.833333333333-2/(3*h));
	return a/(a+b*exp(2*w));
}

double beta(double z, double w)
{
	return exp(lngamma(z)+lngamma(w)-lngamma(z+w));
}

double factorial(double n)
{
	static int ntop=4;
	static double a[33]={1.0,1.0,2.0,6.0,24.0};
	int j;

	if (n < 0) return 0;
	if (n > 32) return exp(lngamma(n+1.0));
	while (ntop<(int)n) {
		j=ntop++;
		a[ntop]=a[j]*ntop;
	}
	return a[(int)n];
}

double lnfactorial(double n)
{
	static double a[101];
	int N = (int) n;

	if (N <= 1) return 0.0;
	if (N <= 100) return a[N] ? a[N] : (a[N]=lngamma(N+1.0));
	else return lngamma(N+1.0);
}

double lngamma(double xx)
{
	double x,y,tmp,ser;
	static double cof[6]={76.18009172947146,-86.50532032941677,
		24.01409824083091,-1.231739572450155,
		0.1208650973866179e-2,-0.5395239384953e-5};
	int j;

	y=x=xx;
	tmp=x+5.5;
	tmp -= (x+0.5)*log(tmp);
	ser=1.000000000190015;
	for (j=0;j<=5;j++) ser += cof[j]/++y;
	return -tmp+log(2.5066282746310005*ser/x);
}

double gammap(double a, double x)
{
	double gamser,gammcf,gln;

	if (x < 0.0 || a <= 0.0) return 0;
	if (x < (a+1.0)) {
		gser(&gamser,a,x,&gln);
		return gamser;
	} else {
		gcf(&gammcf,a,x,&gln);
		return 1.0-gammcf;
	}
}

double gammaq(double a, double x)
{
	double gamser,gammcf,gln;

	if (x < 0.0 || a <= 0.0) return 0;
	if (x < (a+1.0)) {
		gser(&gamser,a,x,&gln);
		return 1.0-gamser;
	} else {
		gcf(&gammcf,a,x,&gln);
		return gammcf;
	}
}

/*
#if !defined (_CM_SOLARIS)
double _IRPFUNC erf(double x)
{
	return x < 0.0 ? -gammap(0.5,x*x) : gammap(0.5,x*x);
}

double _IRPFUNC erfc(double x)
{
	return x < 0.0 ? 1.0+gammap(0.5,x*x) : gammaq(0.5,x*x);
}
#endif
*/

double cumnormal(double x)
{
	return ((x>=0) ? (1+erf(x/M_SQRT2))/2 : (erfc(-x/M_SQRT2))/2);
}

double cumpoisson(double k,double mean)
{
	return gammaq(k+1,mean);
}

double cumbinomial(double n,double k,double p)
{
	return 1-ibeta(k+1,n-k,p);
}

double cumchi(double df,double x)
{
	return gammap(df/2,x/2);
}

double cumt(double df,double t)
{
	return 1-ibeta(df/2,0.5,df/(df+t*t))/2;
}

double cumf(double df1,double df2,double f)
{
	return 1-ibeta(df2/2,df1/2,df2/(df2+df1*f));
}

double invnormal(double x)
{
	static double cof[6]={2.515517,0.802853,0.010328,1.432788,0.189269,0.001308};

	if (x<0 || x>1) return 0;

	int neg = 0;
	double ret;

	if (x>0.5) {
		x = 1-x;
		neg++;
	}

	double t = sqrt(log(1/(x*x)));

	ret = t - (cof[0]+cof[1]*t+cof[2]*t*t)/(1+cof[3]*t+cof[4]*t*t+cof[5]*t*t*t);

	return neg ? ret : -ret;
}

double invf(double df1, double df2, double f)
{
	double x = invibeta(df2/2,df1/2,1-f);
	return (x==0) ? 0 : df2*(1-x)/(df1*x);
}

double invbinomial(double n,double k,double p)
{
	if (k==0 && n>0 && p>=0 && p<=1)
		return (1 - pow(p,1/n));
	return invibeta(k+1,n-k,1-p);
}

double invt(double df,double t)
{
	double x  = invnormal(t);
	double x2 = x*x; double x3 = x2*x; double x5 = x3*x2;
	double x7 = x5*x2; double x9 = x7*x2;

	double g1 = (x3 + x)/4;
	double g2 = (5*x5 + 16*x3 + 3*x)/96;
	double g3 = (3*x7 + 19*x5 + 17*x3 - 15*x)/384;
	double g4 = (79*x9 + 776*x7 + 1482*x5 - 1920*x3 - 945*x)/92160L;

	return (x + g1/df + g2/(df*df) + g3/(df*df*df) + g4/(df*df*df*df));
}

double betacf(double a, double b, double x)
{
	int m,m2;
	double aa,c,d,del,h,qab,qam,qap;

	qab=a+b;
	qap=a+1.0;
	qam=a-1.0;
	c=1.0;
	d=1.0-qab*x/qap;
	if (fabs(d) < FPMIN) d=FPMIN;
	d=1.0/d;
	h=d;
	for (m=1;m<=MAXIT;m++) {
		m2=2*m;
		aa=m*(b-m)*x/((qam+m2)*(a+m2));
		d=1.0+aa*d;
		if (fabs(d) < FPMIN) d=FPMIN;
		c=1.0+aa/c;
		if (fabs(c) < FPMIN) c=FPMIN;
		d=1.0/d;
		h *= d*c;
		aa = -(a+m)*(qab+m)*x/((a+m2)*(qap+m2));
		d=1.0+aa*d;
		if (fabs(d) < FPMIN) d=FPMIN;
		c=1.0+aa/c;
		if (fabs(c) < FPMIN) c=FPMIN;
		d=1.0/d;
		del=d*c;
		h *= del;
		if (fabs(del-1.0) < EPS) break;
	}
	return h;
}

void gcf(double *gammcf, double a, double x, double *gln)
{
	int i;
	double an,b,c,d,del,h;

	*gln=lngamma(a);
	b=x+1.0-a;
	c=1.0/FPMIN;
	d=1.0/b;
	h=d;
	for (i=1;i<=MAXIT;i++) {
		an = -i*(i-a);
		b += 2.0;
		d=an*d+b;
		if (fabs(d) < FPMIN) d=FPMIN;
		c=b+an/c;
		if (fabs(c) < FPMIN) c=FPMIN;
		d=1.0/d;
		del=d*c;
		h *= del;
		if (fabs(del-1.0) < EPS) break;
	}
	*gammcf=exp(-x+a*log(x)-(*gln))*h;
}

void gser(double *gamser, double a, double x, double *gln)
{
	int n;
	double sum,del,ap;

	*gln=lngamma(a);
	if (x <= 0.0) {
		*gamser=0.0;
		return;
	} else {
		ap=a;
		del=sum=1.0/a;
		for (n=1;n<=MAXIT;n++) {
			++ap;
			del *= x/ap;
			sum += del;
			if (fabs(del) < fabs(sum)*EPS) {
				*gamser=sum*exp(-x+a*log(x)-(*gln));
				return;
			}
		}
		return;
	}
}

// Stochastic functions

double exponential(double mn)
{
	static ExponentialVariable E;
	return E(mn);
}

double poisson(double mn)
{
	static PoissonVariable P;
	return P(mn);
}

double geometric(double p)
{
	static GeometricVariable G;
	return G(p);
}

double binomial(double n,double p)
{
	static BinomialVariable B;
	return B((int)n,p);
}

double normal(double r1,double r2)
{
	static NormalVariable N;
	return N(r1,r2);
}

double uniform(double r1,double r2)
{
	static UniformVariable U;
	return U(r1,r2);
}

double weibull(double r1,double r2)
{
	static WeibullVariable W;
	return W(r1,r2);
}

double ifthenelse(double r1,double r2,double r3)
{
	return (r1==0) ? r3 : r2;
}
