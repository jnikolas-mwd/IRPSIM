// function.cpp : implementation file
//
// Copyright © 1998-2015 Casey McSpadden   
//		mailto:casey@crossriver.com
//		http://www.crossriver.com/
//
// ==========================================================================  
// DESCRIPTION:	
// ==========================================================================
// Class CMFunction universal function class.
// ==========================================================================
//
// ==========================================================================  
// HISTORY:	
// ==========================================================================
//			1.00	09 March 2015	- Initial re-write and release.
// ==========================================================================
//
/////////////////////////////////////////////////////////////////////////////
#include "StdAfx.h"
#include "function.h"
#include "advmath.h"
#include "notify.h"
#include "string.h"
#include <ctype.h>
#include <string.h>
#include <math.h>

wchar_t* CMFunction::funcstr[] = {
	L"abs",L"acos",L"asin",L"atan",L"ceil",L"const",L"cosh",L"cos",
	L"cumnormal",L"erfc",L"erf",L"exponential",L"exp10",L"exp",
	L"factorial",L"floor",L"geometric",L"invnormal",L"lnfactorial",L"lngamma",
	L"log10",L"log",L"poisson",L"sinh",L"sin",L"sqrt",L"tanh",L"tan",
	L"beta",L"binomial",L"cumpoisson",L"cumchi",L"cumt",
	L"gammap",L"gammaq",L"invt",L"normal",L"uniform",L"weibull",
 	L"cumbinomial",L"cumf",L"ibeta",L"if",L"invbinomial",L"invf",L"invibeta",
	L"discrete",L"choose",L"match",L"min",L"max",
	NULL
};

CMFunction::CMFunction(const wchar_t* str)
{
	if (!str)
		type = Const;
	else
		FindFunction(str,&type);
}

const wchar_t* CMFunction::FindFunction(const wchar_t* str, int* np)
{
	string s(str);
	int flag = string::set_case_sensitive(0);

	int i;
	for (i=0;funcstr[i];i++) {
		unsigned int len = wcslen(funcstr[i]);
		if (s(0,len) == funcstr[i]) {
//		if (!strncmpi(s.c_str(),funcstr[i],strlen(funcstr[i]))) {
//			int ch = *(str+strlen(funcstr[i]));
			if (len==s.length() || s[len] <= L' ' || s[len] == L'(')
				break;
		}
	}

	if (np) *np = funcstr[i] ? i : Const;

	string::set_case_sensitive(flag);
	return funcstr[i];
}

void CMFunction::check_domain(double val,BOOL lower,double lowval,BOOL upper,double upval)
{
	if ( (lower==TRUE && val<lowval) || (upper==TRUE && val>upval) )
		CMNotifier::Notify(CMNotifier::ERROR, string(L"function domain: ") + GetName());
}

void CMFunction::check_singularity(double val,double sing)
{
	if (val==sing)
		CMNotifier::Notify(CMNotifier::ERROR, string(L"singularity: ") + GetName());
}

int CMFunction::NumParameters()
{
	if (type>=FirstFunc1 && type<=LastFunc1)
		return 1;
	else if (type>=FirstFunc2 && type<=LastFunc2)
		return 2;
	else if (type>=FirstFunc3 && type<=LastFunc3)
		return 3;

	return 0;
}

BOOL CMFunction::IsStochastic()
{
	return
		(type==Exponential || type==Geometric || type==Poisson ||
		type==Binomial || type==Normal || type==Uniform || type==Weibull)	?
	TRUE : FALSE;
}

double CMFunction::operator () (double p1,double p2)
{
	double args[2];
	args[0]=p1;
	args[1]=p2;
	return evaluate(args,2);
}

double CMFunction::operator () (double p1,double p2,double p3)
{
	double args[3];
	args[0]=p1;
	args[1]=p2;
	args[2]=p3;
	return evaluate(args,3);
}

double CMFunction::evaluate(double* parms,int nparms)
{
	double ret;
	switch (type) {
		case Abs:  		 	ret = fabs(parms[0]); break;
		case Acos:
			check_domain(parms[0],TRUE,-1,TRUE,1);
			ret = acos(parms[0]);
			break;
		case Asin:
			check_domain(parms[0],TRUE,-1,TRUE,1);
			ret = asin(parms[0]);
			break;
		case Atan:        ret = atan(parms[0]); break;
		case Ceil:        ret = ceil(parms[0]); break;
		case Cosh:        ret = cosh(parms[0]); break;
		case Cos:         ret = cos(parms[0]); break;
		case CumNormal:   ret = cumnormal(parms[0]); break;
		case Erfc:        ret = erfc(parms[0]); break;
		case Erf:         ret = erf(parms[0]); break;
		case Exponential: ret = exponential(parms[0]); break;
		case Exp10:       ret = exp10(parms[0]); break;
		case Exp:         ret = exp(parms[0]); break;
		case Factorial:   ret = factorial(parms[0]); break;
		case Floor:       ret = floor(parms[0]); break;
		case Geometric:   ret = geometric(parms[0]); break;
		case InvNormal:   ret = invnormal(parms[0]); break;
		case LnFactorial: ret = lnfactorial(parms[0]); break;
		case LnGamma:     ret = lngamma(parms[0]); break;
		case Log10:
			check_domain(parms[0],TRUE,0);
			check_singularity(parms[0]);
			ret = log10(parms[0]);
			break;
		case Log:
			check_domain(parms[0],TRUE,0);
			check_singularity(parms[0]);
			ret = log(parms[0]);
			break;
		case Poisson:     ret = poisson(parms[0]); break;
		case Sinh:        ret = sinh(parms[0]); break;
		case Sin:         ret = sin(parms[0]); break;
		case Sqrt:
			check_domain(parms[0],TRUE,0);
			ret = sqrt(parms[0]);
			break;
		case Tanh:        ret = tanh(parms[0]); break;
		case Tan:         ret = tan(parms[0]); break;

		// 2 parameters

		case Beta:        ret = beta(parms[0],parms[1]); break;
		case Binomial:    ret = binomial(parms[0],parms[1]); break;
		case CumPoisson:  ret = cumpoisson(parms[0],parms[1]); break;
		case CumChi:      ret = cumchi(parms[0],parms[1]); break;
		case CumT:        ret = cumt(parms[0],parms[1]); break;
		case Gammap:      ret = gammap(parms[0],parms[1]); break;
		case Gammaq:      ret = gammaq(parms[0],parms[1]); break;
		case Invt:        ret = invt(parms[0],parms[1]); break;
		case Normal:      ret = normal(parms[0],parms[1]); break;
		case Uniform:     ret = uniform(parms[0],parms[1]); break;
		case Weibull:     ret = weibull(parms[0],parms[1]); break;

		// 3 parameters

		case CumBinomial: ret = cumbinomial(parms[0],parms[1],parms[2]); break;
		case CumF:        ret = cumf(parms[0],parms[1],parms[2]); break;
		case Ibeta:       ret = ibeta(parms[0],parms[1],parms[2]); break;
		case InvBinomial: ret = invbinomial(parms[0],parms[1],parms[2]); break;
		case If: 	      ret = ifthenelse(parms[0],parms[1],parms[2]); break;
		case Invf:        ret = invf(parms[0],parms[1],parms[2]); break;
		case InvIbeta:    ret = invibeta(parms[0],parms[1],parms[2]); break;

		// special cases

		case Discrete:		ret = eval_discrete(parms,nparms); break;
		case Choose:		ret = eval_choose(parms,nparms); break;
		case Match:			ret = eval_match(parms,nparms); break;
		case Min:			ret = eval_minmax(parms,nparms,TRUE); break;
		case Max:			ret = eval_minmax(parms,nparms,FALSE); break;
		
		default:				ret = nparms ? parms[0] : 0; break;
	}
	return ret;
};

double CMFunction::eval_discrete(double* parms,int nparms)
{
	if (nparms%2) {
		CMNotifier::Notify(CMNotifier::ERROR, string(L"must have an even number of arguments: ") + GetName());
      return 0;
   }
	int evaluated=0;
	double ret = 0;
	double val = uniform(0,1);
	double cum = 0;
	for (int i=0;i<nparms;i+=2) {
		cum += parms[i];
		if (val<=cum && !evaluated) {
			evaluated = 1;
			ret = (i+1 < nparms) ?  parms[i+1] : 0;
		}
	}
	if (cum != 1)
		CMNotifier::Notify(CMNotifier::ERROR, string(L"must sum to 1: ") + GetName());
	return ret;
}


double CMFunction::eval_choose(double* parms,int nparms)
{
	int index = (int)parms[0];
   if (index<=0 || index>nparms-1) {
	   CMNotifier::Notify(CMNotifier::ERROR, string(L"index out of range: ") + GetName());
      return 0;
   }
   return parms[index];
}

double CMFunction::eval_match(double* parms,int nparms)
{
	double matchval = parms[0];
   for (int i=1;i<nparms;i++)
   	if (parms[i]==matchval)
      	return i;
   return 0;
}

double CMFunction::eval_minmax(double* parms,int nparms,BOOL bIsMin)
{
   double ret=0;
	for (int i=0;i<nparms;i++)
		ret = (i==0) ? parms[i] : (bIsMin ? dmin(ret,parms[i]) : dmax(ret,parms[i]));
   return ret;
}
