// category.cpp : implementation file
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
#include "category.h"
#include "vartypes.h"
#include "timemach.h"
#include "notify.h"

#include "token.h"
#include "cmlib.h"

#include <iomanip>
#include <math.h>
//#include <fstream.h>
//static ofstream sdebug("category.deb");

wchar_t* CMCategory::header = L"#CATEGORY";
wchar_t* CMCategory::footer = L"#END";

CMTimeMachine* CMCategory::time = 0;
CMVariable* CMCategory::vnetsurplus = 0;
CMPSmallArray<CMCategory> CMCategory::categories;

const wchar_t* CMCategory::IsA() {return L"CMCategory";}

CMCategory::CMCategory(int id) : CMAllocationUnit(NULL,id) 
{
	reset();
}

CMCategory::~CMCategory()
{
	reset();
}

void CMCategory::reset()
{
	member_names.Reset(1);
   member_units.Reset(1);
	default_allocation_mode = CMAllocationUnit::Preserve;
   state=0;
}

int CMCategory::Initialize()
{
	vnetsurplus = CMVariable::Find(CMVariableTypes::AggStringFromInt(CMVariableTypes::aNetSurplus));
	member_units.Reset(1);
	CMCategory* pCat;
	vcPutLimit.Set(strPutLimit);
	vcTakeLimit.Set(strTakeLimit);
	vcCutLimit.Set(strCutLimit);
	for (unsigned short i=0;i<member_names.Count();i++) {
		if (GetName() == member_names[i]) {
			CMString errstring(GetName());
			CMNotifier::Notify(CMNotifier::ERROR, errstring + L" contains itself as a category");
			member_units.Add(NULL);
		}
		else if (pCat=FindCategory(member_names[i])) {
			pCat->Initialize();
			member_units.Add(pCat);
		}
		else 
			member_units.Add(CMNode::AddNode(member_names[i]));
      if (!member_units[i]) state |= csFail;
   }
   return (state&csFail) ? 0 : 1;
}

void CMCategory::InitTimeStep(CMTimeMachine* t)
{
	time = t;
}

double CMCategory::EvaluateAbility()
{
	double ret = 0;
   for (unsigned i=0;i<member_units.Count();i++)
   	ret += member_units[i]->EvaluateAbility();
   return ret;
}

double CMCategory::PutAbility()
{
	double ret = 0;
   for (unsigned i=0;i<member_units.Count();i++)
   	ret += member_units[i]->PutAbility();
	double limit = vcPutLimit.GetValue(time,CM_BIGDOUBLE);
	return cmmin(ret,limit);
}

double CMCategory::TakeAbility()
{
	double ret = 0;
   for (unsigned i=0;i<member_units.Count();i++)
   	ret += member_units[i]->TakeAbility();
	double limit = vcTakeLimit.GetValue(time,CM_BIGDOUBLE);
	return cmmin(ret,limit);
}

double CMCategory::CutAbility()
{
	double ret = 0;
   for (unsigned i=0;i<member_units.Count();i++)
   	ret += member_units[i]->CutAbility();
	double limit = vcCutLimit.GetValue(time,CM_BIGDOUBLE);
	return cmmin(ret,limit);
}

double CMCategory::alloc_ability()
{
	double ret=0;
	for (unsigned i=0;i<member_units.Count();i++)
		ret += member_units[i]->ability;
	return ret;
}

double CMCategory::amount_allocated()
{
	double ret=0;
	for (unsigned i=0;i<member_units.Count();i++)
		ret += member_units[i]->allocamount;
	return ret;
}

void CMCategory::InitProcess(int operation,int rul,int atype)
{
	for (unsigned i=0;i<member_units.Count();i++)
   	member_units[i]->init_process(operation,rul>=0?rul:default_allocation_mode,atype);
}

int CMCategory::PreProcess()
{
	int includednodes = 0;
	for (unsigned i=0;i<member_units.Count();i++)
		if (member_units[i]->pre_process())
      	includednodes++;
	return includednodes;
}

int CMCategory::reset_required()
{
	int reset=0;
	for (unsigned i=0;i<member_units.Count();i++)
		if (member_units[i]->include==2)
			reset=1;
   return reset;
}

void CMCategory::PostProcess(int reduceflag)
{
	for (unsigned i=0;i<member_units.Count();i++)
		member_units[i]->post_process(reduceflag);
}

// returns a flag indicating whether the process is completed
// if 0 -- must reiterate

int CMCategory::Process(double val,double n,double d)
{
	int completed = 1;
	for (unsigned i=0;i<member_units.Count();i++)
		if (member_units[i]->process(val,n,d) < 0)
      	completed = 0;
   return completed;
}

double CMCategory::ProcessAllocation(int operation,double amt,int rul,int atype)
{
	double d_numerator,d_denominator;
   unsigned i;

	double allocated = 0;

   int alloc_rule = rul>=0 ? rul : default_allocation_mode;

	InitProcess(operation,alloc_rule,atype);

	double able=alloc_ability();

	if (fabs(able)<fabs(amt)) amt = cmsign(able,amt);

	double tolerance = 0.000001;

	int includednodes=1;

	if (alloc_rule == CMAllocationUnit::ByRank) {
	//	member_units.Sort();
		for (i=0;i<member_units.Count()&&fabs(amt-allocated)>tolerance;i++)
			allocated += member_units[i]->allocate(amt-allocated);
		return allocated;
	}

	unsigned count=0;
	while (fabs(amt-allocated)>tolerance && includednodes) {
		// pre_process returns include value.  If no include, stop
		includednodes = PreProcess();
		int stop = 0;
		while (!stop) {
			d_numerator = d_denominator = 0;
			for (i=0;i<member_units.Count();i++) {
				if (member_units[i]->include) {
					d_numerator   += member_units[i]->numerator;
					d_denominator += member_units[i]->denominator;
				}
			}
			stop = Process(amt-allocated,d_numerator,d_denominator);
			PostProcess(0);
		}
		if (++count>100) {
			CMNotifier::Notify(CMNotifier::ERROR, L"allocation would not converge");
         break;
      }
		PostProcess(reset_required());
		allocated = amount_allocated();
	}
	return allocated;
}

double CMCategory::Evaluate(double limit,int rul)
{
	if (limit<0) limit = 0;

	double amt = ProcessAllocation(EvaluateOperation,limit,rul);
	for (unsigned j=0;j<member_units.Count();j++)
		member_units[j]->Evaluate(member_units[j]->allocamount);
	return amt;
}

double CMCategory::CutBack(double limit,int rul,int ignorebalance,int nodetype)
{
	double attempt = (nodetype==aDemand ? -1 : 1) * vnetsurplus->GetValue(time);
	attempt = ignorebalance ? limit : attempt;

	// if there is a limit attached to the category, use it
	double cutlimit = vcCutLimit.GetValue(time,CM_BIGDOUBLE);
	attempt = cmmin(attempt,cutlimit);
	attempt=cmmin(attempt,limit);
	if (attempt<=0) return 0;

	double amt = ProcessAllocation(CutBackOperation,-attempt,rul);
	for (unsigned j=0;j<member_units.Count();j++)
		member_units[j]->CutBack(-member_units[j]->allocamount);

   return (-amt);
}

double CMCategory::Put(double limit,int rul,int ignorebalance,int isshift)
{
	double attempt = vnetsurplus->GetValue(time);
	attempt = ignorebalance ? limit : attempt;

	// if there is a limit attached to the category, use it
	double putlimit = vcPutLimit.GetValue(time,CM_BIGDOUBLE);
	attempt = cmmin(attempt,putlimit);
	attempt=cmmin(attempt,limit);
	if (attempt<=0) return 0;

	double amt = ProcessAllocation(PutOperation,attempt,rul);
	for (unsigned j=0;j<member_units.Count();j++)
	  	member_units[j]->Put(member_units[j]->allocamount,isshift);

	return amt;
}

double CMCategory::Take(double limit,int rul,int ignorebalance,int isshift)
{
	double attempt = -vnetsurplus->GetValue(time);
	attempt = ignorebalance ? limit : attempt;
	// if there is a limit attached to the category, use it
	double takelimit = vcTakeLimit.GetValue(time,CM_BIGDOUBLE);
	attempt = cmmin(attempt,takelimit);
	attempt=cmmin(attempt,limit);
	if (attempt<=0) return 0;

	double amt = ProcessAllocation(TakeOperation,-attempt,rul);
	for (unsigned j=0;j<member_units.Count();j++)
	  	member_units[j]->Take(-member_units[j]->allocamount,isshift);

   return (-amt);
}


void CMCategory::Gain()
{
	for (unsigned i=0;i<member_units.Count();i++)
		member_units[i]->Gain();
}

void CMCategory::Loss()
{
	for (unsigned i=0;i<member_units.Count();i++)
		member_units[i]->Gain();
}

wistream& CMCategory::read(wistream& s)
{
    const wchar_t* delims = L" \t\r\n";
	reset();
	CMString line;
	while (!s.eof()) {
		line.read_line(s);
   	line = stripends(line);
		if (line.is_null())
      	continue;
		else if (line(0,wcslen(header)) == header) {
      	name = stripends(CMString(line.c_str()+wcslen(header)));
         continue;
      }
		else if (line(0,wcslen(footer)) == footer)
   		break;
		CMTokenizer next(line);
      CMString token = next(delims);
		if (token[0]==L'#') {
      	if (token(0,8)==L"#allocat")
            default_allocation_mode = CMAllocationUnit::TranslateAllocationRule(next(delims));
			else if (token(0,4)==L"#put")	strPutLimit = next(delims);
			else if (token(0,5)==L"#take")	strTakeLimit = next(delims);
			else if (token(0,4)==L"#cut")	strCutLimit = next(delims);
      }
      else {
			do {
         	member_names.Add(token);
         } while (!(token=next(delims)).is_null());
      }
   }
   return s;
}

wostream& CMCategory::write(wostream& s)
{
	s << setiosflags(ios::left) << header << L' ' << name << ENDL;
	s << L'#' << CMAllocationUnit::TranslateAllocationRule(default_allocation_mode) << ENDL;
   if (!strPutLimit.is_null()) s << L"#putlimit " << strPutLimit << ENDL;
   if (!strTakeLimit.is_null()) s << L"#takelimit " << strTakeLimit << ENDL;
   if (!strCutLimit.is_null()) s << L"#cutlimit " << strCutLimit << ENDL;
		
	for (unsigned short i=0;i<member_names.Count();i++)
   	s << member_names[i] << ENDL;
	s << footer << ENDL;
   return s;
}

CMCategory* CMCategory::FindCategory(const CMString& aName)
{
	for (unsigned short i=0;i<categories.Count();i++)
   	if (aName == categories[i]->GetName())
      	return categories[i];
   return 0;
}

double CMCategory::full_put()
{
	double ret = 0;
	for (unsigned i=0;i<member_units.Count();i++)
		ret += member_units[i]->full_put();
	return ret;
}

double CMCategory::full_take()
{
	double ret = 0;
	for (unsigned i=0;i<member_units.Count();i++)
		ret += member_units[i]->full_take();
	return ret;
}

double CMCategory::get_rank()
{
	return CM_BIGDOUBLE;
}

double CMCategory::Capacity()
{
	double ret = 0;
	for (unsigned i=0;i<member_units.Count();i++)
		ret += member_units[i]->Capacity();
	return ret;
}

double CMCategory::RemainingCapacity()
{
	double ret = 0;
	for (unsigned i=0;i<member_units.Count();i++)
		ret += member_units[i]->RemainingCapacity();
	return ret;
}

double CMCategory::Resolved()
{
	double ret = 0;
	for (unsigned i=0;i<member_units.Count();i++)
		ret += member_units[i]->Resolved();
	return ret;
}

double CMCategory::Evaluate(double limit)
{
	return Evaluate(limit,allocation_mode);
}

double CMCategory::CutBack(double aVal)
{
	return CutBack(aVal,allocation_mode,TRUE,allocation_type);
}

double CMCategory::Put(double aVal,int isshift)
{
	return Put(aVal,allocation_mode,TRUE,isshift);
}

double CMCategory::Take(double aVal,int isshift)
{
	return Take(aVal,allocation_mode,TRUE,isshift);
}



