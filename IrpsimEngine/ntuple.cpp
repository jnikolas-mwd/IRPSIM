// ntuple.cpp : implementation file
//
// Copyright © 1998-2015 Casey McSpadden   
//		mailto:casey@crossriver.com
//		http://www.crossriver.com/
//
// ==========================================================================  
// DESCRIPTION:	
// ==========================================================================
// Class CMNtuple class implements an N-tuple of doubles
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
#include "ntuple.h"

unsigned CMNTuple::sortindex = 0;

CMNTuple::CMNTuple() :
siz(0),
vals(0)
{
}

CMNTuple::CMNTuple(unsigned sz) :
siz(sz)
{
	vals = new double[siz];
}

CMNTuple::CMNTuple(const CMNTuple& nt) :
siz(nt.siz)
{
	vals = new double[siz];
	for (unsigned i=0;i<siz;i++)
		vals[i] = nt.vals[i];
}

CMNTuple::~CMNTuple()
{
	delete [] vals;
}

CMNTuple& CMNTuple::operator = (const CMNTuple& nt)
{
	if (vals) delete [] vals;
	siz = nt.siz;
	vals = new double[siz];
	for (unsigned i=0;i<siz;i++)
		vals[i] = nt.vals[i];
	return *this;
}

int CMNTuple::operator == (const CMNTuple& nt) const
{
	if (Size() != nt.Size())
		return 0;
	for (unsigned i=0;i<Size();i++)
		if (vals[i] != nt.vals[i])
			return 0;
	return 1;
}

int CMNTuple::operator < (const CMNTuple& nt) const
{
	if (Size() < nt.Size())
		return 1;
	if (Size() > nt.Size())
		return 0;
	if (sortindex>=Size())
		sortindex=0;
	return vals[sortindex] < nt.vals[sortindex];
}

ostream& operator << (ostream& s,CMNTuple& nt)
{
	for (unsigned i=0;i<nt.Size();i++)
		s << nt[i] << "  ";
	return s;
}


