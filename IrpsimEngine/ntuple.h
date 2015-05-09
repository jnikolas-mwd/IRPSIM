// ntuple.h : header file
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
#if !defined (__NTUPLE_H)
#define __NTUPLE_H

#include "irp.h"
#include "cmdefs.h"
#include <iostream>

using namespace std;

class _IRPCLASS CMNTuple
{
	static unsigned sortindex;
	unsigned siz;
	double* vals;
public:
	CMNTuple();
	CMNTuple(unsigned sz);
	CMNTuple(const CMNTuple& nt);
	~CMNTuple();
	unsigned Size() const {return siz;}
	CMNTuple& operator = (const CMNTuple& nt);
	double& At(unsigned n) {return vals[n];}
	double& operator [](unsigned n) {return vals[n];}
	int operator == (const CMNTuple& nt) const;
	int operator < (const CMNTuple& nt) const;
	static void SetSortIndex(unsigned n) {sortindex=n;}
   ostream& operator << (ostream& s) const;
};

_IRPFUNC ostream& operator << (ostream& s, CMNTuple& nt);

#endif
