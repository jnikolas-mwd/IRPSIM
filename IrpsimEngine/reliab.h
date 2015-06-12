// reliab.h : header file
//
// Copyright © 1998-2015 Casey McSpadden   
//		mailto:casey@crossriver.com
//		http://www.crossriver.com/
//
// ==========================================================================  
// DESCRIPTION:	
// ==========================================================================
// CMReliability implements IRPSIM's system reliability measures.
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
#include "smparray.h"
#include "bgvarray.h"
#include "cmtime.h"

#include <iostream>
using namespace std;

class _IRPCLASS CMVariable;
class _IRPCLASS CMTimeMachine;

class _IRPCLASS CMReliabilityTarget
{
	string shortagevarname, percentagevarname;
	CMVariable *shortagevar, *percentagevar;
	double shortage_target,percent_target;
	short state;
	enum  {isPercent=0x0001,changeSignShortage=0x0002,changeSignDenom=0x0004};
public:
	CMReliabilityTarget();
	CMReliabilityTarget(const string& def);
	CMReliabilityTarget& operator = (const CMReliabilityTarget& t)
		{shortagevarname=t.shortagevarname;percentagevarname=t.percentagevarname;
		 shortagevar=t.shortagevar;percentagevar=t.percentagevar;
		 shortage_target=t.shortage_target;percent_target=t.percent_target;
		 state = t.state;return *this;}
	int operator == (const CMReliabilityTarget& t) const
		{return (shortagevarname==t.shortagevarname && percentagevarname==t.percentagevarname &&
					shortage_target==t.shortage_target && percent_target==t.percent_target &&
					state==t.state);}
	void Set(const string& def);
	int  Process(CMTimeMachine* t);
	string ShortageVariable() const {return shortagevarname;}
	string PercentageVariable() const {return percentagevarname;}
	short IsPercent() const {return (state&isPercent);}
	short ShortageSign() const {return (state&changeSignShortage) ? -1 : 1;}
	short PercentSign()  const {return (state&changeSignDenom) ? -1 : 1;}
	double ShortageTarget() const {return shortage_target;}
	double PercentTarget() const {return percent_target;}
	int SameCurveAs(const CMReliabilityTarget& t);
	string GetString();
	wostream& WriteBinary(wostream& s);
	wistream& ReadBinary(wistream& s);
	int BinarySize();
};

/*
inline ostream& operator << (ostream& s,CMReliabilityTarget& r)
{return r.WriteBinary(s);}

inline istream& operator >> (istream& s,CMReliabilityTarget& r)
{return r.ReadBinary(s);}

inline int operator < (const CMReliabilityTarget& t1,const CMReliabilityTarget& t2)
{
	return (t1.Shortage()==t2.Shortage()) ? (t1.TargetPct()<t2.TargetPct()) : (t1.Shortage()<t2.Shortage());
}
*/

class _IRPCLASS CMReliability
{
	long count;
	class _IRPCLASS CMPSmallArray<CMReliabilityTarget> targets;
	class _IRPCLASS CMVBigArray<long>* hits;

	CMTIMEUNIT incunits;	   // e.g. CMTime::months, CMTime::years
	short inclength;			// e.g. # of months, # of years in increment
	CMTime beg;          // beginning of period
	CMTime end;          // end of period
	unsigned short ntargets;     // number of targets being recorded
	long	 periodlength; // length of period (e.g. # of months)
	short  state;

	long index(const CMTime& t,unsigned aTarget);
	enum {rsFail=0x0001};
public:
	CMReliability(const CMTimeMachine& t,const string& def);
	CMReliability();
	~CMReliability();
	short Fail() {return (state&rsFail);}

	void Reset();
	void Process(CMTimeMachine* t);

	long Trials() {return count/(periodlength*ntargets);}
	unsigned Targets() {return ntargets;}

	CMTime BeginPeriod() const {return beg;}
	CMTime EndPeriod() const {return end;}
	long TimeSteps() const {return periodlength;}
	long GetPeriod(CMTime& b,CMTime& e,CMTIMEUNIT& units,short& length) const;

	long Success(const CMTime& t,unsigned aTarget);
	double SuccessPct(const CMTime& t,unsigned aTarget);
	double FailPct(const CMTime& t,unsigned aTarget) {return 100-SuccessPct(t,aTarget);}
	CMReliabilityTarget* Target(unsigned short n) {return targets.At(n);}
	wostream& WriteBinary(wostream& s);
	wistream& ReadBinary(wistream& s);
	long BinarySize();
};

/*
inline ostream& operator << (ostream& s,CMReliability& r)
{return r.WriteBinary(s);}

inline istream& operator >> (istream& s,CMReliability& r)
{return r.ReadBinary(s);}
*/
