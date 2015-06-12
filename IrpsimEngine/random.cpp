// random.cpp : implementation file
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
#include "StdAfx.h"
#include "random.h"
#include <time.h>

//#include <fstream.h>
//static ofstream sdebug("random.deb");

const ulong  M31    = 0x7FFFFFFFUL ;
const double dM31   = (double) M31 ;
const ulong  mask16 = 0x0000FFFFUL ;

// Class ModM31
ModM31::ModM31 (ulong m): n(m%M31) {}
ModM31 operator+ (ModM31 r, ModM31 s){
  return r.n + s.n ;
}
ModM31 operator- (ModM31 r, ModM31 s){
  return M31 + r.n - s.n ;
}
ModM31 operator* (ModM31 r, ModM31 s){
  ulong a = r.n >> 16, b = r.n & mask16 ;
  ulong c = s.n >> 16, d = s.n & mask16 ;
  ulong x = a*d + b*c ;
  return ModM31((a*c+(x>>16))<<1) +
		 ModM31(x<<16) + ModM31(b*d) ;
}
static void eqn (ulong a, ulong b,
				 ModM31 &x, ModM31 &y)
{
  ulong r = a % b ;
  if (!r){
	x = 0 ;
	y = 1 ;
	return ;
  }
  eqn (b,r,x,y) ;
  ModM31 s = y ;
  y = x - ModM31(a/b)*y ;
  x = s ;
}
ModM31 operator/ (ModM31 a, ModM31 b){
  ModM31 x,y ;
  eqn (M31,b,x,y) ;
  return a*y ;
}
// Class uVariable
ModM31 RandomVariable::seed = 3847UL ;
ModM31 RandomVariable::multiplier = Hoaglin1 ;

double RandomVariable::nextfloat()
{
  return nextlong() / dM31 ;
}

void RandomVariable::randomize(unsigned long aSeed)
{
	unsigned long s = (aSeed==0UL) ? ((unsigned long)time(0) | 0x0001) : aSeed;
	Reset(s);
}

//Class UniformVariable
UniformVariable::UniformVariable (double minimum,double maximum):
	mn(minimum), mx(maximum)
{
}

double UniformVariable::operator()()
{
	return mn + nextfloat()*(mx-mn);
}

// Class NormalVariable
NormalVariable::NormalVariable (double mean,
								double stddev):
	BoxMullerSwitch(1), mn(mean), std(stddev)
{
}

double NormalVariable::operator()()
{
	BoxMullerSwitch = !BoxMullerSwitch ;
	if (BoxMullerSwitch)
	  return U1*sin(U2) + mn ;
	U1 = std * sqrt(-2*log(nextfloat())) ;
	U2 = 2*M_PI*nextfloat() ;
	double ret = U1*cos(U2) + mn ;
	return ret;
}

// Class ExponentialVariable
ExponentialVariable::ExponentialVariable(double mean):
  mn(mean)
{
}

double ExponentialVariable::operator()()
{
  return -mn*log(nextfloat()) ;
}

// Class PoissonVariable
PoissonVariable::PoissonVariable (double mean):
  mn(exp(-mean)) {}

double PoissonVariable::operator()() {
  int count = 0 ;
  double product = nextfloat() ;
  while (product >= mn){
	count++ ;
	product *= nextfloat() ;
  }
  return (double) count ;
}

void PoissonVariable::setParameter(double mean)
{
	mn = exp(-mean);
}

double PoissonVariable::getMean()
{
	return -log(mn);
}

// Class GeometricVariable
GeometricVariable::GeometricVariable (double p):
  beta(1/log(1-p))
{
}

void GeometricVariable::setParameter(double p)
{
	beta = 1/log(1-p);
}

double GeometricVariable::getProb()
{
	return (1 - exp(1/beta));
}

double GeometricVariable::operator()()
{
  return (double) (1 + (int)(beta*log(nextfloat()))) ;
}

// Class BinomialVariable
BinomialVariable::BinomialVariable (int n, double p):
  nn(n), pr(p)
{
}

double BinomialVariable::operator()()
{
  int count = 0 ;
  for (int i = 0 ; i < nn ; i++)
	if (nextfloat() <= pr) count++ ;
  return (double) count ;
}

WeibullVariable::WeibullVariable (double sc,double sh):
	scale(sc), shape(sh)
{
}

double WeibullVariable::operator()()
{
	ulong n = nextlong();
	if (n==0) n=1;
	return scale * pow( -log((double)n/dM31), 1/shape);
}

template <class T>
double RejectionMethod<T>::operator()(){
  double t ;
  do {
	t = P() ;
  } while (nextfloat()*M(t) > f(t)) ;
  return t ;
}
