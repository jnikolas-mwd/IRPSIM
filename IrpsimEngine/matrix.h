// matrix.h : header file
//
// Copyright © 1998-2015 Casey McSpadden   
//		mailto:casey@crossriver.com
//		http://www.crossriver.com/
//
// ==========================================================================  
// DESCRIPTION:	
// ==========================================================================
// Class CMMatrix class implements a matrix of arbitrary size
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
#include "bgvarray.h"

#include <iostream>

using namespace std;

typedef class _IRPCLASS CMVBigArray<double> CMVBIGDOUBLEARRAY;

class _IRPCLASS CMMatrix : public CMVBIGDOUBLEARRAY
{
	friend ostream& operator << (ostream& s, const CMMatrix& m);
	long rows,cols;
public:
	CMMatrix();
	CMMatrix(long r,long c);
	CMMatrix(const CMMatrix& m);
	CMMatrix& operator = (const CMMatrix& m);
   int operator == (const CMMatrix& m);
   long Rows() const {return rows;}
   long Cols() const {return cols;}
   void Init(long r=0,long c=0,double initval=0.0);
   int Redimension(long r,long col);
	int SetElement(long row,long col,double val) {return AddAt(row*cols+col,val);}
	int AddElement(double val) {return AddAt(Count(),val);}

	double& ElementAt(long row,long col) const {return At(row*cols+col);}
	void ExchangeRows(long i,long j);
	void ExchangeCols(long i,long j);

   CMMatrix& Add(const CMMatrix& a,const CMMatrix& b);
   CMMatrix& Add(const CMMatrix& a);
   CMMatrix& AddScalar(double val);
   CMMatrix& Multiply(const CMMatrix& a,const CMMatrix& b);
   CMMatrix& Multiply(const CMMatrix& a);
   CMMatrix& MultiplyScalar(double val);
	CMMatrix& Transpose(const CMMatrix& a);
	CMMatrix& Transpose();
   CMMatrix& SubMatrix(CMMatrix& a,long row,long col,long nrows,long ncols);
   CMMatrix& SubMatrix(long row,long col,long nrows,long ncols);
   CMMatrix& RemoveRows(CMMatrix& a,const CMVBigArray<long>& r);
   CMMatrix& RemoveRows(const CMVBigArray<long>& r);
   CMMatrix& RemoveCols(CMMatrix& a,const CMVBigArray<long>& c);
   CMMatrix& RemoveCols(const CMVBigArray<long>& c);

//   int LU(const CMMatrix& a,CMMatrix& index);
//   int LUSolve(const CMMatrix& a,CMMatrix& b);
   CMMatrix& SVD(const CMMatrix& a,CMMatrix& v,CMMatrix& w);
   CMMatrix& SVDSolve(const CMMatrix& a,const CMMatrix& b,CMMatrix& w);
   CMMatrix& LeastSquares(const CMMatrix& a,const CMMatrix& b,CMMatrix& cov,double& mse,long& df);
};

ostream& operator << (ostream& s, const CMMatrix& m);
