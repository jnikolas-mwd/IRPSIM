// varcol.cpp : implementation file
//
// Copyright © 1998-2015 Casey McSpadden   
//		mailto:casey@crossriver.com
//		http://www.crossriver.com/
//
// ==========================================================================  
// DESCRIPTION:	
// ==========================================================================
// CMVariableCollection implements a collection of IRPSIM variables used
// in a simulation.
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
#include "varcol.h"
#include "vtime.h"
#include "vsystem.h"
#include "vartypes.h"
#include "notify.h"

#include "cmlib.h"
#include "token.h"
#if defined (_CM_SOLARIS)
	#include <strstream.h>
#else
	//#include <strstrea.h>
	#include <strstream>
	using namespace std;
#endif

//#include <fstream.h>
//static ofstream sdebug("varcol.deb");

CMVariableIterator::CMVariableIterator() : iter(CMVariable::variables)
{
}

CMVariableIterator::CMVariableIterator(CMVariableCollection* v) : iter(v)
{
}

CMVariableCollection::CMVariableCollection(int create_system_variables) :
CMPHashDictionary<CMVariable>(),
state(create_system_variables ? sCreateSystemVariables : 0)
{
	int i;
	if (create_system_variables) {
		for (i=CMVTime::First;i<=CMVTime::Last;i++)
			Add(new CMVTime(i));
		for (i=CMVSystem::First;i<=CMVSystem::Last;i++)
			Add(new CMVSystem(i));
		const wchar_t* name;
		for (i=0;(name=CMVariableTypes::AggStringFromInt(i))!=0;i++) {
			CMVariable* v = new CMVariable(name,CMVariable::vsAggregate|CMVariable::vsDontDestroy|CMVariable::vsDontEdit);
			v->SetType(i-1000);
			if (CMVariableTypes::IsAggSum(i)) v->SetState(CMVariable::vsSum,TRUE);
      	Add(v);
	   }
   }
}

void CMVariableCollection::ResetCollection()
{
	DestroyIfNotState(CMVariable::vsDontDestroy,0);
}

void CMVariableCollection::destroy_variables(ULONG aState,BOOL ontrue,int force)
{
	CMPHashDictionaryIterator<CMVariable> iter(this);
	CMVariable* v;
	CMVSmallArray<string> varnames;
	while ((v=iter())!=0) {
   	BOOL test = FALSE;
      ULONG vstate = v->GetState();
		if (!force && (vstate & CMVariable::vsInUse))
      	continue;
      if ( ((ontrue==TRUE) && (vstate & aState)) || ((ontrue==FALSE) && !(vstate & aState)) )
      	test = TRUE;
		if (test==TRUE)
			varnames.Add(v->GetName());
   }
	for (unsigned short i=0;i<varnames.Count();i++)
   	Detach(varnames[i],1);
}

int CMVariableCollection::DestroyVariable(const string& vname,BOOL bForce,BOOL bUpdate)
{
	CMVariable* v = Find(vname);
	if (v) {
		if (bForce || !(v->GetState() & CMVariable::vsInUse)) {
			Detach(vname,1);
	   	if (bUpdate) UpdateVariableLinks();
         return 1;
      }
   }
   return 0;
}

/*
CMVariable* CMVariableCollection::AddVariable(const string& vdef)
{
	wstrstream s((wchar_t*)vdef.c_str());
	CMVariable* v;
	s >> v;

   if (v) {
		if (Find(v->GetName())) {
      	delete v;
         v=0;
      }
      else {
	   	Add(v);
			UpdateVariableLinks();
	   }
   }
   return v;
}
*/

void CMVariableCollection::UpdateLinkStatus()
{
	SetStateAll(CMVariable::vsLinked,FALSE);
	CMPHashDictionaryIterator<CMVariable> iter(this);
	CMVariable* v;
	while ((v=iter())!=0)
   	if (v->GetState(CMVariable::vsSelected))
			v->UpdateLinkStatus();
}


void CMVariableCollection::UpdateVariableLinks()
{
	SetStateAll(CMVariable::vsLinksUpdated,FALSE);
	CMPHashDictionaryIterator<CMVariable> iter(this);
	CMVariable* v;
	while ((v=iter())!=0)
		v->UpdateVariableLinks();
}

void CMVariableCollection::UpdateVariableTypes()
{
	CMPHashDictionaryIterator<CMVariable> iter(this);
	CMVariable* v;
	string vtype,vname;
	while ((v=iter())!=0) {
		/* loops through all variable associations (e.g. #put put_variable), determines
      	whether the association refers to a variable type, and creates a new variable
         of that type if the named variable is not found.
      */
		if (state & sCreateSystemVariables) {
			create_demand_monitors(v);
			create_supply_monitors(v);
			create_transfer_monitors(v);
			create_storage_monitors(v);
      }
		for (int i=0;v->GetAssociation(i,vtype,vname);i++) {
			CMTokenizer next(vname);
         string token;
         while (!(token=stripends(next(L",;:{}()[]"))).is_null()) {
				size_t space_location = token.find(L' ');
				if (space_location != CM_NPOS) token = token.substr(0,space_location);
				if (!isnumber(token.c_str()) && CMVariableTypes::VarIntFromString(vtype.c_str(),1)>=0) {
	         	CMVariable* vfound = Find(token);
					if (!vfound && (state&sCreateSystemVariables)) {
						Add(vfound=new CMVariable(token,CMVariable::vsAutoCreated));
					CMNotifier::Notify(CMNotifier::WARNING, token + L" was automatically created");
               }
					vfound->SetType(vtype);
				}
         }
		}
	}
   UpdateVariableLinks();
}

CMVariable* CMVariableCollection::create_monitor_variable(CMVariable* v, const wchar_t* suffix, int typeno)
{
   string monitorname = v->GetName() + suffix;
	CMVariable* vfound = Find(monitorname);
   if (!vfound) Add(vfound=new CMVariable(monitorname,CMVariable::vsMonitor|CMVariable::vsDontEdit));
   vfound->SetType(typeno);
   return vfound;
}

void CMVariableCollection::create_demand_monitors(CMVariable* v)
{
	int base = CMVariableTypes::tDemand;
	if (v->IsType(L"demand")) {
		create_monitor_variable(v,L".evaluated",base+1);
		create_monitor_variable(v,L".resolved",base+2);
		create_monitor_variable(v,L".cutback",base+3);
   }
}

void CMVariableCollection::create_supply_monitors(CMVariable* v)
{
	int base = CMVariableTypes::tSupply;
	if (v->IsType(L"supply")) {
		create_monitor_variable(v,L".evaluated",base+1);
		create_monitor_variable(v,L".resolved",base+2);
		create_monitor_variable(v,L".cutback",base+3);
   }
}

void CMVariableCollection::create_transfer_monitors(CMVariable* v)
{
	int base = CMVariableTypes::tTransfer;
	if (v->IsType(L"transfer")) {
		create_monitor_variable(v,L".evaluated",base+1);
		create_monitor_variable(v,L".resolved",base+2);
		create_monitor_variable(v,L".cutback",base+3);
		create_monitor_variable(v,L".firstactivestep",base+4);
		create_monitor_variable(v,L".lastactivestep",base+5);
		create_monitor_variable(v,L".pulls",base+6);
   }
}

void CMVariableCollection::create_storage_monitors(CMVariable* v)
{
	int base = CMVariableTypes::tStorage;
	if (v->IsType(L"storage")) {
		create_monitor_variable(v,L".resolved",base+1);
		create_monitor_variable(v,L".gain",base+2);
		create_monitor_variable(v,L".loss",base+3);
		create_monitor_variable(v,L".put",base+4);
		create_monitor_variable(v,L".take",base+5);
		create_monitor_variable(v,L".shiftin",base+6);
		create_monitor_variable(v,L".shiftout",base+7);
		create_monitor_variable(v,L".seasonalshift",base+8);
		create_monitor_variable(v,L".carryover",base+9);
		create_monitor_variable(v,L".age",base+9);
   }
}

void CMVariableCollection::SetStateAll(ULONG aState,BOOL action)
{
	CMPHashDictionaryIterator<CMVariable> iter(this);
	CMVariable* v;
	while ((v=iter())!=0)
		v->SetState(aState,action);
}


