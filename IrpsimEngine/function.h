// function.h : header file
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
#pragma once

#include "irp.h"
#include "cmdefs.h"
#include "smvarray.h"
#include "string.h"

class _IRPCLASS CMFunction
{
	friend int operator == (const CMFunction& f1,const CMFunction& f2);
	int type;
	static wchar_t* funcstr[];
	enum {FirstFunc1,
			Abs=FirstFunc1,Acos,Asin,Atan,Ceil,Const,Cosh,Cos,
			CumNormal,Erfc,Erf,Exponential,Exp10,Exp,Factorial,
			Floor,Geometric,InvNormal,LnFactorial,LnGamma,
			Log10,Log,Poisson,Sinh,Sin,Sqrt,Tanh,Tan,
			LastFunc1=Tan,

			FirstFunc2,
			Beta=FirstFunc2,Binomial,CumPoisson,CumChi,CumT,
			Gammap,Gammaq,Invt,Normal,Uniform,Weibull,
			LastFunc2=Weibull,

			FirstFunc3,
			CumBinomial=FirstFunc3,CumF,Ibeta,If,
			InvBinomial,Invf,InvIbeta,
			LastFunc3=InvIbeta,

			Discrete,Choose,Match,Min,Max
	};

	void   check_domain(double val,BOOL lower,double lowval,BOOL upper=FALSE,double upval=0);
	void   check_singularity(double val,double sing=0);
	double eval_discrete(double* parms,int nparms);
	double eval_choose(double* parms,int nparms);
	double eval_match(double* parms,int nparms);
	double eval_minmax(double* parms,int nparms,BOOL bIsMin);
	double evaluate(double* parms,int nparms);
public:
	CMFunction(const wchar_t* name = 0);
	CMFunction(const CMFunction& func) : type(func.type) {}
	const wchar_t* GetName() { return funcstr[type]; }
	static const wchar_t* FindFunction(const wchar_t* str, int* n = 0);
	CMFunction& operator = (const CMFunction& func) {type=func.type;return *this;}
	CMFunction& operator = (const wchar_t* str) { FindFunction(str, &type); return *this; }
	double operator () (double* parms,int nparms) {return evaluate(parms,nparms);}
	double operator () (double* parms) {return evaluate(parms,0);}
	double operator () (CMVSmallArray<double>& parms) {return evaluate(parms.Array(),(int)parms.Count());}
	double operator () (double p1) {return evaluate(&p1,1);}
	double operator () (double p1,double p2);
	double operator () (double p1,double p2,double p3);
	int NumParameters();
	BOOL IsStochastic();
};

inline int operator == (const CMFunction& f1,const CMFunction& f2)
{
	return (f1.type == f2.type);
}

