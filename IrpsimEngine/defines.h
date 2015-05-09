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
#include <stdlib.h>

class _IRPCLASS CMDefinition {
	CMString name;
	double value;
public:
	CMDefinition() : name() , value(0) {}
	CMDefinition(const CMString& n,const CMString& v) {name=stripends(n);value=_wtof(v.c_str());}
	CMDefinition(const CMString& n,double v) {name=stripends(n);value=v;}
	CMDefinition(const CMDefinition& o) : name(o.name) , value(o.value) {}
	CMDefinition& operator = (const CMDefinition& o) {name=o.name;value=o.value;return *this;}
	int operator == (const CMDefinition& o) {return name==o.name;}
	int operator < (const CMDefinition& o) {return name<o.name;}
	CMString& GetName() {return name;}
	double GetValue() {return value;}
};

class _IRPCLASS CMDefinitions
{
	static class _IRPCLASS CMPSmallArray<CMDefinition> definition_array;
   static CMDefinition* find(const CMString& n,unsigned short* loc=0);
public:
	static void Add(const CMString& n,const CMString& v) {definition_array.Add(new CMDefinition(n,v));}
	static double GetValue(const CMString& n) {CMDefinition* d = find(n);return d ? d->GetValue() : 0;}
	static int IsDefined(const CMString& n,unsigned short* loc=0) {return find(n,loc) ? 1 : 0;}
	static unsigned short GetIndex(const CMString& n) {unsigned short loc;find(n,&loc);return loc;}
   static void Reset() {definition_array.ResetAndDestroy(1);}
	static double GetDefinition(const CMString& n);
	static const wchar_t* GetDefinitionName(unsigned n);
	static double GetDefinitionValue(unsigned n);
};
