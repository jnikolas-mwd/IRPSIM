// control.h : header file
//
// Copyright © 1998-2015 Casey McSpadden   
//		mailto:casey@crossriver.com
//		http://www.crossriver.com/
//
// ==========================================================================  
// DESCRIPTION:	
// ==========================================================================
// CMControl implements a control (e.g. MaxPut) placed on a node
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

#include "interval.h"
#include "varconst.h"
#include "timemach.h"

#include "smparray.h"

class _IRPCLASS CMControlItem
{
	friend int _IRPFUNC operator == (const CMControlItem& c1,const CMControlItem& c2);
	CMVariableOrConstant controlvalue;
   CMInterval* vinterval;
   long lastindex;
   double limit,amtused;
   int state;
   enum {sDontRenew=0x0001};
public:
	CMControlItem(const CMString& def);
	~CMControlItem() { if (vinterval) delete vinterval; }
   void Initialize(CMTimeMachine* t);
   double LimitAmount(CMTimeMachine* t)
   	{return controlvalue.GetValue(t,CM_BIGDOUBLE,1);}
   double AmountRemaining(CMTimeMachine* t);
   void Add(double amt) {amtused+=amt;}
};

class _IRPCLASS CMControl
{
	class _IRPCLASS CMPSmallArray<CMControlItem> control_items;
   double default_value;
public:
	CMControl(const CMString& def,double dv);
	CMControl(double dv) : control_items() , default_value(dv) {}
	~CMControl();
   void Set(const CMString& def);
   void Initialize(CMTimeMachine* t);
	double LimitAmount(CMTimeMachine* t);
   double AmountRemainingNow(CMTimeMachine* t);
   double AmountRemainingTotal(CMTimeMachine* t);
   void Add(double val);
};

