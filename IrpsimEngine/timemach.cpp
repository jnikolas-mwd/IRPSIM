// timemach.cpp : implementation file
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
#include "StdAfx.h"
#include "timemach.h"
#include <string.h>
#include <stdio.h>

//#include <fstream.h>
//#include <iomanip.h>
//static ofstream sdebug("timemach.deb");

CMTimeCycle::CMTimeCycle() :
current(),
beg(),
end(),
start(),
curstart(),
startincrement(CM_NOTIMEUNIT)
{
}

CMTimeCycle::CMTimeCycle(const CMTime& b,const CMTime& e,const CMTime& s,CMTIMEUNIT se) :
beg(b),
end(e),
start(s),
current(s),
curstart(s),
startincrement(se)
{
}

CMTimeCycle::CMTimeCycle(const CMTime& b,const CMTime& e,CMTIMEUNIT se) :
beg(b),
end(e),
start(b),
current(b),
curstart(b),
startincrement(se)
{
}

CMTimeCycle::CMTimeCycle(const CMTimeCycle& t) :
beg(t.beg),
end(t.end),
start(t.start),
current(t.current),
curstart(t.curstart),
startincrement(t.startincrement)
{
}

CMTimeCycle& CMTimeCycle::operator = (const CMTimeCycle& t)
{
	beg = t.beg;
	end = t.end;
	start = t.start;
	curstart = t.curstart;
	current = t.current;
	startincrement = t.startincrement;
	return *this;
}

void CMTimeCycle::Offset(short n,CMTIMEUNIT units)
{
	current.inc(n,units);
}

void CMTimeCycle::Step(short n,CMTIMEUNIT units)
{
	if (current.inc(n,units) > end)
		current = beg;
}

void CMTimeCycle::Rewind()
{
	if (startincrement!=CM_NOTIMEUNIT) {
		curstart.inc(1,startincrement);
		if (curstart > end)
			curstart = beg;
	}
	current = curstart;
}

wostream& CMTimeCycle::WriteBinary(wostream& s) const
{
	current.Write(s,1);
	beg.Write(s,1);
	end.Write(s,1);
	start.Write(s,1);
	curstart.Write(s,1);
	//s.write((const unsigned char*)&startincrement,sizeof(startincrement));
	s.write((const wchar_t*)&startincrement, sizeof(startincrement));
	return s;
}

wistream& CMTimeCycle::ReadBinary(wistream& s)
{
	current.Read(s,1);
	beg.Read(s,1);
	end.Read(s,1);
	start.Read(s,1);
	curstart.Read(s,1);
	s.read((wchar_t*)&startincrement, sizeof(startincrement));
	return s;
}

int operator == (const CMTimeCycle& t1,const CMTimeCycle& t2)
{
	return (t1.beg==t2.beg && t1.end==t2.end && t1.start==t2.start &&
			  t1.curstart==t2.curstart && t1.current==t2.current
			  && t1.startincrement==t2.startincrement);
}

CMTimeMachine::CMTimeMachine(CMTIMEUNIT units,short length) :
cycles(),
count(0),
incunits(units),
inclength(length),
now()
{
}

CMTimeMachine::CMTimeMachine(const CMTimeMachine& t) :
count(t.count),
incunits(t.incunits),
inclength(t.inclength),
now(t.now)
{
	cycles=t.cycles;
}

CMTimeMachine& CMTimeMachine::operator = (const CMTimeMachine& t)
{
	cycles=t.cycles;count=t.count;now=t.now;
   incunits=t.incunits;inclength=t.inclength;
	return *this;
}

string CMTimeMachine::CycleAsString(unsigned n) const
{
	string ret;
	wchar_t buffer[64];
   static wchar_t *unitstr[] = { L"seconds", L"minutes", L"hours", L"days", L"weeks", L"months", L"years" };
   const wchar_t* unitptr = unitstr[6];
   if (n>=cycles.Count())
   	return ret;

   short oldformat;

	switch (incunits) {
		case CM_SECOND:
      	oldformat = CMTime::SetOutputFormat(CMTime::YYYYMMDDHHMMSS);
         unitptr = unitstr[0];
         break;
		case CM_MINUTE:
      	oldformat = CMTime::SetOutputFormat(CMTime::YYYYMMDDHHMM);
         unitptr = unitstr[1];
         break;
		case CM_HOUR:
      	oldformat = CMTime::SetOutputFormat(CMTime::YYYYMMDDHH);
         unitptr = unitstr[2];
         break;
		case CM_DAY:
      	oldformat = CMTime::SetOutputFormat(CMTime::YYYYMMDD);
         unitptr = unitstr[3];
         break;
		case CM_WEEK:
	      oldformat = CMTime::SetOutputFormat(CMTime::YYYYMMDD);
         unitptr = unitstr[4];
         break;
		case CM_MONTH:
      	oldformat = CMTime::SetOutputFormat(CMTime::YYYYMM);
         unitptr = unitstr[5];
         break;
		case CM_YEAR:
      	oldformat = CMTime::SetOutputFormat(CMTime::YYYY);
         unitptr = unitstr[6];
      	break;
   }

	ret = cycles[n].Begin().GetString() + L" to " + cycles[n].End().GetString();

	swprintf_s(buffer, 64, L" : %ld %s",TimeSteps(n),unitptr);

   ret += buffer;

	CMTime::SetOutputFormat(oldformat);
	return ret;
}

void CMTimeMachine::add_cycle_at(unsigned short n,const CMTime& b,const CMTime& e,const CMTime& s,CMTIMEUNIT si)
{
	CMTime beg(b),end(e),start(s);
   long diff = CMTime::Diff(end,beg,incunits,inclength);
	end = beg.Plus(diff*inclength,incunits);
	if (n==0)
	   now = beg;
   else {
		diff = CMTime::Diff(start,beg,incunits,inclength);
		start = beg.Plus(diff*inclength,incunits);
   }
	cycles.AddAt(n,CMTimeCycle(beg,end,start,si));
}

void CMTimeMachine::AddCycle(const wchar_t* b, const wchar_t* e, const wchar_t* s, int lastmonth, CMTIMEUNIT si)
{
	CMTime beg(b),end(e),start(s);
   int begweek = beg.WeekDay();

	if (CMTime::GetResolution(e)==CM_YEAR) {
		end = CMTime(end.Year(),lastmonth);
		end = end.AtEnd(CM_MONTH);
   	if (incunits==CM_YEAR)
      	end = end.AtBeginning(CM_MONTH);
      else
	      end = end.AtBeginning(incunits,begweek);
   }
	else {
		end = end.AtEnd(CMTime::GetResolution(e));
	   end = end.AtBeginning(incunits,begweek);
   }

	if (CMTime::GetResolution(b)==CM_YEAR) {
   	if (incunits==CM_YEAR)
      	beg = CMTime(beg.Year(),lastmonth).AtBeginning(CM_MONTH);
      else {
			beg = CMTime(lastmonth<12 ? beg.Year()-1 : beg.Year(),lastmonth%12+1);
		   beg = beg.AtBeginning(incunits,begweek);
      }
   }
   else {
		beg = beg.AtBeginning(CMTime::GetResolution(b));
	   beg = beg.AtBeginning(incunits,begweek);
   }

	if (s!=0) {
		if (CMTime::GetResolution(s)==CM_YEAR) {
   		if (incunits==CM_YEAR)
      		start = CMTime(start.Year(),lastmonth).AtBeginning(CM_MONTH);
	      else {
				start = CMTime(lastmonth<12 ? start.Year()-1 : start.Year(),lastmonth%12+1);
			   start = start.AtBeginning(incunits,begweek);
	      }
   	}
	   else {
			start = start.AtBeginning(CMTime::GetResolution(s));
	   	start = start.AtBeginning(incunits,begweek);
	   }
   }
   else
   	start = beg;

   AddCycle(beg,end,start,si);
}

void CMTimeMachine::Reset()
{
	count = 0;
	for (unsigned i=0;i<cycles.Count();i++)
		cycles[i].Reset();
	now=cycles[0].Current();
}

void CMTimeMachine::Offset(short n,CMTIMEUNIT units)
{
	if (units<0) units=incunits;
	for (unsigned i=0;i<cycles.Count();i++)
		cycles[i].Offset(n,units);
}

void CMTimeMachine::Step()
{
	unsigned i;
	if (!cycles.Count())
		return;
	cycles[0].Step(inclength,incunits);
	if ((now=cycles[0].Current()) == cycles[0].Begin()) {
		count++;
		for (i=1;i<cycles.Count();i++)
			cycles[i].Rewind();
	}
	else {
		for (i=1;i<cycles.Count();i++)
			cycles[i].Step(inclength,incunits);
	}
}

short CMTimeMachine::SetOutputFormat()
{
	short fmt;
   switch (incunits) {
   	case CM_SECOND: fmt=CMTime::YYYYMMDDHHMMSS;break;
   	case CM_MINUTE: fmt=CMTime::YYYYMMDDHHMM;break;
   	case CM_HOUR:   fmt=CMTime::YYYYMMDDHH;break;
   	case CM_DAY: case CM_WEEK: fmt=CMTime::YYYYMMDD;break;
   	case CM_MONTH:  fmt=CMTime::YYYYMM;break;
   	default:        fmt=CMTime::YYYY;break;
   }
   return CMTime::SetOutputFormat(fmt);
}

wostream& CMTimeMachine::Write(wostream& os) const
{
	for (unsigned short i=0;i<cycles.Count();i++)
		os << cycles[i].Begin() << L"  "
      	<< cycles[i].End() << L"  "
			<< cycles[i].Current() << ENDL;
	return os;
}

wostream& CMTimeMachine::WriteBinary(wostream& s) const
{
	s.write((const wchar_t*)&count, sizeof(count));
	s.write((const wchar_t*)&incunits, sizeof(incunits));
	s.write((const wchar_t*)&inclength, sizeof(inclength));
	now.Write(s,1);
	unsigned short ncycles = cycles.Count();
	s.write((const wchar_t*)&ncycles, sizeof(ncycles));
	for (unsigned short i=0;i<ncycles;i++)
		cycles[i].WriteBinary(s);
	return s;
}

wistream& CMTimeMachine::ReadBinary(wistream& s)
{
	cycles.Reset(1);
	s.read((wchar_t*)&count, sizeof(count));
	s.read((wchar_t*)&incunits, sizeof(incunits));
	s.read((wchar_t*)&inclength, sizeof(inclength));
   now.Read(s,1);
	unsigned short ncycles;
	s.read((wchar_t*)&ncycles, sizeof(ncycles));
	for (unsigned short i=0;i<ncycles;i++) {
		CMTimeCycle cycle;
		cycle.ReadBinary(s);
		cycles.AddAt(i,cycle);
	}
	return s;
}



