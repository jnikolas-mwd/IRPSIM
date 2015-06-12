// tmarray.cpp : implementation file
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
#include "StdAfx.h"
#include "tmarray.h"
#include "cmdefs.h"

CMTimedArray::CMTimedArray() :
beg(),
end(),
array(10,10),
incunits(CM_MONTH),
inclength(0)
{
}

CMTimedArray::CMTimedArray(const CMTime& b,const CMTime& e,CMTIMEUNIT units,int length) :
beg(b),
end(e),
incunits(units),
inclength(length),
array(1+CMTime::Diff(e,b,units,length))
{
	Reset();
}

void CMTimedArray::Reset()
{
	for (long i=0;i<array.Count();i++)
		array.AddAt(i,CM_NODOUBLE);
}

int CMTimedArray::AddAt(const CMTime& t,double val)
{
	if (t<beg)
		return 0;
	if (t>end) end=t;
	return array.AddAt(CMTime::Diff(t,beg,incunits,inclength),val);
}

int CMTimedArray::ContainsAt(const CMTime& t)
{
	if (t<beg || t>end)
		return -1;
	return array.At(CMTime::Diff(t,beg,incunits,inclength)) == CM_NODOUBLE ? 0 : 1;
}

