// vartypes.h : header file
//
// Copyright © 1998-2015 Casey McSpadden   
//		mailto:casey@crossriver.com
//		http://www.crossriver.com/
//
// ==========================================================================  
// DESCRIPTION:	
// ==========================================================================
// CMVariableTypes is a class for managing IRPSIM variable types.
// ==========================================================================
//
// ==========================================================================  
// HISTORY:	
// ==========================================================================
//			1.00	09 March 2015	- Initial re-write and release.
// ==========================================================================
#pragma once

#include "irp.h"

#include "cmdefs.h"

class _IRPCLASS CMVariableTypes
{
	// integer identifier, name of type, SUM?
	struct _types {int i;const wchar_t* c;int s;};
	static const _types vartypes[];
	static const _types aggnames[];
public:
	/*
	enum _vartypes {
		tDemand = 100, tSupply = 200, tTransfer = 300, tStorage = 400, tPut = 500, tTake = 600,
		tRank = 700, tCapacity = 800, tFloor = 900, tCount = 1000, tCutAccum = 1100, tFullPut = 1200, tFullTake = 1300,
		tCutBack = 1500, tTrigger = 1800, tExpire = 1900,
		tAnnualLimit = 2000, tConveyance = 2100, tAllocation = 2200, tGain = 2300, tLoss = 2400, tStorageShift = 2500,
		tAge = 2600, tOverDraft = 2700, tFlowTo = 2800, tCost = 2900, tNetCut = 3000, tGrossCut = 3100, tCategory = 3200, tShiftOrder = 3300,
		tConstraint = 3400, tMaxDemand = 3500, tMaxSupply = 3600, tMaxSupplyCut = 3700, tMaxDemandCut = 3800,
		tMaxPut = 3900, tMaxTake = 4000, tMaxGain = 4100, tMaxLoss = 4200, tLimit = 4300
	};
	*/

	enum _vartypes {
		tDemand = 100, tSupply = 200, tTransfer = 300, tStorage = 400, tPut = 500, tTake = 600,
		tRank = 700, tCapacity = 800, tFloor = 900, tCount = 1000, tCutAccum = 1100, tFullPut = 1200, tFullTake = 1300,
		tInLieu = 1400, tCutBack = 1500, tSeasonalShift = 1600, tCarryOver = 1700, tTrigger = 1800, tExpire = 1900,
		tAnnualLimit = 2000, tConveyance = 2100, tAllocation = 2200, tGain = 2300, tLoss = 2400, tStorageShift = 2500,
		tAge = 2600, tOverDraft = 2700, tFlowTo = 2800, tCost = 2900, tNetCut = 3000, tGrossCut = 3100, tCategory = 3200, tShiftOrder = 3300,
		tConstraint = 3400, tMaxDemand = 3500, tMaxSupply = 3600, tMaxSupplyCut = 3700, tMaxDemandCut = 3800,
		tMaxPut = 3900, tMaxTake = 4000, tMaxGain = 4100, tMaxLoss = 4200, tLimit = 4300
	};

	enum _aggregatenames {
		aDemand, aSupply, aStorage, aSurplus,
		aNetSurplus, aPut, aTake, aPutTake, aGainLoss, aDemandCut, aSupplyCut, aStorageShift, aCost
	};
	/*
	enum _vartypes {tDemand=100,tSupply=200,tTransfer=300,tStorage=400,tPut=500,tTake=600,
		tRank=700,tCapacity=800,tFloor=900,tCount=1000,tCutAccum=1100,tFullPut=1200,tFullTake=1300,
		tInLieu=1400,tCutBack=1500,tSeasonalShift=1600,tCarryOver=1700,tTrigger=1800,tExpire=1900,
		tAnnualLimit=2000,tConveyance=2100,tAllocation=2200,tGain=2300,tLoss=2400,tStorageShift=2500,
		tAge=2600,tOverDraft=2700,tFlowTo=2800,tCost=2900,tNetCut=3000,tGrossCut=3100,tCategory=3200,tShiftOrder=3300,
      tConstraint=3400,tMaxDemand=3500,tMaxSupply=3600,tMaxSupplyCut=3700,tMaxDemandCut=3800,
      tMaxPut=3900,tMaxTake=4000,tMaxGain=4100,tMaxLoss=4200,tLimit=4300};

	enum _aggregatenames {aDemand,aSupply,aTransfer,aStorage,aSurplus,
		aNetSurplus,aPut,aTake,aPutTake,aGainLoss,aPopulation,aInLieu,aDemandCut,aSupplyCut,
      aTransferCut,aWasted,aSeasonalShift,aCarryOver,aStorageShift,aCost};
	*/

	CMVariableTypes();

	static const wchar_t* VarStringFromInt(int aInt,int test);
	static int VarIntFromString(const wchar_t* aName, int test);
	static unsigned short VarTypesCount();
	static int IsVarSum(int aInt);

	static const wchar_t* AggStringFromInt(int aInt);
	static int AggIntFromString(const wchar_t* aName);
	static unsigned short AggNamesCount();
	static int IsAggSum(int aInt);
};
