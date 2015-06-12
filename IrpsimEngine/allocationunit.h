// allocationunit.h : header file
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
#pragma once

#include "irp.h"
#include "irpobject.h"
#include "string.h"

class _IRPCLASS CMAllocationUnit : public CMIrpObject
{
protected:
	int allocation_type;
public:
	int allocation_mode;
	double sort_value;
	double numerator;      // storage level or remaining capacity
	double denominator;    // maximum take or maximum put
	double ability;        // ability to add or remove water
	double allocamount;    // amt to add or or remove
	int    include;        // flag used during processing
	
	virtual int is_less_than(const CMIrpObject& o2) const;

	double allocate(double val);
	void init_process(int operation,int rule,int atype);
	int  pre_process();
	int  process(double val,double num,double denom);
	void post_process(int reduceflag);

	virtual double full_put() = 0;
	virtual double full_take() = 0;
	virtual double get_rank() = 0;

public:
	enum {aDemand=1,aSupply,aTransfer,aStorage};
	enum {None,Preserve,ByRank,BalancePercent,BalanceAmount,BalanceTake,BalancePut,
			BalancePercentCapacity};
	enum {PutOperation,TakeOperation,CutBackOperation,AddBackOperation,
			EvaluateOperation};

	CMAllocationUnit(int id=-1);
	CMAllocationUnit(const string& aName,int id=-1);
	int GetType() {return allocation_type;}

	virtual double Capacity() = 0;
	virtual double RemainingCapacity() =0;
	virtual double Resolved() = 0;
	virtual double EvaluateAbility() = 0;
	virtual double PutAbility() = 0;
	virtual double TakeAbility() = 0;
	virtual double CutAbility() = 0;

	virtual double Evaluate(double limit) = 0;
	virtual double CutBack(double aVal) = 0;
	virtual double Put(double aVal,int isshift) = 0;
	virtual double Take(double aVal,int isshift) = 0;
	virtual void Gain() = 0;
	virtual void Loss() = 0;

	static int TranslateAllocationRule(const string& rulestr);
	static string TranslateAllocationRule(int ruleid);
};

