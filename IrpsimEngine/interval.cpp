// interval.cpp : implementation file
//
// Copyright © 1998-2015 Casey McSpadden   
//		mailto:casey@crossriver.com
//		http://www.crossriver.com/
//
// ==========================================================================  
// DESCRIPTION:	
// ==========================================================================
// CMInterval class implements IRPSIM's interval mechanism
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
#include "interval.h"

#include "token.h"
#include "cmlib.h"

#include <stdlib.h>
//#include <iostream.h>
#include <iostream>
using namespace std;

//#include <fstream.h>
//static ofstream sdebug("interval.deb");

CMPSmallArray<CMInterval> CMInterval::intervals;
const wchar_t* CMInterval::footer = L"#END";

const wchar_t* CMInterval::IsA() { return L"CMInterval"; }

CMInterval::CMInterval(const string& def) :
CMIrpObject(),
duration_unit(CM_MONTH),
duration_length(12),
intervalstart(L"201501"),
state(0)
{
	static const wchar_t* delims = L" ,;:\t\r\n";
   CMTokenizer next(def);
   string token;
   name = next(delims);
	token = next(delims);
   if (token==L"float") state |= sFloatingStart;
	else intervalstart = token;
	duration_length = _wtoi(((string)next(delims)).c_str());
   duration_unit = CMTime::StringToTimeUnit(next(delims));
   if (duration_length<1) duration_length = 1;
}

CMInterval::CMInterval(CMTimeMachine* t) :
CMIrpObject(),
duration_unit(t->IncUnits()),
duration_length(t->IncLength()),
intervalstart(t->At(0).Begin()),
state(0)
{
}

CMInterval& CMInterval::operator = (const CMInterval& in)
{
	name = in.name;
	duration_unit = in.duration_unit;
	duration_length = in.duration_length;
	intervalstart = in.intervalstart;
	state = in.state;
   return *this;
}

void CMInterval::Initialize(CMTimeMachine* t)
{
	if (state&sFloatingStart) intervalstart = *t;
}

long CMInterval::GetIndex(CMTimeMachine* t)
{
	return (long)((double)CMTime::Diff(*t,intervalstart,duration_unit,1)/duration_length);
}


void CMInterval::Reset()
{
	intervals.ResetAndDestroy(1);
}

CMInterval* CMInterval::AddInterval(const string& def)
{
	CMInterval* ret = new CMInterval(def);
	intervals.Add(ret);
   return ret;
}

CMInterval* CMInterval::Find(const string& aname)
{
	for (unsigned short i=0;i<intervals.Count();i++)
   	if (aname==intervals[i]->GetName())
      	return intervals[i];
   return 0;
}

CMInterval* CMInterval::GetInterval(unsigned short n)
{
	return (n<intervals.Count() ? intervals[n] : NULL);
}

unsigned short CMInterval::NumIntervals()
{
	return intervals.Count();
}

void CMInterval::SetApplicationIdAll(int id)
{
	for (unsigned short i=0;i<intervals.Count();i++)
		intervals[i]->SetApplicationId(id);
}

void CMInterval::SetApplicationIndexAll(long index)
{
	for (unsigned short i = 0; i<intervals.Count(); i++)
		intervals[i]->SetApplicationIndex(index);
}

wistream& CMInterval::Read(wistream& s)
{
	string line;
	while (!s.eof()) {
		line.read_line(s);
   	line = stripends(line);
		if (line.is_null() || line[0]== L'*')
      	continue;
		if (line(0,wcslen(footer)) == footer)
   		break;
		AddInterval(line);
   }
   return s;
}

int CMInterval::is_equal_to(const CMIrpObject& o2) const
{
	CMInterval& i2 = (CMInterval&) o2;
	return (name==i2.name && duration_unit==i2.duration_unit && duration_length==i2.duration_length);
}

