// regions.h : header file
//
// Copyright © 1998-2015 Casey McSpadden   
//		mailto:casey@crossriver.com
//		http://www.crossriver.com/
//
// ==========================================================================  
// DESCRIPTION:	
// ==========================================================================
// CMRegions implements a collection of geographical regions that a node may
// belong to.
// ==========================================================================
//
// ==========================================================================  
// HISTORY:	
// ==========================================================================
//			1.00	09 March 2015	- Initial re-write and release.
// ==========================================================================
//
/////////////////////////////////////////////////////////////////////////////
#if !defined (__CMREGIONS_H)
#define __CMREGIONS_H

#include "irp.h"

#include "smvarray.h"
#include "string.h"

class _IRPCLASS CMRegions
{
	static class _IRPCLASS CMVSmallArray<string> regions;
public:
	static void Reset() {regions.Reset(1);}
	static int GetRegionId(const string& name);
	static string GetRegionName(int id);
    static unsigned RegionCount() {return regions.Count();}
};

#endif

