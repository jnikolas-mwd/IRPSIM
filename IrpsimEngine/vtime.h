// vtime.h : header file
//
// Copyright © 1998-2015 Casey McSpadden   
//		mailto:casey@crossriver.com
//		http://www.crossriver.com/
//
// ==========================================================================  
// DESCRIPTION:	
// ==========================================================================
// CMVTime implements internal IRPSIM variables that vary by time. 
// Inherits from CMVariable.
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

class _IRPCLASS CMVTime : public CMVariable
{
	static wchar_t* varnames[];
	int time_type;
protected:
	virtual double evaluate(CMTimeMachine* t,int index1=0,int index2=0);
public:
	enum {First,Timestep=First,Year,CYear,TYear,CTYear,Period,TPeriod,Month,Day,WeekDay,
			Hour,Minute,Second,DayInYear,DaysInYear,DaysInMonth,WeekInYear,
         FirstDayOfMonth,LastDayOfMonth,
         Jan,Feb,Mar,Apr,May,Jun,Jul,Aug,Sep,Oct,Nov,Dec,
         MidMonthDay,
         Sin1,Sin2,Sin3,Sin4,Sin5,Sin6,Cos1,Cos2,Cos3,Cos4,Cos5,Cos6,
         Sin1D,Sin2D,Sin3D,Sin4D,Sin5D,Sin6D,Cos1D,Cos2D,Cos3D,Cos4D,Cos5D,Cos6D,
         Last=Cos6D};
	CMVTime(int aType);
	virtual string VariableType() {return GetEvalType();}
	static const wchar_t* GetEvalType() { return L"CMVTime"; }
};
