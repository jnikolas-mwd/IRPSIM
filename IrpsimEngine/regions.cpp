// regions.cpp : implementation file
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
#include "StdAfx.h"
#include "regions.h"

CMVSmallArray<CMString> CMRegions::regions;

int CMRegions::GetRegionId(const CMString& name)
{
	unsigned short loc;
	if (name.is_null())
   	return -1;
   else if (regions.Contains(name,&loc))
   	return (int)loc;
	regions.Add(name);
   return regions.Count()-1;
}

CMString CMRegions::GetRegionName(int id)
{
	if (id>=0 && id<regions.Count())
   	return regions.At((unsigned short)id);
   return CMString();
}

