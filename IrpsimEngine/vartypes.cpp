// vartypes.cpp : implementation file
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
#include "StdAfx.h"
#include "vartypes.h"

#include "cmlib.h"

const CMVariableTypes::_types CMVariableTypes::vartypes[] = {
{tDemand,L"demand",1},
{tSupply,L"supply",1},
{tTransfer,L"transfer",1},
{tStorage,L"storage",0},
{tPut,L"put",1},
{tTake,L"take",1},
{tRank,L"rank",0},
{tCapacity,L"capacity",0},
{tFloor,L"floor",0},
{tCount,L"count",0},
{tCutAccum,L"cutaccum",0},
{tFullPut,L"fullput",1},
{tFullTake,L"fulltake",1},
{tInLieu,L"inlieu",1},
{tCutBack,L"cutback",1},
{tSeasonalShift,L"seasonalshift",1},
{tCarryOver,L"carryover",0},
{tTrigger,L"trigger",0},
{tExpire,L"expire",0},
{tAnnualLimit,L"annuallimit",0},
{tConveyance,L"conveyance",0},
{tAllocation,L"allocation",0},
{tGain,L"gain",1},
{tLoss,L"loss",1},
{tStorageShift,L"storageshift",1},
{tAge,L"age",0},
{tOverDraft,L"overdraft",0},
{tFlowTo,L"flowto",0},
{tCost,L"cost",1},
{tNetCut,L"netcut",1},
{tGrossCut,L"grosscut",1},
{tCategory,L"category",0},
{tShiftOrder,L"shiftorder",0},
{tConstraint,L"constraint",0},
{tMaxDemand,L"maxdemand",1},
{tMaxSupply,L"maxsupply",1},
{tMaxSupplyCut,L"maxsupplycut",1},
{tMaxDemandCut,L"maxdemandcut",1},
{tMaxPut,L"maxput",1},
{tMaxTake,L"maxtake",1},
{tMaxGain,L"maxgain",1},
{tMaxLoss,L"maxloss",1},
{tLimit,L"limit",1},
{-1,0,0}
};

const CMVariableTypes::_types CMVariableTypes::aggnames[] = {
{aDemand,L"_demand",1},
{aSupply,L"_supply",1},
{aTransfer,L"_transfers",1},
{aStorage,L"_storage",0},
{aSurplus,L"_surplus",1},
{aNetSurplus,L"_netsurplus",1},
{aPut,L"_put",1},
{aTake,L"_take",1},
{aPutTake,L"_puttake",1},
{aGainLoss,L"_gainloss",1},
{aPopulation,L"_population",0},
{aInLieu,L"_inlieu",1},
{aDemandCut,L"_demandcut",1},
{aSupplyCut,L"_supplycut",1},
{aTransferCut,L"_transfercut",1},
{aWasted,L"_wasted",1},
{aSeasonalShift,L"_seasonalshift",1},
{aCarryOver,L"_carryover",0},
{aStorageShift,L"_storageshift",1},
{aCost,L"_cost",1},
{-1,0,0}
};

CMVariableTypes::CMVariableTypes()
{
}

const wchar_t* CMVariableTypes::VarStringFromInt(int aInt, int)
{
	for (int j=0;vartypes[j].i>=0;j++)
		if (aInt==vartypes[j].i)
			return vartypes[j].c;
	return 0;
}

int CMVariableTypes::VarIntFromString(const wchar_t* aName, int)
{
	for (int j=0;vartypes[j].i>=0;j++)
		if (!_wcsicmp(vartypes[j].c,aName))
			return vartypes[j].i;
	return -1;
}

int CMVariableTypes::IsVarSum(int aInt)
{
	for (int j=0;vartypes[j].i>=0;j++)
		if (aInt==vartypes[j].i)
			return vartypes[j].s;
	return 0;
}

const wchar_t* CMVariableTypes::AggStringFromInt(int aInt)
{
	for (int j=0;aggnames[j].i>=0;j++)
		if (aInt==aggnames[j].i)
			return aggnames[j].c;
	return 0;
}

int CMVariableTypes::AggIntFromString(const wchar_t* aName)
{
	for (int j=0;aggnames[j].i>=0;j++)
		if (!_wcsicmp(aggnames[j].c, aName))
			return aggnames[j].i;
	return -1;
}

unsigned short CMVariableTypes::VarTypesCount()
{
	unsigned j;
	for (j=0;vartypes[j].i>=0;j++);
	return j;
}

unsigned short CMVariableTypes::AggNamesCount()
{
	int j;
	for (j=0;aggnames[j].i>=0;j++);
   return j;
}

int CMVariableTypes::IsAggSum(int aInt)
{
	for (int j=0;aggnames[j].i>=0;j++)
		if (aInt==aggnames[j].i)
			return aggnames[j].s;
	return 0;
}

