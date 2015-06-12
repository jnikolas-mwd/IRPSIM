// vsystem.h : header file
//
// Copyright © 1998-2015 Casey McSpadden   
//		mailto:casey@crossriver.com
//		http://www.crossriver.com/
//
// ==========================================================================  
// DESCRIPTION:	
// ==========================================================================
// CMVSystem is an IRPSIM "system" variable. Inherits from CMVariable.
// ==========================================================================
//
// ==========================================================================  
// HISTORY:	
// ==========================================================================
//			1.00	09 March 2015	- Initial re-write and release.
// ==========================================================================
#pragma once

#include "irp.h"
#include "variable.h"

class _IRPCLASS CMVSystem : public CMVariable
{
	static const wchar_t* varnames[];
	int system_type;
protected:
	virtual double evaluate(CMTimeMachine* t,int index1=0,int index2=0);
public:
	enum {First,PutPotential=First,TakePotential,TransferPotential,Trial,
         Last=Trial};
	CMVSystem(int aType);
	virtual string VariableType() {return GetEvalType();}
	static const wchar_t* GetEvalType() { return L"CMVSystem"; }
};



