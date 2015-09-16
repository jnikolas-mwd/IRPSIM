// node.cpp : implementation file
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
#include "StdAfx.h"
#include <stdlib.h>
#include <math.h>

#include "irp.h"
#include "node.h"
#include "regions.h"
#include "category.h"
#include "notify.h"

#include "token.h"
#include "cmlib.h"

//#include <fstream>
//static wofstream sdebug("debug_node.txt");

CMTimeMachine* CMNode::time = 0;
unsigned short CMNode::naggregates;
unsigned short CMNode::nregions;
CMPSmallArray<CMVariable> CMNode::aggregates;
//CMPBigArray<CMVariable> CMNode::aggregates;
CMPSmallArray<CMNode> CMNode::nodes;
CMVariable* CMNode::vtimestep;

void CMNode::ResetAggregateVariables(bool bCreateNew)
{
	aggregates.Reset(1);

	if (!bCreateNew)
		return;

	naggregates = CMVariableTypes::AggNamesCount();
   nregions = CMRegions::RegionCount();

	const wchar_t* name;
	for (unsigned short i=0;(name=CMVariableTypes::AggStringFromInt(i))!=0;i++) {
		aggregates.AddAt(i,CMVariable::Find(name));
		for (unsigned short j=0;j<nregions;j++)
			aggregates.AddAt((j+1)*naggregates+i,CMVariable::Find(CMString(name) + L"." + CMRegions::GetRegionName(j)));
	}
}

void CMNode::InitAggregateVariables(CMTimeMachine* t)
{
	time = t;
   for (unsigned short i=0;i<aggregates.Count();i++)
   	aggregates.At(i)->SetValue(time,0);
}

const wchar_t* CMNode::IsA() { return L"CMNode"; }

CMNode::CMNode(CMVariable* v) : CMAllocationUnit(v ? v->GetName() : L"",-1),
vmain(v),
vmonitorevaluated(0),
vmonitorresolved(0),
vmonitorcutback(0),
//vmonitorcarryover(0),
vmonitorgain(0),
vmonitorloss(0),
vmonitorput(0),
vmonitortake(0),
vmonitorshiftin(0),
vmonitorshiftout(0),
//vmonitorseasonalshift(0),
vmonitorfirststep(0),
vmonitorlaststep(0),
vmonitorpulls(0),
vmonitorage(0),
vcrank(0),
vcfloor(0),
vctrigger(0),
vcexpire(0),
vcfulltake(0),
vcfullput(0),
vccapacity(0),
vcmaxpulls(0),
ccutback(CM_BIGDOUBLE),
cput(CM_BIGDOUBLE),
ctake(CM_BIGDOUBLE),
cgain(0),
closs(0),
climit(CM_BIGDOUBLE),
storage_fifo(NULL),
state(0),
region(-1),
transfer_type(tSpot),
istriggered(0),
npulls(0),
maxpulls(0),
firststep(0),
laststep(0),
amtcut(0),
maxcut(0),
sumin(0),
sumout(0),
amtput(0),
amttake(0),
maxput(0),
maxtake(0)
{
	if (!v)	return;

   region = v->GetRegion();

	vtimestep = CMVariable::Find(L"_timestep");
	vmonitorevaluated = CMVariable::Find(v->GetName() + L".evaluated");
	vmonitorresolved = CMVariable::Find(v->GetName() + L".resolved");
	vmonitorcutback = CMVariable::Find(v->GetName() + L".cutback");
	//vmonitorcarryover = CMVariable::Find(v->GetName() + L".carryover");
	vmonitorgain = CMVariable::Find(v->GetName() + L".gain");
	vmonitorloss = CMVariable::Find(v->GetName() + L".loss");
	vmonitorput = CMVariable::Find(v->GetName() + L".put");
	vmonitortake = CMVariable::Find(v->GetName() + L".take");
	vmonitorshiftin = CMVariable::Find(v->GetName() + L".shiftin");
	vmonitorshiftout = CMVariable::Find(v->GetName() + L".shiftout");
	//vmonitorseasonalshift = CMVariable::Find(v->GetName() + L".seasonalshift");
	vmonitorfirststep = CMVariable::Find(v->GetName() + L".firstactivestep");
	vmonitorlaststep = CMVariable::Find(v->GetName() + L".lastactivestep");
	vmonitorpulls = CMVariable::Find(v->GetName() + L".pulls");
	vmonitorage = CMVariable::Find(v->GetName() + L".age");

	set_association(vcrank,L"rank");
	set_association(vcfloor,L"floor");
	set_association(vctrigger,L"trigger");
	set_association(vcexpire,L"expire");
	set_association(vccapacity,L"capacity");
	set_association(vcfulltake,L"fulltake");
	set_association(vcfullput,L"fullput");
	set_association(vcmaxpulls,L"maxpulls");

	ccutback.Set(v->GetAssociation(L"cutback"));
	cput.Set(v->GetAssociation(L"put"));
	ctake.Set(v->GetAssociation(L"take"));
	cgain.Set(v->GetAssociation(L"gain"));
	closs.Set(v->GetAssociation(L"loss"));
	climit.Set(v->GetAssociation(L"limit"));

   if (v->IsType(L"demand")) allocation_type = aDemand;
   else if (v->IsType(L"supply")) allocation_type = aSupply;
   else if (v->IsType(L"transfer")) allocation_type = aTransfer;
   else if (v->IsType(L"storage")) {
   	allocation_type = aStorage;
		storage_fifo = new CMFifo();
      v->SetState(CMVariable::vsCarryForward,TRUE);
		vmonitorresolved->SetState(CMVariable::vsCarryForward,TRUE);
		if (v->IsType(L"ageall")) state |= sAgeAll;
   }

	transfer_type = tCore;
	if (vmain->IsType(L"spot"))			  transfer_type = tSpot;
	else if (vmain->IsType(L"option"))  transfer_type = tOption;

	istriggered=0;
}

CMNode::~CMNode()
{
	if (storage_fifo != NULL) delete storage_fifo;
}

wostream& CMNode::write(wostream& s)
{
	if (vmain) s << *vmain;
	return s;
}

double CMNode::get_rank()
{
	return vcrank.GetValue(time,CM_BIGDOUBLE);
}

void CMNode::set_association(CMVariableOrConstant& vc, const wchar_t* varstr)
{
   if (vmain && varstr)
		vc.Set(vmain->GetAssociation(CMString(varstr)));
   else
   	vc.Set(0);
}

double CMNode::max_cut()
{
	if (maxcut == CM_NODOUBLE)
		maxcut = ccutback.AmountRemainingNow(time);
	return maxcut;
}

double CMNode::EvaluateAbility()
{
	double amt = vmain->GetValue(time);
   double limit = 0;
	if (!istriggered) {
   	if (vctrigger.GetValue(time,1,1) != 0)
	      limit = climit.LimitAmount(time);
   }
   else
		limit = climit.AmountRemainingNow(time);
	return cmmin(amt,limit);
}

double CMNode::CutAbility()
{
	double amt = max_cut() - amtcut;
	double flr = vcfloor.GetValue(time,0);
	double avail = vmonitorresolved->GetValue(time);
	return cmmin(amt,(avail-flr));
}

double CMNode::PutAbility()
{
	double putability  = max_puttake(sPut) - amtput;
	double remcapacity = RemainingCapacity();
	return cmmin(putability,remcapacity);
}

double CMNode::TakeAbility()
{
	double takeability = max_puttake(sTake) - amttake;
	double level = vmonitorresolved->GetValue(time);
	return cmmin(takeability,level);
}

double CMNode::max_puttake(int which)
{
	if (maxput == CM_NODOUBLE) {
		maxput  = cput.AmountRemainingNow(time);
		maxtake = ctake.AmountRemainingNow(time);
	}
	return (which==sPut ? maxput : maxtake);
}

double CMNode::full_put()
{
	return vcfullput.IsNull() ? max_puttake(sPut) : vcfullput.GetValue(time,CM_BIGDOUBLE);
}

double CMNode::full_take()
{
	return vcfulltake.IsNull() ? max_puttake(sTake) : vcfulltake.GetValue(time,CM_BIGDOUBLE);
}

/*
double CMNode::adjust_shift_and_carryover(double aVal)
{
	if (aVal > 0) sumin += aVal;
	else  sumout -= aVal;

	if (vmonitorseasonalshift) vmonitorseasonalshift->SetValue(time,((sumin<sumout) ? sumin : sumout));
	if (vmonitorcarryover) vmonitorcarryover->SetValue(time,sumin-sumout);

	return (sumin-sumout);
}
*/

void CMNode::init_time_step(CMTimeMachine* t)
{
	time=t;
	sort_value=maxcut=maxput=maxtake=CM_NODOUBLE;
	amtcut=amtput=amttake=0;
	int atbeginning = time->AtBeginning();
	if (atbeginning) {
		istriggered=firststep=laststep=npulls=maxpulls=0;
		ccutback.Initialize(0);
		cput.Initialize(0);
		ctake.Initialize(0);
		cgain.Initialize(0);
		closs.Initialize(0);
      climit.Initialize(0);
   }
	CMTime current = time->At(0).Current();
	if (current.Month() == time->At(0).Begin().Month()) {
		sumin = sumout = 0;
	}
	if (vmonitorfirststep)	vmonitorfirststep->SetValue(time,firststep);
	if (vmonitorlaststep)	vmonitorlaststep->SetValue(time,laststep);
	if (vmonitorpulls)	vmonitorpulls->SetValue(time,npulls);
	if (vmonitorevaluated)	vmonitorevaluated->SetValue(time,0);
	if (vmonitorcutback)	vmonitorcutback->SetValue(time,0);
	if (vmonitorgain)	vmonitorgain->SetValue(time,0);
	if (vmonitorloss)	vmonitorloss->SetValue(time,0);
	if (vmonitorput)	vmonitorput->SetValue(time,0);
	if (vmonitortake)	vmonitortake->SetValue(time,0);
	if (vmonitorshiftin)	vmonitorshiftin->SetValue(time,0);
	if (vmonitorshiftout)	vmonitorshiftout->SetValue(time,0);
	//if (vmonitorseasonalshift)	vmonitorseasonalshift->SetValue(time,sumin<sumout ? sumin : sumout);
	//if (vmonitorcarryover)	vmonitorcarryover->SetValue(time,sumin-sumout);
   if (allocation_type==aStorage) {
 		double initstorage = atbeginning ? vmain->GetValue(time) : vmonitorresolved->GetValue(time);
		if (atbeginning) {
			vmonitorresolved->SetValue(time,initstorage);
			if (storage_fifo) {
				storage_fifo->Clear();
				if (state&sAgeAll) storage_fifo->Push(0,initstorage);
			}
		}
      else 
			vmain->SetValue(time,initstorage);
		if (vmonitorage && vtimestep && storage_fifo) vmonitorage->SetValue(time,storage_fifo->AvgAge((int)vtimestep->GetValue(time)));
		aggregates[CMVariableTypes::aStorage]->AddTo(time,initstorage);
		if (region>=0)
			aggregates[(region+1)*naggregates+CMVariableTypes::aStorage]->AddTo(time,initstorage);
   }
   else
		vmonitorresolved->SetValue(time,0);
	if (climit.AmountRemainingTotal(t)<=0)
	   	istriggered=0;
}

void CMNode::end_time_step(CMTimeMachine* t)
{
	climit.Add(vmonitorresolved->GetValue(t));
   if (allocation_type==aStorage && storage_fifo) {
		if (amtput!=0) storage_fifo->Push((int)vtimestep->GetValue(t),amtput);
		if (amttake!=0) storage_fifo->Pop(amttake);
	}
	if (istriggered) {
	   laststep = (int)vtimestep->GetValue(time);
      if (vmonitorlaststep) vmonitorlaststep->SetValue(time,laststep);
//		if (climit.AmountRemainingTotal(t)<=0)
//	   	istriggered=0;
   }
}

double CMNode::Evaluate(double limit)
{
	double amt = 0;

   if (!istriggered) {
   	if (istriggered = vctrigger.GetValue(time,1,1) ? 1 : 0) {
			if (++npulls > maxpulls) {
	         firststep = (int)vtimestep->GetValue(time);
	         if (vmonitorfirststep) vmonitorfirststep->SetValue(time,firststep);
         	maxpulls = (int)vcmaxpulls.GetValue(time,1);
				npulls=1;
         }
   		if (vmonitorpulls) vmonitorpulls->SetValue(time,npulls);
	      climit.Initialize(time);
			ccutback.Initialize(time);
      }
   }

	if (istriggered) {
		amt = vmain->GetValue(time);
   	amt = cmmin(amt,limit);
	   amt = cmmin(amt,climit.AmountRemainingNow(time));
   }

   if ((allocation_type==aDemand || allocation_type==aSupply || allocation_type==aTransfer) && amt!=0) {
   	int sgn = (allocation_type==aDemand) ? -1 : 1;
		int tp = CMVariableTypes::aDemand;
      if (allocation_type==aSupply) tp = CMVariableTypes::aSupply;
      //else if (allocation_type==aTransfer) tp = CMVariableTypes::aTransfer;
      aggregates[tp]->AddTo(time,amt);
      if (allocation_type != aTransfer) aggregates[CMVariableTypes::aSurplus]->AddTo(time,sgn*amt);
		aggregates[CMVariableTypes::aNetSurplus]->AddTo(time,sgn*amt);
		if (region>=0) {
			aggregates[(region+1)*naggregates+tp]->AddTo(time,amt);
			if (allocation_type != aTransfer)
				aggregates[(region+1)*naggregates+CMVariableTypes::aSurplus]->AddTo(time,sgn*amt);
			aggregates[(region+1)*naggregates+CMVariableTypes::aNetSurplus]->AddTo(time,sgn*amt);
      }
   }
	if (vmonitorevaluated) vmonitorevaluated->SetValue(time,amt);
	if (vmonitorresolved) vmonitorresolved->SetValue(time,amt);
   return amt;
}

double CMNode::CutBack(double aVal)
{
	if (!(allocation_type==aDemand || allocation_type==aSupply || allocation_type==aTransfer))
   	return 0;

  	int sgn = (allocation_type==aDemand) ? 1 : -1;
   int tp = CMVariableTypes::aDemandCut;
   if (allocation_type==aSupply) tp = CMVariableTypes::aSupplyCut;
   //else if (allocation_type==aTransfer) tp = CMVariableTypes::aTransferCut;

	double cutability = CutAbility();
	double amt = (aVal < cutability) ? aVal : cutability;
	if (amt<=0) return 0;

	vmonitorresolved->AddTo(time,-amt);
	ccutback.Add(amt);
	amtcut += amt;
	if (vmonitorcutback) vmonitorcutback->SetValue(time,amtcut);

	aggregates[tp]->AddTo(time,amt);
	aggregates[CMVariableTypes::aNetSurplus]->AddTo(time,sgn*amt);
	if (region>=0) {
		aggregates[(region+1)*naggregates+tp]->AddTo(time,amt);
		aggregates[(region+1)*naggregates+CMVariableTypes::aNetSurplus]->AddTo(time,sgn*amt);
	}
   return amt;
}

double CMNode::Put(double aVal,int isshift)
{
	if (allocation_type != aStorage)
   	return 0;

	double putability = PutAbility();
	double amt = (aVal < putability) ? aVal : putability;
	if (amt <= 0) return 0;

	amtput += amt;
	cput.Add(amt);
	if (vmonitorput) vmonitorput->SetValue(time,amtput);
	vmonitorresolved->AddTo(time,amt);
	//adjust_shift_and_carryover(amt);
	if (isshift) {
	   if (vmonitorshiftin) vmonitorshiftin->AddTo(time,amt);
		aggregates[CMVariableTypes::aStorageShift]->AddTo(time,amt);
		if (region>=0)
			aggregates[(region+1)*naggregates+CMVariableTypes::aStorageShift]->AddTo(time,amt);
	}
	else {
		aggregates[CMVariableTypes::aPut]->AddTo(time,amt);
		aggregates[CMVariableTypes::aPutTake]->AddTo(time,amt);
		aggregates[CMVariableTypes::aStorage]->AddTo(time,amt);
		aggregates[CMVariableTypes::aNetSurplus]->AddTo(time,-amt);
		if (region>=0) {
			aggregates[(region+1)*naggregates+CMVariableTypes::aPut]->AddTo(time,amt);
			aggregates[(region+1)*naggregates+CMVariableTypes::aPutTake]->AddTo(time,amt);
			aggregates[(region+1)*naggregates+CMVariableTypes::aStorage]->AddTo(time,amt);
			aggregates[(region+1)*naggregates+CMVariableTypes::aNetSurplus]->AddTo(time,-amt);
	   }
   }
   return amt;
}

double CMNode::Take(double aVal,int isshift)
{
	if (allocation_type != aStorage)
   	return 0;

	double takeability = TakeAbility();

	double amt = (aVal < takeability) ? aVal : takeability;
	if (amt <= 0) return 0;

	amttake += amt;
    ctake.Add(amt);
	if (vmonitortake) vmonitortake->SetValue(time,amttake);
	vmonitorresolved->AddTo(time,-amt);
	//adjust_shift_and_carryover(-amt);

	if (isshift) {
	   if (vmonitorshiftout) vmonitorshiftout->AddTo(time,amt);
   }
   else {
		aggregates[CMVariableTypes::aTake]->AddTo(time,amt);
		aggregates[CMVariableTypes::aPutTake]->AddTo(time,-amt);
		aggregates[CMVariableTypes::aStorage]->AddTo(time,-amt);
		aggregates[CMVariableTypes::aNetSurplus]->AddTo(time,amt);
		if (region>=0) {
			aggregates[(region+1)*naggregates+CMVariableTypes::aTake]->AddTo(time,amt);
			aggregates[(region+1)*naggregates+CMVariableTypes::aPutTake]->AddTo(time,-amt);
			aggregates[(region+1)*naggregates+CMVariableTypes::aStorage]->AddTo(time,-amt);
			aggregates[(region+1)*naggregates+CMVariableTypes::aNetSurplus]->AddTo(time,amt);
   	}
   }
   return amt;
}

void CMNode::Gain()
{
	if (allocation_type != aStorage)
		return;
	double capac   = RemainingCapacity();
	double maxgain = cgain.AmountRemainingNow(time);
	double amtgain = (maxgain < capac) ? maxgain : capac;
	if (amtgain<=0) return;

	cgain.Add(amtgain);
	vmonitorresolved->AddTo(time,amtgain);
	if (vmonitorgain) vmonitorgain->SetValue(time,amtgain);

	aggregates[CMVariableTypes::aGainLoss]->AddTo(time,amtgain);
	aggregates[CMVariableTypes::aStorage]->AddTo(time,amtgain);
	if (region>=0) {
		aggregates[(region+1)*naggregates+CMVariableTypes::aGainLoss]->AddTo(time,amtgain);
		aggregates[(region+1)*naggregates+CMVariableTypes::aStorage]->AddTo(time,amtgain);
   }
}

void CMNode::Loss()
{
	if (allocation_type != aStorage)
		return;
	double instorage = vmonitorresolved->GetValue(time);
	double maxloss   = closs.AmountRemainingNow(time);
	double amtloss = (maxloss < instorage) ? maxloss : instorage;
	if (amtloss<=0) return;

	closs.Add(amtloss);
	vmonitorresolved->AddTo(time,-amtloss);
	if (vmonitorloss) vmonitorloss->SetValue(time,amtloss);
	aggregates[CMVariableTypes::aGainLoss]->AddTo(time,-amtloss);
	aggregates[CMVariableTypes::aStorage]->AddTo(time,-amtloss);
	if (region>=0) {
		aggregates[(region+1)*naggregates+CMVariableTypes::aGainLoss]->AddTo(time,-amtloss);
		aggregates[(region+1)*naggregates+CMVariableTypes::aStorage]->AddTo(time,-amtloss);
   }
}

double CMNode::Capacity()
{
	return vccapacity.GetValue(time,CM_BIGDOUBLE);
}

double CMNode::RemainingCapacity()
{
	if (allocation_type != aStorage) return CM_BIGDOUBLE;
	double rem = vccapacity.GetValue(time,CM_BIGDOUBLE);
	rem -= vmonitorresolved->GetValue(time);
	return (rem < 0) ? 0 : rem;
}

void CMNode::DestroyNodes()
{
	nodes.ResetAndDestroy(1);
}

CMNode* CMNode::AddNode(const CMString& name)
{
	if (name.is_null())
   	return 0;

  	for (unsigned i=0;i<nodes.Count();i++)
	  	if (name == nodes.At(i)->GetName())
      	return nodes.At(i);

   CMVariable* v = CMVariable::Find(name);
   if (v == 0) {
	   CMNotifier::Notify(CMNotifier::ERROR, name + L": node variable not defined");
      return 0;
   }

   //v->SetState(CMVariable::vsSelected,TRUE);
   CMNode* n = new CMNode(v);
   nodes.Add(n);
   return n;
}

void CMNode::InitTimeStep(CMTimeMachine* t)
{
	for (unsigned short i=0;i<nodes.Count();i++)
   	nodes.At(i)->init_time_step(t);
}

void CMNode::EndTimeStep(CMTimeMachine* t)
{
	for (unsigned short i=0;i<nodes.Count();i++)
   	nodes.At(i)->end_time_step(t);
}

double CMNode::PutPotential()
{
	double ret = 0;
	for (unsigned short i=0;i<nodes.Count();i++)
   	if (nodes[i]->GetType() == aStorage)
			ret += nodes[i]->PutAbility();
	return ret;
}

double CMNode::TakePotential()
{
	double ret = 0;
	for (unsigned short i=0;i<nodes.Count();i++)
   	if (nodes[i]->GetType() == aStorage)
			ret += nodes[i]->TakeAbility();
	return ret;
}

/*
double CMNode::TransferPotential()
{
	return 0;
}
*/
