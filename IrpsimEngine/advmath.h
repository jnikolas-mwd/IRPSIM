// XXX.h : header file
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
#pragma once

#include "irp.h"
#include "cmdefs.h"

/*
#if !defined (_CM_SOLARIS)
double _IRPFUNC erf(double x);
double _IRPFUNC erfc(double x);
#endif
*/

double _IRPFUNC exp10(double r);
double _IRPFUNC dmin(double r1,double r2);
double _IRPFUNC dmax(double r1,double r2);
double _IRPFUNC pythag(double a, double b);

double _IRPFUNC ibeta(double a, double b, double x);
double _IRPFUNC invibeta(double a, double b, double p);
double _IRPFUNC beta(double z, double w);
double _IRPFUNC factorial(double n);
double _IRPFUNC lnfactorial(double n);
double _IRPFUNC lngamma(double xx);
double _IRPFUNC gammap(double a, double x);
double _IRPFUNC gammaq(double a, double x);
double _IRPFUNC cumnormal(double x);
double _IRPFUNC cumpoisson(double k,double mean);
double _IRPFUNC cumbinomial(double n,double k,double p);
double _IRPFUNC cumchi(double df,double x);
double _IRPFUNC cumt(double df,double t);
double _IRPFUNC cumf(double df1,double df2,double f);

double _IRPFUNC invnormal(double x);
double _IRPFUNC invf(double df1, double df2, double f);
double _IRPFUNC invbinomial(double n, double k, double p);
double _IRPFUNC invt(double df, double t);

double _IRPFUNC exponential(double mn);
double _IRPFUNC poisson(double mn);
double _IRPFUNC geometric(double p);
double _IRPFUNC binomial(double n,double p);
double _IRPFUNC normal(double r1,double r2);
double _IRPFUNC uniform(double r1,double r2);
double _IRPFUNC weibull(double r1,double r2);

double _IRPFUNC ifthenelse(double r1,double r2,double r3);

