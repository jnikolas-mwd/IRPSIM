// varcol.h : header file
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
#pragma once

#include "irp.h"
#include "variable.h"

#include "cmstring.h"
#include "cmdefs.h"
#include "smvarray.h"
#include "phashdic.h"

typedef class _IRPCLASS CMPHashDictionary<CMVariable> CMPVARIABLEHASHDICTIONARY;
typedef class _IRPCLASS CMPHashDictionaryIterator<CMVariable> CMPVARIABLEHASHDICTIONARYITERATOR;

class _IRPCLASS CMVariableCollection : public CMPVARIABLEHASHDICTIONARY
{
	void destroy_variables(ULONG aState,BOOL ontrue,int force);
	CMVariable* create_monitor_variable(CMVariable* v, const wchar_t* suffix, int typeno);
	void create_demand_monitors(CMVariable* v);
	void create_supply_monitors(CMVariable* v);
	void create_transfer_monitors(CMVariable* v);
	void create_storage_monitors(CMVariable* v);
   int state;
   enum {sCreateSystemVariables=0x0001};
public:
	CMVariableCollection(int create_system_variables);
   void DestroyIfState(ULONG aState,int force) {destroy_variables(aState,TRUE,force);}
   void DestroyIfNotState(ULONG aState,int force) {destroy_variables(aState,FALSE,force);}
	void UpdateVariableTypes();
   void UpdateVariableLinks();
	void UpdateLinkStatus();
	void SetStateAll(ULONG aState,BOOL action);

   void ResetCollection();

   int DestroyVariable(const CMString& vdef,BOOL bForce,BOOL bUpdate=TRUE);
   //CMVariable* AddVariable(const CMString& vdef);
};

class _IRPCLASS CMVariableIterator
{
	CMPVARIABLEHASHDICTIONARYITERATOR iter;
public:
	CMVariableIterator();
	CMVariableIterator(CMVariableCollection* v);
	void Reset() {iter.Reset();}
	CMVariable* operator ()() {return iter();}
};

