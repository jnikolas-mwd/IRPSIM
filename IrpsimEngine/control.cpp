// control.cpp : implementation file
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
#include "StdAfx.h"
#include "control.h"
#include "vtime.h"
#include "notify.h"

#include "token.h"
#include "cmdefs.h"

#include <limits.h>

//#include <fstream.h>
//static ofstream sdebug("control.deb");

// a return value if time limit has been exceeded
#define CM_TIME_LIMIT_EXCEEDED -10000

CMControlItem::CMControlItem(const CMString& def) :
controlvalue(0),
vinterval(0),
lastindex(-LONG_MAX),
limit(0),
amtused(0),
state(0)
{
	int flag = CMString::set_case_sensitive(0);
	CMTokenizer next(def);
	CMString token = next();
   controlvalue.Set(token);
	while (!(token=next()).is_null()) {
	   if (token==L"per") {
   			token=next();
			CMInterval* in = CMInterval::Find(token);
		    if (in==0)
			   CMNotifier::Notify(CMNotifier::ERROR, L": interval <" + token + L"> not found");
			else
         		vinterval = new CMInterval(*in);
	   }
      else if (token==L"dontrenew")
      	state |= sDontRenew;
   }
	CMString::set_case_sensitive(flag);
}

int operator == (const CMControlItem& c1,const CMControlItem& c2)
{
	if (!(c1.controlvalue==c2.controlvalue))
   	return 0;
   if (c1.vinterval==0 && c2.vinterval==0)
   	return 1;
   if ( (c1.vinterval==0 && c2.vinterval!=0) || (c1.vinterval!=0 && c2.vinterval==0) )
   	return 0;
	return (*c1.vinterval == *c2.vinterval);
}

void CMControlItem::Initialize(CMTimeMachine* t)
{
	if (t==0) {
   	lastindex=-LONG_MAX;
   	limit=amtused=0;
   }
   else if (vinterval) {
   	vinterval->Initialize(t);
		if (vinterval->IsFloatingStart()) {
	   	lastindex=-LONG_MAX;
   		limit=amtused=0;
      }
   }
}

// Returns the amount remaining under given the corresponding limit. If
// "dontrenew" has been set and the limit time has expired, return
// a negative number

double CMControlItem::AmountRemaining(CMTimeMachine* t)
{
	if (!vinterval) vinterval = new CMInterval(t);
   long index = vinterval->GetIndex(t);
	if (index != lastindex) {
		if ((state&sDontRenew) && lastindex!=-LONG_MAX)
	   	return CM_TIME_LIMIT_EXCEEDED;  // a hack for a negative number
   	lastindex = index;
      limit = LimitAmount(t);
      amtused=0;
   }
   return limit-amtused;
}

CMControl::CMControl(const CMString& def,double dv) :
default_value(dv)
{
	Set(def);
}

void CMControl::Set(const CMString& def)
{
	control_items.ResetAndDestroy(1);
	CMTokenizer next(def);
   CMString token;
   while (!(token=next(L";:,")).is_null())
   	control_items.Add(new CMControlItem(token));
}

CMControl::~CMControl()
{
	control_items.ResetAndDestroy(1);
}

void CMControl::Initialize(CMTimeMachine* t)
{
	for (unsigned short i=0;i<control_items.Count();i++)
   	control_items.At(i)->Initialize(t);
}

// double CMControl::LimitAmount(CMTimeMachine* t)
// Returns the minimum (i.e. most constraining)
// limit amount

double CMControl::LimitAmount(CMTimeMachine* t)
{
	if (!control_items.Count())
   	return default_value;
	double ret = CM_BIGDOUBLE;
	for (unsigned short i=0;i<control_items.Count();i++)
   	ret = cmmin(ret,control_items.At(i)->LimitAmount(t));
   return (ret>=0) ? ret : 0;
}

// double CMControl::AmountRemainingNow(CMTimeMachine* t)
// Returns the minimum amount amount remaining for all
// limits. Reflects conveyance constraints

double CMControl::AmountRemainingNow(CMTimeMachine* t)
{
	if (!control_items.Count())
   	return default_value;
	double ret = CM_BIGDOUBLE;
	for (unsigned short i=0;i<control_items.Count();i++)
   	ret = cmmin(ret,control_items.At(i)->AmountRemaining(t));
   return (ret>=0) ? ret : 0;
}

// double CMControl::AmountRemainingTotal(CMTimeMachine* t)
// Returns the maximum amount amount remaining for all
// limits. The idea is to ignore those limits which
// are conveyance constraints to get at the overarching
// limit

double CMControl::AmountRemainingTotal(CMTimeMachine* t)
{
	if (!control_items.Count())
   	return default_value;
	double ret = 0;
	for (unsigned short i=0;i<control_items.Count();i++) {
   	double itemremaining = control_items.At(i)->AmountRemaining(t);
      if (itemremaining == CM_TIME_LIMIT_EXCEEDED)
      	return 0;
   	ret = cmmax(ret,itemremaining);
   }
   return (ret>=0) ? ret : 0;
}

void CMControl::Add(double val)
{
	for (unsigned short i=0;i<control_items.Count();i++)
   	control_items.At(i)->Add(val);
}


