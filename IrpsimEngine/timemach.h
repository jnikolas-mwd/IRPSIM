// timemach.h : header file
//
// Copyright © 1998-2015 Casey McSpadden   
//		mailto:casey@crossriver.com
//		http://www.crossriver.com/
//
// ==========================================================================  
// DESCRIPTION:	
// ==========================================================================
// Implements CMTimeMachine class for handling simulation time steps and
// hydro;ogy traces
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
#include "cmtime.h"
#include "string.h"
#include "smvarray.h"

class _IRPCLASS CMTimeCycle
{
private:
	friend int operator == (const CMTimeCycle& t1,const CMTimeCycle& t2);
	CMTime current;		// current time
	CMTime beg;          // beginning of cycle
	CMTime end;          // ending of cycle
	CMTime start;        // start point of cycle
	CMTime curstart;     // current cycle start
	CMTIMEUNIT startincrement;
public:
	CMTimeCycle();
	CMTimeCycle(const CMTime& b,const CMTime& e,const CMTime& s,CMTIMEUNIT si=CM_NOTIMEUNIT);
	CMTimeCycle(const CMTime& b,const CMTime& e,CMTIMEUNIT si=CM_NOTIMEUNIT);
	CMTimeCycle(const CMTimeCycle& t);

	void Begin(CMTime& t) {beg = t;}
	void End(CMTime& t) {end = t;}
	void Start(CMTime& t) {start = t;}
	operator CMTime& () {return current;}
	CMTimeCycle& operator = (const CMTimeCycle& t);

	CMTime& Begin() {return beg;}
	CMTime& End()  {return end;}
	CMTime& Start() {return start;}
	CMTime& CurStart() {return curstart;}
	CMTime& Current() {return current;}

	void Reset() {current=curstart=start;} // resets cycle
	void Rewind();                         // rewinds and increments start if dynamic
	void Offset(short n,CMTIMEUNIT units);
	void Step(short n,CMTIMEUNIT units);
	wostream& Write(wostream& os) const {return current.Write(os);}
	wostream& WriteBinary(wostream& os) const;
	wistream& ReadBinary(wistream& is);

	static const short BinarySize() {return 5*CMTime::BinarySize() + (short)sizeof(short);}
};

inline wostream& operator << (wostream& s , const CMTimeCycle& t)
{
	return t.Write(s);
}

class _IRPCLASS CMTimeMachine
{
private:
	class _IRPCLASS CMVSmallArray<CMTimeCycle> cycles;
	ULONG count;
	CMTIMEUNIT incunits;
	short inclength;
	CMTime now;
	static int get_resolution(const wchar_t* str);
	void add_cycle_at(unsigned short n,const CMTime& b,const CMTime& e,const CMTime& s,CMTIMEUNIT si);
public:
	CMTimeMachine(CMTIMEUNIT units,short length);
	CMTimeMachine(const CMTimeMachine& t);

	CMTimeMachine& operator = (const CMTimeMachine& t);

	CMTIMEUNIT IncUnits() const {return incunits;}
	short IncLength() const {return inclength;}

	const CMTime& Now() {return now;}
	unsigned short Cycles() const {return cycles.Count();}
	ULONG Count() const {return count;}
	operator CMTime& () const {return cycles[0];}

	string CycleAsString(unsigned n) const;
	CMTimeCycle& At(unsigned short n) const {return cycles.At(n);}
	CMTimeCycle& operator [] (unsigned short n) const {return cycles.At(n);}
	void AddCycle(const wchar_t* b, const wchar_t* e, const wchar_t* s = 0, int lastmonth = 12, CMTIMEUNIT si = CM_NOTIMEUNIT);
	void AddCycle(const CMTime& b,const CMTime& e,const CMTime& s,CMTIMEUNIT si=CM_NOTIMEUNIT)
		{add_cycle_at(Cycles(),b,e,s,si);}
	void AddCycle(const CMTime& b,const CMTime& e,CMTIMEUNIT si=CM_NOTIMEUNIT)
		{add_cycle_at(Cycles(),b,e,b,si);}
//	long StartPoints(unsigned short n) {return cycles[n].StartPoints();}
	long TimeSteps(unsigned short n) const {return 1+CMTime::Diff(cycles[n].End(),cycles[n].Begin(),incunits,inclength);}
	long TimeStep() const {return 1+CMTime::Diff(now,cycles[0].Begin(),incunits,inclength);}
	void Reset();
	void Offset(short n,CMTIMEUNIT units);
	void Step();
	int  AtBeginning() {return cycles.At(0).Current()==cycles.At(0).Begin();}
	short SetOutputFormat();

	wostream& Write(wostream& os) const;
	wostream& WriteBinary(wostream& os) const;
	wistream& ReadBinary(wistream& is);

	short BinarySize() const
		{return (short)sizeof(long)+2*(short)sizeof(short)+(short)sizeof(CMTIMEUNIT)+CMTime::BinarySize()+cycles.Count()*CMTimeCycle::BinarySize();}
};

inline wostream& operator << (wostream& s , const CMTimeMachine& t)
{
	return t.Write(s);
}
