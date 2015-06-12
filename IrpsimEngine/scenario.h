// scenario.h : header file
//
// Copyright © 1998-2015 Casey McSpadden   
//		mailto:casey@crossriver.com
//		http://www.crossriver.com/
//
// ==========================================================================  
// DESCRIPTION:	
// ==========================================================================
// CMScenario implements IRPSIM's concept of a simulation "scenario"
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
#include "options.h"
#include "irpobject.h"
#include "cmdefs.h"
#include "smvarray.h"
#include "smparray.h"
#include "string.h"

class _IRPCLASS CMScenario : public CMIrpObject
{
	class _IRPCLASS CMPSmallArray<CMOption> options;
	class _IRPCLASS CMVSmallArray<string> varnames;
	//class _IRPCLASS CMVSmallArray<int>    flags;
	int maxwidth;

protected:
	void AddEntry(const string& name, const string& value, int forceoption = 0);
	virtual wistream& read(wistream& is);
	virtual wostream& write(wostream& os);

public:
	virtual const wchar_t* IsA() { return L"CMScenario"; };
	//enum { SaveFlag = 0x01, WriteFlag = 0x02 };
	CMScenario(int id=-1) : CMIrpObject(id),
		options() , varnames(), maxwidth(0) {}
	CMScenario(const string& aName,int id = -1) : CMIrpObject(aName,id),
		options() , varnames(), maxwidth(0) {}
	~CMScenario();
	void Use(CMOptions& op);
	unsigned short Variables() {return varnames.Count();}
	string VariableName(unsigned short n) {return varnames[n];}
    //int Flags(unsigned short n) {return flags[n];}
};
