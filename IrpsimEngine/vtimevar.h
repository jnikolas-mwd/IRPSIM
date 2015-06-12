// vtimevar.h : header file
//
// Copyright © 1998-2015 Casey McSpadden   
//		mailto:casey@crossriver.com
//		http://www.crossriver.com/
//
// ==========================================================================  
// DESCRIPTION:	
// ==========================================================================
// CMVTimevar evaluates as the number of intervals from the start of the
// simulation.
// Inherits from CMVariable.
// ==========================================================================
//
// ==========================================================================  
// HISTORY:	
// ==========================================================================
//			1.00	09 March 2015	- Initial re-write and release.
// ==========================================================================
#pragma once

#include "variable.h"

class _IRPCLASS CMVTimevar : public CMVariable
{
	CMTIMEUNIT duration_unit;
   int duration_length;
   CMTime intervalstart;
   int vtimevar_state;
   enum {vtFloatingStart=0x0001};
protected:
	virtual double evaluate(CMTimeMachine* t,int index1=0,int index2=0);
	virtual void update_variable_links();
public:
	CMVTimevar(const CMString& aName);
	~CMVTimevar();
	virtual CMString VariableType() {return GetEvalType();}
    virtual void SetSpecialValues(CMTimeMachine* t,double v1=0,double v2=0,double v3=0,double v4=0);
	static const wchar_t* GetEvalType() {return L"CMVTimevar";}
};

