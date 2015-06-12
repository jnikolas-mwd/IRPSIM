// vtimevar.cpp : implementation file
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
#include "StdAfx.h"
#include "vtimevar.h"

#include "token.h"

//#include <fstream.h>
//static ofstream vdebug("vtimevar.deb");

CMVTimevar::CMVTimevar(const CMString& aName) :
CMVariable(aName),
duration_unit(CM_MONTH),
duration_length(12),
intervalstart(L"201601"),
vtimevar_state(0)
{
}

CMVTimevar::~CMVTimevar()
{
}

double CMVTimevar::evaluate(CMTimeMachine* t,int,int)
{
	return (double)(int)((double)CMTime::Diff(*t,intervalstart,duration_unit,1)/duration_length);
}


void CMVTimevar::update_variable_links()
{
	static const wchar_t* delims = L" ,;:\t\r\n";
	CMString period = GetAssociation(L"period");
   CMTokenizer next(period);
   CMString token = next(delims);
   if (token==L"float") vtimevar_state |= vtFloatingStart;
	else intervalstart = token;
	duration_length = _wtoi(((CMString)next(delims)).c_str());
   duration_unit = CMTime::StringToTimeUnit(next(delims));
   if (duration_length<1) duration_length = 1;
}

void CMVTimevar::SetSpecialValues(CMTimeMachine* t,double ,double ,double ,double )
{
	if (vtimevar_state&vtFloatingStart)	intervalstart = *t;
}

