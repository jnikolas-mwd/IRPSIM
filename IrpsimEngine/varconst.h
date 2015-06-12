// varconst.h : header file
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
#pragma once

#include "irp.h"
#include "cmdefs.h"
#include "string.h"

class _IRPCLASS CMVariable;
class _IRPCLASS CMTimeMachine;

class _IRPCLASS CMVariableOrConstant 
{
	enum {Constant,Variable,Null};
   friend int operator == (const CMVariableOrConstant& vc1,const CMVariableOrConstant& vc2);

	BYTE type;
	union {
		double c;
		CMVariable* v;
	} u;

public:
	CMVariableOrConstant(double val=0);
	CMVariableOrConstant(const string& str);
	CMVariableOrConstant(const CMVariableOrConstant& vc);
	void 	 Set(const string& str); // str may be a number or a variable name
	void 	 Set(double val);
	double GetValue(CMTimeMachine* t,double defaultvalue,int force_evaluation=0) const;
	void   SetValue(CMTimeMachine* t,double amt);
	void   AddTo(CMTimeMachine* t,double amt);
   int	 IsNull() const {return (type==Null);}
};

inline int operator == (const CMVariableOrConstant& vc1,const CMVariableOrConstant& vc2)
{
	return ((vc1.type==CMVariableOrConstant::Null && vc2.type==CMVariableOrConstant::Null) ||
			  (vc1.type==CMVariableOrConstant::Constant && vc2.type==CMVariableOrConstant::Constant &&
            vc1.u.c == vc2.u.c) ||
			  (vc1.type==CMVariableOrConstant::Variable && vc2.type==CMVariableOrConstant::Variable &&
            vc1.u.v == vc2.u.v));
}
