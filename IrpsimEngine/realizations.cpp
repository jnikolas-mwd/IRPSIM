// realizations.cpp : implementation file
//
// Copyright © 1998-2015 Casey McSpadden   
//		mailto:casey@crossriver.com
//		http://www.crossriver.com/
//
// ==========================================================================  
// DESCRIPTION:	
// ==========================================================================
// CMRealizations implements a repository of time-indexed store of variable
// realizations
// ==========================================================================
//
// ==========================================================================  
// HISTORY:	
// ==========================================================================
//			1.00	09 March 2015	- Initial re-write and release.
// ==========================================================================
//
/////////////////////////////////////////////////////////////////////////////
#include "realizations.h"
#include <fstream>

//static wofstream sdebug("debug_realizations.txt", ios::binary);

void CMRealizations::Reset()
{
	dict.clear();
}

bool CMRealizations::ContainsAt(const CMTime&t)
{
	return dict.find((ULONG)t) == dict.end() ? false : true;
}

double CMRealizations::At(const CMTime& t)
{
	return ContainsAt(t) ? dict.at((ULONG)t) : 0.0;
}

bool CMRealizations::SetValue(const CMTime& t, double v, bool bAccumulate)
{
	double exist = At(t);
	if (bAccumulate)
		v += exist;
	dict[(ULONG)t] = v;
	return true;
}

