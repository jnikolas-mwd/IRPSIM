// allocationunit.cpp : implementation file
//
// Copyright © 1998-2015 Casey McSpadden   
//		mailto:casey@crossriver.com
//		http://www.crossriver.com/
//
// ==========================================================================  
// DESCRIPTION:	
// ==========================================================================
// CMAllocationUnit implements a member of a set of devices (generally
// storage devices) used during balancing (e.g. balance put) operations
// ==========================================================================
//
// ==========================================================================  
// HISTORY:	
// ==========================================================================
//			1.00	09 March 2015	- Initial re-write and release.
// ==========================================================================
//
/////////////////////////////////////////////////////////////////////////////
#include "allocationunit.h"
#include "cmdefs.h"
#include "cmlib.h"
#include <math.h>


CMAllocationUnit::CMAllocationUnit(const CMString& aName,int id) : CMIrpObject(aName,id),
allocation_type(0),
allocation_mode(Preserve),
sort_value(CM_BIGDOUBLE),
numerator(0),
denominator(1),
ability(0),
allocamount(0),
include(0)
{
}

CMAllocationUnit::CMAllocationUnit(const wchar_t* aName,int id) : CMIrpObject(aName,id),
allocation_type(0),
allocation_mode(Preserve),
sort_value(CM_BIGDOUBLE),
numerator(0),
denominator(1),
ability(0),
allocamount(0),
include(0)
{
}

int CMAllocationUnit::is_less_than(const CMIrpObject& o2) const
{
	return (sort_value < ((CMAllocationUnit&)o2).sort_value);
}

double CMAllocationUnit::allocate(double val)
{
	allocamount = (fabs(ability) < fabs(val)) ? ability : val;
	return allocamount;
}

void CMAllocationUnit::init_process(int operation,int rule,int atype)
{
	// if the process is initializing for a particular category and this node is not a member,
	// or if the allocation mode is set to "preserve", set ability to 0
	if (atype && !allocation_type) allocation_type = atype;
	numerator=denominator=1;
  	ability=0;
	allocation_mode=rule;
	allocamount=0;
	double variablevalue = Resolved();
  	switch (allocation_mode) {
		case BalancePercent:	numerator=1;denominator=variablevalue;break;
		case BalancePut:		numerator=RemainingCapacity();denominator=full_put();break;
		case BalanceTake: 	numerator=variablevalue;denominator=full_take();break;
		case BalancePercentCapacity: numerator=variablevalue;denominator=Capacity();break;
	}
	if (denominator==0) denominator=0.000001;
	switch (operation) {
   	case EvaluateOperation: ability = EvaluateAbility(); break;
		case CutBackOperation:	ability = -CutAbility(); break;
		case AddBackOperation: 	ability = RemainingCapacity();
		case PutOperation: ability = PutAbility(); break;
		case TakeOperation: ability = -TakeAbility(); break;
	}
	if (allocation_mode==Preserve)
   	ability = 0;
	sort_value = (allocation_mode==ByRank) ? get_rank() : CM_BIGDOUBLE;
}

int CMAllocationUnit::pre_process()
{
	return (include = (allocamount==0&&ability!=0) ? 1 : 0);
}

int CMAllocationUnit::process(double val,double n,double d)
{
	if (!include)
		return 0;
	if (allocation_mode==BalancePut)
		allocamount = (denominator*(val-n+numerator)+numerator*(d-denominator))/d;
	else if (allocation_mode==BalanceTake||allocation_mode==BalancePercentCapacity)
		allocamount = (denominator*(val+n-numerator)-numerator*(d-denominator))/d;
	else if (allocation_mode==BalancePercent)
		allocamount = denominator*val/d;
	else
		allocamount = val/d;
	if (sign(allocamount)!=sign(val)) {
   	include = -1;
		allocamount=0;
	}
	else if (fabs(allocamount)>fabs(ability)) {
		allocamount = ability;
		include = 2;
	}
   else					// bug fix: added this else clause 11/17/97
   	include = 1;
	return include;
}

void CMAllocationUnit::post_process(int reduceflag)
{
	if (include < 0) include = 0;
	else if (include==1 && reduceflag)
		allocamount = 0;
}

CMString CMAllocationUnit::TranslateAllocationRule(int ruleid)
{
	CMString ret = L"preserve";
   switch (ruleid) {
		case ByRank: ret = L"byrank"; break;
   	case BalancePercent: ret = L"balancepercent"; break;
   	case BalanceAmount: ret = L"balanceamount"; break;
   	case BalanceTake: ret = L"balancetake"; break;
   	case BalancePut: ret = L"balanceput"; break;
   	case BalancePercentCapacity: ret = L"balancepercentcapacity"; break;
   }
   return ret;
}

int CMAllocationUnit::TranslateAllocationRule(const CMString& modename)
{
	if 	  (modename.contains(L"rank"))				return ByRank;
	else if (modename.contains(L"percent"))			return BalancePercent;
	else if (modename.contains(L"amount"))			return BalanceAmount;
	else if (modename.contains(L"take"))				return BalanceTake;
	else if (modename.contains(L"put"))				return BalancePut;
	else if (modename.contains(L"capacity"))  		return BalancePercentCapacity;
	return Preserve;
}
