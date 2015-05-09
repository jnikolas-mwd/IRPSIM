// random.h : header file
//
// Copyright © 1998-2015 Casey McSpadden   
//		mailto:casey@crossriver.com
//		http://www.crossriver.com/
//
// ==========================================================================  
// DESCRIPTION:	
// ==========================================================================
// Defines the following Random variable classes:
// RandomVariable: Base virtual class
// RandomIntegerVariable
// BinomialVariable
// ExponentialVariable
// GeometricVariable
// NormalVariable
// PoissonVariable
// UniformVariable
// WeibullVariable
// ==========================================================================
//
// ==========================================================================  
// HISTORY:	
// ==========================================================================
//			1.00	09 March 2015	- Initial re-write and release.
// ==========================================================================
//
/////////////////////////////////////////////////////////////////////////////
#if !defined (__RANDOM_H)
#define __RANDOM_H

#include "irp.h"
#include "cmdefs.h"
#include <math.h>

typedef unsigned long ulong ;

const ulong Hoaglin1 = 397204094UL ;
const ulong Hoaglin2 = 764261123UL ;
const ulong APL      = 16807UL ;

class _IRPCLASS ModM31{
	ulong n ;
public:
	ModM31 (ulong m = 0) ;
	operator ulong () {return n ;}
	friend ModM31 operator+ (ModM31, ModM31) ;
	friend ModM31 operator- (ModM31, ModM31) ;
	friend ModM31 operator* (ModM31, ModM31) ;
	friend ModM31 operator/ (ModM31, ModM31) ;
} ;

class _IRPCLASS RandomVariable {
	static ModM31 seed ;
	static ModM31 multiplier ;
public:
	static ulong nextlong(void) {return seed = seed * multiplier;}
	static double nextfloat();
	static void Reset (ulong Seed, ulong mult = 0){
		if (Seed) seed = Seed ;
		if (mult) multiplier = mult ;
	}
	virtual double operator ()() {return 0;}
	static void randomize(unsigned long aSeed=0UL);
} ;

class _IRPCLASS RandomIntegerVariable : public RandomVariable {
  ulong nvals;
public:
  RandomIntegerVariable (ulong n = 65536UL) {nvals = n;}
  void setParameter(ulong n) {nvals = n;}
  ulong getLimit() {return nvals;}
  virtual double operator ()() {return (double)(nextlong()%nvals);}
  double operator () (ulong n) {nvals = n; return (operator())();}
} ;

class _IRPCLASS UniformVariable : public RandomVariable {
  double mn, mx;
public:
  UniformVariable (double minimum = 0.0, double maximum = 1.0);
  void setParameters(double minimum,double maximum) {mn=minimum,mx=maximum;}
  double getMin() {return mn;}
  double getMax() {return mx;}
  virtual double operator ()();
  double operator () (double minimum,double maximum)
	{mn=minimum;mx=maximum;return (operator ())();}
} ;

class _IRPCLASS NormalVariable : public RandomVariable {
  int BoxMullerSwitch ;
  double U1, U2 ;
  double mn, std;
public:
  NormalVariable (double mean = 0.0, double stddev = 1.0);
  void setParameters(double mean,double stddev) {mn=mean,std=stddev,BoxMullerSwitch=1;}
  double getMean() {return mn;}
  double getStd()  {return std;}
  virtual double operator ()();
  double operator () (double mean,double stddev)
	{setParameters(mean,stddev);return (operator())();}
} ;

class _IRPCLASS ExponentialVariable : public RandomVariable {
  double mn ;
public:
  ExponentialVariable (double mean = 1.0) ;
  void setParameter(double mean) {mn=mean;}
  double getMean() {return mn;}
  virtual double operator ()();
  double operator() (double mean) {mn=mean;return (operator())();}
} ;

class _IRPCLASS PoissonVariable : public RandomVariable {
  double mn ;
public:
  PoissonVariable (double mean = 1.0) ;
  void setParameter(double mean);
  double getMean();
  virtual double operator ()();
  double operator() (double mean) {setParameter(mean);return (operator())();}
} ;

class _IRPCLASS GeometricVariable : public RandomVariable {
  double beta ;
public:
  GeometricVariable (double p = 0.5) ;
  void setParameter(double p);
  double getProb();
  virtual double operator ()();
  double operator() (double p) {setParameter(p);return (operator())();}
} ;

class _IRPCLASS BinomialVariable : public RandomVariable {
  int nn ;
  double pr ;
public:
  BinomialVariable (int n = 1, double p = 0.5) ;
  void setParameters(int n=1,double p=0.5) {nn=n;pr=p;}
  int  getN() {return nn;}
  double getProb() {return pr;}
  virtual double operator ()();
  double operator() (int n,double p) {nn=n;pr=p;return (operator())();}
} ;

class _IRPCLASS WeibullVariable : public RandomVariable {
  double scale,shape;
public:
  WeibullVariable (double sc = 1, double sh = 1) ;
  void setParameters(double sc=1,double sh=1) {scale=sc;shape=sh;}
  double getScale() {return scale;}
  double getShape() {return shape;}
  virtual double operator ()();
  double operator() (double sc,double sh) {scale=sc;shape=sh;return (operator())();}
} ;

template <class T>
class _IRPCLASS RejectionMethod : public RandomVariable {
  T P ;
  double (*M) (double) ;
  double (*f) (double) ;
public:
  virtual double operator ()();
  RejectionMethod (double(*Mn)(double),
				   double(*fn)(double)):
	M(Mn), f(fn) {}
} ;

/*
template <class T>
double RejectionMethod<T>::operator()(){
  double t ;
  do {
	t = P() ;
  } while (nextfloat()*M(t) > f(t)) ;
  return t ;
}
*/

#endif // RANDOM_H
