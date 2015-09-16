// node.h : header file
//
// Copyright © 1998-2015 Casey McSpadden   
//		mailto:casey@crossriver.com
//		http://www.crossriver.com/
//
// ==========================================================================  
// DESCRIPTION:	
// ==========================================================================
// CMNode is the base class for IRPSIM "nodes" (supply, demand, storage).
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
#include "variable.h"
#include "vartypes.h"
#include "varconst.h"
#include "control.h"
#include "irpobject.h"
#include "allocationunit.h"

#include "smparray.h"
#include "bgparray.h"
#include "fifo.h"

class CMNode;

class _IRPCLASS CMNode : public CMAllocationUnit
{
protected:
	friend class _IRPCLASS CMNodeCollection;
	friend class _IRPCLASS CMCategory;

	static unsigned short naggregates;
	static unsigned short nregions;
	static class _IRPCLASS CMPSmallArray<CMVariable> aggregates; // system aggregate vars
	//static class _IRPCLASS CMPBigArray<CMVariable> aggregates; // system aggregate vars
   														 // naggregates*(nregions+1)
	static class _IRPCLASS CMPSmallArray<CMNode> nodes;
	static CMTimeMachine* time;
   static CMVariable* vtimestep;

   int state,region;
	int transfer_type,maxpulls,istriggered,npulls,firststep,laststep;

	/*
	CMVariable *vmain,*vmonitorevaluated,*vmonitorresolved,*vmonitorcutback,
   				*vmonitorcarryover,*vmonitorgain,*vmonitorloss,*vmonitorput,
               *vmonitortake,*vmonitorshiftin,*vmonitorshiftout,
               *vmonitorseasonalshift,*vmonitorfirststep,*vmonitorlaststep,
               *vmonitorpulls,*vmonitorage;
    */
	CMVariable *vmain, *vmonitorevaluated, *vmonitorresolved, *vmonitorcutback,
		*vmonitorgain, *vmonitorloss, *vmonitorput,
		*vmonitortake, *vmonitorshiftin, *vmonitorshiftout,
		*vmonitorfirststep, *vmonitorlaststep,
		*vmonitorpulls, *vmonitorage;

	CMVariableOrConstant vcrank, vcfloor, vctrigger, vcexpire,
   							vcmaxpulls,vcfulltake,vcfullput,vccapacity;

   CMControl ccutback,cput,ctake,cgain,closs,climit;

	double amtcut,maxcut;
	double sumin,sumout,amtput,amttake,maxput,maxtake;
	CMFifo* storage_fifo;

	virtual double get_rank();
	virtual double full_put();
	virtual double full_take();
	
	virtual wostream& write(wostream& s);
	virtual const wchar_t* IsA();

	void 	 init_time_step(CMTimeMachine* t);
	void 	 end_time_step(CMTimeMachine* t);
	double max_cut();
	double max_puttake(int which);
	//double adjust_shift_and_carryover(double aVal);
	void set_association(CMVariableOrConstant& vc,const wchar_t* varstr);
	enum {tCore,tSpot,tOption,sPut,sTake};
	enum {sAgeAll=0x0001};
public:
	CMNode(CMVariable* v=0);
	~CMNode();

   int GetRegion() const {return region;}

	virtual double Resolved() {return vmonitorresolved->GetValue(time);}
	virtual double Capacity();
   virtual double RemainingCapacity();
	virtual double EvaluateAbility();
	virtual double PutAbility();
	virtual double TakeAbility();
	virtual double CutAbility();

	double Evaluate(double limit);
	double CutBack(double aVal);
	double Put(double aVal,int isshift);
	double Take(double aVal,int isshift);
	void Gain();
	void Loss();

   // Static functions

	static void ResetAggregateVariables(bool bCreateNew);
   static void InitAggregateVariables(CMTimeMachine* t);
	static void DestroyNodes();
   static unsigned short NodeCount() {return nodes.Count();}
   static CMNode* AddNode(const CMString& name);
	static CMNode* GetNode(unsigned short n) {return n<nodes.Count() ? nodes[n] : 0;}
   static double PutPotential();
   static double TakePotential();
   //static double TransferPotential();

	/* Call InitTimeStep at the beginning of each time step. The function
		will determine whether this is a new trial */
   static void InitTimeStep(CMTimeMachine* t);
   static void EndTimeStep(CMTimeMachine* t);
};

