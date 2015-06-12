// interval.h : header file
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
#pragma once

#include "irpobject.h"
#include "timemach.h"

#include "cmstring.h"
#include "smparray.h"
#include <iostream>
using namespace std;


class CMInterval;

class _IRPCLASS CMInterval : public CMIrpObject
{
   static class _IRPCLASS CMPSmallArray<CMInterval> intervals;
   static const wchar_t* footer;

	CMTIMEUNIT duration_unit;
   int duration_length;
   CMTime intervalstart;
   int state;
   enum {sFloatingStart=0x0001};

	virtual int is_equal_to(const CMIrpObject& o2) const;
	virtual const wchar_t* IsA();
public:
	CMInterval(const CMString& def);
	CMInterval(CMTimeMachine* t);
	CMInterval(const CMInterval& in) {operator = (in);}
	void Initialize(CMTimeMachine* t);
   long GetIndex(CMTimeMachine* t);
   int IsFloatingStart() {return (state&sFloatingStart);}
   CMInterval& operator = (const CMInterval& in);

   static void Reset();
   static CMInterval* AddInterval(const CMString& def);
   static CMInterval* Find(const CMString& aname);
	static CMInterval* GetInterval(unsigned short n);
	static unsigned short NumIntervals();
	static void SetApplicationIdAll(int id);
	static void SetApplicationIndexAll(long index);
   static wistream& Read(wistream& s);
};

