// defines.h : header file
//
// Copyright © 1998-2015 Casey McSpadden   
//		mailto:casey@crossriver.com
//		http://www.crossriver.com/
//
// ==========================================================================  
// DESCRIPTION:	
// ==========================================================================
// CMDefine implements a user-defined value (string key, double value)
// CMDefines implements a dictionary of CMDefine members
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

#include "cmdefs.h"
#include "smparray.h"
#include "cmlib.h"
#include "irpobject.h"
#include <stdlib.h>

class _IRPCLASS CMDefinition : public CMIrpObject {
	//CMString name;
	double value;
public:
	CMDefinition() : CMIrpObject() , value(0) {}
	CMDefinition(const CMString& n,const CMString& v) : CMIrpObject(n) {value=_wtof(v.c_str());}
	CMDefinition(const CMString& n, double v) : CMIrpObject(n), value(v) {}
	CMDefinition(const CMString& n, const CMString& v, int app_id) : CMDefinition(n,v) { this->app_id = app_id; }
	CMDefinition(const CMDefinition& o) : value(o.value) { name = o.name;  app_id = o.app_id; }
	CMDefinition& operator = (const CMDefinition& o) { name = o.name; value = o.value; app_id = o.app_id; return *this; }
	//int operator == (const CMDefinition& o) {return name==o.name;}
	//int operator < (const CMDefinition& o) {return name<o.name;}
	//CMString& GetName() {return name;}
	double GetValue() {return value;}
protected:
	virtual const wchar_t* IsA() { return L"CMDefinition"; }
};

class _IRPCLASS CMDefinitions
{
   static class _IRPCLASS CMPSmallArray<CMDefinition> definition_array;
public:
	static CMDefinition* Find(const CMString& n, unsigned short* loc = 0);
	static void Add(const CMString& n,const CMString& v, int app_id) {definition_array.Add(new CMDefinition(n,v,app_id));}
	static double GetValue(const CMString& n) {CMDefinition* d = Find(n);return d ? d->GetValue() : 0;}
	static int IsDefined(const CMString& n,unsigned short* loc=0) {return Find(n,loc) ? 1 : 0;}
	static unsigned short GetIndex(const CMString& n) {unsigned short loc;Find(n,&loc);return loc;}
    static void Reset() {definition_array.ResetAndDestroy(1);}
	static double GetDefinition(const CMString& n);
	static CMDefinition* GetDefinition(unsigned n);
	static const wchar_t* GetDefinitionName(unsigned n);
	static double GetDefinitionValue(unsigned n);
};
