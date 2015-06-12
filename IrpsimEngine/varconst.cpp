// varconst.cpp : implementation file
//
// Copyright © 1998-2015 Casey McSpadden   
//		mailto:casey@crossriver.com
//		http://www.crossriver.com/
//
// ==========================================================================  
// DESCRIPTION:	
// ==========================================================================
// CMVariableOrConstant is an optimization class used to accelerate the
// evaluation of a variable if it is a simple constant.
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
#include <stdlib.h>

#include "varconst.h"
#include "variable.h"
#include "timemach.h"
#include "cmlib.h"

CMVariableOrConstant::CMVariableOrConstant(double val) :
type(Null)
{
	Set(val);
}

CMVariableOrConstant::CMVariableOrConstant(const CMString& str) :
type(Null)
{
	Set(str);
}

CMVariableOrConstant::CMVariableOrConstant(const CMVariableOrConstant& vc)
{
	type = vc.type;
   if (type==Constant)
   	u.c = vc.u.c;
   else if (type==Variable)
   	u.v = vc.u.v;
}

void CMVariableOrConstant::Set(const CMString& str)
{
	if (str.is_null())
   	type = Null;
   else {
   	if (isnumber(str.c_str())) {
      	type = Constant;
         u.c = _wtof(str.c_str());
      }
      else {
      	u.v = CMVariable::Find(str);
         type = (u.v==0) ? Null : Variable;
      }
   }
}

void CMVariableOrConstant::Set(double val)
{
	type = Constant;
   u.c = val;
}

double CMVariableOrConstant::GetValue(CMTimeMachine* t,double defaultvalue,int force_evaluation) const
{
	if (type == Constant)
		return u.c;
   else if (type == Variable)
		return u.v->GetValue(t,1,0,0,1);
	return defaultvalue;
}


void CMVariableOrConstant::SetValue(CMTimeMachine* t,double amt)
{
	if (type==Variable && u.v!=0)
   	u.v->SetValue(t,amt);
}


void CMVariableOrConstant::AddTo(CMTimeMachine* t,double amt)
{
	if (type==Variable && u.v!=0)
   	u.v->AddTo(t,amt);
}

