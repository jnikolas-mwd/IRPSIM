// category.h : header file
//
// Copyright © 1998-2015 Casey McSpadden   
//		mailto:casey@crossriver.com
//		http://www.crossriver.com/
//
// ==========================================================================  
// DESCRIPTION:	
// ==========================================================================
// Implements IRPSIM CMCategory class.
// Categories allow you to establish different classes of demand, supply, 
// transfers, storage. Categories have a name and an integer value that 
// determines how quantities are to be balanced or allocated within the
// category.For example, storage may be set to "tier1", "tier2", etc.with a 
// different balancing rule for each tier.
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

#include "variable.h"
#include "varconst.h"
#include "node.h"
#include "irpobject.h"
#include "allocationunit.h"

#include "cmdefs.h"
#include "smparray.h"
#include "smvarray.h"

/*
	Categories allow you to establish different classes of demand, supply, transfers, storage
	Categories have a name and an integer value that determines how quantities are to be
	balanced or allocated within the category. For example, storage may be set to "tier1",
	"tier2", etc. with a different balancing rule for each tier.
*/

class _IRPCLASS CMTimeMachine;
class _IRPCLASS CMCategory;

class _IRPCLASS CMCategory : public CMAllocationUnit
{
	static CMTimeMachine* time;
   static CMVariable* vnetsurplus;
   static wchar_t* header;
   static wchar_t* footer;
	static class _IRPCLASS CMPSmallArray<CMCategory> categories;

	class _IRPCLASS CMVSmallArray<string> member_names;
	class _IRPCLASS CMPSSmallArray<CMAllocationUnit> member_units;
	
	int default_allocation_mode,state;
	string strPutLimit,strTakeLimit,strCutLimit;
	CMVariableOrConstant vcPutLimit,vcTakeLimit,vcCutLimit;

   virtual wistream& read(wistream& s);
   virtual wostream& write(wostream& s);
   virtual const wchar_t* IsA() { return L"CMCategory"; };

	void reset();

	double alloc_ability();
	double amount_allocated();
	void InitProcess(int operation,int rul,int atype);  
	int PreProcess();
	int reset_required();
	void PostProcess(int reduceflag);
	int Process(double val,double n,double d);
	double ProcessAllocation(int operation,double amt,int rul,int atype=0); // last parameter is allocation type

	virtual double full_put();
	virtual double full_take();
	virtual double get_rank();

   enum {csFail=0x0001};
public:
	CMCategory(int id = -1);
	~CMCategory();

	int Fail() const {return (state&csFail);}

   int  ContainsMember(const string& name);

	double Evaluate(double limit,int rule);
	double CutBack(double limit,int rule,int ignorebalance,int nodetype);
	double Put(double limit,int rule,int ignorebalance,int isshift);
	double Take(double limit,int rule,int ignorebalance,int isshift);

	int Initialize();

	virtual double EvaluateAbility();
	virtual double PutAbility();
	virtual double TakeAbility();
	virtual double CutAbility();
	virtual void Gain();
	virtual void Loss();
	virtual double Capacity();
	virtual double RemainingCapacity();
	virtual double Resolved();
	virtual double Evaluate(double limit);
	virtual double CutBack(double aVal);
	virtual double Put(double aVal,int isshift);
	virtual double Take(double aVal,int isshift);

   static void InitTimeStep(CMTimeMachine* t);
	static void DestroyCategories() 	{categories.ResetAndDestroy(1);}
   static unsigned short CategoryCount() {return categories.Count();}
   static CMCategory* AddCategory(CMCategory* pCat) {categories.Add(pCat);return pCat;}
	static CMCategory* GetCategory(unsigned short n) {return n<categories.Count() ? categories[n] : 0;}
	static CMCategory* FindCategory(const string& aName);
};
