// matrix.cpp : implementation file
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
#include "StdAfx.h"
#include "matrix.h"
#include "advmath.h"
#include <math.h>

#define CM_SMALL 1e-15

CMMatrix::CMMatrix() : CMVBigArray<double>(0,100)
{
	Init(0,0);
}

CMMatrix::CMMatrix(long r,long c) : CMVBigArray<double>(0,100)
{
	Init(r,c);
}

CMMatrix::CMMatrix(const CMMatrix& m) : CMVBigArray<double>(0,100)
{
 	rows=m.rows;
   cols=m.cols;
   for (long i=0;i<m.Count();i++)
   	this->AddAt(i,m[i]);
}


CMMatrix& CMMatrix::operator = (const CMMatrix& m)
{
	Reset(1);
 	rows=m.rows;
   cols=m.cols;
   for (long i=0;i<m.Count();i++)
   	AddAt(i,m[i]);
   return (*this);
}

int CMMatrix::operator == (const CMMatrix& m)
{
	if (rows!=m.rows || cols!=m.cols || Count() != m.Count()) return 0;
   for (long i=0;i<Count();i++)
		if (At(i) != m.At(i))
      	return 0;
   return 1;
}

void CMMatrix::Init(long r,long c,double initval)
{
	Reset(1);
	rows = r; cols = c;
   for (long i=0;i<r*c;i++)
   	AddAt(i,initval);
}

int CMMatrix::Redimension(long r,long c)
{
	if (r*c != Count())
   	return 0;
	rows=r;cols=c;
   return 1;
}

void CMMatrix::ExchangeRows(long i,long j)
{
	if (i>=rows || j>=rows)
   	return;
   for (long k=0;k<cols;k++) {
   	double temp = ElementAt(i,k);
	   SetElement(i,k,ElementAt(j,k));
      SetElement(j,k,temp);
   }
}

void CMMatrix::ExchangeCols(long i,long j)
{
	if (i>=cols || j>=cols)
   	return;
   for (long k=0;k<rows;k++) {
   	double temp = ElementAt(k,i);
      SetElement(k,i,ElementAt(k,j));
      SetElement(k,j,temp);
   }
}

CMMatrix& CMMatrix::Add(const CMMatrix& a,const CMMatrix& b)
{
	Init();
	if (a.rows==b.rows && a.cols==b.cols && a.Count() == b.Count()) {
		rows = a.rows;
   	cols = a.cols;
	   for (long i=0;i<a.Count();i++)
			AddAt(i,a[i]+b[i]);
   }
   return *this;
}

CMMatrix& CMMatrix::Multiply(const CMMatrix& a,const CMMatrix& b)
{
	Init();
	if (a.cols == b.rows) {
		rows = a.rows;
   	cols = b.cols;
	   for (long i=0;i<rows;i++) {
		   for (long j=0;j<cols;j++) {
      		double sum=0;
         	for (long k=0;k<a.cols;k++)
         		sum += a.ElementAt(i,k)*b.ElementAt(k,j);
	         SetElement(i,j,sum);
   	   }
	   }
   }
   return *this;
}

CMMatrix& CMMatrix::Add(const CMMatrix& a)
{
	if (rows==a.rows && cols==a.cols) {
	   for (long i=0;i<Count();i++)
			AddAt(i,At(i)+a[i]);
   }
   return *this;
}

CMMatrix& CMMatrix::AddScalar(double val)
{
	for (long i=0;i<Count();i++)
   	At(i) += val;
	return *this;
}

CMMatrix& CMMatrix::Multiply(const CMMatrix& b)
{
	if (cols == b.rows) {
		CMMatrix a(*this);
      Multiply(a,b);
   }
   return *this;
}

CMMatrix& CMMatrix::MultiplyScalar(double val)
{
	for (long i=0;i<Count();i++)
   	At(i) *= val;
	return *this;
}

CMMatrix& CMMatrix::Transpose(const CMMatrix& a)
{
	Reset(1);

   rows = a.cols;
   cols = a.rows;

   for (long i=0;i<rows;i++)
	   for (long j=0;j<cols;j++)
      	SetElement(i,j,a.ElementAt(j,i));

   return *this;
}

CMMatrix& CMMatrix::Transpose()
{
	CMMatrix a(*this);
	return Transpose(a);
}

CMMatrix& CMMatrix::SubMatrix(CMMatrix& a,long row,long col,long nrows,long ncols)
{
 	if (row<0 || row>=a.Rows()) row=0;
 	if (col<0 || col>=a.Cols()) col=0;

   if (nrows+row > a.Rows()) nrows=a.Rows()-row;
   if (ncols+col > a.Cols()) ncols=a.Cols()-col;

   Init(nrows,ncols);

	for (long i=0;i<rows;i++)
		for (long j=0;j<cols;j++)
			SetElement(i,j,a.ElementAt(row+i,col+j));

   return *this;
}

CMMatrix& CMMatrix::SubMatrix(long row,long col,long nrows,long ncols)
{
	CMMatrix a(*this);
   return SubMatrix(a,row,col,nrows,ncols);
}

CMMatrix& CMMatrix::RemoveRows(CMMatrix& a,const CMVBigArray<long>& r)
{
   long i,j,k;

	if (r.Count()>a.Rows())
   	return *this;

	Init(a.Rows()-r.Count(),a.Cols());

	for (j=0;j<a.Cols();j++) {
      long row=0;
   	for (i=0;i<a.Rows();i++) {
			for (k=0;k<r.Count();k++)
         	if (r[k]==i)
            	break;
			if (k==r.Count())
         	ElementAt(row++,j) = a.ElementAt(i,j);
		}
   }
	return *this;
}


CMMatrix& CMMatrix::RemoveRows(const CMVBigArray<long>& r)
{
	CMMatrix a(*this);
   return RemoveRows(a,r);
}

CMMatrix& CMMatrix::RemoveCols(CMMatrix& a,const CMVBigArray<long>& c)
{
   long i,j,k;

	if (c.Count()>a.Cols())
   	return *this;

	Init(a.Rows(),a.Cols()-c.Count());

	for (i=0;i<a.Rows();i++) {
      long col=0;
   	for (j=0;j<a.Cols();j++) {
			for (k=0;k<c.Count();k++)
         	if (c[k]==j)
            	break;
			if (k==c.Count())
         	ElementAt(i,col++) = a.ElementAt(i,j);
		}
   }
	return *this;
}

CMMatrix& CMMatrix::RemoveCols(const CMVBigArray<long>& c)
{
 	CMMatrix a(*this);
   return RemoveCols(a,c);
}

/*
int CMMatrix::LU(const CMMatrix& a,CMMatrix& index)
{
	long i,imax,j,k;
	double big,dum,sum,temp;
	CMVBigArray<double> scale(0,100);

	if (a.rows != a.cols)
   	return 0;

	int ret = 1;

	rows=cols=a.rows;

	index.Init(rows,1);

	for (i=0;i<rows;i++) {
		big=0.0;
		for (j=0;j<cols;j++) {
      	double x = a.ElementAt(i,j);
         SetElement(i,j,x);
			if ((temp=fabs(x)) > big) big=temp;
		}
		cout << "big = " << big << ENDL;
		if (big == 0.0) // ERROR
      	return 0;
		scale.AddAt(i,1.0/big);
	}

	for (j=0;j<cols;j++) {
  		for (i=0;i<j;i++) {
			sum=ElementAt(i,j);
			for (k=0;k<i;k++) sum -= (ElementAt(i,k)*ElementAt(k,j));
			ElementAt(i,j)=sum;
		}
		big=0.0;
		for (i=j;i<rows;i++) {
			sum=ElementAt(i,j);
			for (k=0;k<j;k++)
				sum -= (ElementAt(i,k)*ElementAt(k,j));
			ElementAt(i,j)=sum;
			cout << "big = " << big << " dum = " << (scale[i]*fabs(sum)) << ENDL;
			if ((dum=(scale[i]*fabs(sum))) >= big) {
				big=dum;
            cout << "imax = " << i << ENDL;
				imax=i;
			}
		}
		if (j != imax) {
         ExchangeRows(imax,j);
			ret = -ret;
			scale[imax]=scale[j];
//			scale.Exchange(imax,j); //***
		}
		index.SetElement(j,0,imax);
//		if (ElementAt(j,j) == 0.0) ElementAt(j,j)=1.0e-20;
		if (j != (cols-1)) {
			dum=1.0/ElementAt(j,j);
			for (i=j+1;i<cols;i++) ElementAt(i,j) *= dum;
		}
	}
	return ret;
}

int CMMatrix::LUSolve(const CMMatrix& a,CMMatrix& b)
{
	Init();
	if ( (a.rows != a.cols) || (a.cols != b.rows))
   	return 0;

   CMMatrix lu,index;

   *this = b;

   long i,j,k,ii;
   double sum;

   lu.LU(a,index);

	for (j=0;j<cols;j++) {
   	ii=-1;
   	for (i=0;i<rows;i++) {
      	long ip = (long)index.ElementAt(i,0);
			sum = ElementAt(ip,j);
			SetElement(ip,j,ElementAt(i,j));
         if (ii>=0) {
         	for (k=ii;k<i;k++)
            	sum -= (lu.ElementAt(i,k)*ElementAt(k,j));
         }
         else if (sum != 0)
         	ii=i;
         SetElement(i,j,sum);
      }
      for (i=rows-1;i>=0;i--) {
      	sum = ElementAt(i,j);
        	for (k=i+i;k<rows;k++)
           	sum -= (lu.ElementAt(i,k)*ElementAt(k,j));
			SetElement(i,j,sum/lu.ElementAt(i,i));
      }
   }

   return 1;
}
*/



CMMatrix& CMMatrix::SVD(const CMMatrix& a,CMMatrix& v,CMMatrix& w)
{
	int  flag,its;
	long i,j,jj,k,l,nm;
	double anorm,c,f,g,h,s,scale,x,y,z;

   CMVBigArray<double> rv1;
	g=scale=anorm=0.0;

	*this = a;
   v.Init(cols,cols);
   w.Init(cols,1);

	for (i=0;i<cols;i++) {
		l=i+1;
		rv1.AddAt(i,scale*g);
		g=s=scale=0.0;
		if (i < rows) {
			for (k=i;k<rows;k++) scale += fabs(ElementAt(k,i));
			if (scale!=0.0) {
				for (k=i;k<rows;k++) {
					ElementAt(k,i) /= scale;
					s += (ElementAt(k,i)*ElementAt(k,i));
				}
				f=ElementAt(i,i);
				g = -cmsign(sqrt(s),f);
				h=f*g-s;
				ElementAt(i,i)=f-g;
				for (j=l;j<cols;j++) {
					for (s=0.0,k=i;k<rows;k++) s += ElementAt(k,i)*ElementAt(k,j);
					f=s/h;
					for (k=i;k<rows;k++) ElementAt(k,j) += (f*ElementAt(k,i));
				}
				for (k=i;k<rows;k++) ElementAt(k,i) *= scale;
			}
		}
		w.SetElement(i,0,scale*g);
		g=s=scale=0.0;
		if (i < rows && i != (cols-1)) {
			for (k=l;k<cols;k++) scale += fabs(ElementAt(i,k));
			if (scale!=0.0) {
				for (k=l;k<cols;k++) {
					ElementAt(i,k) /= scale;
					s += (ElementAt(i,k)*ElementAt(i,k));
				}
				f=ElementAt(i,l);
				g = -cmsign(sqrt(s),f);
				h=f*g-s;
				ElementAt(i,l)=f-g;
				for (k=l;k<cols;k++) rv1.AddAt(k,ElementAt(i,k)/h);
				for (j=l;j<rows;j++) {
					for (s=0.0,k=l;k<cols;k++) s += (ElementAt(j,k)*ElementAt(i,k));
					for (k=l;k<cols;k++) ElementAt(j,k) += s*rv1.At(k);
				}
				for (k=l;k<cols;k++) ElementAt(i,k) *= scale;
			}
		}
		anorm=cmmax(anorm,(fabs(w.ElementAt(i,0))+fabs(rv1[i])));
	}
	for (i=cols-1;i>=0;i--) {
		if (i < (cols-1)) {
			if (g) {
				for (j=l;j<cols;j++)
					v.ElementAt(j,i)=(ElementAt(i,j)/ElementAt(i,l))/g;
				for (j=l;j<cols;j++) {
					for (s=0.0,k=l;k<cols;k++) s += (ElementAt(i,k)*v.ElementAt(k,j));
					for (k=l;k<cols;k++) v.ElementAt(k,j) += (s*v.ElementAt(k,i));
				}
			}
			for (j=l;j<cols;j++) {
         	v.ElementAt(i,j)=0.0;
            v.ElementAt(j,i)=0.0;
         }
		}
		v.ElementAt(i,i)=1.0;
		g=rv1[i];
		l=i;
	}
	for (i=cmmin(rows,cols)-1;i>=0;i--) {
		l=i+1;
		g=w.ElementAt(i,0);
		for (j=l;j<cols;j++) ElementAt(i,j)=0.0;
		if (g) {
			g=1.0/g;
			for (j=l;j<cols;j++) {
				for (s=0.0,k=l;k<rows;k++) s += (ElementAt(k,i)*ElementAt(k,j));
				f=(s/ElementAt(i,i))*g;
				for (k=i;k<rows;k++) ElementAt(k,j) += (f*ElementAt(k,i));
			}
			for (j=i;j<rows;j++) ElementAt(j,i) *= g;
		} else for (j=i;j<rows;j++) ElementAt(j,i)=0.0;
		ElementAt(i,i)+=1;
	}
	for (k=cols-1;k>=0;k--) {
		for (its=0;its<30;its++) {
			flag=1;
			for (l=k;l>=0;l--) {
				nm=l-1;
				if ((fabs(rv1[l])+anorm) == anorm) {
					flag=0;
					break;
				}
				if ((fabs(w.ElementAt(nm,0))+anorm) == anorm) break;
			}
			if (flag) {
				c=0.0;
				s=1.0;
				for (i=l;i<=k;i++) {
					f=s*rv1[i];
					rv1.AddAt(i,c*rv1[i]);
					if ((fabs(f)+anorm) == anorm) break;
					g=w.ElementAt(i,0);
					h=pythag(f,g);
					w.SetElement(i,0,h);
					h=1.0/h;
					c=g*h;
					s = -f*h;
					for (j=0;j<rows;j++) {
						y=ElementAt(j,nm);
						z=ElementAt(j,i);
						ElementAt(j,nm)=y*c+z*s;
						ElementAt(j,i)=z*c-y*s;
					}
				}
			}
			z=w.ElementAt(k,0);
			if (l == k) {
				if (z < 0.0) {
					w.SetElement(k,0,-z);
					for (j=0;j<cols;j++) v.ElementAt(j,k) = -v.ElementAt(j,k);
				}
				break;
			}
			if (its == 29) {
//         	cout << "no convergence in 30 svdcmp iterations" << ENDL;
            return *this;
         }
			x=w.ElementAt(l,0);
			nm=k-1;
			y=w.ElementAt(nm,0);
			g=rv1[nm];
			h=rv1[k];
			f=((y-z)*(y+z)+(g-h)*(g+h))/(2.0*h*y);
			g=pythag(f,1.0);
			f=((x-z)*(x+z)+h*((y/(f+cmsign(g,f)))-h))/x;
			c=s=1.0;
			for (j=l;j<=nm;j++) {
				i=j+1;
				g=rv1[i];
				y=w.ElementAt(i,0);
				h=s*g;
				g=c*g;
				z=pythag(f,h);
				rv1.AddAt(j,z);
				c=f/z;
				s=h/z;
				f=x*c+g*s;
				g = g*c-x*s;
				h=y*s;
				y *= c;
				for (jj=0;jj<cols;jj++) {
					x=v.ElementAt(jj,j);
					z=v.ElementAt(jj,i);
					v.ElementAt(jj,j)=x*c+z*s;
					v.ElementAt(jj,i)=z*c-x*s;
				}
				z=pythag(f,h);
				w.SetElement(j,0,z);
				if (z!=0) {
					z=1.0/z;
					c=f*z;
					s=h*z;
				}
				f=c*g+s*y;
				x=c*y-s*g;
				for (jj=0;jj<rows;jj++) {
					y=ElementAt(jj,j);
					z=ElementAt(jj,i);
					ElementAt(jj,j)=y*c+z*s;
					ElementAt(jj,i)=z*c-y*s;
				}
			}
			rv1.AddAt(l,0.0);
			rv1.AddAt(k,f);
			w.SetElement(k,0,x);
		}
	}
   return *this;
}

/*
CMMatrix& CMMatrix::SVDSolve(const CMMatrix& a,CMMatrix& b,CMMatrix& w)
{
   long i,j,jj;
   double s;
	CMMatrix v;
	CMMatrix svd;
   CMVBigArray<double> tmp;

	if (!svd.SVD(a,w,v))
   	return 0;

	Init(w.Rows(),1);

	double wmax = 0;
   for (j=0;j<w.Rows();j++) if (w.ElementAt(j,0) >wmax) 	wmax=w.ElementAt(j,0);
   double wmin=wmax*1.0e-8;
   for (j=0;j<w.Rows();j++) if (w.ElementAt(j,0)<wmin) 	w.ElementAt(j,0)=0.0;

	for (j=0;j<rows;j++) {
		s=0.0;
		if (w.ElementAt(j,0) != 0) {
			for (i=0;i<b.Rows();i++) s += (svd.ElementAt(i,j)*b.ElementAt(i,0));
			s /= w.ElementAt(j,0);
		}
		tmp.AddAt(j,s);
	}
	for (j=0;j<rows;j++) {
		s=0.0;
		for (jj=0;jj<rows;jj++) s += (v.ElementAt(j,jj)*tmp[jj]);
		ElementAt(j,0) = s;
	}
	return 1;
}
*/

CMMatrix& CMMatrix::SVDSolve(const CMMatrix& a,const CMMatrix& b,CMMatrix& w)
{
   long i,ii,j;
   double s;
	CMMatrix v;
	CMMatrix svd;
   CMVBigArray<double> tmp;

	svd.SVD(a,w,v);

	Init(w.Rows(),b.Cols());

   // Set weights to 0 if below threshold

	double wmax = 0;
   for (i=0;i<w.Rows();i++) if (w.ElementAt(i,0) >wmax) 	wmax=w.ElementAt(i,0);
   double wmin=wmax*1.0e-8;
   for (i=0;i<w.Rows();i++) if (w.ElementAt(i,0)<wmin) 	w.ElementAt(i,0)=0.0;

	for (j=0;j<cols;j++) {
		for (i=0;i<rows;i++) {
			s=0.0;
			if (w.ElementAt(i,0) != 0) {
				for (ii=0;ii<b.Rows();ii++) s += (svd.ElementAt(ii,i)*b.ElementAt(ii,j));
				s /= w.ElementAt(i,0);
			}
			tmp.AddAt(i,s);
		}

		for (i=0;i<rows;i++) {
			s=0.0;
			for (ii=0;ii<rows;ii++) s += (v.ElementAt(i,ii)*tmp[ii]);
			ElementAt(i,j) = s;
		}
   }

	return *this;
}

CMMatrix& CMMatrix::LeastSquares(const CMMatrix& a,const CMMatrix& b,CMMatrix& cov,double& mse,long& df)
{
	CMMatrix u,v,w;
   double sum,pred;
   long i,j,k;

   Init(a.Cols(),1);
   cov.Init(a.Cols(),a.Cols());

   if (b.Cols()!=1 || (a.Rows() != b.Rows()))
   	return *this;

   u.SVD(a,v,w);

	CMMatrix weight(w.Rows(),w.Rows());
   for (i=0;i<weight.Rows();i++)
   	weight.ElementAt(i,i) = w.ElementAt(i,0);

   CMMatrix test(u),test2(v);
   test.Multiply(weight);
   test.Multiply(test2.Transpose());

	double wmax = 0;
   for (i=0;i<w.Rows();i++) if (w.ElementAt(i,0) >wmax) 	wmax=w.ElementAt(i,0);
   double wmin=wmax*CM_SMALL;
   for (i=0;i<w.Rows();i++) if (w.ElementAt(i,0)<wmin)  w.ElementAt(i,0)=0.0;

	CMMatrix tmp(rows,1);

	for (i=0;i<rows;i++) {
		double wgt = w.ElementAt(i,0);
		if (wgt != 0) {
	   	for (sum=0,j=0;j<u.Rows();j++)
         	sum += u.ElementAt(j,i)*b.ElementAt(j,0);
         tmp.SetElement(i,0,sum/wgt);
			wgt = 1/(wgt*wgt);
      }
      w.SetElement(i,0,wgt);
   }

	for (i=0;i<rows;i++)
	   for (long j=0;j<v.Cols();j++)
      	ElementAt(i,0) += tmp.ElementAt(j,0)*v.ElementAt(i,j);

	df = a.Rows()-a.Cols();
   mse = 0;

   for (i=0;i<b.Rows();i++) {
		for (pred=0,j=0;j<Rows();j++)
      	pred += ElementAt(j,0)*a.ElementAt(i,j);
      double resid = (b.ElementAt(i,0) - pred);
      mse += resid*resid;
   }

   mse /= df;

   for (i=0;i<cov.Rows();i++) {
	   for (long j=0;j<=i;j++) {
			for(sum=0,k=0;k<cov.Cols();k++)
         	sum += (v.ElementAt(i,k)*v.ElementAt(j,k)*w.ElementAt(k,0));
			cov.ElementAt(j,i) = cov.ElementAt(i,j) = sum*mse;
      }
   }

   return *this;
}

ostream& operator << (ostream& s,const CMMatrix& m)
{
	for (long i=0;i<m.rows;i++) {
		for (long j=0;j<m.cols;j++)
			s << m.ElementAt(i,j) << " ";
      s << ENDL;
   }
   return s;
}
