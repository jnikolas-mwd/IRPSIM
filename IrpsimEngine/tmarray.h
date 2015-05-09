// tmarray.h : header file
//
// Copyright © 1998-2015 Casey McSpadden   
//		mailto:casey@crossriver.com
//		http://www.crossriver.com/
//
// ==========================================================================  
// DESCRIPTION:	
// ==========================================================================
// CMTimedArray implements an array that can be accessed using CMTime indexes
// ==========================================================================
//
// ==========================================================================  
// HISTORY:	
// ==========================================================================
//			1.00	09 March 2015	- Initial re-write and release.
// ==========================================================================
//
/////////////////////////////////////////////////////////////////////////////
#if !defined (__TMARRAY_H)
#define __TMARRAY_H


#include "irp.h"
#include "bgvarray.h"
#include "cmtime.h"
#include "cmdefs.h"


class _IRPCLASS CMTimedArray
{
	CMTime beg;
	CMTime end;
	CMTIMEUNIT incunits;
	int inclength;
	class _IRPCLASS CMVBigArray<double> array;
public:
	CMTimedArray();
	CMTimedArray(const CMTime& b,const CMTime& e,CMTIMEUNIT units,int length);
	void Reset();
	CMTime Begin() {return beg;}
	CMTime End() {return end;}
	int AddAt(const CMTime& t,double val);
	int ContainsAt(const CMTime& t);
	double At(const CMTime& t) const {return array.At(CMTime::Diff(t,beg,incunits,inclength));}
	double operator [] (const CMTime& t) const {return At(t);}
};

#endif
