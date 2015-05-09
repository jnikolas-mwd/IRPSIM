// realizations.h : header file
//
// Copyright © 1998-2015 Casey McSpadden   
//		mailto:casey@crossriver.com
//		http://www.crossriver.com/
//
// ==========================================================================  
// DESCRIPTION:	
// ==========================================================================
// CMRealizations implements a time-indexed store of variable realizations
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

#include <map>
#include "irp.h"
#include "cmtime.h"

class _IRPCLASS CMRealizations
{
	//class _IRPCLASS map<CMTime, double> dict;
	class _IRPCLASS map<ULONG, double> dict;
public:
	CMRealizations() {};
	~CMRealizations() {};
	//	static void SetIncrement(int units,int length) {incunits=units;inclength=length;}
	void Reset();
	bool ContainsAt(const CMTime&t);
	double At(const CMTime& t);
	bool SetValue(const CMTime& t, double v, bool bAccumulate);
};
